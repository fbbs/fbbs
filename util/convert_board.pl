#!/usr/bin/env perl

use strict;
use warnings;

use lib '.';
use Helper qw(get_options db_connect convert read_boards $dir $dbh);

$| = 1;

get_options();
db_connect();

my ($sectors, $codes) = read_sectors();
insert_sectors($sectors);

my $users = load_users();

my $boards = read_boards();
insert_boards($boards, $codes, $users);

$dbh->disconnect;

sub load_users
{
	my %users;
	my $arr = $dbh->selectall_arrayref("SELECT id, name FROM alive_users");
	for (@$arr) {
		$users{$_->[1]} = $_->[0];
	}
	\%users;
}

sub read_sectors
{
	my %sectors;
	my %codes;
	open my $fh, '<', "$dir/etc/menu.ini" or die $!;
	my @lines = grep { /^EGROUP/ } <$fh>;
	my $i = 1;
	for (@lines) {
		my ($sector, $codes) = ($_ =~ /^EGROUP(.)\s*=\s*"([^"]+)"/);
		$sectors{$sector} = $i;
		for (split //, $codes) {
			$codes{$_} = $i if not exists $codes{$_};
		}
		++$i;
	}
	close $fh;
	(\%sectors, \%codes);
}

sub insert_sectors
{
	my $sectors = shift;
	my $query = $dbh->prepare("INSERT INTO board_sectors (name) VALUES (?)");
	for (sort { $sectors->{$a} <=> $sectors->{$b} } keys %$sectors) {
		$query->execute($_);
	}
	$dbh->commit;
}

sub insert_boards
{
	my ($boards, $codes, $users) = @_;

	my $bqry = $dbh->prepare("INSERT INTO boards (name, descr, parent, flag, perm, categ, sector, bms) VALUES (?, ?, ?, ?, ?, ?, ?, ?)");
	my $bmqry = $dbh->prepare("INSERT INTO bms (user_id, board_id) VALUES (?, ?)") or die $dbh->errstr;
	my $cid = 1;
	my $bid = 1;

	for my $board (@$boards) {
		my ($name, $descr, $parent, $flag, $perm, $categ, $sector, $bms) = @{$board}[0, 9, 2, 5, 10, 7, 6, 4];
		$name = convert($name);
		$descr = convert($descr);
		$categ = convert($categ);
		$bms = convert($bms);
		$sector = $codes->{substr $sector, 0, 1} || 0;

		$bqry->execute($name, $descr, $parent, $flag, $perm, $categ, $sector, $bms) or die $dbh->errstr;

		my %inserted = ();
		for my $bm (split /\s+/, $bms) {
			if (exists $users->{$bm} and not exists $inserted{$bm}) {
				$bmqry->execute($users->{$bm}, $bid) or die $dbh->errstr;
				$inserted{$bm} = 1;
			}
		}
		++$bid;
	}
	$dbh->commit;
}
