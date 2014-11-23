Lua bindings for Xapian
***********************

These bindings require Lua version 5.1 or later, and have been tested with Lua 5.1 and 5.2.

The Lua bindings for Xapian are packaged in the ``xapian`` namespace,
and largely follow the C++ API, with the following differences and
additions.

The ``examples`` subdirectory contains examples showing how to use the
Lua bindings based on the simple examples from ``xapian-examples``:
`simpleindex.lua <examples/simpleindex.lua>`_,
`simplesearch.lua <examples/simplesearch.lua>`_,
`simpleexpand.lua <examples/simpleexpand.lua>`_.

There's also
`simplematchdecider.lua <examples/simplematchdecider.lua>`_
which shows how to define a MatchDecider in Lua.

Unicode Support
###############

In Xapian 1.0.0 and later, the Xapian::Stem, Xapian::QueryParser, and
Xapian::TermGenerator classes all assume text is in UTF-8.  A Lua string
is an arbitrary sequence of values which have at least 8 bits (octets);
they map directly into the char type of the C compiler. Lua does not
reserve any value, including NUL. That means that Lua can store a UTF-8
string without problems.

Method names
############

Most methods are named the same as in the C++ API - the exceptions are:

``end`` is a keyword in Lua, so such methods are renamed to
``_end`` - e.g. in Lua you'd use ``mset:_end()`` to get an
end iterator for an MSet object called mset.
The C++ method ``get_description()`` is mapped to the
``str`` function in Lua, so ``str(x)`` will return a string
describing object x.

Exceptions
##########

Exceptions thrown by Xapian are translated into Lua xapian.Error objects
and raised as Lua errors, which you can catch by using ``pcall``
like so:

::

   ok,res = pcall(db.get_document, db, docid)
   if ok then
      print("Got document data: " .. res:get_data())
   else
      print("Got exception: " .. tostring(res))
   end

Iterators
#########


All iterators support ``next`` and ``equals`` methods
to move through and test iterators (as for all language bindings).
MSetIterator and ESetIterator also support ``prev``. As "end" is
a keyword in Lua, we rename it to "_end" that means the end of the iterator.
The following shows an example of iterating the MSet to get the rank,
document id, and data for each entry in the MSet:

::

   for m in mset:items() do
      print(m:get_rank() + 1, m:get_docid(), m:get_document():get_data())
   end

Iterator dereferencing
######################

C++ iterators are often dereferenced to get information, eg
``(*it)``. With Lua these are all mapped to named methods, as
follows:

+------------------+----------------------+
| Iterator         | Dereferencing method |
+==================+======================+
| PositionIterator |     ``get_termpos``  |
+------------------+----------------------+
| PostingIterator  |     ``get_docid``    |
+------------------+----------------------+
| TermIterator     |     ``get_term``     |
+------------------+----------------------+
| ValueIterator    |     ``get_value``    |
+------------------+----------------------+
| MSetIterator     |     ``get_docid``    |
+------------------+----------------------+
| ESetIterator     |     ``get_term()``   |
+------------------+----------------------+

Other methods, such as ``MSetIterator:get_document``, are
available under the same names.

MSet
####

MSet objects have some additional methods to simplify access (these
work using the C++ array dereferencing):

+-----------------------------------+----------------------------------------+
| Method name                       |            Explanation                 |
+===================================+========================================+
| ``get_hit(index)``                |  returns MSetItem at index             |
+-----------------------------------+----------------------------------------+
| ``get_documentPercentage(index)`` | ``convert_to_percent(get_hit(index))`` |
+-----------------------------------+----------------------------------------+
| ``get_document(index)``           | ``get_hit(index):get_document()``      |
+-----------------------------------+----------------------------------------+
| ``get_docid(index)``              | ``get_hit(index):get_docid()``         |
+-----------------------------------+----------------------------------------+

The C++ API contains a few non-class functions (the Database factory
functions, and some functions reporting version information), which are
wrapped like so for Lua:


-  ``Xapian::version_string()`` is wrapped as ``xapian.version_string()``
-  ``Xapian::major_version()`` is wrapped as ``xapian.major_version()``
-  ``Xapian::minor_version()`` is wrapped as ``xapian.minor_version()``
-  ``Xapian::revision()`` is wrapped as ``xapian.revision()``
-  ``Xapian::Auto::open_stub()`` is wrapped as ``xapian.open_stub()`` (but is now deprecated)
-  ``Xapian::Chert::open()`` is wrapped as ``xapian.chert_open()`` (but is now deprecated)
-  ``Xapian::InMemory::open()`` is wrapped as ``xapian.inmemory_open()``
-  ``Xapian::Remote::open()`` is wrapped as ``xapian.remote_open()`` (both the TCP and "program" versions are wrapped - the SWIG wrapper checks the parameter list to decide which to call).
-  ``Xapian::Remote::open_writable()`` is wrapped as ``xapian.remote_open_writable()`` (both the TCP and "program" versions are wrapped - the SWIG wrapper checks the parameter list to decide which to call).


Constants
#########

For Lua, constants are wrapped as ``xapian.CONSTANT_NAME``
or ``xapian.ClassName_CONSTANT_NAME``.
So ``Xapian::DB_CREATE_OR_OPEN`` is available as
``xapian.DB_CREATE_OR_OPEN``, ``Xapian::Query::OP_OR`` is
available as ``xapian.Query_OP_OR``, and so on.

As of 1.3.2, you can also use the form ``xapian.ClassName.CONSTANT_NAME``, e.g.
``xapian.Query.OP_OR``.

Query
#####

In C++ there's a Xapian::Query constructor which takes a query operator and
start/end iterators specifying a number of terms or queries, plus an optional
parameter. In Lua, it is wrapped to accept Lua tables to give the terms/queries,
and you can specify a mixture of terms and queries if you wish.  For example:

::

   subq = xapian.Query(xapian.Query_OP_AND, {"hello", "world"})
   q = xapian.Query(xapian.Query_OP_AND, {subq, "foo", xapian.Query("bar", 2)})

MatchAll and MatchNothing
#########################

These are wrapped for Lua as ``xapian.Query_MatchAll`` and
``xapian.Query_MatchNothing``.

As of 1.3.2, you can also use the forms ``xapian.Query.MatchAll`` and
``xapian.Query.MatchNothing``.

Enquire
#######

There is an additional method ``get_matching_terms`` which takes
an MSetIterator and returns a list of terms in the current query which
match the document given by that iterator.  You may find this
more convenient than using the TermIterator directly.

MatchDecider
############

Custom MatchDeciders can be created in Lua in the form of lua function; simply
function ensures you create a subclass of xapian.MatchDecider, which calls
the super-constructor, and overloads the operator method to callback the lua function
that will do the work. The simplest example (which does nothing
useful) would be as follows:

::

   function mymatchdecider(doc)
      return 1
   end
