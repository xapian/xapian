.. Copyright (C) 2008 Lemur Consulting Ltd
.. Copyright (C) 2010,2014 Olly Betts

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

The protocol used to transfer the updates is based on the RemoteConnection
system (also used for remote Xapian databases).  This provides a "message"
layer abstraction for the connections; so the communication is based on a set
of messages, each with a type, and some associated data.

Where the following description refers to "packed" strings or integers, this
means packed according to the same methods for packing these into databases.

Client messages
---------------

The client sends a single message type to the server: this is a message of type
'R', and includes the name of a database to be replicated and a revision string
for that database.  This message is sent whenever the client wants to receive
updates for a database.

Server messages
---------------

The server can send a variety of messages.  The message types are currently
defined in an enum in common/replicationprotocol.h (in which each type is
preceded by ``REPL_REPLY_``):

 - END_OF_CHANGES: this indicates that no further changes are needed, and ends
   the response to the original request.  It contains no data.

 - FAIL: this indicates that a consistent set of changes couldn't be sent.  It
   may occur because the database is being changed too quickly at the senders
   end, or for other reasons.  It ends the response to the original request,
   and may occur when any other messages are expected.

 - DB_HEADER: this indicates that an entire database copy is about to be sent.
   It contains a string representing the UUID of the database which is about to
   be sent, followed by a (packed) unsigned integer, representing the revision
   number of the copy which is about to be sent.

 - DB_FILENAME: this contains the name of the next file to be sent in a DB copy
   operation.

 - DB_FILEDATA: this contains the contents of a file in a DB copy operation.
   The contents of the message are the details of the file.

 - DB_FOOTER: this indicates the end of a DB copy operation.  The contents of
   this message are a single (packed) unsigned integer, which represents a
   revision number.  The newly copied database is not safe to make live until
   changesets up to the specified revision have been applied.

 - CHANGESET: this indicates that a changeset file (see below) is being sent.

Changeset files
===============

Changes are represented by changeset files.  When changeset logging is enabled
for a database, just before each commit a changeset file is created in
the database directory.  This file contains a record of the changes made,
currently in the following format (but note that this format may change in
the future):

 - 12 bytes holding the string "FlintChanges", "ChertChanges" or "GlassChanges"
   (used to check that a file is a changeset file).

 - The format of the changeset (as a variable length unsigned integer).

 - The revision number of the database before the changes were applied (as a
   variable length unsigned integer).

 - The revision number of the database after the changes were applied (as a
   variable length unsigned integer).

 - A byte:

   - 0 if the changes can safely be applied to a live database

   - 1 if the changes cannot be applied while searching is in progress.  (This
     will be set if the database was modified in "DANGEROUS" mode).

 - A series of items:

   - A byte: 0 if there are no more items in the changeset, 1 if the next item
     is a base file, 2 if the next item is a list of blocks.

   - A (packed) string, holding a table name.

   - If a base file:

     - The letter of the base file (currently 'A' or 'B').

     - The length of the file (as a variable length unsigned integer).

     - The contents of the base file.

   - If a list of blocks:

     - The blocksize in use (for glass, divided by 2048).

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
   situation further changesets will be needed to ensure that these parts of
   the database are fully integrated.
