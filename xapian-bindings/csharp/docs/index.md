% C# bindings for Xapian

<p>
The C# bindings for Xapian are packaged in the <code>Xapian</code> namespace
and largely follow the C++ API, with the following differences and
additions.  C# strings and other types are converted automatically
in the bindings, so generally it should just work as expected.
</p>

<p>
The <code>examples</code> subdirectory contains examples showing how to use the
C# bindings based on the simple examples from <code>xapian-examples</code>:
<a href="examples/SimpleIndex.cs">SimpleIndex.cs</a>,
<a href="examples/SimpleSearch.cs">SimpleSearch.cs</a>,
<a href="examples/SimpleExpand.cs">SimpleExpand.cs</a>.
</p>

<p>
Note: the passing of strings from C# into Xapian and back isn't currently
zero byte safe.  If you try to handle string containing zero bytes, you'll
find they get truncated at the zero byte.
</p>

## Unicode Support

<p>
In Xapian 1.0.0 and later, the Xapian::Stem, Xapian::QueryParser, and
Xapian::TermGenerator classes all assume text is in UTF-8.  If you're
using Mono on UNIX with a UTF-8 locale (which is the default on most
modern Linux distributions), then Xapian appears to get passed Unicode
strings as UTF-8, so it should just work.  We tested with Mono 1.2.3.1
using the Mono C# 2.0 compiler (gmcs).
</p>

<p>
However, Microsoft and Mono's C# implementations apparently take
rather different approaches to Unicode, and we've not tested with
Microsoft's implementation.  If you try it, please report how well
it works (or how badly it fails...)
</p>

## Method Naming Conventions

<p>
   Methods are renamed to use the "CamelCase" capitalisation convention which C#
   normally uses.  So in C# you use <code>GetDescription</code> instead of
   <code>get_description</code>.
</p>

## Exceptions

<p>
   Exceptions are thrown as SWIG exceptions instead of Xapian
   exceptions. This isn't done well at the moment; in future we will
   throw wrapped Xapian exceptions. For now, it's probably easier to
   catch all exceptions and try to take appropriate action based on
   their associated string.
</p>

## Iterators

<p>
   The C#-wrapped iterators work much like their C++ counterparts, with
   operators "++", "--", "==", and "!=" overloaded.  E.g.:
</p>

<pre>
   Xapian.MSetIterator m = mset.begin();
   while (m != mset.end()) {
     // do something
     ++m;
   }
</pre>

## Iterator dereferencing

<p>
   C++ iterators are often dereferenced to get information, eg
   <code>(*it)</code>.  In C# these are all mapped to named methods, as
   follows:
</p>

<table title='Iterator deferencing methods'>
<thead><td>Iterator</td><td>Dereferencing method</td></thead>
<tr><td>PositionIterator</td>	<td><code>GetTermPos()</code></td></tr>
<tr><td>PostingIterator</td>	<td><code>GetDocId()</code></td></tr>
<tr><td>TermIterator</td>	<td><code>GetTerm()</code></td></tr>
<tr><td>ValueIterator</td>	<td><code>GetValue()</code></td></tr>
<tr><td>MSetIterator</td>	<td><code>GetDocId()</code></td></tr>
<tr><td>ESetIterator</td>	<td><code>GetTerm()</code></td></tr>
</table>

<p>
   Other methods, such as <code>MSetIterator.GetDocument()</code>, are
   available unchanged.
</p>
   
## MSet

<p>
   MSet objects have some additional methods to simplify access (these
   work using the C++ array dereferencing):
</p>

<table title='MSet additional methods'>
<thead><td>Method name</td><td>Explanation</td></thead>
<tr><td><code>GetHit(index)</code></td><td>returns MSetIterator at index</td></tr>
<tr><td><code>GetDocumentPercentage(index)</code></td><td><code>ConvertToPercent(GetHit(index))</code></td></tr>
<tr><td><code>GetDocument(index)</code></td><td><code>GetHit(index).GetDocument()</code></td></tr>
<tr><td><code>GetDocumentId(index)</code></td><td><code>GetHit(index).GetDocId()</code></td></tr>
</table>

## Non-Class Functions

<p>The C++ API contains a few non-class functions (the Database factory
functions, and some functions reporting version information), but C# doesn't
allow functions which aren't in a class so these are wrapped as static
member functions of abstract classes like so:
<ul>
<ul>
<li> <code>Xapian::version_string()</code> is wrapped as <code>Xapian.Version.String()</code></li>
<li> <code>Xapian::major_version()</code> is wrapped as <code>Xapian.Version.Major()</code></li>
<li> <code>Xapian::minor_version()</code> is wrapped as <code>Xapian.Version.Minor()</code></li>
<li> <code>Xapian::revision()</code> is wrapped as <code>Xapian.Version.Revision()</code></li>
</ul>
<ul>
<li> <code>Xapian::Auto::open_stub()</code> is wrapped as <code>Xapian.Auto.OpenStub()</code></li>
<li> <code>Xapian::Chert::open()</code> is wrapped as <code>Xapian.Chert.Open()</code></li>
<li> <code>Xapian::Flint::open()</code> is wrapped as <code>Xapian.Flint.Open()</code>
<li> <code>Xapian::InMemory::open()</code> is wrapped as <code>Xapian.InMemory.Open()</code></li>
<li> <code>Xapian::Remote::open()</code> is wrapped as <code>Xapian.Remote.Open()</code> (both
the TCP and "program" versions are wrapped - the SWIG wrapper checks the parameter list to
decide which to call).</li>
<li> <code>Xapian::Remote::open_writable()</code> is wrapped as <code>Xapian.Remote.OpenWritable()</code> (both
the TCP and "program" versions are wrapped - the SWIG wrapper checks the parameter list to
decide which to call).</li>
</ul>
</ul>

## Constants

<p>
   The <code>Xapian::DB_*</code> constants are currently wrapped in a Xapian
   class within the Xapian namespace, so have a double Xapian prefix!
   So <code>Xapian::DB_CREATE_OR_OPEN</code> is available as
   <code>Xapian.Xapian.DB_CREATE_OR_OPEN</code>.
   The <code>Query::OP_*</code> constants are wrapped a little oddly too:
   <code>Query::OP_OR</code> is wrapped as <code>Xapian.Query.op.OP_OR</code>.
   Similarly, <code>QueryParser::STEM_SOME</code> as
   <code>Xapian.QueryParser.stem_strategy.STEM_SOME</code>.
   The naming here needs sorting out...
</p>

## Query

<p>
   In C++ there's a Xapian::Query constructor which takes a query operator and
   start/end iterators specifying a number of terms or queries, plus an optional
   parameter.
   This isn't currently wrapped in C#.
<!-- FIXME implement this wrapping! 
 
   In C#, this is wrapped to accept any C# sequence (for
   example a list or tuple) to give the terms/queries, and you can specify
   a mixture of terms and queries if you wish.  For example:
   -->
</p>

<!--
<pre>
   subq = xapian.Query(xapian.Query.OP_AND, "hello", "world")
   q = xapian.Query(xapian.Query.OP_AND, [subq, "foo", xapian.Query("bar", 2)])
</pre>
-->

## MatchAll and MatchNothing

These aren't yet wrapped for C#, but you can use `xapian.Query("")`
instead of MatchAll and `xapian.Query()` instead of MatchNothing.

<!-- FIXME: Need to define the custom output typemap to handle this if it
actually seems useful...
## Enquire

<p>
   There is an additional method <code>GetMatchingTerms()</code> which takes
   an MSetIterator and returns a list of terms in the current query which
   match the document given by that iterator.  You may find this
   more convenient than using the TermIterator directly.
</p>
-->

## MatchDecider

<p>
Custom MatchDeciders can be created in C#; simply subclass
Xapian.MatchDecider, and define an
Apply method that will do the work. The simplest example (which does nothing
useful) would be as follows:
</p>

    class MyMatchDecider : Xapian.MatchDecider {
        public override bool Apply(Xapian.Document doc) {
            return true;
        }
    }
