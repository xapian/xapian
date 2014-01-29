package Search::Xapian::Stem;

use 5.006;
use strict;
use warnings;

# In a new thread, copy objects of this class to unblessed, undef values.
sub CLONE_SKIP { 1 }

1;

__END__

=head1 NAME

Search::Xapian::Stem - Snowball stemmer

=head1 SYNOPSIS

    my $stemmer = Search::Xapian::Stem->new('norwegian');
    my $stemmed_foo = $stemmer->stem_word($foo);

=head1 DESCRIPTION

=head1 METHODS

=over 4

=item new <language>

Constructor. Takes the language to stem as a parameter.

=item stem_word <word>

Returns the stemmed form of the given word.

=back

=head1 SEE ALSO

L<Search::Xapian>, L<Search::Xapian::QueryParser>, L<Search::Xapian::TermGenerator>

=cut
