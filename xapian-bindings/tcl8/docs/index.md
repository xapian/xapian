% Tcl8 bindings for Xapian


The Tcl8 bindings for Xapian are packaged in the <code>xapian</code> namespace,
and largely follow the C++ API, with the following differences and
additions. Tcl8 strings and lists, etc., are converted automatically
in the bindings, so generally it should just work as expected.



The <code>examples</code> subdirectory contains examples showing how to use the
Tcl8 bindings based on the simple examples from <code>xapian-examples</code>:
<a href="examples/simpleindex.tcl">simpleindex.tcl</a>,
<a href="examples/simplesearch.tcl">simplesearch.tcl</a>,
<a href="examples/simpleexpand.tcl">simpleexpand.tcl</a>.


## Unicode Support


In Xapian 1.0.0 and later, the Xapian::Stem, Xapian::QueryParser, and
Xapian::TermGenerator classes all assume text is in UTF-8.  Tcl8 uses
UTF-8 as its internal representation, except that ASCII nul (character value
0) is represented as the overlong (and thus invalid) UTF-8 sequence
<code>\xc0\x80</code>.  We don't current convert this to/from
<code>\x00</code> so you should avoid passing strings containing ASCII nul
to/from Xapian from Tcl8.


## Destructors


   To destroy an object <code><i>obj</i></code>, you need to use one of
   <code><i>obj</i> -delete</code> or <code>rename <i>obj</i> ""</code>
   (either should work, but see below).



   SWIG's Tcl wrapping doesn't handle an object returned by a factory function
   correctly.  This only matters for the Xapian::WritableDatabase class, and we
   avoid wrapping the problematic factory functions to avoid setting a
   trap for the unwary - these are the WritableDatabase versions of
   <code>Xapian::Chert::open</code>, and <code>Xapian::Flint::open</code>.
   You can just use a <code>Xapian::WritableDatabase</code> constructor
   instead (and set <code>XAPIAN_PREFER_CHERT</code> in the environment to
   select chert rather than flint).



  Michael Schlenker reports that this form works (i.e. the destructor gets
  called):

    xapian::WritableDatabase xapiandb testdir $::xapian::DB_CREATE_OR_OVERWRITE
    rename xapiandb ""

  However, apparently none of these forms works:

    xapian::WritableDatabase xapiandb testdir $::xapian::DB_CREATE_OR_OVERWRITE
    set db xapiandb
    $db -delete

    set db [xapian::WritableDatabase xapiandb testdir $::xapian::DB_CREATE_OR_OVERWRITE]
    $db -delete

    set db [xapian::WritableDatabase xapiandb testdir $::xapian::DB_CREATE_OR_OVERWRITE]
    rename $db ""


## Exceptions


Xapian::Error exceptions can be handled in Tcl like so:

    if [catch {
        # Code which might throw an exception.
    } msg] {
        # Code to handle exceptions.
        # $errorCode is "XAPIAN <error_class>" (e.g. "XAPIAN DocNotFoundError".)
        # $msg is the result of calling get_msg() on the Xapian::Error object.
    }

## Iterators


   All iterators support <code>next</code> and <code>equals</code> methods
   to move through and test iterators (as for all language bindings).
   MSetIterator and ESetIterator also support <code>prev</code>.


## Iterator dereferencing


   C++ iterators are often dereferenced to get information, eg
   <code>(*it)</code>. With Tcl8 these are all mapped to named methods, as
   follows:


<table title='Iterator deferencing methods'>
<thead><td>Iterator</td><td>Dereferencing method</td></thead>
<tr><td>PositionIterator</td>	<td><code>get_termpos</code></td></tr>
<tr><td>PostingIterator</td>	<td><code>get_docid</code></td></tr>
<tr><td>TermIterator</td>	<td><code>get_term</code></td></tr>
<tr><td>ValueIterator</td>	<td><code>get_value</code></td></tr>
<tr><td>MSetIterator</td>	<td><code>get_docid</code></td></tr>
<tr><td>ESetIterator</td>	<td><code>get_term</code></td></tr>
</table>


   Other methods, such as <code>MSetIterator::get_document</code>, are
   available under the same names.

   
## MSet


   MSet objects have some additional methods to simplify access (these
   work using the C++ array dereferencing):


<table title='MSet additional methods'>
<thead><td>Method name</td><td>Explanation</td></thead>
<tr><td><code>mset get_hit index</code></td><td>returns MSetIterator at index</td></tr>
<tr><td><code>mset get_document_percentage index</code></td><td><code>mset convert_to_percent [mset get_hit index]</code></td></tr>
<tr><td><code>mset get_document index</code></td><td><code>[mset get_hit index] get_document</code></td></tr>
<tr><td><code>mset get_docid index</code></td><td><code>[mset get_hit index] get_docid</code></td></tr>
</table>

## Non-Class Functions

The C++ API contains a few non-class functions (the Database factory
functions, and some functions reporting version information), which are
wrapped like so for Tcl:

* <code>Xapian::version_string()</code> is wrapped as <code>xapian::version_string</code>
* <code>Xapian::major_version()</code> is wrapped as <code>xapian::major_version</code>
* <code>Xapian::minor_version()</code> is wrapped as <code>xapian::minor_version</code>
* <code>Xapian::revision()</code> is wrapped as <code>xapian::revision</code>

* <code>Xapian::Auto::open_stub()</code> is wrapped as <code>xapian::open_stub</code>
* <code>Xapian::Chert::open()</code> is wrapped as <code>xapian::chert_open</code> (but note that the WritableDatabase version isn't wrapped - see the 'Destructors' section above for an explanation).
* <code>Xapian::Flint::open()</code> is wrapped as <code>xapian::flint_open</code> (but note that the WritableDatabase version isn't wrapped - see the 'Destructors' section above for an explanation).
* <code>Xapian::InMemory::open()</code> is wrapped as <code>xapian::inmemory_open</code>
* <code>Xapian::Remote::open()</code> is wrapped as <code>xapian::remote_open</code> (both the TCP and "program" versions are wrapped - the SWIG wrapper checks the parameter list to decide which to call).
* <code>Xapian::Remote::open_writable()</code> is wrapped as <code>xapian::remote_open_writable</code> (both the TCP and "program" versions are wrapped - the SWIG wrapper checks the parameter list to decide which to call).

## Constants

For Tcl, constants are wrapped as:

    $xapian::<i>CONSTANT_NAME</i>

or:

    $xapian::<i>ClassName<i>_<i>CONSTANT_NAME</i>

So ``Xapian::DB_CREATE_OR_OPEN`` is available as ``$xapian::DB_CREATE_OR_OPEN``, ``Xapian::Query::OP_OR`` is available as ``$xapian::Query_OP_OR``, and so on.
</p>


## Query


   In C++ there's a Xapian::Query constructor which takes a query operator and
   start/end iterators specifying a number of terms or queries, plus an optional
   parameter.  In Tcl, this is wrapped to accept a Tcl list
   to give the terms/queries, and you can specify
   a mixture of terms and queries if you wish.  For example:


    set terms [list "hello" "world"]
    xapian::Query subq $xapian::Query_OP_AND $terms
    xapian::Query bar_term "bar" 2
    xapian::Query query $xapian::Query_OP_AND [list subq "foo" bar_term]

## MatchAll and MatchNothing

As of Xapian 1.1.1, these are wrapped for Tcl as
`$xapian::Query_MatchAll` and `$xapian::Query_MatchNothing`.

## Enquire


   There is an additional method <code>get_matching_terms</code> which takes
   an MSetIterator and returns a list of terms in the current query which
   match the document given by that iterator.  You may find this
   more convenient than using the TermIterator directly.
