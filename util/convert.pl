#!/usr/bin/perl -w
use strict;

use DBI;
use Encode;
use Getopt::Long;
use POSIX;

$| = 1;

my ($host, $port, $db, $user, $dir);
GetOptions(
	"h|host=s" => \$host,
	"p|port:s" => \$port,
	"d|database=s" => \$db,
	"u|user=s" => \$user,
	"b|basedir:s" => \$dir,
);
$port = 5432 if (not $port);
$dir = '/home/bbs' if (not $dir);
die "Usage: $0 -h host [-p port] -d database -u user -b [base dir]\n" if (not $host or not $db or not $user);

my $dbconf = {
	'host' => $host,
	'db' => $db,
	'user' => $user
};

my $dsn = "DBI:Pg:database=$db;host=$host;port=$port";
my $dbh;
$dbh = DBI->connect($dsn, $user, $user, { RaiseError => 1, AutoCommit => 0 }) or die $dbh->errstr;

my (%users, %boards, @posts);
my $pcount = 0;

&insert_users;
&insert_boards;
&insert_posts;

$dbh->disconnect;

sub insert_users
{
	# 0 uid 1 userlevel 2 numlogins 3 numposts 4 stay
	# 5 nummedals 6 money 7 bet 8 flags 9 passwd
	# 10 nummails 11 gender 12 byear 13 bmonth 14 bday
	# 15 signature 16 userdefine 17 prefs 18 noteline 19 firstlogin
	# 20 lastlogin 21 lastlogout 22 dateforbet 23 notedate 24 userid
	# 25 lasthost 26 username 27 email 28 reserved
	my ($buf, %hash);
	my $i = 1;
	open my $fh, '<', "$dir/.PASSWDS" or die "can't open .PASSWDS\n";
	while (1) {
		last if (read($fh, $buf, 256) != 256);
		my @t = unpack "I3iIi3sA14IcC3iI2iq5Z16Z40Z40Z40a8", $buf;
		if ($t[24] and not exists $hash{$t[24]}) {
			$hash{$t[24]} = \@t;
		}
	}
	close $fh;

	my @temp = values %hash;
	@temp = sort { $a->[19] <=> $b->[19] } @temp;

	my $query = $dbh->prepare("INSERT INTO users (name, passwd, nick, email, flag, logins, posts, stay, medals, money, birth, gender, creation, lastlogin, lastlogout, lasthost) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)") or die $dbh->errstr;

	print "inserting users...";
	$i = 0;
	foreach (@temp) {
		print "$i..." if ($i % 500 == 0);
		my $nick = &convert($_->[26]);
		my $email = &check_email($_->[27]);
		my $lasthost = &convert($_->[25]);
		my $birth = &check_birth($_->[12], $_->[13], $_->[14]);
		$query->execute($_->[24], $_->[9], $nick, $email, $_->[16], $_->[2], $_->[3], $_->[4], $_->[5], $_->[6], $birth, chr($_->[11]), &mytime($_->[19]), &mytime($_->[20]), &mytime($_->[21]), $lasthost) or die $dbh->errstr;
		$users{$_->[24]} = ++$i;
	}

	$dbh->commit;
	print "finished\n";
}

sub insert_boards
{
	#0 filename 1 nowid 2 group 3 owner 4 bm 5 flag
	#6 sector 7 category 8 nonsense 9 title
	#10 level 11 accessed
	my ($buf, %hash, @temp);
	my $i = 0;
	my $id = 1;
	open my $fh, '<', "$dir/.BOARDS" or die "can't open .BOARDS\n";
	while (1) {
		last if (read($fh, $buf, 256) != 256);
		my @t = unpack "Z72IiZ20Z56ia2a4a5Z69Ia12", $buf;
		++$i;
		if ($t[0]) {
			$hash{$i} = $id;
			$boards{$t[0]} = $id;
			++$id;
			push @temp, \@t;
		}
	}

	print "inserting boards...";
	my $query = $dbh->prepare("INSERT INTO boards (name, description, category, sector, parent, flag) VALUES (?, ?, ?, ?, ?, ?)") or die $dbh->errstr;

	foreach (@temp) {
		$query->execute(&convert($_->[0]), &convert($_->[9]), &convert($_->[7]), ord(substr($_->[6], 0, 1)), $_->[2] ? $hash{$_->[2]} : 0, $_->[5]) or die $dbh->errstr;
	}

	$dbh->commit;
	print "finished\n";

	print "inserting managers...";
	$query = $dbh->prepare("INSERT INTO managers (user_id, board_id) VALUES (?, ?)") or die $dbh->errstr;
	foreach my $brd (@temp) {
		my @bm = split / /, $brd->[4];
		foreach my $bm (@bm) {
			if (exists $users{$bm}) {
				$query->execute($users{$bm}, $boards{$brd->[0]}) or die $dbh->errstr;
			}
		}
	}
	$dbh->commit;
	print "finished\n";
}

sub read_index
{
	my @indices = qw/.DIR .TRASH .JUNK .NOTICE .DIGEST/;
	my ($board, $index) = @_;
	my $file = $indices[$index];
	my $bid = $boards{$board};
	my ($fh, $buf);
	open $fh, '<', "$dir/boards/$board/$file" and do {
		while (1) {
			last if (read($fh, $buf, 256) != 256);
			my @t = unpack "Z72I2Z80Z67Z13i4", $buf;
			my $date;
			if ($t[0] =~ /^.\.(\d+)\./) {
				$date = $1;
				$t[0] = $board . '/' . $t[0];
				push @t, $date, $bid, $index;
				push @posts, \@t;
			}
			print "$pcount..." if (++$pcount % 100000 == 0);
		}
		close $fh;
	}
}

sub insert_posts
{
	my $query = $dbh->prepare("INSERT INTO posts (reid, gid, board_id, user_id, user_name, title, flag, time, t2, itype) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?)") or die $dbh->errstr;

	print "reading indices...";
	foreach my $board (keys %boards) {
		foreach my $idx (0..4) {
			&read_index($board, $idx);
		}
	}
	print "finished ($pcount posts)\n";

	# 0 filename 1 id 2 gid 3 owner 4 title
	# 5 eraser 6 level 7 accessed 8 reid 9 timedeleted
	# 10 (date) 11 (board_id) 12 (type)
	print "sorting...";
	@posts = sort { $a->[10] <=> $b->[10] } @posts;
	print "finished\n";

	print "inserting posts...";
	my $id = 0;
	my %hash;
	my $dest = '/home/fbbs/posts/';
	foreach (@posts) {
		$hash{$_->[11] . '_' . $_->[1]} = ++$id;
		mkdir $dest . int($id / 10000) if ($id % 10000 == 1);

		my $fh;
		open $fh, '<', $dir . '/boards/' . $_->[0] and do {
			my $wh;
			if (open $wh, '>', $dest . int($id / 10000) . "/$id") {
				my $str;
				{
					local $/ = undef;
					$str = <$fh>;
				}
				close $fh;
				print $wh &convert($str);
				close $wh;
			} else {
				close $fh;
			}
		};

		$query->execute($hash{$_->[11] . '_' . $_->[8]}, $hash{$_->[11] . '_' . $_->[2]}, $_->[11], $users{$_->[3]}, &convert($_->[3]), &convert($_->[4]), $_->[7], &mytime($_->[10]), $_->[9] ? &mytime($_->[9]) : undef, $_->[12]) or die $dbh->errstr;
		if ($id % 1000 == 0) {
			$dbh->commit;
			print "$id..." ;
		}
	}
	print "finished\n";
}

sub convert
{
	my $s = shift;
	encode('utf8', decode('gbk', $s));
}

sub check_email
{
	my $s = shift;
	return $s if ($s =~ /^(?:[.\-\w]+)@(?:[.\-\w]+)$/);
	return undef;
}

sub check_birth
{
	my ($y, $m, $d) = @_;
	my $t = mktime(0, 0, 0, $d, $m - 1, $y);
	return undef if (not $t);
	mytime($t);
}

sub mytime
{
	my $t = shift;
	my @t = localtime($t);
	my $s = sprintf "%d-%d-%d %d:%d:%d +8:00", $t[5] + 1900, $t[4] + 1, $t[3], $t[2], $t[1], $t[0];
}
