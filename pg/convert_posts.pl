#!/usr/bin/env perl

use strict;
use warnings;

use lib '.';
use Convert;

$| = 1;

get_options();
db_connect();

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
	my @posts;

	while (my ($bname, $bid) = each %$boards) {
		for (qw/.DIR .TRASH .JUNK .NOTICE .DIGEST/) {
			read_index($bname, $bid, $_, \@posts);
		}
	}

	print "sorting...";
	@posts = sort { $a->[10] <=> $b->[10] } @posts;
	print "finished\n";

	\@posts;
}

sub read_index
{
	# 0 filename 1 id 2 gid 3 owner 4 title
	# 5 eraser 6 level 7 accessed 8 reid 9 timedeleted
	# 10 (date) 11 (board_id) 12 (type)
	my ($bname, $bid, $index, $posts) = @_;
	my ($fh, $buf, $pcount);
	open $fh, '<', "$dir/boards/$bname/$index" and do {
		while (1) {
			last if (read($fh, $buf, 256) != 256);
			my @t = unpack "Z72I2Z80Z67Z13i4", $buf;
			my $date;
			if ($t[0] =~ /^.\.(\d+)\./) {
				$date = $1;
				$t[0] = $bname . '/' . $t[0];
				push @t, $date, $bid, $index;
				push @$posts, \@t;
			}
			print "$pcount..." if (++$pcount % 1000 == 0);
		}
		close $fh;
	}
}

sub insert_posts
{
	my ($posts, $alive_users, $past_users) = @_;

	my $sth = $dbh->prepare(q{
		INSERT INTO posts (id, reid, tid, owner, uname, stamp, board,
			digest, marked, locked, imported, title, content)
		VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)
	}) or die $!;
	my $sth_deleted = $dbh->prepare(q{
		INSERT INTO posts_deleted (id, reid, tid, owner, uname, stamp, board,
			digest, marked, locked, imported, title, content,
			eraser, deleted, junk, bm_visible)
		VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)
	}) or die $!;

	my $pid_hash = {};
	my $base = 200_000_000;
	my $pid = 0;
	for (@$posts) {
		my ($file, $id, $gid, $owner, $title, $eraser,
			$level, $access, $reid, $deleted, $date, $bid, $type) = @$_;
		my $uid = get_uid($owner, $date, $alive_users, $past_users);

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
		$gid = $pid_hash->{$bid}{$gid} + $base;
		$reid = $pid_hash->{$bid}{$reid} + $base;

		my $digest = (($access & 0x10) or $type eq '.DIGEST') ? 1 : 0;
		my $marked = ($access & 0x8) ? 1 : 0;
		my $locked = ($access & 0x40) ? 1 : 0;
		my $imported = ($access & 0x0800) ? 1 : 0;

		$title = convert($title);
		my $content = convert_file("$dir/boards/$file");

		my $stamp = convert_time($date);

		if ($type eq '.TRASH' or $type eq '.JUNK') {
			my $eraser = get_uid($owner, $deleted, $alive_users, $past_users);
			$deleted = convert_time($deleted);
			my $junk = ($access & 0x200) ? 1 : 0;
			my $bm_visible = ($type eq '.TRASH') ? 1 : 0;
			$sth_deleted->execute($id, $reid, $gid, $uid, $owner, $stamp, $bid,
				$digest, $marked, $locked, $imported, $title, $content,
				$eraser, $deleted, $junk, $bm_visible);
		} else {
			$sth->execute($id, $reid, $gid, $uid, $owner, $stamp, $bid,
				$digest, $marked, $locked, $imported, $title, $content);
			if ($type eq '.NOTICE') {
				$dbh->do("INSERT INTO posts_sticked (pid, stamp) VALUES (?, ?)",
					undef, $id, $stamp);
			}
		}
		
		if ($pid % 100 == 0) {
			print "$pid...";
			$dbh->commit;
		}
	}
	$dbh->do("SELECT setval('posts_base_id_seq', ?)", undef, $pid + $base + 1);
	$dbh->commit;
}

sub get_uid
{
	my ($uname, $stamp, $alive_users, $past_users) = @_;
	if (exists $alive_users->{$uname} and $alive_users->{$uname}[1] <= $stamp) {
		return $alive_users->{$uname}[0];
	}
	$past_users->{$uname};
}
