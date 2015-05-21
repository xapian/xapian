use strict;
# Before `make install' is performed this script should be runnable with
# `make test'. After `make install' it should work as `perl test.pl'

#########################

use Test::More;
BEGIN { plan tests => 3 };

my ($srcdir) = ($0 =~ m!(.*/)!);
chdir("${srcdir}symbol-test") or die $!;

open ARGS, "../../makefile-pl-args" or die $!;
my @args = <ARGS>;
close ARGS;
chomp @args;

# Avoid inheriting values with Search/Xapian path in.
delete $ENV{LD};
delete $ENV{MAKEFLAGS};

system($^X, "Makefile.PL", @args) == 0 or die $!;
system("make 2>&1") == 0 or die $!;

use lib (
	# For shared objects when built with uninstalled xapian-core (libtool):
	"blib/arch/auto/SymbolTest/.libs",
	# For shared objects when built with installed xapian-core (no libtool):
	"blib/arch/auto/SymbolTest",
	# For .pm files:
	"blib/lib"
    );

use_ok("SymbolTest");
eval { SymbolTest::throw_from_libxapian() };
like( $@, qr/DatabaseOpeningError caught in SymbolTest/, 'Correct exception caught');

use Search::Xapian qw( :all );

eval { SymbolTest::throw_from_libxapian() };
like( $@, qr/DatabaseOpeningError caught in SymbolTest/, 'Correct exception caught');

1;
