
.. Copyright (C) 2006 Lemur Consulting Ltd
.. Copyright (C) 2007,2008,2009,2010,2011,2012,2016 Olly Betts

============================
Xapian Administrator's Guide
============================

.. contents:: Table of contents

Introduction
============

This document is intended to provide general hints, tips and advice to
administrators of Xapian systems.  It assumes that you have installed Xapian
on your system, and are familiar with the basics of creating and searching
Xapian databases.

The intended audience is system administrators who need to be able to perform
general management of a Xapian database, including tasks such as taking
backups and optimising performance.  It may also be useful introductory
reading for Xapian application developers.

The document is up-to-date for Xapian version 1.4.21.

Databases
=========

Xapian databases hold all the information needed to perform searches in a set
of tables.  The default database backend for the 1.4 release series is called
`glass`.  The default backend for the 1.2 release series was called `chert`,
and this is also supported by 1.4.

Glass Backend
-------------

The following table always exists:

 - The `postlist` table holds a list of all the documents indexed by each term
   in the database (`postings`), and also chunked streams of the values in each
   value slot.

The following table exists by default, but you can choose not to have it:

 - The `termlist` table holds a list of all the terms which index each
   document, and also the value slots used in each document.  Without this,
   some features aren't supported - see `Xapian::DB_NO_TERMLIST` for details.

And the following optional tables exist only when there is data to store in
them:

 - The `docdata` table holds the document data associated with each document
   in the database.
 - The `position` table stores all the word positions in each document
   which each term occurs at.
 - The `spelling` table holds data for suggesting spelling corrections.
 - The `synonym` table holds a synonym dictionary.

Each of the tables is held in a separate file with extension `.glass` (e.g.
`postlist.glass`), allowing an administrator to see how much data is being used
for each of the above purposes.

The `.glass` file actually stores the data, and is structured as a tree of
blocks, which have a default size of 8KB (though this can be set, either
through the Xapian API, or with the ``xapian-compact`` tool described below).

Changing the blocksize may have performance implications, but it is hard to
know whether these will be positive or negative for a particular combination
of hardware and software without doing some profiling.

The `.baseA` and `.baseB` files you may remember if you've worked with older
Xapian database backends no longer exist in glass databases - the information
about unused blocks is stored in a freelist (itself stored in unused blocks in
the `.glass` file, and the other information is stored in the `iamglass`
file.

Glass also supports databases stored in a single file - currently these only
support read operations, and have to be created by compacting an existing
glass database.  It won't save you diskspace, but it means only one file needs
to be opened to open the database so reduces initialisation overhead a little,
and a single file is more convenient if you need to copy it around. You can
even embed the database in another file so you can ship a single file
containing content and a Xapian database which provides a search of it.

Chert Backend
-------------

The following tables always exist:

 - The `postlist` holds a list of all the documents indexed by
   each term in the database, and also chunked streams of the values in each
   value slot.
 - The `record` holds the document data associated with each document
   in the database.
 - The `termlist` holds a list of all the terms which index each
   document, and also the value slots used in each document.

And the following optional tables exist only when there is data to store in
them:

 - The `position` holds a list of all the word positions in each
   document which each term occurs at.
 - The `spelling` holds data for suggesting spelling corrections.
 - The `synonym` holds a synonym dictionary.

Each of the tables is held in a separate file, allowing an administrator to
see how much data is being used for each of the above purposes.  It is not
always necessary to fully populate these tables: for example, if phrase
searches are never going to be performed on the database, it is not necessary
to store any positionlist information.

If you look at a Xapian database, you will see that each of these tables
actually uses 2 or 3 files.  For example, for a "chert" format database the
termlist table is stored in the files ``termlist.baseA``, ``termlist.baseB``
and ``termlist.DB``.

The ``.DB`` file actually stores the data, and is structured as a tree of
blocks, which have a default size of 8KB (though this can be set, either
through the Xapian API, or with some of the tools detailed later in this
document).

The ``.baseA`` and ``.baseB`` files are used to keep track of where to start
looking for data in the ``.DB`` file (the root of the tree), and which blocks are
in use.  Often only one of the ``.baseA`` and ``.baseB`` files will be present;
each of these files refers to a revision of the database, and there may be more
than one valid revision of the database stored in the ``.DB`` file at once.

Changing the blocksize may have performance implications, but it is hard to
tell whether these will be positive or negative for a particular combination
of hardware and software without doing some profiling.

Atomic modifications
--------------------

Xapian ensures that all modifications to its database are performed
atomically.  This means that:

 - From the point of view of a separate process (or a separate database object
   in the same process) reading the database, all modifications made to a
   database are invisible until the modifications is committed.
 - The database on disk is always in a consistent state.
 - If the system is interrupted during a modification, the database should
   always be left in a valid state.  This applies even if the power is cut
   unexpectedly, as long as the disk does not become corrupted due to hardware
   failure.

Committing a modification requires several calls to the operating system to
make it flush any cached modifications to the database to disk.  This is to
ensure that if the system fails at any point, the database is left in a
consistent state.  Of course, this is a fairly slow process (since the system
has to wait for the disk to physically write the data), so grouping many
changes together will speed up the throughput considerably.

Many modifications can be explicitly grouped into a single transaction, so
that lots of changes are applied at once.  Even if an application doesn't
explicitly protect modifications to the database using transactions, Xapian
will group modifications into transactions, applying the modifications in
batches.

Note that it is not currently possible to extend Xapian's transactions to
cover multiple databases, or to link them with transactions in external
systems, such as an RDBMS.

Finally, note that it is possible to compile Xapian such that it doesn't make
modifications in an atomic manner, in order to build very large databases more
quickly (search the Xapian mailing list archives for "DANGEROUS" mode for more
details).  This isn't yet integrated into standard builds of Xapian, but may
be in future, if appropriate protections can be incorporated.

Single writer, multiple reader
------------------------------

Xapian implements a "single writer, multiple reader" model.  This means that,
at any given instant, there is only permitted to be a single object modifying
a database, but there may (simultaneously) be many objects reading the
database at once.

Xapian enforces this restriction using by having a writer lock the database.
Each Xapian database directory contains a lock file named
``flintlock`` (we've kept the same name as flint used, since the locking
technique is the same).

This lock-file will always exist, but will be locked using ``fcntl()`` when the
database is open for writing.  A major advantage of ``fnctl()`` locks is that
if a writer exits without being given a chance to clean up (for example, if the
application holding the writer is killed), any ``fcntl()`` locks held will be
automatically released by the operating system so stale locks can't happen.

Unfortunately, ``fcntl()`` locking has some unhelpful semantics (if a process
closes *ANY* open file descriptor on the file that releases the lock) so on
most POSIX platforms we spawn a child process to hold the lock for each
database opened for writing, which then exec-s ``cat``, so you will see a
``cat`` subprocess of any writer process in the output of ``ps``, ``top``, etc.

"Open File Description" locks are like traditional ``fcntl()`` locks but with
this problem addressed, and Xapian will use these if available and avoid these
extra child processes.  At the time of writing it seems only Linux (since kernel
3.15) actually supports these, but they've been accepted for POSIX issue 8.

Under Microsoft Windows, we use a different locking technique which doesn't
require a child process, but still means the lock is released automatically
when the writing process exits.

Revision numbers
----------------

Xapian databases contain a revision number.  This is essentially a count of
the number of modifications since the database was created, and is needed to
implement the atomic modification functionality.  It is stored as a 32 bit
integer, so there is a chance that a very frequently updated database could
cause this to overflow.  The consequence of such an overflow would be to throw
an exception reporting that the database has run out of revision numbers.

This isn't likely to be a practical problem, since it would take nearly a year
for a database to reach this limit if 100 modifications were committed every
second, and no normal Xapian system will commit more than once every few
seconds.  However, if you are concerned, you can use the ``xapian-compact``
tool to make a fresh copy of the database with the revision number set to 1.

The revision number of each table can be displayed by the ``xapian-check``
tool.

Network file systems
--------------------

Xapian should work correctly over a network file system.  However, there are
various potential issues with such file systems, so we recommend
extensive testing of your particular network file system before deployment.

Be warned that Xapian is heavily I/O dependent, and therefore performance over
a network file system is likely to be slow unless you've got a very well tuned
setup.

Xapian needs to be able to lock a file in a database directory when
modifications are being performed.  On some network files systems (e.g., NFS)
this requires a lock daemon to be running.

Which database format to use?
-----------------------------

As of release 1.4.0, you should generally use the glass format (which is now
the default).

Support for the pre-1.0 quartz format (deprecated in 1.0) was removed in 1.1.0.
See below for how to convert a quartz database to a flint one.

The flint backend (the default for 1.0, and still supported by 1.2.x) was
removed in 1.3.0.  See below for how to convert a flint database to a chert one.

The chert backend (the default for 1.2) is still supported by 1.4.x, but
deprecated - only use it if you already have databases in this format; and plan
to migrate away.

.. There's also a development backend called XXXXX.  The main distinguishing
.. feature of this is that the format may change incompatibly from time to time.
.. It passes Xapian's extensive testsuite, but has seen less real world use
.. than glass.

Can I put other files in the database directory?
------------------------------------------------

If you wish to store meta-data or other information relating to the Xapian
database, it is reasonable to wish to put this in files inside the Xapian
database directory, for neatness.  For example, you might wish to store a list
of the prefixes you've applied to terms for specific fields in the database.

Current Xapian backends don't do anything
which will break this technique, so as long as you don't choose a filename
that Xapian uses itself, there should be no problems.  However, be aware that
new versions of Xapian may use new files in the database directory, and it is
also possible that new backend formats may not be compatible with the
technique.  And of course you can't do this with a single-file glass database.


Backup Strategies
=================

Summary
-------

 - The simplest way to perform a backup is to temporarily halt modifications,
   take a copy of all files in the database directory, and then allow
   modifications to resume.  Read access can continue while a backup is being
   taken.

 - If you have a filesystem which allows atomic snapshots to be taken of
   directories (such as an LVM filesystem), an alternative strategy is to take
   a snapshot and simply copy all the files in the database directory to the
   backup medium.  Such a copy will always be a valid database.

 - Progressive backups are not easily possible; modifications are typically
   spread throughout the database files.

Detail
------

Even though Xapian databases are often automatically generated from source
data which is stored in a reliable manner, it is usually desirable to keep
backups of Xapian databases being run in production environments.  This is
particularly important in systems with high-availability requirements, since
re-building a Xapian database from scratch can take many hours.  It is also
important in the case where the data stored in the database cannot easily be
recovered from external sources.

Xapian databases are managed such that at any instant in time, there is at
least one valid revision of the database written to disk (and if there are
multiple valid revisions, Xapian will always open the most recent).
Therefore, if it is possible to take an instantaneous snapshot of all the
database files (for example, on an LVM filesystem), this snapshot is suitable
for copying to a backup medium.  Note that it is not sufficient to take a
snapshot of each database file in turn - the snapshot must be across all
database files simultaneously.  Otherwise, there is a risk that the snapshot
could contain database files from different revisions.

If it is not possible to take an instantaneous snapshot, the best backup
strategy is simply to ensure that no modifications are committed during the
backup procedure.  While the simplest way to implement this may be to stop
whatever processes are used to modify the database, and ensure that they close
the database, it is not actually necessary to ensure that no writers are open
on the database; it is enough to ensure that no writer makes any modification
to the database.

Because a Xapian database can contain more than one valid revision of the
database, it is actually possible to allow a limited number of modifications
to be performed while a backup copy is being made, but this is tricky and we
do not recommend relying on it.  Future versions of Xapian are likely to
support this better, by allowing the current revision of a database to be
preserved while modifications continue.

Progressive backups are not recommended for Xapian databases: Xapian database
files are block-structured, and modifications are spread throughout the
/database file.  Therefore, a progressive backup tool will not be able to take
a backup by storing only the new parts of the database.  Modifications will
normally be so extensive that most parts of the database have been modified,
however, if only a small number of modifications have been made, a binary diff
algorithm might make a usable progressive backup tool.


Inspecting a database
=====================

When designing an indexing strategy, it is often useful to be able to check
the contents of the database.  Xapian includes a simple command-line program,
``xapian-delve``, to allow this (prior to 1.3.0, ``xapian-delve`` was usually
called ``delve``, though some packages were already renaming it).

For example, to display the list of terms in document "1" of the database
"foo", use:

.. code-block:: sh

  xapian-delve foo -r 1

It is also possible to perform simple searches of a database.  Xapian includes
another simple command-line program, ``quest``, to support this.  ``quest`` is
only able to search for un-prefixed terms, the query string must be quoted to
protect it from the shell.  To search the database "foo" for the phrase "hello
world", use:

.. code-block:: sh

  quest -d foo '"hello world"'

If you have installed the "Omega" CGI application built on Xapian, this can
also be used with the built-in "godmode" template to provide a web-based
interface for browsing a database.  See Omega's documentation for more details
on this.

Database maintenance
====================

Compacting a database
---------------------

Xapian databases normally have some spare space in each block to allow
new information to be efficiently slotted into the database.  However, the
smaller a database is, the faster it can be searched, so if there aren't
expected to be many further modifications, it can be desirable to compact the
database.

Xapian includes a tool called ``xapian-compact`` for compacting databases.
This tool makes a copy of a database, and takes advantage of
the sorted nature of the source Xapian database to write the database out
without leaving spare space for future modifications.  This can result in a
large space saving.

The downside of compaction is that future modifications may take a little
longer, due to needing to reorganise the database to make space for them.
However, modifications are still possible, and if many modifications are made,
the database will gradually develop spare space.

There's an option (``-F``) to perform a "fuller" compaction.  This option
compacts the database as much as possible, but it violates the design of the
Btree format slightly to achieve this, so it is not recommended if further
modifications are at all likely in future.  If you do need to modify a "fuller"
compacted database, we recommend you run ``xapian-compact`` on it without ``-F``
first.

You can specify the blocksize to use for the compacted database (which should
be a power of 2 between 2KB and 64KB, with the default being 8KB).

Making the blocksize a multiple of (or the same as) both the sector size of the
device and the blocksize of the filing system which the database is on is
a good idea, but sector size seems to always be 4K or less
(at least according to https://en.wikipedia.org/wiki/Disk_sector) and FS block
size still seems to be 4K by default (the widely used Linux ext4 FS potentially
supports up to 64K but only up to the system page size which is 4K on e.g. x86
and x86-64).  So in practice a Xapian blocksize of 4KB or more will satisfy
this.

The main benefits a larger blocksize gives are slightly more efficient packing
and reduced total per-block overheads (and the additional gains here are
likely to be smaller for each extra block size doubling), while the downside is
needing to read/write more data to read/write a single block. The extra data is
at least contiguous (at least in file offset terms - maybe not always on disk
if the file is fragmented) but there are potentially significant negative
factors like added pressure on the drive cache and OS file cache. The
additional losses are likely to grow for each extra block size doubling.

In general for most people just using the default block size is sensible. It's
something you might tune when you either care more about reducing size over
anything else, or if you're prepared to profile your complete system with
different block sizes to see what works best for your own situation.

If profiling different blocksizes including the 8KB default, remember to use a
compacted version for the 8KB block size database or else you won't get a fair
comparison.


Merging databases
-----------------

When building an index for a very large amount of data, it can be desirable to
index the data in smaller chunks (perhaps on separate machines), and then
merge the chunks together into a single database.  This can be performed using
the ``xapian-compact`` tool, simply by supplying several source database paths.

Normally, merging works by reading the source databases in parallel, and
writing the contents in sorted order to the destination database.  This will
work most efficiently if excessive disk seeking can be avoided; if you have
several disks, it may be worth placing the source databases and the
destination database on separate disks to obtain maximum speed.

The ``xapian-compact`` tool supports an additional option, ``--multipass``,
which is useful when merging more than three databases.  This will cause the
postlist tables to be grouped and merged into temporary tables, which are then
grouped and merged, and so on until a single postlist table is created, which
is usually faster, but requires more disk space for the temporary files.


Checking database integrity
---------------------------

Xapian includes a command-line tool to check that a database is
self-consistent.  This tool, ``xapian-check``, runs through the entire database,
checking that all the internal nodes are correctly connected.  It can also be
used on a single table, for example, this command will check the termlist table
of database "foo":

.. code-block:: sh

  xapian-check foo/termlist.DB


Fixing corrupted databases
--------------------------

The "xapian-check" tool is capable of fixing corrupted databases in certain
limited situations.  Currently it only supports this for chert, where it is
capable of:

 * Regenerating a damaged ``iamchert`` file (if you've lost yours completely
   just create an invalid one, e.g. with ``touch iamchert``).

 * Regenerating damaged or lost base files from the corresponding DB files.
   This was developed for the scenario where the database is freshly compacted
   but should work provided the last update was cleanly applied.  If the last
   update wasn't actually committed, then it is possible that it will try to
   pick the root block for the partial update, which isn't what you want.
   If you are in this situation, come and talk to us - with a testcase we
   should be able to make it handle this better.

To fix such issues, run xapian-check like so:

.. code-block:: sh

  xapian-check /path/to/database F


Converting a chert database to a glass database
-----------------------------------------------

This can be done using the ``copydatabase`` example program included with Xapian.
This is a lot slower to run than ``xapian-compact``, since it has to perform the
sorting of the term occurrence data from scratch, but should be faster than a
re-index from source data since it doesn't need to perform the tokenisation
step.  It is also useful if you no longer have the source data available.

The following command will copy a database from "SOURCE" to "DESTINATION",
creating the new database at "DESTINATION" as a chert database:

.. code-block:: sh

  copydatabase SOURCE DESTINATION

By default copydatabase will renumber your documents starting with docid 1.
If the docids are stored in or come from some external system, you should
preserve them by using the ``--no-renumber`` option:

.. code-block:: sh

  copydatabase --no-renumber SOURCE DESTINATION


Converting a pre-1.1.4 chert database to a chert database
---------------------------------------------------------

The chert format changed in 1.1.4 - at that point the format hadn't been
finalised, but a number of users had already deployed it, and it wasn't hard
to write an updater, so we provided one called ``xapian-chert-update`` which
makes a copy with the updated format:

.. code-block:: sh

  xapian-chert-update SOURCE DESTINATION

It works much like ``xapian-compact`` so should take a similar amount of time
(and results in a compact database).  The initial version had a few bugs, so
use xapian-chert-update from Xapian 1.2.5 or later.

The ``xapian-chert-update`` utility was removed in Xapian 1.3.0, so you'll need
to install Xapian 1.2.x to use it.


Converting a flint database to a chert database
-----------------------------------------------

It is possible to convert a flint database to a chert database by installing
Xapian 1.2.x (since this has support for both flint and chert)
using the ``copydatabase`` example program included with Xapian.  This is a
lot slower to run than ``xapian-compact``, since it has to perform the
sorting of the term occurrence data from scratch, but should be faster than a
re-index from source data since it doesn't need to perform the tokenisation
step.  It is also useful if you no longer have the source data available.

The following command will copy a database from "SOURCE" to "DESTINATION",
creating the new database at "DESTINATION" as a chert database:

.. code-block:: sh

  copydatabase SOURCE DESTINATION

By default ``copydatabase`` will renumber your documents starting with docid 1.
If the docids are stored in or come from some external system, you should
preserve them by using the ``--no-renumber`` option (new in Xapian 1.2.5):

.. code-block:: sh

  copydatabase --no-renumber SOURCE DESTINATION

Converting a quartz database to a flint database
------------------------------------------------

It is possible to convert a quartz database to a flint database by installing
Xapian 1.0.x (since this has support for both quartz and flint)
and using the ``copydatabase`` example program included with Xapian.  This is a
lot slower to run than ``xapian-compact``, since it has to perform the
sorting of the term occurrence data from scratch, but should be faster than a
re-index from source data since it doesn't need to perform the tokenisation
step.  It is also useful if you no longer have the source data available.

The following command will copy a database from "SOURCE" to "DESTINATION",
creating the new database at "DESTINATION" as a flint database:

.. code-block:: sh

  copydatabase SOURCE DESTINATION


Converting a 0.9.x flint database to work with 1.0.y
----------------------------------------------------

In 0.9.x, flint was the development backend.

Due to a bug in the flint position list encoding in 0.9.x which made flint
databases non-portable between platforms, we had to make an incompatible
change in the flint format.  It's not easy to write an upgrader, but you
can convert a database using the following procedure (although it might
be better to rebuild from scratch if you want to use the new UTF-8 support
in ``Xapian::QueryParser``, ``Xapian::Stem``, and
``Xapian::TermGenerator``).

Run the following command in your Xapian 0.9.x installation to copy your
0.9.x flint database "SOURCE" to a new quartz database "INTERMEDIATE":

.. code-block:: sh

  copydatabase SOURCE INTERMEDIATE

Then run the following command in your Xapian 1.0.y installation to copy
your quartz database to a 1.0.y flint database "DESTINATION":

.. code-block:: sh

  copydatabase INTERMEDIATE DESTINATION
