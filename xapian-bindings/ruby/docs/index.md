% Ruby bindings for Xapian

The Ruby bindings for Xapian are packaged in the <code>xapian</code> module.
Ruby strings and arrays are converted automatically in the bindings, so
generally they should just work naturally.



The <code>examples</code> subdirectory contains examples showing how to use the
Ruby bindings based on the simple examples from <code>xapian-examples</code>:
<a href="examples/simpleindex.rb">simpleindex.rb</a>,
<a href="examples/simplesearch.rb">simplesearch.rb</a>,
<a href="examples/simpleexpand.rb">simpleexpand.rb</a>.
There's also 
<a href="examples/simplematchdecider.rb">simplematchdecider.rb</a>
which shows how to define a MatchDecider in Ruby.


## Usage

To use the bindings, you need to use <code>require 'xapian'</code>
in your ruby program. 


Most standard Xapian methods are available directly
to your Ruby program. Names have been altered to conform to the
standard Ruby naming conventions (i.e. get_foo() in C++ becomes foo()
in Ruby; set_foo() becomes foo=().)  C++ 'operator()' methods are
renamed to 'call' methods in Ruby.



The C++ methods are not yet documented in the <a href="rdocs/">RDocs</a>.
In the meantime, refer to the 
<a href="http://xapian.org/docs/apidoc/html/annotated">C++ API
documentation</a> for information on how to use the various methods. Most are
available directly in the Ruby version. The RDocs currently provide information
only on methods that are unique to the Ruby version.



The dangerous/non-Rubish methods from the C++ API have been renamed to
start with underscores ('\_') in the Ruby bindings. You can see them in
use in xapian.rb. It is strongly recommended that you do not call any
method that starts with _ directly in your code, but instead use the
wrappers defined in xapian.rb. Improper use of an _ method can cause
the Ruby process to segfault.


## Unicode Support


In Xapian 1.0.0 and later, the Xapian::Stem, Xapian::QueryParser, and
Xapian::TermGenerator classes all assume text is in UTF-8.  If you want
to index strings in a different encoding, use the Ruby
<a href="http://www.ruby-doc.org/stdlib/libdoc/iconv/rdoc/index.html"
><code>Iconv</code> class</a>
to convert them to UTF-8 before passing them to Xapian, and
when reading values back from Xapian.


<!--
## Exceptions


   Exceptions are thrown as SWIG exceptions instead of Xapian
   exceptions. This isn't done well at the moment; in future we will
   throw wrapped Xapian exceptions. For now, it's probably easier to
   catch all exceptions and try to take appropriate action based on
   their associated string.

-->

## Iterators


One important difference from the C++ API is that \*Iterator
classes should not be used from Ruby, as they fit awkwardly into
standard Ruby iteration paradigms, and as many of them cause segfaults
if used improperly. They have all been wrapped with appropriate
methods that simply return the \*Iterator objects in an Array, so that
you can use 'each' to iterate through them.


    mset.matches.each {|match|
      # do something
    }

## Iterator dereferencing


   C++ iterators are often dereferenced to get information, eg
   <code>(*it)</code>. With Python these are all mapped to named methods, as
   follows:


<table title="Iterator deferencing methods">
<thead><td>Iterator</td><td>Dereferencing method</td></thead>
<tr><td>PositionIterator</td>	<td><code>get_termpos()</code></td></tr>
<tr><td>PostingIterator</td>	<td><code>get_docid()</code></td></tr>
<tr><td>TermIterator</td>	<td><code>get_term()</code></td></tr>
<tr><td>ValueIterator</td>	<td><code>get_value()</code></td></tr>
<tr><td>MSetIterator</td>	<td><code>get_docid()</code></td></tr>
<tr><td>ESetIterator</td>	<td><code>get_term()</code></td></tr>
</table>


   Other methods, such as <code>MSetIterator.get_document()</code>, are
   available unchanged.


## MSet


   MSet objects have some additional methods to simplify access (these
   work using the C++ array dereferencing):


<table title="MSet additional methods">
<thead><td>Method name</td><td>Explanation</td></thead>
<tr><td><code>get_hit(index)</code></td><td>returns MSetIterator at index</td></tr>
<tr><td><code>get_document_percentage(index)</code></td><td><code>convert_to_percent(get_hit(index))</code></td></tr>
<tr><td><code>get_document(index)</code></td><td><code>get_hit(index).get_document()</code></td></tr>
<tr><td><code>get_docid(index)</code></td><td><code>get_hit(index).get_docid()</code></td></tr>
</table>

## Non-Class Functions

The C++ API contains a few non-class functions (the Database factory
functions, and some functions reporting version information), which are
wrapped like so for Ruby:

* <code>Xapian::version_string()</code> is wrapped as <code>Xapian::version_string()</code>
* <code>Xapian::major_version()</code> is wrapped as <code>Xapian::major_version()</code>
* <code>Xapian::minor_version()</code> is wrapped as <code>Xapian::minor_version()</code>
* <code>Xapian::revision()</code> is wrapped as <code>Xapian::revision()</code>

* <code>Xapian::Auto::open_stub()</code> is wrapped as <code>Xapian::open_stub()</code>
* <code>Xapian::Chert::open()</code> is wrapped as <code>Xapian::chert_open()</code>
* <code>Xapian::Flint::open()</code> is wrapped as <code>Xapian::flint_open()</code>
* <code>Xapian::InMemory::open()</code> is wrapped as <code>Xapian::inmemory_open()</code>
* <code>Xapian::Remote::open()</code> is wrapped as <code>Xapian::remote_open()</code> (both the TCP and "program" versions are wrapped - the SWIG wrapper checks the parameter list to decide which to call).
* <code>Xapian::Remote::open_writable()</code> is wrapped as <code>Xapian::remote_open_writable()</code> (both the TCP and "program" versions are wrapped - the SWIG wrapper checks the parameter list to decide which to call).

## Query


   In C++ there's a Xapian::Query constructor which takes a query operator and
   start/end iterators specifying a number of terms or queries, plus an optional
   parameter.  In Ruby, this is wrapped to accept a Ruby array containing
   terms, or queries, or even a mixture of terms and queries.  For example:


    subq = Xapian::Query.new(Xapian::Query::OP_AND, "hello", "world")
    q = Xapian::Query.new(Xapian::Query::OP_AND, [subq, "foo", Xapian::Query.new("bar", 2)])

## MatchAll and MatchNothing

These aren't yet wrapped for Ruby, but you can use `Xapian.Query.new("")`
instead of MatchAll and `Xapian.Query.new()` instead of MatchNothing.

## MatchDecider


Custom MatchDeciders can be created in Ruby; simply subclass
Xapian::MatchDecider, ensure you call the superclass constructor, and define a
<code>__call__</code> method that will do the work. The simplest example (which does nothing
useful) would be as follows:

    class MyMatchDecider < Xapian::MatchDecider
      def __call__(doc):
        return true
      end
    end
