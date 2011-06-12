# Before `make install' is performed this script should be runnable with
# `make test'. After `make install' it should work as `perl test.pl'

#########################

use Test::More;
BEGIN { plan tests => 3 };

my ($srcdir) = ($0 =~ m!(.*/)!);
chdir("${srcdir}symbol-test") or die $!;

system($^X, "Makefile.PL", @args) == 0 or die $!;
system("make 2>&1") == 0 or die $!;

use lib ("blib/arch/auto/SymbolTest/.libs", "blib/lib");

use_ok("SymbolTest");
eval { SymbolTest::throw_from_libxapian() };
like( $@, qr/DatabaseOpeningError caught in SymbolTest/, 'Correct exception caught');

use Search::Xapian qw( :all );

eval { SymbolTest::throw_from_libxapian() };
like( $@, qr/DatabaseOpeningError caught in SymbolTest/, 'Correct exception caught');

1;
