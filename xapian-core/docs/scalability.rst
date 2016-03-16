Scalability
===========

People often want to know how Xapian will scale. The short answer is
"very well" - an early version of the software powered the (now defunct)
Webtop search engine, which offered a search over around 500 million web
pages (around 1.5 terabytes of database files). Searches took less than
a second.

In terms of current deployments, `gmane <http://search.gmane.org/>`_
indexes and searches nearly 100 million mail messages on a single server
at the time of writing (2012), and we've had user reports of systems with
more than 250 million documents.

Benchmarking
------------

One effect to be aware of when designing benchmarks is that queries will
be a lot slower when nothing is cached. So the first few queries on a
database which hasn't been searched recently will be unrepresentatively
slow compared to the typical case.

In real use, pretty much all the non-leaf blocks from the B-trees being
used for the search will be cached pretty quickly, as well as many
commonly used leaf blocks.

General Scalability Considerations
----------------------------------

In a large search application, I/O will end up being the limiting
factor. So you want a RAID setup optimised for fast reading, lots of RAM
in the box so the OS can cache lots of disk blocks (the access patterns
typically mean that you only need to cache a few percent of the database
to eliminate most disk cache misses).

It also means that reducing the database size is usually a win.  Xapian's
disk-based databases compress the information in the tables in ways which
work well given the nature of the data but aren't too expensive to
unpack (e.g. lists of sorted docids are stored as differences with
smaller values encoded in fewer bytes). There is further potential for
improving the encodings used.

Another way to reduce disk I/O is to run databases through
xapian-compact. The Btree manager usually leaves some spare space in
each block so that updates are more efficient (though there are
heuristics which will fill blocks fuller when they detect a long
sequence of sequential insertions, which means adding documents to the
end of an empty database will produce fairly compact tables, apart from
the postlist table). Compacting makes all blocks as full as possible,
and so reduces the size of the database. It also produces a database
with revision 1 which is inherently faster to search. The penalty is
that updates will be slow for a while, as they'll result in a lot of
block splitting when all blocks are full.

Splitting the data over several databases is generally a good strategy.
Once each has finished being updated, compact it to make it small and
faster to search.

A multiple-database scheme works particularly well if you want a rolling
web index where the contents of the oldest database can be rechecked and
live links put back into a new database which, once built, replaces the
oldest database. It's also good for a news-type application where older
documents should expire from the index.

Size Limits in Xapian
---------------------

The glass backend (which is currently the default and recommended
backend) stores the indexes in several files containing Btree tables. If
you're indexing with positional information (for phrase searching) the
term positions table is usually the largest.

The current limits are:

-  Xapian uses unsigned 32-bit integers for document ids by default, which
   means a limit of just over 4 billion documents in a database.  Xapian 1.4
   can be built to use 64-bit document ids and term counts, and the glass
   backend will then handle 64-bit document ids (and the databases are
   compatible with a standard build provided you don't actually use docids >=
   2\ :sup:`32`).
-  If you search many databases concurrently, you may hit the
   per-process file-descriptor limit - each glass database uses between
   1 and 6 fds depending which tables are present. Some Unix-like OSes
   allow this limit to be raised. Another way to avoid it (and to spread
   the search load) is to use the remote backend to search databases on
   a cluster of machines.
-  If the OS has a filesize limit, that obviously applies to Xapian (a
   2GB limit used to be common for older operating systems). The
   xapian-core configure script will attempt to detect and automatically
   enable support for "LARGE FILES" where possible.

   So what is the limit for a modern OS? Taking Linux 2.6 as an example,
   ext4 allows files up to 16TB and filesystems up to 1EB, while btrfs
   allows files and filesystems up to 16EB (`figures from
   Wikipedia <http://en.wikipedia.org/wiki/Comparison_of_file_systems>`_).
-  The B-trees use a 32-bit unsigned block count. The default blocksize
   is 8K which limits you to 32TB tables. You can increase the blocksize
   if this is a problem, but it's best to do it before you create the
   database as otherwise you need to use xapian-compact to make a
   compacted copy of the database with the new blocksize, and that will
   take a while for such a large database. The maximum blocksize
   currently allowed is 64K, which limits you to 256TB tables.
-  Xapian stores the total length (i.e. number of terms) of all the
   documents in a database so it can calculate the average document
   length. This is currently handled as an unsigned 64-bit quantity so
   it's not likely to be a limit you'll hit. It's listed here for
   completeness.

If you've further questions about scalability, ask on the mailing lists
- people using Xapian to search large databases may be able to make
further suggestions.
