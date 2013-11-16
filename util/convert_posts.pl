#!/usr/bin/env perl

use strict;
use warnings;

use Digest::MD5 qw(md5);

use lib '.';
use Helper qw(get_options db_connect convert convert_time convert_file read_users $dir $dbh);

$| = 1;

my %file_handles;
my $POST_INDEX_PER_FILE = 100000;
my $POST_INDEX_SIZE = 128;

get_options();
db_connect();

my $post_digests = {};
my $boards = load_boards();
my $posts = read_posts($boards);

my ($alive_users, $past_users) = load_users();
insert_posts($posts, $alive_users, $past_users);

$dbh->disconnect;

sub load_boards
{
	my %boards;
	my $arr = $dbh->selectall_arrayref("SELECT id, name FROM boards");
	$boards{$_->[1]} = $_->[0] for (@$arr);
	\%boards;
}

sub load_users
{
	my $passwd_users = read_users();
	my %firstlogin = map { $_->[24] => $_->[19] } @$passwd_users;

	my $arr = $dbh->selectall_arrayref("SELECT id, name FROM alive_users");
	my %alive_users = map { $_->[1] => [$_->[0], $firstlogin{$_->[1]}] } @$arr;

	$arr = $dbh->selectall_arrayref("SELECT id, name FROM users WHERE NOT alive");
	my %past_users;
	for (@$arr) {
		if (not exists $past_users{$_->[1]} or $past_users{$_->[1]} < $_->[0]) {
			$past_users{$_->[1]} =  $_->[0];
		}
	}
	(\%alive_users, \%past_users);
}

sub read_posts
{
	my $boards = shift;
	my %posts;
	my $count = 0;

	while (my ($bname, $bid) = each %$boards) {
		for (qw/.NOTICE .DIGEST .DIR .TRASH .JUNK/) {
			$count += read_index($bname, $bid, $_, \%posts);
		}
		print "$count...";
	}
	\%posts;
}

sub read_index
{
	# 0 filename 1 id 2 gid 3 owner 4 title
	# 5 eraser 6 level 7 accessed 8 reid 9 timedeleted
	# 10 (date) 11 (board_id) 12 (type)
	my ($bname, $bid, $index, $posts, $inode) = @_;
	my ($fh, $buf);
	my $pcount = 0;
	open $fh, '<', "$dir/boards/$bname/$index" and do {
		while (1) {
			last if (read($fh, $buf, 256) != 256);
			my @t = unpack "Z72I2Z80Z67Z13i4", $buf;
			my $date;
			if ($t[0] =~ /^.\.(\d+)\./) {
				$date = $1;
				$t[0] = $bname . '/' . $t[0];
				push @t, $date, $bid, $index;

				if ($index eq '.NOTICE' or $index eq '.DIGEST') {
					my $content = convert_file("$dir/boards/$t[0]");
					my $md5 = md5($content);
					if (exists $post_digests->{$bid}{$md5}) {
						next;
					} else {
						$post_digests->{$bid}{$md5} = undef;
					}
				}

				my ($month, $year) = (localtime $date)[4, 5];
				my $archive = ($year + 1900) * 100 + $month + 1;
				push @{$posts->{$archive}}, \@t;
				++$pcount;
			}
		}
		close $fh;
	};
	$pcount;
}

sub insert_posts
{
	my ($posts, $alive_users, $past_users) = @_;
	my ($DIGEST, $MARKED, $LOCKED, $IMPORTED, $WATER, $STICKY, $JUNK) = (0x1, 0x2, 0x4, 0x8, 0x20, 0x10, 0x80);

	my $pid_hash = {};
	my $base = 200_000_000;
	my $pid = 0;

	my @keys = sort keys %$posts;
	for (@keys) {
		my $array = delete $posts->{$_};
		print "sorting $_...";
		@$array = sort { $a->[10] <=> $b->[10] } @$array;
		print "finished\n";

		for (@$array) {
			my ($file, $id, $gid, $owner, $title, $eraser,
					$level, $access, $reid, $deleted, $date, $bid, $type) = @$_;
			my $uid = get_uid($owner, $date, $alive_users, $past_users) || 0;

			$owner = convert($owner);
			$title = convert($title);
			my $content = convert_file("$dir/boards/$file");
			my $md5 = md5($content);
			if (exists $post_digests->{$bid}{$md5} and $type ne '.NOTICE'
					and $type ne '.DIGEST') {
				next;
			}

			if ($id != $gid) {
				if (not exists $pid_hash->{$bid}{$gid}) {
					$pid_hash->{$bid}{$gid} = ++$pid;
				}
				if (not exists $pid_hash->{$bid}{$reid}) {
					$pid_hash->{$bid}{$reid} = ++$pid;
				}
			}
			$pid_hash->{$bid}{$id} = ++$pid;
			$id = $pid + $base;
			$gid = $pid_hash->{$bid}{$gid} + $base - $id;
			$reid = $pid_hash->{$bid}{$reid} + $base - $id;

			my $flag = 0;
			$flag |= $DIGEST if (($access & 0x10) or $type eq '.DIGEST');
			$flag |= $MARKED if ($access & 0x8);
			$flag |= $LOCKED if ($access & 0x40);
			$flag |= $IMPORTED if ($access & 0x0800);
			$flag |= $WATER if ($access & 0x80);
			my $stamp = $date;

			if ($type eq '.TRASH' or $type eq '.JUNK') {
				$flag |= $JUNK if ($access & 0x200);

				my $buf = pack 'qLLllLLLZ16', $id, $reid, $gid, $uid, $flag, $stamp, 0, $deleted, $eraser;

				my $filename = lc $type;
				my $fh = get_record("$dir/brdidx/$bid$filename");
				print $fh $buf;
			} else {
				my $buf = pack 'qLLllLL', $id, $reid, $gid, $uid, $flag, $stamp, 0;
				my $fh = get_record("$dir/brdidx/$bid");
				print $fh $buf;
				if ($type eq '.NOTICE') {
					$flag |= $STICKY;
					my $buf = pack 'qLLllLL', $id, $reid, $gid, $uid, $flag, $stamp, 0;
					my $fh = get_record("$dir/brdidx/$bid.sticky");
					print $fh $buf;
				}
			}

			my $buf = pack 'qLLLlllSSSZ13Z77', $id, $reid, $gid, $stamp, $uid, $flag, $bid, 0, 0, 0, $owner, $title;
			write_global_index($id, $buf);
			write_content($id, $content);

			if ($pid % 1000 == 0) {
				print "$pid...";
				$dbh->commit;
			}
		}
	}
}

sub get_uid
{
	my ($uname, $stamp, $alive_users, $past_users) = @_;
	if (exists $alive_users->{$uname} and $alive_users->{$uname}[1] <= $stamp) {
		return $alive_users->{$uname}[0];
	}
	$past_users->{$uname};
}

sub get_record {
	my ($file, $callback) = @_;
	if (not exists $file_handles{$file}) {
		open my $fh, '>', $file or die $!;
		$file_handles{$file} = $fh;
		$callback->($fh) if defined $callback;
	}
	$file_handles{$file};
}

sub write_content {
	my ($id, $content) = @_;
	my $POST_CONTENT_PER_FILE = 10000;
	my $HEADER_SIZE = 8;

	my $file = int(($id - 1) / $POST_CONTENT_PER_FILE);
	my $base = $file * $POST_CONTENT_PER_FILE + 1;

	my $fh = get_record("$dir/post/$file");

	my $offset = (stat($fh))[7];
	$offset = $HEADER_SIZE * $POST_CONTENT_PER_FILE if ($offset < 4 * $POST_CONTENT_PER_FILE);
	my $length = length($content);

	seek $fh, ($id - $base) * $HEADER_SIZE, 0;
	my $buf = pack "LL", $offset, $length;
	print $fh $buf;

	seek $fh, $offset, 0;
	$buf = pack "S", $id - $base;
	print $fh "\n", $buf, $content, "\0";
}

sub write_global_index {
	my ($id, $buf) = @_;
	my $base = int(($id - 1) / $POST_INDEX_PER_FILE) * $POST_INDEX_PER_FILE + 1;
	my $file = "$dir/index/" . int(($id - 1) / $POST_INDEX_PER_FILE);
	my $fh = get_record($file, sub {
		my $fh = shift;
		seek $fh, $POST_INDEX_SIZE * $POST_INDEX_PER_FILE - 1, 0;
		print $fh "\0";
	});
	seek $fh, ($id - $base) * $POST_INDEX_SIZE, 0;
	print $fh $buf;
}
