#!/usr/bin/env perl

use strict;
use warnings;

use lib '.';
use Helper;

$| = 1;

get_options();
db_connect();

my $boards = read_boards();
my $users = $dbh->selectall_arrayref('SELECT id, name FROM alive_users');
convert_favboard($boards, $users);

$dbh->disconnect;

sub convert_favboard
{
	my ($boards, $users) = @_;

	# build board hash
	my %hash;
	for (@$boards) {
		$hash{$_->[12]} = $_->[13];  # pos => bid
	}

	my $fqry = $dbh->prepare('INSERT INTO fav_board_folders (user_id, name, descr) VALUES (?, ?, ?) RETURNING id');
	my $bqry = $dbh->prepare('INSERT INTO fav_boards (user_id, board, folder) VALUES (?, ?, ?)');

	for (@$users) {
		my ($uid, $name) = @$_;
		my $initial = uc substr $name, 0, 1;
		my $file = "${dir}/home/${initial}/${name}/.goodbrd";
		print "processing $uid, $name\n";

		my %cdir;
		my $favs = read_favboard($file);
		next if (not defined $favs or @$favs == 0);

		my @folders = grep { $_->[3] < 0 } @$favs;
		for (@folders) {
			my ($n, $descr) = @{$_}[4, 6];
			$n = convert($n);
			$descr = convert($descr);
			$fqry->execute($uid, $n, $descr);
			my ($id) = $fqry->fetchrow_array;
			$cdir{$_->[0]} = $id;
		}

		my @boards = grep { $_->[3] >= 0} @$favs;
		my %inserted = ();
		for (@boards) {
			if (exists $hash{$_->[2] + 1}) {
				my $bid = $hash{$_->[2] + 1};
				if (not exists $inserted{$bid}) {
					$bqry->execute($uid, $bid, $_->[1] ? $cdir{$_->[1]} : 1);
					$inserted{$bid} = 1;
				}
			}
		}
		$dbh->commit;
	}
}

sub read_favboard
{
	my $file = shift;
	my @favs;
	my $buf;
	open my $fh, '<', $file or return;
	while (1) {
		last if (read($fh, $buf, 168) != 168);
		# 0 id (order in the file, 1-based) 1 pid 2 pos 3 flag 4 filename
		# 5 nonsense 6 title
		my @t = unpack "i4Z72a11Z69", $buf;
		push @favs, \@t;
	}
	close $fh;
	\@favs;
}
