Tcl8 bindings for Xapian
************************

The Tcl8 bindings for Xapian are packaged in the ``xapian`` namespace,
and largely follow the C++ API, with the following differences and
additions. Tcl8 strings and lists, etc., are converted automatically
in the bindings, so generally it should just work as expected.

The ``examples`` subdirectory contains examples showing how to use the
Tcl8 bindings based on the simple examples from ``xapian-examples``:
`simpleindex.tcl <examples/simpleindex.tcl>`_,
`simplesearch.tcl <examples/simplesearch.tcl>`_,
`simpleexpand.tcl <examples/simpleexpand.tcl>`_.

Unicode Support
###############

In Xapian 1.0.0 and later, the Xapian::Stem, Xapian::QueryParser, and
Xapian::TermGenerator classes all assume text is in UTF-8.  Tcl8 uses
UTF-8 as its internal representation, except that ASCII nul (character value
0) is represented as the overlong (and thus invalid) UTF-8 sequence
``\xc0\x80``.  We don't currently convert this to/from
``\x00`` so you should avoid passing strings containing ASCII nul
between Tcl and Xapian.


Destructors
###########

To destroy an object ``obj``, you need to use one of
``obj -delete`` or ``rename obj ""``
(either should work, but see below).

SWIG's Tcl wrapping doesn't handle an object returned by a factory function
correctly.  This only matters for the Xapian::WritableDatabase class, and we
avoid wrapping the problematic factory functions to avoid setting a
trap for the unwary - these are the WritableDatabase versions of
``Xapian::Brass::open`` and ``Xapian::Chert::open``.
You can just use a ``Xapian::WritableDatabase`` constructor
instead (and to specify brass rather than chert, pass
``$::xapian::DB_BACKEND_BRASS`` in the flags, or set
``XAPIAN_PREFER_BRASS`` in the environment).

As of Xapian 1.1.0, you can explicitly close the database, so the lack
of a call to the destructor isn't an issue:

::

  xapian::WritableDatabase xapiandb testdir $::xapian::DB_CREATE_OR_OVERWRITE
  xapian_db close

If you want compatibility with Xapian 1.0.x, then
Michael Schlenker reports that this form works (i.e. the destructor gets
called):

::

  xapian::WritableDatabase xapiandb testdir $::xapian::DB_CREATE_OR_OVERWRITE
  rename xapiandb ""

However, apparently none of these forms work:

::

  xapian::WritableDatabase xapiandb testdir $::xapian::DB_CREATE_OR_OVERWRITE
  set db xapiandb
  $db -delete

  set db [xapian::WritableDatabase xapiandb testdir $::xapian::DB_CREATE_OR_OVERWRITE]
  $db -delete

  set db [xapian::WritableDatabase xapiandb testdir $::xapian::DB_CREATE_OR_OVERWRITE]
  rename $db ""


Exceptions
##########

Xapian::Error exceptions can be handled in Tcl like so:

::

  if [catch {
      # Code which might throw an exception.
    } msg] {
      # Code to handle exceptions.
      # $errorCode is "XAPIAN <error_class>" (e.g. "XAPIAN DocNotFoundError".)
      # $msg is the result of calling get_msg() on the Xapian::Error object.
  }


Iterators
#########

All iterators support ``next`` and ``equals`` methods
to move through and test iterators (as for all language bindings).
MSetIterator and ESetIterator also support ``prev``.

Iterator dereferencing
######################

C++ iterators are often dereferenced to get information, eg
``(*it)``. With Tcl8 these are all mapped to named methods, as
follows:

.. table:: Iterator deferencing methods

  +------------------+----------------------+
  | Iterator         | Dereferencing method |
  +==================+======================+
  | PositionIterator |    ``get_termpos``   |
  +------------------+----------------------+
  | PostingIterator  |  ``get_docid``       |
  +------------------+----------------------+
  | TermIterator     |     ``get_term``     |
  +------------------+----------------------+
  | ValueIterator    |     ``get_value``    |
  +------------------+----------------------+
  | MSetIterator     |     ``get_docid``    |
  +------------------+----------------------+
  | ESetIterator     |     ``get_term``     |
  +------------------+----------------------+

Other methods, such as ``MSetIterator::get_document``, are
available under the same names.


MSet
####

MSet objects have some additional methods to simplify access (these
work using the C++ array dereferencing):

.. table:: MSet additional methods

  +---------------------------------------+--------------------------------------------------+
  | Method name                           |            Explanation                           |
  +=======================================+==================================================+
  | ``mset get_hit index``                |   returns MSetIterator at index                  |
  +---------------------------------------+--------------------------------------------------+
  | ``mset get_document_percentage index``| ``mset convert_to_percent [mset get_hit index]`` |
  +---------------------------------------+--------------------------------------------------+
  | ``mset get_document index``           | ``[mset get_hit index] get_document``            |
  +---------------------------------------+--------------------------------------------------+
  | ``mset get_docid index``              | ``[mset get_hit index] get_docid``               |
  +---------------------------------------+--------------------------------------------------+


Non-Class Functions
###################

The C++ API contains a few non-class functions (the Database factory
functions, and some functions reporting version information), which are
wrapped like so for Tcl:

- ``Xapian::version_string()`` is wrapped as ``xapian::version_string``
- ``Xapian::major_version()`` is wrapped as ``xapian::major_version``
- ``Xapian::minor_version()`` is wrapped as ``xapian::minor_version``
- ``Xapian::revision()`` is wrapped as ``xapian::revision``
- ``Xapian::Auto::open_stub()`` is wrapped as ``xapian::open_stub``
- ``Xapian::Brass::open()`` is wrapped as ``xapian::brass_open`` (but note that the WritableDatabase version isn't wrapped - see the 'Destructors' section above for an explanation - and this function is deprecated anyway).
- ``Xapian::Chert::open()`` is wrapped as ``xapian::chert_open`` (but note that the WritableDatabase version isn't wrapped - see the 'Destructors' section above for an explanation - and this function is deprecated anyway).
- ``Xapian::InMemory::open()`` is wrapped as ``xapian::inmemory_open``
- ``Xapian::Remote::open()`` is wrapped as ``xapian::remote_open`` (both the TCP and "program" versions are wrapped - the SWIG wrapper checks the parameter list to decide which to call).
- ``Xapian::Remote::open_writable()`` is wrapped as ``xapian::remote_open_writable`` (both the TCP and "program" versions are wrapped - the SWIG wrapper checks the parameter list to decide which to call).


Constants
#########

For Tcl, constants are wrapped as ``$xapian::CONSTANT_NAME``
or ``$xapian::ClassName_CONSTANT_NAME``.
So ``Xapian::DB_CREATE_OR_OPEN`` is available as
``$xapian::DB_CREATE_OR_OPEN``, ``Xapian::Query::OP_OR`` is
available as ``$xapian::Query_OP_OR``, and so on.

Query
#####

In C++ there's a Xapian::Query constructor which takes a query operator and
start/end iterators specifying a number of terms or queries, plus an optional
parameter.  In Tcl, this is wrapped to accept a Tcl list
to give the terms/queries, and you can specify
a mixture of terms and queries if you wish.  For example:


::

  set terms [list "hello" "world"]
  xapian::Query subq $xapian::Query_OP_AND $terms
  xapian::Query bar_term "bar" 2
  xapian::Query query $xapian::Query_OP_AND [list subq "foo" bar_term]


MatchAll and MatchNothing
-------------------------

As of Xapian 1.1.1, these are wrapped for Tcl as
``$xapian::Query_MatchAll`` and
``$xapian::Query_MatchNothing``.

Enquire
#######

There is an additional method ``get_matching_terms`` which takes
an MSetIterator and returns a list of terms in the current query which
match the document given by that iterator.  You may find this
more convenient than using the TermIterator directly.

*Last updated $Date$*
