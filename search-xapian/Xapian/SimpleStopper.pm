package Search::Xapian::SimpleStopper;

use 5.006;
use strict;
use warnings;
use Carp;

require Exporter;
require DynaLoader;

#@Search::Xapian::SimpleStopper::ISA = qw(Search::Xapian::Stopper Exporter DynaLoader);
our @ISA = qw(Exporter DynaLoader);
# Items to export into callers namespace by default. Note: do not export
# names by default without a very good reason. Use EXPORT_OK instead.
# Do not simply export all your public functions/methods/constants.

# If you do not need this, moving things directly into @EXPORT or @EXPORT_OK
# will save memory.
our @EXPORT_OK = ( );

our @EXPORT = qw( );

# Preloaded methods go here.

#use overload '='  => sub { $_[0]->clone() },
#             'fallback' => 1;

sub new {
    my $class = shift;
    my $stopper = new0();

    bless $stopper, $class;
    foreach (@_) {
	$stopper->add($_);
    }

    return $stopper;
}

1;
