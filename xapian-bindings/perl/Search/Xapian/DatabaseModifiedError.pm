package Search::Xapian::DatabaseModifiedError;

=head1 NAME

Search::Xapian::DatabaseModifiedError -  DatabaseModifiedError indicates a database was modified.

=head1 DESCRIPTION

  To recover after catching this error, you need to call
  Xapian::Database::reopen() on the Database and repeat the operation
  which failed.


=cut
1;
