use strict;
# Make warnings fatal
use warnings;
BEGIN {$SIG{__WARN__} = sub { die "Terminating test due to warning: $_[0]" } };

use Test::More;

eval "use Test::Pod 1.14";
plan skip_all => 'Test::Pod 1.14 required' if $@;
plan skip_all => 'set TEST_POD to enable this test' unless $ENV{TEST_POD};

all_pod_files_ok( all_pod_files( '.' ) );
