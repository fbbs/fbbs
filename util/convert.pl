#!/usr/bin/perl -w
use strict;

use DBI;
use Encode;
use Getopt::Long;
use POSIX;

my ($host, $port, $db, $user, $file);
GetOptions(
	"h|host=s" => \$host,
	"p|port=s" => \$port,
	"d|database=s" => \$db,
	"u|user=s" => \$user,
	"f|file=s" => \$file,
);
$port = 5432 if (not $port);
die "Usage: $0 -h host [-p port] -d database -u user -f file\n" if (not $host or not $db or not $user or not $file);

my $dbconf = {
	'host' => $host,
	'db' => $db,
	'user' => $user
};

my $dsn = "DBI:Pg:database=$db;host=$host;port=$port";
my $dbh;
$dbh = DBI->connect($dsn, $user, $user, { RaiseError => 1, AutoCommit => 0 }) or die $dbh->errstr;

&insert_users;

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
	open my $fh, '<', $file or die "can't open file '$file'\n";
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

my $query = $dbh->prepare("INSERT INTO users (name, passwd, nick, email, options, logins, posts, stay, medals, money, birth, gender, creation, lastlogin, lastlogout, lasthost) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)") or die $dbh->errstr;

print "inserting users...";
my $i = 0;
foreach (@temp) {
	print "$i..." if ($i % 500 == 0);
	my $nick = &convert($_->[26]);
	my $email = &check_email($_->[27]);
	my $lasthost = &convert($_->[25]);
	my $birth = &check_birth($_->[12], $_->[13], $_->[14]);
	$query->execute($_->[24], $_->[9], $nick, $email, $_->[16], $_->[2], $_->[3], $_->[4], $_->[5], $_->[6], $birth, chr($_->[11]), &mytime($_->[19]), &mytime($_->[20]), &mytime($_->[21]), $lasthost) or die $dbh->errstr;
	++$i;
}

$dbh->commit;
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
