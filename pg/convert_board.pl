#!/usr/bin/env perl

use strict;
use warnings;

use lib '.';
use Convert;

$| = 1;

get_options();
db_connect();

my ($sectors, $codes) = read_sectors();
insert_sectors($sectors);

my $boards = read_boards();
insert_boards($boards, $codes);

$dbh->disconnect;

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
	my ($boards, $codes) = @_;

	my $cqry = $dbh->prepare("INSERT INTO board_categs (name) VALUES (?)");
	my $bqry = $dbh->prepare("INSERT INTO boards (name, descr, parent, flag, perm, categ, sector, bms) VALUES (?, ?, ?, ?, ?, ?, ?, ?)");
	my %categs;
	my $cid = 1;

	for my $board (@$boards) {
		my ($name, $descr, $parent, $flag, $perm, $categ, $sector, $bms) = @{$board}[0, 9, 2, 5, 10, 7, 6, 4];
		$name = convert($name);
		$descr = convert($descr);
		$categ = convert($categ);
		$bms = convert($bms);
		$sector = $codes->{substr $sector, 0, 1} || 0;

		if (not exists $categs{$categ}) {
			$cqry->execute($categ);
			$categs{$categ} = $cid++;
			$dbh->commit;
		}
		$categ = $categs{$categ};

		$bqry->execute($name, $descr, $parent, $flag, $perm, $categ, $sector, $bms) or die $dbh->errstr;
	}
	$dbh->commit;
}
