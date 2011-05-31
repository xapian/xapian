Index
=====

-  `ROOT <#ROOT>`_
-  `api <#api>`_
-  `backends <#backends>`_

   -  `backends/brass <#backends_brass>`_
   -  `backends/chert <#backends_chert>`_
   -  `backends/flint <#backends_flint>`_
   -  `backends/inmemory <#backends_inmemory>`_
   -  `backends/multi <#backends_multi>`_
   -  `backends/remote <#backends_remote>`_

-  `bin <#bin>`_
-  `common <#common>`_
-  `docs <#docs>`_
-  `examples <#examples>`_
-  `expand <#expand>`_
-  `include <#include>`_
-  `languages <#languages>`_
-  `matcher <#matcher>`_
-  `net <#net>`_
-  `queryparser <#queryparser>`_
-  `tests <#tests>`_

   -  `tests/harness <#tests_harness>`_
   -  `tests/perftest <#tests_perftest>`_

-  `unicode <#unicode>`_
-  `weight <#weight>`_

--------------

Directory structure
===================

`ROOT <http://trac.xapian.org/browser/tags/1.2.5/xapian-core/>`_
----------------------------------------------------------------

Top level directory.

`api <http://trac.xapian.org/browser/tags/1.2.5/xapian-core/api/>`_
-------------------------------------------------------------------

API classes and their PIMPL internals.

`backends <http://trac.xapian.org/browser/tags/1.2.5/xapian-core/backends/>`_
-----------------------------------------------------------------------------

This directory contains a subdirectory for each of the available
database backends. Each backend corresponds to a different underlying
file structure. For example, the inmemory backend holds its databases
entirely in RAM, and the flint backend is a fully featured disk based
backend.

The directory also contains the implementation of
Xapian::Database::Internal (the base class for each backend database
class) and the factory functions which are the public interface for
instantiating the database backend classes.

`backends/brass <http://trac.xapian.org/browser/tags/1.2.5/xapian-core/backends/brass/>`_
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Brass is the development backend for the Xapian 1.2.x series. It's
highly efficient, and also (aims to) use significantly less disk space
than previous Xapian backends. All Xapian features are supported.

`backends/chert <http://trac.xapian.org/browser/tags/1.2.5/xapian-core/backends/chert/>`_
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Chert is the default backend for the Xapian 1.2.x release series. It
uses a custom written Btree management system to store posting lists and
termlists. This is a highly efficient backend, using compression to
store the postlists, and supporting the full range of indexing
functionality (positional information, transactions, etc).

`backends/flint <http://trac.xapian.org/browser/tags/1.2.5/xapian-core/backends/flint/>`_
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Flint is the default Xapian backend as of Xapian 1.0. It uses a custom
written Btree management system to store posting lists and termlists.
This is a highly efficient backend, using compression to store the
postlists, and supporting the full range of indexing functionality
(positional information, transactions, etc).

`backends/inmemory <http://trac.xapian.org/browser/tags/1.2.5/xapian-core/backends/inmemory/>`_
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

This backend stores a database entirely in memory. When the database is
closed these indexed contents are lost.

This is useful for searching through relatively small amounts of data
(such as a single large file) which hasn't previously been indexed.

`backends/multi <http://trac.xapian.org/browser/tags/1.2.5/xapian-core/backends/multi/>`_
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The MultiDatabase backend, which enables searches to be performed across
several databases. Opening this database involves opening each of the
sub-databases and merging them together.

Searches are performed across the sub-databases via MultiPostList and
MultiTermList objects, which represent merged sets of postlist and
termlist objects.

`backends/remote <http://trac.xapian.org/browser/tags/1.2.5/xapian-core/backends/remote/>`_
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The remote backend, which enables searches to be performed across
databases on remote machines. Opening this database involves opening a
communications channel with a remote database.

RemoteDatabase objects are used with RemoteSubMatch objects.

`bin <http://trac.xapian.org/browser/tags/1.2.5/xapian-core/bin/>`_
-------------------------------------------------------------------

Programs relating to the Xapian library.

`common <http://trac.xapian.org/browser/tags/1.2.5/xapian-core/common/>`_
-------------------------------------------------------------------------

Header files which are used in various places within the Xapian library
code. It does not contain header files which are externally visible:
these are kept in the "include" directory.

`docs <http://trac.xapian.org/browser/tags/1.2.5/xapian-core/docs/>`_
---------------------------------------------------------------------

Documentation, and scripts to automatically generate further
documentation from the source code. If you have the appropriate packages
installed (currently, this means Perl), these scripts will be run by
make.

`examples <http://trac.xapian.org/browser/tags/1.2.5/xapian-core/examples/>`_
-----------------------------------------------------------------------------

This directory contains example programs which use the Xapian library.
These programs are intended to be a good starting point for those trying
to get acquainted with the Xapian API. Some of them are really just toy
programs, but others are actually useful utilities in their own right
(for example: delve, quest, and copydatabase).

`expand <http://trac.xapian.org/browser/tags/1.2.5/xapian-core/expand/>`_
-------------------------------------------------------------------------

This directory houses the query expansion code.

`include <http://trac.xapian.org/browser/tags/1.2.5/xapian-core/include/>`_
---------------------------------------------------------------------------

This directory contains the externally visible header files. Internal
header files are kept in the "common" directory.

`languages <http://trac.xapian.org/browser/tags/1.2.5/xapian-core/languages/>`_
-------------------------------------------------------------------------------

Utilities for performing processing of text in various different
languages. Current these comprise stemming algorithms. In future
language detection, character set normalisation, and other language
related utilities will be added.

`matcher <http://trac.xapian.org/browser/tags/1.2.5/xapian-core/matcher/>`_
---------------------------------------------------------------------------

The code for performing the best match algorithm lives here. This is the
heart of the Xapian system, and is the code which calculates relevance
rankings for the documents in the collection for a given query.

`net <http://trac.xapian.org/browser/tags/1.2.5/xapian-core/net/>`_
-------------------------------------------------------------------

The code implementing the network protocols lives here.

`queryparser <http://trac.xapian.org/browser/tags/1.2.5/xapian-core/queryparser/>`_
-----------------------------------------------------------------------------------

Implementations of the Xapian::QueryParser and Xapian::TermGenerator
classes.

`tests <http://trac.xapian.org/browser/tags/1.2.5/xapian-core/tests/>`_
-----------------------------------------------------------------------

This directory contains various test programs which exercise most parts
of the Xapian library.

`tests/harness <http://trac.xapian.org/browser/tags/1.2.5/xapian-core/tests/harness/>`_
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

This contains the test suite harness, which is linked with by most of
the C++ test programs to perform sets of tests.

`tests/perftest <http://trac.xapian.org/browser/tags/1.2.5/xapian-core/tests/perftest/>`_
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

This directory contains various the performance test suite.

`unicode <http://trac.xapian.org/browser/tags/1.2.5/xapian-core/unicode/>`_
---------------------------------------------------------------------------

Unicode and UTF-8 handling classes and functions.

`weight <http://trac.xapian.org/browser/tags/1.2.5/xapian-core/weight/>`_
-------------------------------------------------------------------------

Implementations of weighting schemes for Xapian.

--------------

Generated for xapian-core 1.2.5 on 2011-04-15 by
gen\_codestructure\_doc.
