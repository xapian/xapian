% Scalability

People often want to know how Xapian will scale. The short answer is "very
well" - a previous version of the software powered BrightStation's Webtop
search engine, which offered a search over around 500 million web pages
(around 1.5 terabytes of database files). Searches took less than a second.

The largest recent installation we're aware of is probably [gmane][1], which
currently indexes over 65 million mail messages.

   [1]: http://search.gmane.org/

## Benchmarking

One effect to be aware of when designing benchmarks is that queries will be a
lot slower when nothing is cached. So the first few queries on a database
which hasn't been searched recently will be unrepresentatively slow compared
to the typical case.

In real use, pretty much all the non-leaf blocks from the B-trees being used
for the search will be cached pretty quickly, as well as many commonly used
leaf blocks.

## General Scalability Considerations

In a large search application, I/O will end up being the limiting factor. So
you want a RAID setup optimised for fast reading, lots of RAM in the box so
the OS can cache lots of disk blocks (the access patterns typically mean that
you only need to cache a few percent of the database to eliminate most disk
cache misses).

It also means that reducing the database size is usually a win. The Flint
backend compresses the information in the tables in ways which
work well given the nature of the data but aren't too expensive to unpack (e.g.
lists of sorted docids are stored as differences with smaller values encoded in
fewer bytes).  There is further potential for improving the encodings used.

Another way to reduce disk I/O is to run databases through xapian-compact. The
Btree manager usually leaves some spare space in each block so that updates
are more efficient (though there are heuristics which will fill blocks fuller
if they detect a long sequence of sequential insertions so adding documents to
the end of an empty database will produce fairly compact tables, apart from
the postlist table). Compacting makes all blocks as full as possible, and so
reduces the size of the database. It also produces a database with revision 1
which is inherently faster to search. The penalty is that updates will be slow
for a while, as they'll result in a lot of block splitting when all blocks are
full.

Splitting the data over several databases is generally a good strategy. Once
each has finished being updated, compact it to make it small and faster to
search.

A multiple-database scheme works particularly well if you want a rolling web
index where the contents of the oldest database can be rechecked and live
links put back into a new database which, once built, replaces the oldest
database. It's also good for a news-type application where older documents
should expire from the index.

You could take this idea further by implementing an ultra-compact read-only
backend which would take a flint Btree and convert it to something like a cdb
hashed file. The same information would be stored, but with less overhead than
the flint Btrees (which need to allow for update). If you need serious
performance, implementing such a backend is worth considering.

## Size Limits in Xapian

The flint backend (which is now the recommended backend) stores the indexes in
several files containing Btree tables. If you're indexing with positional
information (for phrase searching) the term positions table is usually the
largest.

The current limits are:

  * Xapian uses unsigned 32-bit ints for document ids, so you're limited to
just over 4 billion documents in a database (as of 2003-09-10 that's more than
a third larger than Google). The other limits will cut in first for a single
database, but searches over multiple databases are done by interleaving the
document ids, so this might start to matter (especially if one database is
much larger than the others). This interleaving technique could be changed
fairly easily if it proves problematic.

  * If you search many databases concurrently, you may hit the per-process
file-descriptor limit - each flint database uses 5 fds. Some Unix-like OSes
allow this limit to be raised. Another way to avoid it (and to spread the
search load) is to use the remote backend to search databases on a cluster of
machines. Flint could be made to not open fds for tables which aren't being
used during search (values and positions may not be), or to juggle fds - the
record table is typically only used for results, while the posting table is
typically only used during matching.

  * If the OS has a filesize limit, that obviously applies to Xapian (a 2GB
limit is common for older operating systems). The xapian-core configure script
will attempt to detect and automatically enable support for "LARGE FILES"
where possible.

So what is the limit for a modern OS? Taking Linux 2.4 with the ext2 filing
system as an example, quoting from linux/Documentation/filesystems/ext2.txt:

>
>       Filesystem block size:     1kB        2kB        4kB        8kB
>
>       File size limit:          16GB      256GB     2048GB     2048GB
>       Filesystem size limit:  2047GB     8192GB    16384GB    32768GB
>
>       There is a 2.4 kernel limit of 2048GB for a single block device,
>       so no filesystem larger than that can be created at this time.
>       There is also an upper limit on the block size imposed by the page
>       size of the kernel, so 8kB blocks are only allowed on Alpha systems
>       (and other architectures which support larger pages).

  * The B-trees use a 32-bit unsigned block count. The default blocksize is 8K
which limits you to 32TB tables. You can increase the blocksize if this is a
problem, but it's best to do it before you create the database as otherwise
you need to use xapian-compact to rebuild the database with the new blocksize,
and that will take a while for such a large database. The maximum blocksize
allowed is 64K, which limits you to 256TB tables.

  * Flint stores the total length (i.e. number of terms) of all the documents
in a database so it can calculate the average document length. This is
currently stored as an unsigned 64-bit quantity so it's not likely to be a
limit you'll hit. It's listed for completeness.

If you've further questions about scalability, ask on the mailing lists -
people using Xapian to search large databases may be able to make further
suggestions.
