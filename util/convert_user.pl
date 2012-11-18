#!/usr/bin/env perl

use strict;
use warnings;

use lib '.';
use Helper qw(get_options db_connect read_users $dbh);

$| = 1;

get_options();
db_connect();

my $users = read_users();

insert_users($users);

$dbh->disconnect;

sub insert_users
{
	my $users = shift;

	my $sth = $dbh->prepare("INSERT INTO users (name, passwd) VALUES (?, ?)") or die $!;

	for (@$users) {
		$sth->execute($_->[24], $_->[9]);
	}
	$dbh->commit;
}
