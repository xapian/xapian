.. Copyright (C) 2008 Lemur Consulting Ltd

====================================
Xapian Database Replication Protocol
====================================

.. contents:: Table of contents

This document contains details of the implementation of the replication
protocol, version 1.  For details of how and why to use the replication
protocol, see the separate `Replication Users Guide <replication.html>`_
document.

Protocol description
====================

FIXME - describe the protocol used to transfer the updates.


Changeset files
===============

Changes are represented by changeset files.  When changeset logging is enabled
for a flint database, just before each commit a changeset file is created in
the database directory.  This file contains a record of the changes made,
currently in the following format (but note that this format may change between
implementations of flint):

 - 12 bytes holding the string "FlintChanges" (used to check that a file is a
   changeset file).

 - The format of the changeset (as a variable length unsigned integer).

 - The revision number of the database before the changes were applied (as a
   variable length unsigned integer).

 - The revision number of the database after the changes were applied (as a
   variable length unsigned integer).

 - A byte:

   - 0 if the changes can safely be applied to a live database
   
   - 1 if the changes cannot be applied while searching is in progress.  (This
     will be set if the database was modified in "DANGEROUS" mode).

   - 2 if the changes contain a whole database copy (or at least, a copy of all
     active blocks in a database), in which case the changes should be used to
     make a brand new database.

 - A series of items:

   - A byte: 0 if there are no more items in the changeset, 1 if the next item
     is a base file, 2 if the next item is a list of blocks.

   - A string, holding a table name.  (preceded by the length of the string as
     a variable length unsigned integer)

   - If a base file:

     - The letter of the base file (currently 'A' or 'B').

     - The length of the file (as a variable length unsigned integer).

     - The contents of the base file.

   - If a list of blocks:

     - The blocksize in use.

     - A list of items:

       - A variable length unsigned integer holding 0 if the list is at an end,
	 or holding (block number + 1) otherwise.

       - The contents of the block.

 - A revision number that the database must be upgraded to, with more
   changesets, before it is safe to be made live.  This will normally be the
   revision number of the database after the changes were applied, but if the
   changeset was created by copying parts of the database without a read lock,
   and modifications were made to the database while the copy was in progress,
   parts of the copy may contain later revisions than intended - in this
   situation futher changesets will be needed to ensure that these parts of the
   database are fully integrated.
