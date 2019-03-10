package Xapian::LogicError;

=head1 NAME

Xapian::LogicError -  The base class for exceptions indicating errors in the program logic.

=head1 DESCRIPTION

  This is an abstract class in C++, i.e. it cannot be instantiated directly,
  hence in Perl it has no C<new> method.

  A subclass of LogicError will be thrown if Xapian detects a violation
  of a class invariant or a logical precondition or postcondition, etc.


=cut
1;
