PHP7 bindings for Xapian
************************

The PHP7 bindings for Xapian are packaged in the ``xapian``
extension.  The PHP API provided by this extension largely follows Xapian's C++
API.  This document lists the differences and additions.

PHP strings, arrays, etc., are converted automatically to and from the
corresponding C++ types in the bindings, so generally you can pass arguments as
you would expect.  One thing to be aware of though is that SWIG implements
dispatch functions for overloaded methods based on the types of the parameters,
so you can't always pass in a string containing a number (e.g.
``"42"``) where a number is expected as you usually can in PHP.
You need to
explicitly convert to the type required - e.g. use ``(int)`` to
convert to an integer, ``(string)`` to string, ``(double)``
to a floating point number.

You can subclass Xapian classes in PHP and virtual methods defined in PHP are
called from C++ in the way you'd expect.

PHP has a lot of reserved words of various sorts, which sadly clash with common
method names.  Because of this ``empty()`` methods of various
container-like classes are wrapped as ``is_empty()`` for PHP.

The ``examples`` subdirectory contains examples showing how to use the
PHP bindings based on the simple examples from ``xapian-examples``:
`simpleindex.php7 <examples/simpleindex.php7>`_,
`simplesearch.php7 <examples/simplesearch.php7>`_,
`simpleexpand.php7 <examples/simpleexpand.php7>`_,
`simplematchdecider.php7 <examples/simplematchdecider.php7>`_.

Note that these examples are written to work with the command line (CLI)
version of the PHP interpreter, not through a webserver.  Xapian's PHP
bindings may of course also be used under CGI, Apache's modphp, ISAPI,
etc.

Installation
============

Assuming you have a suitable version of PHP7 installed, running
configure will automatically enable the PHP7 bindings, and
``make install`` will install the extension shared library in
the location reported by ``php-config --extension-dir``.

Check that php.ini has a line like ``extension_dir = "<location reported by php-config --extension-dir>"``.


Then add this line to php.ini: ``extension = xapian.so`` (or
whatever the library is called - not all UNIX systems use ``.so``
as the extension, and MS Windows uses ``.dll``).

If you're using PHP as a webserver module (e.g. mod_php with Apache), you
may need to restart the webserver for this change to take effect.

You also need to add ``include&nbsp;"xapian.php"``
to your PHP scripts which use Xapian in order to get the PHP class wrappers.

Exceptions
##########

Exceptions thrown by Xapian are translated into PHP Exception objects
which are thrown into the PHP script.

Object orientation
##################

These PHP bindings use a PHP object oriented style.

To construct an object, use
``$object = new XapianClassName(...);``.  Objects are destroyed
when they go out of scope - to explicitly destroy an object you can use
``unset($object);`` or ``$object = Null;``

You invoke a method on an object using ``$object->method_name()``.

Unicode Support
###############

The Xapian::Stem, Xapian::QueryParser, and
Xapian::TermGenerator classes all assume text is in UTF-8.  If you want
to index strings in a different encoding, use the PHP `iconv function <https://secure.php.net/iconv>`_ to convert them to UTF-8 before passing them to Xapian, and when reading values back from Xapian.

Iterators
#########

Xapian's iterators (except ``XapianLatLongCoordsIterator``)
are wrapped as PHP iterators, so can be used in ``foreach``.

There's one important thing to beware of currently - the ``rewind()`` method
on ``XapianPositionIterator``, ``XapianPostingIterator``,
``XapianTermIterator`` and ``XapianValueIterator`` currently does nothing.  We
can't make it simply throw an exception, as ``foreach`` calls ``rewind()``
before iteration starts - each iterator needs to track if ``next()`` has been
called yet, and we've not yet implemented machinery for that.  This doesn't
affect the standard pattern of iterating once with ``foreach``, but if you want
to iterate a second time, you can't reuse the iterator (but it will currently
fail quietly).

You can safely call ``rewind()`` on ``XapianESetIterator`` and
``XapianMSetIterator``.

The ``current()`` method returns the result of dereferencing the iterator
in C++ (e.g. for a ``TermIterator``, it returns the term as a string - see
the section below for more details) and the ``key()`` method returns the
iterator object, which you can call other methods on, for example::

    foreach ($db->allterms_begin() as $k => $term) {
	print "{$k->get_termfreq()}\t$term\n";
    }

As well as the standard PHP iterator methods, MSetIterator and ESetIterator
also support ``prev()`` to go back one place.

Iterator dereferencing
######################

C++ iterators are often dereferenced to get information, eg
``(*it)``. With PHP these are all mapped to named methods, as
follows:

+------------------+----------------------+
| Iterator         | Dereferencing method |
+==================+======================+
| PositionIterator |   ``get_termpos()``  |
+------------------+----------------------+
| PostingIterator  |   ``get_docid()``    |
+------------------+----------------------+
| TermIterator     |   ``get_term()``     |
+------------------+----------------------+
| ValueIterator    |   ``get_value()``    |
+------------------+----------------------+
| MSetIterator     |   ``get_docid()``    |
+------------------+----------------------+
| ESetIterator     |   ``get_term()``     |
+------------------+----------------------+

Other methods, such as ``MSetIterator::get_document()``, are
available unchanged.

MSet
####

MSet objects have some additional methods to simplify access (these
work using the C++ array dereferencing):

+------------------------------------+----------------------------------------+
| Method name                        |            Explanation                 |
+====================================+========================================+
| ``get_hit(index)``                 |   returns MSetIterator at index        |
+------------------------------------+----------------------------------------+
| ``get_document_percentage(index)`` | ``convert_to_percent(get_hit(index))`` |
+------------------------------------+----------------------------------------+
| ``get_document(index)``            | ``get_hit(index)->get_document()``     |
+------------------------------------+----------------------------------------+
| ``get_docid(index)``               | ``get_hit(index)->get_docid()``        |
+------------------------------------+----------------------------------------+


Database Factory Functions
##########################

- ``Xapian::Remote::open(...)`` is wrapped as ``Xapian::remote_open(...)`` (both the TCP and "program" versions are wrapped - the SWIG wrapper checks the parameter list to decide which to call).
- ``Xapian::Remote::open_writable(...)`` is wrapped as ``Xapian::remote_open_writable(...)`` (both the TCP and "program" versions are wrapped - the SWIG wrapper checks the parameter list to decide which to call).

Constants
#########

Constants are wrapped as ``const`` members of the appropriate class.
So ``Xapian::DB_CREATE_OR_OPEN`` is available as
``Xapian::DB_CREATE_OR_OPEN``, ``Xapian::Query::OP_OR`` is
available as ``XapianQuery::OP_OR``, and so on.

Functions
#########

Non-class functions are wrapped in the natural way, so the C++
function ``Xapian::version_string`` is wrapped under the same
name in PHP.

Query
#####

In C++ there's a Xapian::Query constructor which takes a query operator and
start/end iterators specifying a number of terms or queries, plus an optional
parameter.  In PHP, this is wrapped to accept an array listing the terms
and/or queries (you can specify a mixture of terms and queries if you wish)
For example:

::

   $subq = new XapianQuery(XapianQuery::OP_AND, "hello", "world");
   $q = new XapianQuery(XapianQuery::OP_AND, array($subq, "foo", new XapianQuery("bar", 2)));



MatchAll and MatchNothing
-------------------------

These are wrapped as static methods
``XapianQuery::MatchAll()`` and ``XapianQuery::MatchNothing()``.

If you want to be compatible with version 1.2.x of Xapian's PHP5 bindings, you
can continue to use ``new XapianQuery('')`` for MatchAll and
``new XapianQuery()`` for MatchNothing.


Enquire
#######

There is an additional method ``get_matching_terms()`` which takes
an MSetIterator and returns a list of terms in the current query which
match the document given by that iterator.  You may find this
more convenient than using the TermIterator directly.
