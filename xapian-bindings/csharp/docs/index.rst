**********************
C# bindings for Xapian
**********************

The C# bindings for Xapian are packaged in the ``Xapian`` namespace
and largely follow the C++ API, with the following differences and
additions.  C# strings and other types are converted automatically
in the bindings, so generally it should just work as expected.

The ``examples`` subdirectory contains examples showing how to use the
C# bindings based on the simple examples from ``xapian-examples``:
`SimpleIndex.cs <examples/SimpleIndex.cs>`_,
`SimpleSearch.cs <examples/SimpleSearch.cs>`_,
`SimpleExpand.cs <examples/SimpleExpand.cs>`_.

Note: the passing of strings from C# into Xapian and back isn't currently zero byte safe.  If you try to handle string containing zero bytes, you'll find they get truncated at the zero byte.


Unicode Support
###############


In Xapian 1.0.0 and later, the Xapian::Stem, Xapian::QueryParser, and
Xapian::TermGenerator classes all assume text is in UTF-8.  If you're
using Mono on UNIX with a UTF-8 locale (which is the default on most
modern Linux distributions), then Xapian appears to get passed Unicode
strings as UTF-8, so it should just work.  We tested with Mono 2.6.7
using the Mono C# 2.0 compiler (gmcs).

However, Microsoft and Mono's C# implementations apparently take
rather different approaches to Unicode, and we've not tested with
Microsoft's implementation.  If you try it, please report how well
it works (or how badly it fails...)


Method Naming Conventions
#########################

Methods are renamed to use the "CamelCase" capitalisation convention which C# normally uses.  So in C# you use ``GetDescription`` instead of
``get_description``.


Exceptions
##########

Exceptions are thrown as SWIG exceptions instead of Xapian
exceptions. This isn't done well at the moment; in future we will
throw wrapped Xapian exceptions. For now, it's probably easier to
catch all exceptions and try to take appropriate action based on
their associated string.


Iterators
#########

The C#-wrapped iterators work much like their C++ counterparts, with
operators "++", "--", "==", and "!=" overloaded.  E.g.:

::

   Xapian.MSetIterator m = mset.begin();
   while (m != mset.end()) {
     // do something
     ++m;
   }


Iterator dereferencing
######################

C++ iterators are often dereferenced to get information, eg
``(*it)``.  In C# these are all mapped to named methods, as
follows:

+------------------+----------------------+
| Iterator         | Dereferencing method |
+==================+======================+
| PositionIterator |     ``GetTermPos()`` |
+------------------+----------------------+
| PostingIterator  |     ``GetDocId()``	  |
+------------------+----------------------+
| TermIterator     |     ``GetTerm()``    |
+------------------+----------------------+
| ValueIterator    |     ``GetValue()``   |
+------------------+----------------------+
| MSetIterator     |     ``GetDocId()``   |
+------------------+----------------------+
| ESetIterator     |     ``GetTerm()``    |
+------------------+----------------------+


Other methods, such as ``MSetIterator.GetDocument()``, are available unchanged.


MSet
####

MSet objects have some additional methods to simplify access (these
work using the C++ array dereferencing):

+---------------------------------+-------------------------------------+
| Method name                     |            Explanation              |
+=================================+=====================================+
| ``GetHit(index)``               |  returns MSetIterator at index      |
+---------------------------------+-------------------------------------+
|``GetDocumentPercentage(index)`` | ``ConvertToPercent(GetHit(index))`` |
+---------------------------------+-------------------------------------+
| ``GetDocument(index)``          | ``GetHit(index).GetDocument()``     |
+---------------------------------+-------------------------------------+
| ``GetDocumentId(index)``        | ``GetHit(index).GetDocId()``        |
+---------------------------------+-------------------------------------+


Non-Class Functions
###################

The C++ API contains a few non-class functions (the Database factory
functions, and some functions reporting version information), but C# doesn't
allow functions which aren't in a class so these are wrapped as static
member functions of abstract classes like so:

- ``Xapian::version_string()`` is wrapped as ``Xapian.Version.String()``
- ``Xapian::major_version()`` is wrapped as ``Xapian.Version.Major()``
- ``Xapian::minor_version()`` is wrapped as ``Xapian.Version.Minor()``
- ``Xapian::revision()`` is wrapped as ``Xapian.Version.Revision()``
- ``Xapian::Auto::open_stub()`` is wrapped as ``Xapian.Auto.OpenStub()`` (but is now deprecated)
- ``Xapian::Brass::open()`` is wrapped as ``Xapian.Brass.Open()`` (but is now deprecated)
- ``Xapian::Chert::open()`` is wrapped as ``Xapian.Chert.Open()`` (but is now deprecated)
- ``Xapian::InMemory::open()`` is wrapped as ``Xapian.InMemory.Open()``
- ``Xapian::Remote::open()`` is wrapped as ``Xapian.Remote.Open()`` (both the TCP and "program" versions are wrapped - the SWIG wrapper checks the parameter list to decide which to call).
- ``Xapian::Remote::open_writable()`` is wrapped as ``Xapian.Remote.OpenWritable()`` (both the TCP and "program" versions are wrapped - the SWIG wrapper checks the parameter list to decide which to call).



Constants
#########

The ``Xapian::DB_*`` constants are currently wrapped in a Xapian
class within the Xapian namespace, so have a double Xapian prefix!
So ``Xapian::DB_CREATE_OR_OPEN`` is available as
``Xapian.Xapian.DB_CREATE_OR_OPEN``.
The ``Query::OP_*`` constants are wrapped a little oddly too:
``Query::OP_OR`` is wrapped as ``Xapian.Query.op.OP_OR``.
Similarly, ``QueryParser::STEM_SOME`` as
``Xapian.QueryParser.stem_strategy.STEM_SOME``.
The naming here needs sorting out...


Query
#####

In C++ there's a Xapian::Query constructor which takes a query operator and
start/end iterators specifying a number of terms or queries, plus an optional
parameter.
This isn't currently wrapped in C#.

.. FIXME implement this wrapping!
..    In C#, this is wrapped to accept any C# sequence (for
..    example a list or tuple) to give the terms/queries, and you can specify
..    a mixture of terms and queries if you wish.  For example:
..        subq = xapian.Query(xapian.Query.OP_AND, "hello", "world")
..        q = xapian.Query(xapian.Query.OP_AND, [subq, "foo", xapian.Query("bar", 2)])


MatchAll and MatchNothing
-------------------------

In Xapian 1.3.0 and later, these are wrapped as static constants
``xapian.Query.MatchAll`` and ``xapian.Query.MatchNothing``.

If you want to be compatible with earlier versions, you can continue to use
``new xapian.Query("")`` instead of ``xapian.Query.MatchAll``
and ``new xapian.Query()`` instead of
``xapian.Query.MatchNothing``.

.. FIXME: Need to define the custom output typemap to handle this if it
..       actually seems useful...
..       -------
..       Enquire
..       -------
..		 There is an additional method `GetMatchingTerms()` which takes
..		 an MSetIterator and returns a list of terms in the current query which
..		 match the document given by that iterator.  You may find this
..		 more convenient than using the TermIterator directly.


MatchDecider
############

Custom MatchDeciders can be created in C#; simply subclass
Xapian.MatchDecider, and define an
Apply method that will do the work. The simplest example (which does nothing
useful) would be as follows:

::

	class MyMatchDecider : Xapian.MatchDecider {
	    public override bool Apply(Xapian.Document doc) {
		return true;
	    }
	}
