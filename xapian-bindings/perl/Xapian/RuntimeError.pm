package Xapian::RuntimeError;

=head1 NAME

Xapian::RuntimeError -  The base class for exceptions indicating errors only detectable at runtime.

=head1 DESCRIPTION

  This is an abstract class in C++, i.e. it cannot be instantiated directly,
  hence in Perl it has no C<new> method.

  A subclass of RuntimeError will be thrown if Xapian detects an error
  which is exception derived from RuntimeError is thrown when an
  error is caused by problems with the data or environment rather
  than a programming mistake.


=cut
1;
