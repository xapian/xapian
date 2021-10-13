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
standard Ruby naming conventions (i.e. ``get_foo()`` in C++ becomes ``foo()``
in Ruby; ``set_foo()`` becomes ``foo=()``).  C++ ``operator()`` methods are
renamed to ``call`` methods in Ruby.

The C++ methods are not yet documented in the `RDocs <rdocs/>`_.
In the meantime, refer to the
`C++ API documentation <https://xapian.org/docs/apidoc/html/annotated.html>`_
for information on how to use the various methods. Most are
available directly in the Ruby version. The RDocs currently provide information
only on methods that are unique to the Ruby version.

The dangerous/non-Rubish methods from the C++ API have been renamed to
start with underscores (``_``) in the Ruby bindings. You can see them in
use in xapian.rb. It is strongly recommended that you do not call any
method that starts with ``_`` directly in your code, but instead use the
wrappers defined in xapian.rb. Improper use of an ``_`` method can cause
the Ruby process to segfault.

Unicode Support
###############

In Xapian 1.0.0 and later, the ``Xapian::Stem``, ``Xapian::QueryParser``, and
``Xapian::TermGenerator`` classes all assume text is in UTF-8.  If you want
to index or search for strings in a different encoding, convert them to UTF-8
before passing them to Xapian, and when getting strings back from Xapian.
The recommended way to do this is using the `String#encode
<https://ruby-doc.org/core/String.html#method-i-encode>`_ method.

.. Exceptions
.. ##########
.. Exceptions are thrown as SWIG exceptions instead of Xapian
.. exceptions. This isn't done well at the moment; in future we will
.. throw wrapped Xapian exceptions. For now, it's probably easier to
.. catch all exceptions and try to take appropriate action based on
.. their associated string.

Iterators
#########

The iterator classes in the Xapian C++ API are wrapped to allow them
to be used in a more idiomatic way from Ruby.  Where the C++ API
has a pair of methods to return a begin and end iterator, the Ruby
API has a single method which (in Xapian 1.4.12 and later) supports block
iteration, for example::

  mset.matches {|match|
    # do something
  }

If no block is specified, an Array is returned instead (which was the only
option prior to Xapian 1.4.12).  You can use ``each`` on this Array to achieve
a similar result to passing a block, except the C++ iterator is read eagerly
rather than lazily::

  mset.matches.each {|match|
    # do something
  }

Non-Class Functions
###################

The C++ API contains a few non-class functions (the Database factory
functions, and some functions reporting version information), which are
wrapped like so for Ruby:

- ``Xapian::version_string()`` is wrapped as ``Xapian::version_string()``
- ``Xapian::major_version()`` is wrapped as ``Xapian::major_version()``
- ``Xapian::minor_version()`` is wrapped as ``Xapian::minor_version()``
- ``Xapian::revision()`` is wrapped as ``Xapian::revision()``
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
