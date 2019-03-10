package Xapian::Error;

=head1 NAME

Xapian::Error - Base class for all exceptions in Xapian

=head1 DESCRIPTION

This is an abstract class in C++, i.e. it cannot be instantiated directly,
hence in Perl it has no C<new> method.

=head1 METHODS

All exception objects have the following methods

=head2 get_msg

Returns a string with a descriptive error message, useful for outputting

=head2 get_type

The type of this error (e.g. "DocNotFoundError").

=head2 get_context

Optional context information, returned as a string

=head2 get_error_string

Returns any error string from errno or similar associated with this error

=cut
1;
