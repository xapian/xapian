.. Copyright (C) 2008 Lemur Consulting Ltd
.. Copyright (C) 2008,2010,2011,2012 Olly Betts

=======================================
Xapian Database Replication Users Guide
=======================================

.. contents:: Table of contents

Introduction
============

It is often desirable to maintain multiple copies of a Xapian database, having
a "master" database which modifications are made on, and a set of secondary
(read-only, "slave") databases which these modifications propagate to.  For
example, to support a high query load there may be many search servers, each
with a local copy of the database, and a single indexing server.  In order to
allow scaling to a large number of search servers, with large databases and
frequent updates, we need an database replication implementation to have the
following characteristics:

 - Data transfer is (at most) proportional to the size of the updates, rather
   than the size of the database, to allow frequent small updates to large
   databases to be replicated efficiently.

 - Searching (on the slave databases) and indexing (on the master database) can
   continue during synchronisation.

 - Data cached (in memory) on the slave databases is not discarded (unless it's
   actually out of date) as updates arrive, to ensure that searches continue to
   be performed quickly during and after updates.

 - Synchronising each slave database involves low overhead (both IO and CPU) on
   the server holding the master database, so that many slaves can be updated
   from a single master without overloading it.

 - Database synchronisation can be recovered after network outages or server
   failures without manual intervention and without excessive data transfer.

The database replication protocol is intended to support replicating a single
writable database to multiple (read-only) search servers, while satisfying all
of the above properties.  It is not intended to support replication of multiple
writable databases - there must always be a single master database to which all
modifications are made.

This document gives an overview of how and why to use the replication protocol.
For technical details of the implementation of the replication protocol, see
the separate `Replication Protocol <replication_protocol.html>`_ document.

Backend Support
===============

Replication is supported by the chert, flint, and brass database backends,
and can cleanly handle the
master switching database type (a full copy is sent in this situation).  It
doesn't make a lot of sense to support replication for the remote backend.
Replication of inmemory databases isn't currently available.  We have a longer
term aim to replace the current inmemory backend with the current disk based
backend (e.g. chert) but storing its data in memory.  Once this is done, it
would probably be easy to support replication of inmemory databases.

Setting up replicated databases
===============================

.. FIXME - expand this section.

To replicate a database efficiently from one master machine to other machines,
there is one configuration step to be performed on the master machine, and two
servers to run.

Firstly, on the master machine, the indexer must be run with the environment
variable `XAPIAN_MAX_CHANGESETS` set to a non-zero value, which will cause
changeset files to be created whenever a transaction is committed.  A
changeset file allows the transaction to be replayed efficiently on a replica
of the database.

The value which `XAPIAN_MAX_CHANGESETS` is set to determines the maximum number
of changeset files which will be kept.  The best number to keep depends on how
frequently you run replication and how big your transactions are - if all
the changeset files needed to update a replica aren't present, a full copy of
the database will be sent, but at some point that becomes more efficient
anyway.  `10` is probably a good value to start with.

Secondly, also on the master machine, run the `xapian-replicate-server` server
to serve the databases which are to be replicated.  This takes various
parameters to control the directory that databases are found in, and the
network interface to serve on.  The `--help` option will cause usage
information to be displayed.  For example, if `/var/search/dbs`` contains a
set of Xapian databases to be replicated::

  xapian-replicate-server /var/search/dbs -p 7010

would run a server allowing access to these databases, on port 7010.

Finally, on the client machine, run the `xapian-replicate` server to keep an
individual database up-to-date.  This will contact the server on the specified
host and port, and copy the database with the name (on the master) specified in
the `-m` option to the client.  One non-option argument is required - this is
the name that the database should be stored in on the slave machine.  For
example, contacting the above server from the same machine::

  xapian-replicate -h 127.0.0.1 -p 7010 -m foo foo2

would produce a database "foo2" containing a replica of the database
"/var/search/dbs/foo".  Note that the first time you run this, this command
will create the foo2 directory and populate it with appropriate files; you
should not create this directory yourself.

As of 1.2.5, if you don't specify the master name, the same name is used
remotely and locally, so this will replicate remote database "foo2" to
local database "foo2"::

  xapian-replicate -h 127.0.0.1 -p 7010 foo2

Both the server and client can be run in "one-shot" mode, by passing `-o`.
This may be particularly useful for the client, to allow a shell script to be
used to cycle through a set of databases, updating each in turn (and then
probably sleeping for a period).

Limitations
===========

It is possible to confuse the replication system in some cases, such that an
invalid database will be produced on the client.  However, this is easy to
avoid in practice.

To confuse the replication system, the following needs to happen:

 - Start with two databases, A and B.
 - Start a replication of database A.
 - While the replication is in progress, swap B in place of A (ie, by moving
   the files around, such that B is now at the path of A).
 - While the replication is still in progress, swap A back in place of B.

If this happens, the replication process will not detect the change in
database, and you are likely to end up with a database on the client which
contains parts of A and B mixed together.  You will need to delete the damaged
database on the client, and re-run the replication.

To avoid this, simply avoid swapping a database back in place of another one.
Or at least, if you must do this, wait until any replications in progress when
you were using the original database have finished.

Calling reopen
--------------

`Database::reopen()` is usually an efficient way to ensure that a database is
up-to-date with the latest changes.  Unfortunately, it does not currently work
as you might expect with databases which are being updated by the replication
client.  The workaround is simple; don't use the reopen() method on such
databases: instead, you should close the database and open it
again from scratch.

Briefly, the issue is that the databases created by the replication client are
created in a subdirectory of the target path supplied to the client, rather
than at that path.  A "stub database" file is then created in that directory,
pointing to the database.  This allows the database which readers open to be
switched atomically after a database copy has occurred.  The reopen() method
doesn't re-read the stub database file in this situation, so ends up
attempting to read the old database which has been deleted.

We intend to fix this issue in the Brass backend (currently under development
by eliminating this hidden use of a stub database file).

Alternative approaches
======================

Without using the database replication protocol, there are various ways in
which the "single master, multiple slaves" setup could be implemented.

 - Copy database from master to all slaves after each update, then swap the new
   database for the old.

 - Synchronise databases from the master to the slaves using rsync.

 - Keep copy of database on master from before each update, and use a binary
   diff algorithm (e.g., xdelta) to calculate the changes, and then apply these
   same changes to the databases on each slave.

 - Serve database from master to slaves over NFS (or other remote file system).

 - Use the "remote database backend" facility of Xapian to allow slave servers
   to search the database directly on the master.

All of these could be made to work but have various drawbacks, and fail to
satisfy all the desired characteristics.  Let's examine them in detail:

Copying database after each update
----------------------------------

Databases could be pushed to the slaves after each update simply by copying the
entire database from the master (using scp, ftp, http or one of the many other
transfer options).  After the copy is completed, the new database would be made
live by indirecting access through a stub database and switching what it points to.

After a sufficient interval to allow searches in progress on the old database to
complete, the old database would be removed.  (On UNIX filesystems, the old
database could be unlinked immediately, and the resources used by it would be
automatically freed as soon as the current searches using it complete.)

This approach has the advantage of simplicity, and also ensures that the
databases can be correctly re-synchronised after network outages or hardware
failure.

However, this approach would involve copying a large amount of data for each
update, however small the update was.  Also, because the search server would
have to switch to access new files each time an update was pushed, the search
server will be likely to experience poor performance due to commonly accessed
pages falling out of the disk cache during the update.  In particular, although
some of the newly pushed data would be likely to be in the cache immediately
after the update, if the combination of the old and new database sizes exceeds
the size of the memory available on the search servers for caching, either some
of the live database will be dropped from the cache resulting in poor
performance during the update, or some of the new database will not initially
be present in the cache after update.

Synchronise database using rsync
--------------------------------

Rsync works by calculating hashes for the content on the client and the server,
sending the hashes from the client to the server, and then calculating (on the
server) which pieces of the file need to be sent to update the client.  This
results in a fairly low amount of network traffic, but puts a fairly high CPU
load on the server.  This would result in a large load being placed on the
master server if a large number of slaves tried to synchronise with it.

Also, rsync will not reliably update the database in a manner which allows the
database on a slave to be searched while being updated - therefore, a copy or
snapshot of the database would need to be taken first to allow searches to
continue (accessing the copy) while the database is being synchronised.

If a copy is used, the caching problems discussed in the previous section would
apply again.  If a snapshotting filesystem is used, it may be possible to take
a read-only snapshot copy cheaply (and without encountering poor caching
behaviour), but filesystems with support for this are not always available, and
may require considerable effort to set up even if they are available.

Use a binary diff algorithm
---------------------------

If a copy of the database on the master before the update was kept, a binary
diff algorithm (such as "xdelta") could be used to compare the old and new
versions of the database.  This would produce a patch file which could be
transferred to the slaves, and then applied - avoiding the need for specific
calculations to be performed for each slave.

However, this requires a copy or snapshot to be taken on the master - which has
the same problems as previously discussed.  A copy or snapshot would also need
to be taken on the slave, since a patch from xdelta couldn't safely be applied
to a live database.

Serve database from master to slaves over NFS
---------------------------------------------

NFS allows a section of a filesystem to be exported to a remote host.  Xapian
is quite capable of searching a database which is exported in such a manner,
and thus NFS can be used to quickly and easily share a database from the master
to multiple slaves.

A reasonable setup might be to use a powerful machine with a fast disk as the
master, and use that same machine as an NFS server.  Then, multiple slaves can
connect to that NFS server for searching the database. This setup is quite
convenient, because it separates the indexing workload from the search workload
to a reasonable extent, but may lead to performance problems.

There are two main problems which are likely to be encountered.  Firstly, in
order to work efficiently, NFS clients (or the OS filesystem layer above NFS)
cache information read from the remote file system in memory.  If there is
insufficient memory available to cache the whole database in memory, searches
will occasionally need to access parts of the database which are held only on
the master server.  Such searches will take a long time to complete, because
the round-trip time for an access to a disk block on the master is typically a
lot slower than the round-trip time for access to a local disk.  Additionally,
if the local network experiences problems, or the master server fails (or gets
overloaded due to all the search requests), the searches will be unable to be
completed.

Also, when a file is modified, the NFS protocol has no way of indicating that
only a small set of blocks in the file have been modified.  The caching is all
implemented by NFS clients, which can do little other than check the file
modification time periodically, and invalidate all cached blocks for the file
if the modification time has changed. For the Linux client, the time between
checks can be configured by setting the acregmin and acregmax mount options,
but whatever these are set to, the whole file will be dropped from the cache
when any modification is found.

This means that, after every update to the database on the master, searches on
the slaves will have to fetch all the blocks required for their search across
the network, which will likely result in extremely slow search times until the
cache on the slaves gets populated properly again.

Use the "remote database backend" facility
------------------------------------------

Xapian has supported a "remote" database backend since the very early days of
the project.  This allows a search to be run against a database on a remote
machine, which may seem to be exactly what we want.  However, the "remote"
database backend works by performing most of the work for a search on the
remote end - in the situation we're concerned with, this would mean that most
of the work was performed on the master, while slaves remain largely idle.

The "remote" database backend is intended to allow a large database to be
split, at the document level, between multiple hosts.  This allows systems to
be built which search a very large database with some degree of parallelism
(and thus provide faster individual searches than a system searching a single
database locally).  In contrast, the database replication protocol is intended
to allow a database to be copied to multiple machines to support a high
concurrent search load (and thus to allow a higher throughput of searches).

In some cases (i.e., a very large database and a high concurrent search load)
it may be perfectly reasonable to use both the database replication protocol in
conjunction with the "remote" database backend to get both of these advantages
- the two systems solve different problems.
