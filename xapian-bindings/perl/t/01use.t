use Test::More tests => 3;

use_ok('Search::Xapian');

# Check that module's version is defined and has a sane value.
ok(defined($Search::Xapian::VERSION));
open F, ">>/home/olly/output-from-perl-01use.log" and print F "$$: [$Search::Xapian::VERSION]\n" and close F;
ok($Search::Xapian::VERSION =~ /^\d+\.\d+\.\d+\.\d+$/);
