# Before `make install' is performed this script should be runnable with
# `make test'. After `make install' it should work as `perl test.pl'

#########################

use Test::More;
BEGIN { plan tests => 3 };

my ($srcdir) = ($0 =~ m!(.*/)!);
chdir("${srcdir}symbol-test") or die $!;

system($^X, "Makefile.PL") == 0 or die $!;
SKIP: {

# Building this module can fail for reasons such as Perl being built with a
# different compiler with incompatible flags.  So treat this build failing
# as a reason to skip to avoid test failures in such cases.
system("make 2>&1") == 0 or skip "Failed to build symbol-test module", 3;

use lib ("blib/arch/auto/SymbolTest", "blib/arch/auto/SymbolTest/.libs", "blib/lib");

use_ok("SymbolTest");
eval { SymbolTest::throw_from_libxapian() };
like( $@, qr/DatabaseOpeningError caught in SymbolTest/, 'Correct exception caught');

use Search::Xapian qw( :all );

eval { SymbolTest::throw_from_libxapian() };
like( $@, qr/DatabaseOpeningError caught in SymbolTest/, 'Correct exception caught');

}

1;
