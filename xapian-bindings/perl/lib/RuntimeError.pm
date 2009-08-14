package Search::Xapian::RuntimeError;

=head1 NAME

Search::Xapian::RuntimeError -  The base class for exceptions indicating errors only detectable at runtime.

=head1 DESCRIPTION

  A subclass of RuntimeError will be thrown if Xapian detects an error
  which is exception derived from RuntimeError is thrown when an
  error is caused by problems with the data or environment rather
  than a programming mistake.


=cut
1;
