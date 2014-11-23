Ruby bindings for Xapian
************************

The Ruby bindings for Xapian are packaged in the ``xapian`` module.
Ruby strings and arrays are converted automatically in the bindings, so
generally they should just work naturally.

The ``examples`` subdirectory contains examples showing how to use the
Ruby bindings based on the simple examples from ``xapian-examples``:
`simpleindex.rb <examples/simpleindex.rb>`_,
`simplesearch.rb <examples/simplesearch.rb>`_,
`simpleexpand.rb <examples/simpleexpand.rb>`_.
There's also
`simplematchdecider.rb <examples/simplematchdecider.rb>`_
which shows how to define a MatchDecider in Ruby.


Usage
#####

To use the bindings, you need to use ``require 'xapian'``
in your ruby program.

Most standard Xapian methods are available directly
to your Ruby program. Names have been altered to conform to the
standard Ruby naming conventions (i.e. get_foo() in C++ becomes foo()
in Ruby; set_foo() becomes foo=().)  C++ 'operator()' methods are
renamed to 'call' methods in Ruby.

The C++ methods are not yet documented in the `RDocs <rdocs/>`_.
In the meantime, refer to the
`C++ API documentation <http://xapian.org/docs/apidoc/html/annotated.html>`_
for information on how to use the various methods. Most are
available directly in the Ruby version. The RDocs currently provide information
only on methods that are unique to the Ruby version.

The dangerous/non-Rubish methods from the C++ API have been renamed to
start with underscores ('_') in the Ruby bindings. You can see them in
use in xapian.rb. It is strongly recommended that you do not call any
method that starts with _ directly in your code, but instead use the
wrappers defined in xapian.rb. Improper use of an _ method can cause
the Ruby process to segfault.

Unicode Support
###############

In Xapian 1.0.0 and later, the Xapian::Stem, Xapian::QueryParser, and
Xapian::TermGenerator classes all assume text is in UTF-8.  If you want
to index strings in a different encoding, use the Ruby
`Iconv Class <http://www.ruby-doc.org/stdlib/libdoc/iconv/rdoc/index.html>`_
to convert them to UTF-8 before passing them to Xapian, and
when reading values back from Xapian.

.. Exceptions
.. ##########
.. Exceptions are thrown as SWIG exceptions instead of Xapian
.. exceptions. This isn't done well at the moment; in future we will
.. throw wrapped Xapian exceptions. For now, it's probably easier to
.. catch all exceptions and try to take appropriate action based on
.. their associated string.

Iterators
#########

One important difference from the C++ API is that \*Iterator
classes should not be used from Ruby, as they fit awkwardly into
standard Ruby iteration paradigms, and as many of them cause segfaults
if used improperly. They have all been wrapped with appropriate
methods that simply return the \*Iterator objects in an Array, so that
you can use 'each' to iterate through them.

::

  mset.matches.each {|match|
    # do something
  }


.. Iterator dereferencing
.. ######################
.. C++ iterators are often dereferenced to get information, eg
.. ``(*it)``. With Python these are all mapped to named methods, as
.. follows:

.. .. table:: Iterator deferencing methods

.. +------------------+----------------------+
.. | Iterator         | Dereferencing method |
.. +==================+======================+
.. | PositionIterator |    ``get_termpos()`` |
.. +------------------+----------------------+
.. | PostingIterator  |  ``get_docid()``     |
.. +------------------+----------------------+
.. | TermIterator     |     ``get_term()``   |
.. +------------------+----------------------+
.. | ValueIterator    |     ``get_value()``  |
.. +------------------+----------------------+
.. | MSetIterator     |     ``get_docid()``  |
.. +------------------+----------------------+
.. | ESetIterator     |     ``get_term()``   |
.. +------------------+----------------------+

.. Other methods, such as ``MSetIterator.get_document()``, are
.. available unchanged.

.. MSet
.. ####

.. MSet objects have some additional methods to simplify access (these
.. work using the C++ array dereferencing):

.. ..table:: MSet additional methods

.. +-----------------------------------+----------------------------------------+
.. | Method name                       |            Explanation                 |
.. +===================================+========================================+
.. | ``get_hit(index)``                |   returns MSetIterator at index        |
.. +-----------------------------------+----------------------------------------+
.. | ``get_document_percentage(index)``| ``convert_to_percent(get_hit(index))`` |
.. +-----------------------------------+----------------------------------------+
.. | ``get_document(index)``           | ``get_hit(index).get_document()``      |
.. +-----------------------------------+----------------------------------------+
.. | ``get_docid(index)``              | ``get_hit(index).get_docid()``         |
.. +-----------------------------------+----------------------------------------+


Non-Class Functions
###################

The C++ API contains a few non-class functions (the Database factory
functions, and some functions reporting version information), which are
wrapped like so for Ruby:

- ``Xapian::version_string()`` is wrapped as ``Xapian::version_string()``
- ``Xapian::major_version()`` is wrapped as ``Xapian::major_version()``
- ``Xapian::minor_version()`` is wrapped as ``Xapian::minor_version()``
- ``Xapian::revision()`` is wrapped as ``Xapian::revision()``
- ``Xapian::Auto::open_stub()`` is wrapped as ``Xapian::open_stub()``
- ``Xapian::Chert::open()`` is wrapped as ``Xapian::chert_open()`` (now deprecated)
- ``Xapian::InMemory::open()`` is wrapped as ``Xapian::inmemory_open()``
- ``Xapian::Remote::open()`` is wrapped as ``Xapian::remote_open()`` (both the TCP and "program" versions are wrapped - the SWIG wrapper checks the parameter list to decide which to call).
- ``Xapian::Remote::open_writable()`` is wrapped as ``Xapian::remote_open_writable()`` (both the TCP and "program" versions are wrapped - the SWIG wrapper checks the parameter list to decide which to call).


Query
#####

In C++ there's a Xapian::Query constructor which takes a query operator and
start/end iterators specifying a number of terms or queries, plus an optional
parameter.  In Ruby, this is wrapped to accept a Ruby array containing
terms, or queries, or even a mixture of terms and queries.  For example:

::

  subq = Xapian::Query.new(Xapian::Query::OP_AND, "hello", "world")
  q = Xapian::Query.new(Xapian::Query::OP_AND, [subq, "foo", Xapian::Query.new("bar", 2)])


MatchAll and MatchNothing
-------------------------

In Xapian 1.3.0 and later, these are wrapped as class constants
``Xapian::Query::MatchAll`` and ``Xapian::Query::MatchNothing``.

If you want to be compatible with earlier versions, you can continue to use
``Xapian::Query.new("")`` for MatchAll and
``Xapian::Query.new()`` for MatchNothing.


MatchDecider
############

Custom MatchDeciders can be created in Ruby; simply subclass
Xapian::MatchDecider, ensure you call the superclass constructor, and define a
__call__ method that will do the work. The simplest example (which does nothing
useful) would be as follows:

::

  class MyMatchDecider < Xapian\::MatchDecider
    def __call__(doc):
      return true
    end
  end
