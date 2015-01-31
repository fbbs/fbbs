package Helper;

use Exporter 'import';
@EXPORT_OK = qw(get_options db_connect convert convert_file convert_time
		read_boards read_users read_index timestamp_to_unix $dir $dbh);

use Config;
use DBI;
use Encode;
use POSIX qw(strftime mktime);

our ($host, $port, $db, $user, $dir, $dbh);

sub get_options
{
	my $file = '/etc/fbbs/fbbs.conf';
	my %config;
	open my $fh, '<', $file;
	while (<$fh>) {
		chomp;
		if (m/=/) {
			my ($key, $val) = split / += +/;
			$config{$key} = $val;
		}
	}
	close $fh;

	($host, $user, $db, $port) = @config{qw(host user dbname port)};
	$dir = '/home/bbs' if not $dir;
	die if (not $host or not $db or not $user);
}

sub db_connect
{
	my $dsn = "DBI:Pg:database=$db;host=$host;port=$port";
	$dbh = DBI->connect($dsn, $user, $user, { RaiseError => 1, AutoCommit => 0 }) or die $dbh->errstr;
}

sub convert
{
	my $s = shift;
	encode('utf8', decode('gbk', $s));
}

sub convert_file
{
	my $file = shift;
	open my $fh, '<', $file or return '';
	my $s;
	{
		local $/ = undef;
		$s = <$fh>;
	}
	close $fh;
	convert($s);
}

sub convert_time
{
	my $stamp = shift;
	strftime "%b %e %H:%M:%S %Y %z", localtime($stamp);
}

sub timestamp_to_unix
{
	my $stamp = shift;
	$stamp =~ /(\d{4})-(\d\d)-(\d\d) (\d\d):(\d\d):(\d\d)/;
	mktime($6, $5, $4, $3, $2 - 1, $1 - 1900);
}

sub read_boards
{
	#0 filename 1 nowid 2 group 3 owner 4 bm 5 flag
	#6 sector 7 category 8 nonsense 9 title
	#10 level 11 accessed (12 bid [1-based]) (13 db_bid)
	my ($buf, %hash, @temp, %boards);
	my $i = 0;
	my $id = 1;
	open my $fh, '<', "$dir/.BOARDS" or die "can't open .BOARDS\n";
	while (1) {
		last if (read($fh, $buf, 256) != 256);
		my @t = unpack "Z72IiZ20Z56ia2a4a5Z69ia12", $buf;
		++$i;
		if ($t[0]) {
			push @t, $i, $id;
			$hash{$i} = $id;
			$boards{$t[0]} = $id;
			++$id;
			push @temp, \@t;
		}
	}

	$_->[2] = $_->[2] ? $hash{$_->[2]} : undef foreach (@temp);
	return \@temp;
}

sub read_users
{
	# 0 uid 1 userlevel 2 numlogins 3 numposts 4 stay
	# 5 nummedals 6 money 7 bet 8 flags 9 passwd
	# 10 nummails 11 gender 12 byear 13 bmonth 14 bday
	# 15 signature 16 userdefine 17 prefs 18 noteline 19 firstlogin
	# 20 lastlogin 21 lastlogout 22 dateforbet 23 notedate 24 userid
	# 25 lasthost 26 username 27 email 28 reserved
	my ($buf, %hash);
	open my $fh, '<', "$dir/.PASSWDS" or die "can't open .PASSWDS\n";
	while (1) {
		last if (read($fh, $buf, 256) != 256);
		my @t;
		if ($Config{use64bitint}) {
			@t = unpack "IiIiIi3sA14IcC3iI2iq5Z16Z40Z40Z40a8", $buf;
		} else {
			@t = unpack "IiIiIi3sA14IcC3iI2il10Z16Z40Z40Z40a8", $buf;
			for (my $i = 19; $i < 24; ++$i) {
				$t[$i] += $t[$i + 1] << 32;
				splice @t, $i + 1, 1;
			}
		}
		if ($t[24] and not exists $hash{$t[24]}) {
			$hash{$t[24]} = \@t;
		}
	}
	close $fh;

	my @temp = values %hash;
	@temp = sort { $a->[19] <=> $b->[19] } @temp;

	return \@temp;
}

sub read_index {
	# 0 filename 1 id 2 gid 3 owner 4 title
	# 5 eraser 6 level 7 accessed 8 reid 9 timedeleted
	# 10 (index) 11 (date)
	my ($board_name, $index, $posts) = @_;
	my $fh;
	open $fh, '<', "$dir/boards/$board_name/$index" and do {
		while (1) {
			my $buf;
			last if (read($fh, $buf, 256) != 256);
			my @t = unpack "Z72I2Z80Z67Z13i4", $buf;
			push @t, $index;

			my $date;
			if ($t[0] =~ /^.\.(\d+)\./) {
				$date = $1;
				push @t, $date;
			}

			push @$posts, \@t;
		}
	}
}

1;
