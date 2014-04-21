Python bindings for Xapian
**************************

The Python bindings for Xapian are packaged in the ``xapian`` module,
and largely follow the C++ API, with the following differences and
additions. Python strings and lists, etc., are converted automatically
in the bindings, so generally it should just work as expected.

The ``examples`` subdirectory contains examples showing how to use the
Python bindings based on the simple examples from ``xapian-examples``:
:ref:`simpleindex`,
:ref:`simplesearch`,
:ref:`simpleexpand`,
There's also
:ref:`simplematchdecider`,
which shows how to define a MatchDecider in Python.


The Python bindings come with a test suite, consisting of two test files:
``smoketest.py`` and ``pythontest.py``. These are run by the
"``make check``" command, or may be run manually.  By default, they
will display the names of any tests which failed, and then display a count of
tests which run and which failed.  The verbosity may be increased by setting
the "``VERBOSE``" environment variable: a value of 1 will display
detailed information about failures, and a value of 2 will display further
information about the progress of tests.


Exceptions
##########

Xapian exceptions are translated into Python exceptions with the same names
and inheritance hierarchy as the C++ exception classes.  The base class of
all Xapian exceptions is the ``xapian.Error`` class, and this in
turn is a child of the standard python ``exceptions.Exception``
class.

This means that programs can trap all xapian exceptions using "``except
xapian.Error``", and can trap all exceptions which don't indicate that
the program should terminate using "``except Exception``".


Unicode
#######

The xapian Python bindings accept unicode strings as well as simple strings
(ie, "str" type strings) at all places in the API which accept string data.
Any unicode strings supplied will automatically be translated into UTF-8
simple strings before being passed to the Xapian core.  The Xapian core is
largely agnostic about character encoding, but in those places where it does
process data in a character encoding dependent way it assumes that the data
is in UTF-8.  The Xapian Python bindings always return string data as simple
strings.

Therefore, in order to avoid issues with character encodings, you should
always pass text data to Xapian as unicode strings, or UTF-8 encoded simple
strings.  There is, however, no requirement for simple strings passed into
Xapian to be valid UTF-8 encoded strings, unless they are being passed to a
text processing routine (such as the query parser, or the stemming
algorithms).  For example, it is perfectly valid to pass arbitrary binary
data in a simple string to the ``xapian.Document.set_data()``
method.

It is often useful to normalise unicode data before passing it to Xapian -
Xapian currently has no built-in support for normalising unicode
representations of data.  The standard python module
"``unicodedata``" provides support for normalising unicode: you
probably want the "``NFKC``" normalisation scheme: in other words,
use something like

::

  unicodedata.normalize('NFKC', u'foo')

to normalise the string "foo" before passing it to Xapian.


Iterators
#########

The iterator classes in the Xapian C++ API are wrapped in a "Pythonic" style.
The following are supported (where marked as default iterator, it means
``__iter__()`` does the right
thing so you can for instance use ``for term in document`` to
iterate over terms in a Document object):


+----------------------+------------------------------------------+---------------------------------------+-----------------+
| Class                | Method                                   | Equivalent to                         | Iterator type   |
+======================+==========================================+=======================================+=================+
|``MSet``              | default iterator                         | ``begin()``                           | ``MSetIter``    |
+----------------------+------------------------------------------+---------------------------------------+-----------------+
|``ESet``              |default iterator                          | ``begin()``                           | ``ESetIter``    |
+----------------------+------------------------------------------+---------------------------------------+-----------------+
|``Enquire``           | ``matching_terms()``                     | ``get_matching_terms_begin()``        | ``TermIter``    |
+----------------------+------------------------------------------+---------------------------------------+-----------------+
|``Query``             | default iterator                         | ``get_terms_begin()``                 | ``TermIter``    |
+----------------------+------------------------------------------+---------------------------------------+-----------------+
|``Database``          | ``allterms()`` (also as default iterator)| ``allterms_begin()``                  | ``TermIter``    |
+----------------------+------------------------------------------+---------------------------------------+-----------------+
|``Database``          | ``postlist(tname)``                      | ``postlist_begin(tname)``             | ``PostingIter`` |
+----------------------+------------------------------------------+---------------------------------------+-----------------+
|``Database``          | ``termlist(docid)``                      | ``termlist_begin(docid)``             | ``TermIter``    |
+----------------------+------------------------------------------+---------------------------------------+-----------------+
|``Database``          | ``positionlist(docid, tname)``           | ``positionlist_begin(docid, tname)``  | ``PositionIter``|
+----------------------+------------------------------------------+---------------------------------------+-----------------+
|``Database``          | ``metadata_keys(prefix)``                | ``metadata_keys(prefix)``             | ``TermIter``    |
+----------------------+------------------------------------------+---------------------------------------+-----------------+
|``Database``          | ``spellings()``                          | ``spellings_begin(term)``             | ``TermIter``    |
+----------------------+------------------------------------------+---------------------------------------+-----------------+
|``Database``          | ``synonyms(term)``                       | ``synonyms_begin(term)``              | ``TermIter``    |
+----------------------+------------------------------------------+---------------------------------------+-----------------+
|``Database``          | ``synonym_keys(prefix)``                 | ``synonym_keys_begin(prefix)``        | ``TermIter``    |
+----------------------+------------------------------------------+---------------------------------------+-----------------+
|``Document``          | ``values()``                             | ``values_begin()``                    | ``ValueIter``   |
+----------------------+------------------------------------------+---------------------------------------+-----------------+
|``Document``          | ``termlist()`` (also as default iterator)| ``termlist_begin()``                  | ``TermIter``    |
+----------------------+------------------------------------------+---------------------------------------+-----------------+
|``QueryParser``       | ``stoplist()``                           | ``stoplist_begin()``                  | ``TermIter``    |
+----------------------+------------------------------------------+---------------------------------------+-----------------+
|``QueryParser``       | ``unstemlist(tname)``                    | ``unstem_begin(tname)``               | ``TermIter``    |
+----------------------+------------------------------------------+---------------------------------------+-----------------+
|``ValueCountMatchSpy``|  ``values()``                            | ``values_begin()``                    | ``TermIter``    |
+----------------------+------------------------------------------+---------------------------------------+-----------------+
|``ValueCountMatchSpy``|  ``top_values()``                        | ``top_values_begin()``                | ``TermIter``    |
+----------------------+------------------------------------------+---------------------------------------+-----------------+


The pythonic iterators generally return Python objects, with properties
available as attribute values, with lazy evaluation where appropriate.  An
exception is the ``PositionIter`` object returned by
``Database.positionlist``, which returns an integer.

The lazy evaluation is mainly transparent, but does become visible in one situation: if you keep an object returned by an iterator, without evaluating its properties to force the lazy evaluation to happen, and then move the iterator forward, the object may no longer be able to efficiently perform the lazy evaluation.  In this situation, an exception will be raised indicating that the information requested wasn't available.  This will only happen for a few of the properties - most are either not evaluated lazily (because the underlying Xapian implementation doesn't evaluate them lazily, so there's no advantage in lazy evaluation), or can be accessed even after the iterator has moved.  The simplest work around is simply to evaluate any properties you wish to use which are affected by this before moving the iterator.  The complete set of iterator properties affected by this is:


- Database.allterms (also accessible as Database.__iter__): **termfreq**
- Database.termlist: **termfreq** and **positer**
- Document.termlist (also accessible as Document.__iter__): **termfreq** and **positer**
- Database.postlist: **positer**

In older releases, the pythonic iterators returned lists representing the
appropriate item when their ``next()`` method was called.  These were
removed in Xapian 1.1.0.


Non-Pythonic Iterators
######################

Before the pythonic iterator wrappers were added, the python bindings provided
thin wrappers around the C++ iterators.  However, these iterators don't behave
like most iterators do in Python, so the pythonic iterators were implemented to
replace them.  The non-pythonic iterators were removed in Xapian 1.3.0 -
the documentation below is provided to aid migration away from them.

All non-pythonic iterators support ``next()`` and
``equals()`` methods
to move through and test iterators (as for all language bindings).
MSetIterator and ESetIterator also support ``prev()``.
Python-wrapped iterators also support direct comparison, so something like:

::

  m=mset.begin()
  while m!=mset.end():
    # do something
    m.next()

C++ iterators are often dereferenced to get information, eg
``(*it)``. With Python these are all mapped to named methods, as
follows:

+------------------+----------------------+
| Iterator         | Dereferencing method |
+==================+======================+
| PositionIterator |    ``get_termpos()`` |
+------------------+----------------------+
| PostingIterator  |  ``get_docid()``     |
+------------------+----------------------+
| TermIterator     |     ``get_term()``   |
+------------------+----------------------+
| ValueIterator    |     ``get_value()``  |
+------------------+----------------------+
| MSetIterator     |     ``get_docid()``  |
+------------------+----------------------+
| ESetIterator     |     ``get_term()``   |
+------------------+----------------------+


Other methods, such as ``MSetIterator.get_document()``, are
available unchanged.

MSet
####

MSet objects have some additional methods to simplify access (these
work using the C++ array dereferencing):

+-----------------------------------+----------------------------------------+
| Method name                       |            Explanation                 |
+===================================+========================================+
| ``get_hit(index)``                |  returns MSetItem at index             |
+-----------------------------------+----------------------------------------+
|``get_document_percentage(index)`` | ``convert_to_percent(get_hit(index))`` |
+-----------------------------------+----------------------------------------+
| ``get_document(index)``           | ``get_hit(index).get_document()``      |
+-----------------------------------+----------------------------------------+
| ``get_docid(index)``              | ``get_hit(index).get_docid()``         |
+-----------------------------------+----------------------------------------+

Additionally, the MSet has a property, ``mset.items``, which returns a
list of tuples representing the MSet.  This is now deprecated - please use the
property API instead (it works in Xapian 1.0.x too).  The tuple members and the
equivalent property names are as follows:


+-------------------------+---------------+---------------------------------------------------------------------------+
|   Index                 | Property name | Contents                                                                  |
+=========================+===============+===========================================================================+
| ``xapian.MSET_DID``     | docid         | Document id                                                               |
+-------------------------+---------------+---------------------------------------------------------------------------+
| ``xapian.MSET_WT``      | weight        |  Weight                                                                   |
+-------------------------+---------------+---------------------------------------------------------------------------+
| ``xapian.MSET_RANK``    | rank          | Rank                                                                      |
+-------------------------+---------------+---------------------------------------------------------------------------+
| ``xapian.MSET_PERCENT`` |  percent      | Percentage weight                                                         |
+-------------------------+---------------+---------------------------------------------------------------------------+
| ``xapian.MSET_DOCUMENT``| document      | Document object (Note: this member of the tuple was never actually set!)  |
+-------------------------+---------------+---------------------------------------------------------------------------+


Two MSet objects are equal if they have the same number and maximum possible
number of members, and if every document member of the first MSet exists at the
same index in the second MSet, with the same weight.


ESet
####

The ESet has a property, ``eset.items``, which returns a list of
tuples representing the ESet.  This is now deprecated - please use the
property API instead (it works in Xapian 1.0.x too).  The tuple members and the
equivalent property names are as follows:


+------------------------+---------------+-----------+
|   Index                | Property name | Contents  |
+========================+===============+===========+
| ``xapian.ESET_TNAME``  | term          | Term name |
+------------------------+---------------+-----------+
| ``xapian.ESET_WT``     | weight        |  Weight   |
+------------------------+---------------+-----------+


Non-Class Functions
###################

The C++ API contains a few non-class functions (the Database factory
functions, and some functions reporting version information), which are
wrapped like so for Python:

- ``Xapian::version_string()`` is wrapped as ``xapian.version_string()``
- ``Xapian::major_version()`` is wrapped as ``xapian.major_version()``
- ``Xapian::minor_version()`` is wrapped as ``xapian.minor_version()``
- ``Xapian::revision()`` is wrapped as ``xapian.revision()``
- ``Xapian::Auto::open_stub()`` is wrapped as ``xapian.open_stub()`` (now deprecated)
- ``Xapian::Brass::open()`` is wrapped as ``xapian.brass_open()`` (now deprecated)
- ``Xapian::Chert::open()`` is wrapped as ``xapian.chert_open()`` (now deprecated)
- ``Xapian::InMemory::open()`` is wrapped as ``xapian.inmemory_open()``
- ``Xapian::Remote::open()`` is wrapped as ``xapian.remote_open()`` (both the TCP and "program" versions are wrapped - the SWIG wrapper checks the parameter list to decide which to call).
- ``Xapian::Remote::open_writable()`` is wrapped as ``xapian.remote_open_writable()`` (both the TCP and "program" versions are wrapped - the SWIG wrapper checks the parameter list to decide which to call).


Query
#####

In C++ there's a Xapian::Query constructor which takes a query operator and
start/end iterators specifying a number of terms or queries, plus an optional
parameter.  In Python, this is wrapped to accept any Python sequence (for
example a list or tuple) to give the terms/queries, and you can specify
a mixture of terms and queries if you wish.  For example:


::

  subq = xapian.Query(xapian.Query.OP_AND, "hello", "world")
  q = xapian.Query(xapian.Query.OP_AND, [subq, "foo", xapian.Query("bar", 2)])


MatchAll and MatchNothing
-------------------------

As of 1.1.1, these are wrapped as ``xapian.Query.MatchAll`` and
``xapian.Query.MatchNothing``.


MatchDecider
############

Custom MatchDeciders can be created in Python; simply subclass
xapian.MatchDecider, ensure you call the super-constructor, and define a
__call__ method that will do the work. The simplest example (which does nothing
useful) would be as follows:

::

  class mymatchdecider(xapian.MatchDecider):
    def __init__(self):
      xapian.MatchDecider.__init__(self)

    def __call__(self, doc):
      return 1

ValueRangeProcessors
####################

The ValueRangeProcessor class (and its subclasses) provide an operator() method
(which is exposed in python as a __call__() method, making the class instances
into callables).  This method checks whether a beginning and end of a range are
in a format understood by the ValueRangeProcessor, and if so, converts the
beginning and end into strings which sort appropriately.  ValueRangeProcessors
can be defined in python (and then passed to the QueryParser), or there are
several default built-in ones which can be used.

Unfortunately, in C++ the operator() method takes two std::string arguments by
reference, and returns values by modifying these arguments.  This is not
possible in Python, since strings are immutable objects.  Instead, in the
Python implementation, when the __call__ method is called, the resulting values
of these arguments are returned as part of a tuple.  The operator() method in
C++ returns a value number; the return value of __call__ in python consists of
a 3-tuple starting with this value number, followed by the returned "begin"
value, followed by the returned "end" value.  For example:

::

  vrp = xapian.NumberValueRangeProcessor(0, '$', True)
  a = '$10'
  b = '20'
  slot, a, b = vrp(a, b)

Additionally, a ValueRangeProcessor may be implemented in Python.  The Python
implementation should override the __call__() method with its own
implementation, and, again, since it cannot return values by reference, it
should return a tuple of (value number, begin, end).  For example:

::

  class MyVRP(xapian.ValueRangeProcessor):
      def __init__(self):
          xapian.ValueRangeProcessor.__init__(self)
      def __call__(self, begin, end):
          return (7, "A"+begin, "B"+end)


Apache and mod_python/mod_wsgi
##############################

Prior to Xapian 1.3.0, the you had to tell mod_python and mod_wsgi to run
applications which use Xapian in the main interpreter.  Xapian 1.3.0 no
longer uses the simplified GIL state API, and so this restriction should
no longer apply.
