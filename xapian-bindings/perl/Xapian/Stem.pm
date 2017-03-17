package Xapian::Stem;

=head1 NAME

Xapian::Stem - Snowball stemmer

=head1 SYNOPSIS

    my $stemmer = Xapian::Stem->new('norwegian');
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

L<Xapian>,
L<Xapian::QueryParser>,
L<Xapian::TermGenerator>

=cut
1;
