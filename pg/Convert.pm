package Convert;

use Exporter 'import';
@EXPORT = qw(get_options db_connect convert read_boards $dir $dbh);

use DBI;
use Encode;
use Getopt::Long;

our ($host, $port, $db, $user, $dir, $dbh);

sub get_options
{
	GetOptions(
			"h|host=s" => \$host,
			"p|port:s" => \$port,
			"d|database=s" => \$db,
			"u|user=s" => \$user,
			"b|basedir:s" => \$dir,
			);

	$host = $ENV{PGHOST} if not $host;
	$user = $ENV{PGUSER} if not $user;
	$db = $user if not $db;
	$port = 5432 if not $port;
	$dir = '/home/bbs' if not $dir;
	die "Usage: $0 -h host [-p port] -d database -u user -b [base dir]\n" if (not $host or not $db or not $user);
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

1;
