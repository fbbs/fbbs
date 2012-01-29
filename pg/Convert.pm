package Convert;

use Exporter 'import';
@EXPORT = qw(get_options db_connect convert $host $port $db $user $dir $dbh);

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

1;
