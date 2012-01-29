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

sub read_boards
{
	#0 filename 1 nowid 2 group 3 owner 4 bm 5 flag
	#6 sector 7 category 8 nonsense 9 title
	#10 level 11 accessed
	my ($buf, %hash, @temp, %boards);
	my $i = 0;
	my $id = 1;
	open my $fh, '<', "$dir/.BOARDS" or die "can't open .BOARDS\n";
	while (1) {
		last if (read($fh, $buf, 256) != 256);
		my @t = unpack "Z72IiZ20Z56ia2a4a5Z69ia12", $buf;
		++$i;
		if ($t[0]) {
			$hash{$i} = $id;
			$boards{$t[0]} = $id;
			++$id;
			push @temp, \@t;
		}
	}

	$_->[2] = $_->[2] ? $hash{$_->[2]} : undef foreach (@temp);
	return \@temp;
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
