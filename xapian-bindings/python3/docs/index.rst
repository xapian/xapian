.. Copyright (C) 2003 James Aylett
.. Copyright (C) 2004,2005,2006,2007,2008,2009,2011,2013 Olly Betts
.. Copyright (C) 2007,2008,2010 Richard Boulton

===========================
Python3 bindings for Xapian
===========================

.. contents:: Table of contents

Xapian's Python3 bindings are packaged in the `xapian` module - to use
them, you'll need to add this to your code::

  import xapian

They currently require at least Python 3.2.  We've not tested with
Python 3.1 - test results and any patches needed are most welcome.

The Python API largely follows the C++ API - the differences and
additions are noted below.

The `examples` subdirectory contains examples (based on the simple C++ example)
showing how to use the Python bindings:
`simpleindex.py <examples/simpleindex.py>`_,
`simplesearch.py <examples/simplesearch.py>`_,
`simpleexpand.py <examples/simpleexpand.py>`_.
There's also
`simplematchdecider.py <examples/simplematchdecider.py>`_
which shows how to define a MatchDecider in Python.

Strings
=======

The Xapian C++ API is largely agnostic about character encoding, and uses the
`std::string` type as an opaque container for a sequence of bytes.
In places where the bytes represent text (for example, in the
`Stem`, `QueryParser` and `TermGenerator` classes), UTF-8 encoding is used.  In
order to wrap this for Python, `std::string` is mapped to/from the Python
`bytes` type.

As a convenience, you can also pass Python
`str` objects as parameters where this is appropriate, which will be
converted to UTF-8 encoded text.  Where `std::string` is
returned, it's always mapped to `bytes` in Python, which you can
convert to a Python `str` by calling `.decode('utf-8')`
on it like so::

  for i in doc.termlist():
    print(i.term.decode('utf-8'))

Unicode
=======

Currently Xapian doesn't have built-in support for normalising Unicode, so
if you want to normalise Unicode text, you'll need to do so in Python.  The
standard `unicodedata` module provides a way to do this - you probably want the
`NFKC` normalisation scheme, so normalising a query string prior to parsing it
would look something like this::

   query_string = get_query_string()
   query_string = unicodedata.normalize('NFKC', query_string)
   qp = xapian.QueryParser()
   query_obj = qp.parse_query(query_string)

Exceptions
==========

Xapian exceptions are translated into Python exceptions with the same names
and inheritance hierarchy as the C++ exception classes.  The base class of
all Xapian exceptions is the `xapian.Error` class, and this in
turn is a child of the standard python `exceptions.Exception`
class.

This means that programs can trap all xapian exceptions using `except
xapian.Error`, and can trap all exceptions which don't indicate that
the program should terminate using `except Exception`.

Iterators
=========

The iterator classes in the Xapian C++ API are wrapped in a Pythonic style.
The following are supported (where marked as "default iterator", it means
`__iter__()` does the right thing, so you can for instance use
`for term in document` to iterate over terms in a Document object):

.. table:: Python iterators

 ==================== =================================== ================================= =============
 Class                Python Method                       Equivalent C++ Method             Iterator type
 ==================== =================================== ================================= =============
 `MSet`               default iterator                    `begin()`                         `MSetIter`
 `ESet`               default iterator                    `begin()`                         `ESetIter`
 `Enquire`            `matching_terms()`                  `get_matching_terms_begin()`      `TermIter`
 `Query`              default iterator                    `get_terms_begin()`               `TermIter`
 `Database`           `allterms()` (and default iterator) `allterms_begin()`                `TermIter`
 `Database`           `postlist(term)`                    `postlist_begin(term)`            `PostingIter`
 `Database`           `termlist(docid)`                   `termlist_begin(docid)`           `TermIter`
 `Database`           `positionlist(docid, term)`         `positionlist_begin(docid, term)` `PositionIter`
 `Database`           `metadata_keys(prefix)`             `metadata_keys(prefix)`           `TermIter`
 `Database`           `spellings()`                       `spellings_begin(term)`           `TermIter`
 `Database`           `synonyms(term)`                    `synonyms_begin(term)`            `TermIter`
 `Database`           `synonym_keys(prefix)`              `synonym_keys_begin(prefix)`      `TermIter`
 `Document`           `values()`                          `values_begin()`                  `ValueIter`
 `Document`           `termlist()` (and default iterator) `termlist_begin()`                `TermIter`
 `QueryParser`        `stoplist()`                        `stoplist_begin()`                `TermIter`
 `QueryParser`        `unstemlist(term)`                  `unstem_begin(term)`              `TermIter`
 `ValueCountMatchSpy` `values()`                          `values_begin()`                  `TermIter`
 `ValueCountMatchSpy` `top_values()`                      `top_values_begin()`              `TermIter`
 ==================== =================================== ================================= =============

The pythonic iterators generally return Python objects, with properties
available as attribute values, with lazy evaluation where appropriate.  An
exception is `PositionIter` (as returned by `Database.positionlist`), which
returns an integer.

The lazy evaluation is mainly transparent, but does become visible in one situation: if you keep an object returned by an iterator, without evaluating its properties to force the lazy evaluation to happen, and then move the iterator forward, the object may no longer be able to efficiently perform the lazy evaluation.  In this situation, an exception will be raised indicating that the information requested wasn't available.  This will only happen for a few of the properties - most are either not evaluated lazily (because the underlying Xapian implementation doesn't evaluate them lazily, so there's no advantage in lazy evaluation), or can be accessed even after the iterator has moved.  The simplest work around is to evaluate any properties you wish to use which are affected by this before moving the iterator.  The complete set of iterator properties affected by this is:

 * `Database.allterms` (also accessible as `Database.__iter__`): `termfreq`
 * `Database.termlist`: `termfreq` and `positer`
 * `Document.termlist` (also accessible as `Document.__iter__`): `termfreq` and `positer`
 * `Database.postlist`: `positer`

MSet
====

MSet objects have some additional methods to simplify access (these
work using the C++ array dereferencing):

.. table:: MSet additional methods

 ============================ ================================
 Method name                  Explanation
 ============================ ================================
 `get_hit(i)`                 returns MSetItem at index i
 `get_document_percentage(i)` `convert_to_percent(get_hit(i))`
 `get_document(i)`            `get_hit(i).get_document()`
 `get_docid(i)`               `get_hit(i).get_docid()`
 ============================ ================================

Two MSet objects are equal if they have the same number and maximum possible
number of members, and if every document member of the first MSet exists at the
same index in the second MSet, with the same weight.

Non-Class Functions
===================

The C++ API contains a few non-class functions (the Database factory
functions, and some functions reporting version information), which are
wrapped like so for Python 3:

 * `Xapian::version_string()` is wrapped as `xapian.version_string()`
 * `Xapian::major_version()` is wrapped as `xapian.major_version()`
 * `Xapian::minor_version()` is wrapped as `xapian.minor_version()`
 * `Xapian::revision()` is wrapped as `xapian.revision()`

 * `Xapian::Auto::open_stub()` is wrapped as `xapian.open_stub()`
 * `Xapian::Brass::open()` is deprecated in C++, so not wrapped for Python 3
 * `Xapian::Chert::open()` is deprecated in C++, so not wrapped for Python 3
 * `Xapian::InMemory::open()` is wrapped as `xapian.inmemory_open()`
 * `Xapian::Remote::open()` is wrapped as `xapian.remote_open()` (both
   the TCP and "program" versions are wrapped - the SWIG wrapper checks the parameter list to
   decide which to call).
 * `Xapian::Remote::open_writable()` is wrapped as `xapian.remote_open_writable()` (both
   the TCP and "program" versions are wrapped - the SWIG wrapper checks the parameter list to
   decide which to call).

The version of the bindings in use is available as `xapian.__version__` (as
recommended by PEP 396).  This may not be the same as `xapian.version_string()`
as the latter is the version of xapian-core (the C++ library) in use.

Query
=====

In C++ there's a Xapian::Query constructor which takes a query operator and
start/end iterators specifying a number of terms or queries, plus an optional
parameter.  In Python, this is wrapped to accept any Python sequence (for
example a list or tuple) of terms or queries (or even a mixture of terms
and queries).  For example::

   subq = xapian.Query(xapian.Query.OP_AND, "hello", "world")
   q = xapian.Query(xapian.Query.OP_AND, [subq, "foo", xapian.Query("bar", 2)])

MatchAll and MatchNothing
-------------------------

As of 1.1.1, these are wrapped as `xapian.Query.MatchAll` and
`xapian.Query.MatchNothing`.

MatchDecider
============

Custom MatchDeciders can be created in Python by subclassing
`xapian.MatchDecider` and defining a `__call__` method
that will do the work.  Make sure you call the base class constructor in
your constructor.  The simplest example (which does nothing useful) would be as
follows::

  class mymatchdecider(xapian.MatchDecider):
    def __init__(self):
      xapian.MatchDecider.__init__(self)

    def __call__(self, doc):
      return 1

ValueRangeProcessor
===================

The ValueRangeProcessor class (and its subclasses) provide an operator() method
(which is exposed in python as a __call__() method, making the class instances
into callables).  This method checks whether a beginning and end of a range are
in a format understood by the ValueRangeProcessor, and if so, converts the
beginning and end into strings which sort appropriately.  ValueRangeProcessors
can be defined in python (and then passed to the QueryParser), or there are
several default built-in ones which can be used.

In C++ the operator() method takes two std::string arguments by reference,
which the subclassed method can modify, and returns a value slot number.
In Python, we wrap this by passing two `bytes` objects to
__call__ and having it return a tuple of (value_slot, modified_begin,
modified_end).  For example::

  vrp = xapian.NumberValueRangeProcessor(0, '$', True)
  a = '$10'
  b = '20'
  slot, a, b = vrp(a, b)

You can implement your own ValueRangeProcessor in Python.  The Python
implementation should override the __call__() method with its own
implementation, which returns a tuple as above.  For example::

  class MyVRP(xapian.ValueRangeProcessor):
    def __init__(self):
      xapian.ValueRangeProcessor.__init__(self)
    def __call__(self, begin, end):
      return (7, "A"+begin, "B"+end)

Apache and mod_python/mod_wsgi
==============================

Prior to Xapian 1.3.0, applications which use the xapian module had to be
run in the main interpreter under mod_python and mod_wsgi.  As of 1.3.0,
the xapian module no longer uses Python's simplified GIL state API, and so this
restriction should no longer apply.

Test Suite
==========

The Python bindings come with a test suite, consisting of two test files:
`smoketest.py` and `pythontest.py`. These are run by the `make check` command,
or may be run manually.  By default, they will display the names of any tests
which failed, and then display a count of tests which run and which failed.
The verbosity may be increased by setting the `VERBOSE` environment variable,
for example::

 make check VERBOSE=1

Setting VERBOSE to 1 will display detailed information about failures, and a
value of 2 will display further information about the progress of tests.
