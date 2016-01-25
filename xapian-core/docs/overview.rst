Overview
========

This document provides an introduction to the native C++ Xapian API.
This API provides programmers with the ability to index and search
through (potentially very large) bodies of data using probabilistic
methods.

*Note:* The portion of the API currently documented here covers only the
part of Xapian concerned with searching through existing databases, not
that concerned with creating them.

This document assumes you already have Xapian installed, so if you
haven't, it is a good idea to read `Installing Xapian <install.html>`_
first.

You may also wish to read the `Introduction
to Information Retrieval <intro_ir.html>`_ for a background into the
Information Retrieval theories behind Xapian.

This document does not detail the exact calling conventions (parameters
passed, return value, exceptions thrown, etc...) for each method in the
API. For such documentation, you should refer to the automatically
extracted documentation, which is generated from detailed comments in
the source code, and should thus remain up-to-date and accurate. This
documentation is generated using the
`Doxygen <http://www.doxygen.org/>`_ application. To save you having
to generate this documentation yourself, we include the `built
version <apidoc/html/index.html>`_ in our distributions, and also keep
the `latest version <https://xapian.org/docs/apidoc/html/index.html>`_ on
our website.

Design Principles
-----------------

API classes are either very lightweight or a wrapper around a reference
counted pointer (this style of class design is sometimes known as PIMPL
for "Private IMPLementation"). In either case copying is a cheap
operation as classes are at most a few words of memory.

API objects keep a reference to other objects they rely on so the user
doesn't need to worry about whether an object is still valid or not.

Where appropriate, API classes can be used as containers and iterators
just like those in the C++ STL.

Errors and exceptions
---------------------

It is important to understand the errors which may be caused by the
operations which you are trying to perform.

This becomes particularly relevant when using a large system, with such
possibilities as databases which are being updated while you search
through them, and distributed enquiry systems.

Errors in Xapian are all reported by means of exceptions. All exceptions
thrown by Xapian will be subclasses of
`Xapian::Error <apidoc/html/classXapian_1_1Error.html>`_. Note that
``Xapian::Error`` is an abstract class; thus you must catch exceptions
by reference rather than by value.

There are two flavours of error, derived from ``Xapian::Error``:

-  `Xapian::LogicError <apidoc/html/classXapian_1_1LogicError.html>`_
   - for error conditions due to programming errors, such as a misuse of
   the API. A finished application should not receive these errors
   (though it would still be sensible to catch them).
-  `Xapian::RuntimeError <apidoc/html/classXapian_1_1RuntimeError.html>`_
   - for error conditions due to run time problems, such as failure to
   open a database. You must always be ready to cope with such errors.

Each of these flavours is further subdivided, such that any particular
error condition can be trapped by catching the appropriate exception. If
desired, a human readable explanation of the error can be retrieved by
calling
`Xapian::Error::get_msg() <apidoc/html/classXapian_1_1Error.html>`_.

In addition, standard system errors may occur: these will be reported by
throwing appropriate exceptions. Most notably, if the system runs out of
memory, a ``std::bad_alloc`` exception will be thrown.

Terminology
-----------

Databases
~~~~~~~~~

These may also occasionally be called *Indexes*. In Xapian (as opposed
to a database package) a database consists of little more than indexed
documents: this reflects the purpose of Xapian as an information
retrieval system, rather than an information storage system.

The exact contents of a database depend on the type (see "`Database
Types <#database_types>`_" for more details of the database types
currently provided).

Queries
~~~~~~~

The information to be searched for is specified by a *Query*. In Xapian,
queries are made up of a structured boolean tree, upon which
probabilistic weightings are imposed: when the search is performed, the
documents returned are filtered according to the boolean structure, and
weighted (and sorted) according to the probabilistic model of
information retrieval.

Memory handling
---------------

The user of Xapian does not usually need to worry about how Xapian
performs its memory allocation: Xapian objects can all be created and
deleted as any other C++ objects. The convention is that whoever creates
an object is ultimately responsible for deleting it. This becomes
relevant when passing a pointer to data to Xapian: it will be the caller's
responsibility to delete the object once it is finished with.  Where
an object is set by one API call and used by another, Xapian will assume
that such pointers remain valid.

The Xapian::Enquire class
-------------------------

The `Xapian::Enquire <apidoc/html/classXapian_1_1Enquire.html>`_
class is central to all searching operations. It provides an interface
for

-  Specifying the database, or databases, to search across.
-  Specifying a query to perform.
-  Specifying a set of documents which a user considers relevant.
-  Given the supplied information, returning a ranked set of documents
   for the user.
-  Given the supplied information, suggesting a ranked set of terms to
   add to the query.
-  Returning information about the documents which matched, such as
   their associated data, and which terms from the query were found
   within them.

A typical enquiry session will consist of most of these operations, in
various orders. The Xapian::Enquire class presents as few restrictions
as possible on the order in which operations should be performed.
Although you must set the query before any operation which uses it, you
can call any of the other methods in any order.

Many operations performed by the Xapian::Enquire class are performed
lazily (ie, just before their results are needed). This need not concern
the user except to note that, as a result, errors may not be reported as
soon as would otherwise be expected.

Specifying a database
---------------------

When creating a Xapian::Enquire object, a database to search must be
specified. Databases are specified by creating a `Xapian::Database
object <apidoc/html/classXapian_1_1Database.html>`_. Generally, you can
just construct the object, passing the pathname to the database. Xapian
looks at the path and autodetects the database type.

In some cases (with the Remote backend, or if you want more control) you
need to use a factory function such as ``Xapian::Flint::open()`` - each
backend type has one or more. The parameters the function takes depend
on the backend type, and whether we are creating a read-only or a
writable database.

You can also create a "stub database" file which lists one or more
databases. These files are recognised by the autodetection in the
Database constructor (if the pathname is file rather than a directory,
it's treated as a stub database file) or you can open them explicitly
using Xapian::Auto::open\_stub(). The stub database format specifies one
database per line. For example::

     remote localhost:23876
     flint /var/spool/xapian/webindex

Database types
~~~~~~~~~~~~~~

The current types understood by Xapian are:

auto
    This isn't an actual database format, but rather auto-detection of one of
    the disk based backends (e.g. "flint" or "chert") from a single specified
    path (which can be to a file or directory).

brass
    Brass is the current development backend, and it is intended to be the
    default backend in Xapian 1.4.x.

chert
    Chert is the default backend in Xapian 1.2.x. It supports incremental
    modifications, concurrent single-writer and multiple-reader access to a
    database. It's very efficient and highly scalable.

flint
    Flint was the default backend in Xapian 1.0.x. It supports incremental
    modifications, concurrent single-writer and multiple-reader access to a
    database. It's very efficient and highly scalable. Flint takes lessons
    learned from studying Quartz in action, and is appreciably faster (both
    when indexing and searching), more compact, and features an improved
    locking mechanism which automatically releases the lock if a writing
    process dies.

    For more information, see the `Xapian Wiki
    <https://trac.xapian.org/wiki/FlintBackend>`_.

inmemory
    This type is a database held entirely in memory. It was originally written
    for testing purposes only, but may prove useful for building up temporary
    small databases.

quartz
    Quartz was the default backend prior to Xapian 1.0, and has been removed as
    of Xapian 1.1.0. If you want to migrate an existing Quartz database to
    Flint, `see the 'Admin Notes'
    <admin_notes.html#converting-a-quartz-database-to-a-flint-database%60>`_
    for a way to do this.

remote
    This can specify either a "program" or TCP remote backend, for example::

        remote :ssh xapian-prog.example.com xapian-progsrv /srv/xapian/db1

    or::

        remote xapian-tcp.example.com:12345

    If the first character of the second word is a colon (``:``), then this is
    skipped and the remainder of the line is used as the command to run
    xapian-progsrv and the "program" variant of the remote backend is used.
    Otherwise the TCP variant of the remote backend is used, and the rest of
    the line specifies the host and port to connect to.

Multiple databases
~~~~~~~~~~~~~~~~~~

Xapian can search across several databases as easily as searching across
a single one. Simply call
`Xapian::Database::add_database() <apidoc/html/classXapian_1_1Database.html>`_
for each database that you wish to search through.

You can also set up "pre-canned" listed of databases to search over
using a "stub database" - see above for details.

Specifying a query
------------------

Xapian implements both boolean and probabilistic searching. There are
two obvious ways in which a pure boolean query could be combined with a
pure probabilistic query:

-  First perform the boolean search to create a subset of the whole
   document collection, and then do the probabilistic search on this
   subset, or
-  Do the probabilistic search, and then filter out the resulting
   documents with a boolean query.

There is in fact a subtle difference in these two approaches. In the
first, the collection statistics for the probabilistic query will be
determined by the document subset which is obtained by running the
boolean query. In the second, the collection statistics for the
probabilistic query are determined by the whole document collection.
These differences can affect the final result.

Suppose for example the boolean query is being used to retrieve
documents in English in a database containing English and French
documents. A word like "*grand*", exists in both languages (with similar
meanings), but is more common in French than English. In the English
subset it could therefore be expected to have a higher weight than it
would get in the joint English and French databases.

Xapian takes the second approach simply because this can be implemented
very efficiently. The first approach is more exact, but inefficient to
implement.

Rather than implementing this approach as described above and first
performing the probabilistic search and then filtering the results,
Xapian actually performs both tasks simultaneously. This allows various
optimisations to be performed, such as giving up on calculating a
boolean AND operation when the probabilistic weights that could result
from further documents can have no effect on the result set. These
optimisations have been found to often give a several-fold performance
increase. The performance is particularly good for queries containing
many terms.

A query for a single term
~~~~~~~~~~~~~~~~~~~~~~~~~

A search query is represented by a
`Xapian::Query <apidoc/html/classXapian_1_1Query.html>`_ object. The
simplest useful query is one which searches for a single term (and
several of these can be combined to form more complex queries). A single
term query can be created as follows (where ``term`` is a
``std::string`` holding the term to be searched for)::

    Xapian::Query query(term);

A term in Xapian is represented simply by a string of bytes.  Usually, when
searching text, these bytes will represent the characters of the word which
the term represents, but during the information retrieval process Xapian
attaches no specific meaning to the term.

This constructor actually takes a couple of extra parameters, which may
be used to specify positional and frequency information for terms in the
query::

    Xapian::Query(const string & tname_,
            Xapian::termcount wqf_ = 1,
            Xapian::termpos term_pos_ = 0)

The ``wqf`` (Within Query Frequency) is a measure of how common a term is
in the query. This isn't useful for a single term query unless it is going
to form part of a more complex query. In that case, it's particularly
useful when generating a query from an existing document, but may also be
used to increase the "importance" of a term in a query. Another way to
increase the "importance" of a term is to use ``OP_SCALE_WEIGHT``. But if
the intention is simply to ensure that a particular term is in the query
results, you should use a boolean AND or AND\_MAYBE rather than setting a
high wqf.

The ``term_pos`` represents the position of the term in the query.
Again, this isn't useful for a single term query by itself, but is used
for phrase searching, passage retrieval, and other operations which
require knowledge of the order of terms in the query (such as returning
the set of matching terms in a given document in the same order as they
occur in the query). If such operations are not required, the default
value of 0 may be used.

Note that it may not make much sense to specify a wqf other than 1 when
supplying a term position (unless you are trying to affect the
weighting, as previously described).

Note also that the results of ``Xapian::Query(tname, 2)`` and
``Xapian::Query(Xapian::Query::OP_OR, Xapian::Query(tname), Xapian::Query(tname))``
are exactly equivalent.

Compound queries
~~~~~~~~~~~~~~~~

Compound queries can be built up from single term queries by combining
them a connecting operator. Most operators can operate on either a
single term query or a compound query. You can combine pair-wise using
the following constructor::

    Xapian::Query(Xapian::Query::op op_,
            const Xapian::Query & left,
            const Xapian::Query & right)

The two most commonly used operators are ``Xapian::Query::OP_AND`` and
``Xapian::Query::OP_OR``, which enable us to construct boolean queries
made up from the usual AND and OR operations. But in addition to this, a
probabilistic query in its simplest form, where we have a list of terms
which give rise to weights that need to be added together, is also made
up from a set of terms joined together with ``Xapian::Query::OP_OR``.

Some of the available ``Xapian::Query::op`` operators are:

+---------------------------------+-----------------------------------------------------------------------------------------------------------------------+
| Xapian::Query::OP\_AND          | Return documents returned by both subqueries.                                                                         |
+---------------------------------+-----------------------------------------------------------------------------------------------------------------------+
| Xapian::Query::OP\_OR           | Return documents returned by either subquery.                                                                         |
+---------------------------------+-----------------------------------------------------------------------------------------------------------------------+
| Xapian::Query::OP\_AND\_NOT     | Return documents returned by the left subquery but not the right subquery.                                            |
+---------------------------------+-----------------------------------------------------------------------------------------------------------------------+
| Xapian::Query::OP\_FILTER       | As Xapian::Query::OP\_AND, but use only weights from left subquery.                                                   |
+---------------------------------+-----------------------------------------------------------------------------------------------------------------------+
| Xapian::Query::OP\_AND\_MAYBE   | Return documents returned by the left subquery, but adding document weights from both subqueries.                     |
+---------------------------------+-----------------------------------------------------------------------------------------------------------------------+
| Xapian::Query::OP\_XOR          | Return documents returned by one subquery only.                                                                       |
+---------------------------------+-----------------------------------------------------------------------------------------------------------------------+
| Xapian::Query::OP\_NEAR         | Return documents where the terms are with the specified distance of each other.                                       |
+---------------------------------+-----------------------------------------------------------------------------------------------------------------------+
| Xapian::Query::OP\_PHRASE       | Return documents where the terms are with the specified distance of each other and in the given order.                |
+---------------------------------+-----------------------------------------------------------------------------------------------------------------------+
| Xapian::Query::OP\_ELITE\_SET   | Select an elite set of terms from the subqueries, and perform a query with all those terms combined as an OR query.   |
+---------------------------------+-----------------------------------------------------------------------------------------------------------------------+

Understanding queries
~~~~~~~~~~~~~~~~~~~~~

Each term in the query has a weight in each document. Each document may
also have an additional weight not associated with any of the terms. By
default the probabilistic weighting scheme `BM25 <bm25.html>`_ is used
to provide the formulae which give these weights.

A query can be thought of as a tree structure. At each node is an
``Xapian::Query::op`` operator, and on the left and right branch are two
other queries. At each leaf node is a term, t, transmitting documents
and scores, D and w\ :sub:`D`\ (t), up the tree.

A Xapian::Query::OP\_OR node transmits documents from both branches up
the tree, summing the scores when a document is found in both the left
and right branch. For example,
::

                               docs       1    8    12    16    17    18
                               scores    7.3  4.1   3.2  7.6   3.8   4.7 ...
                                 |
                                 |
                       Xapian::Query::OP_OR
                             /       \
                            /         \
                           /           \
                          /             \
       docs     1   12   16   17         1   8   16   18
       scores  3.1 3.2  3.1  3.8 ...    4.2 4.1 4.5  4.7 ...

A Xapian::Query::OP\_AND node transmits only the documents found on both
branches up the tree, again summing the scores,
::

                               docs       1   16
                               scores    7.3  7.6  ...
                                 |
                                 |
                       Xapian::Query::OP_AND
                             /       \
                            /         \
                           /           \
                          /             \
       docs     1   12   16   17         1   8   16   18
       scores  3.1 3.2  3.1  3.8 ...    4.2 4.1 4.5  4.7 ...

A Xapian::Query::OP\_AND\_NOT node transmits up the tree the documents
on the left branch which are not on the right branch. The scores are
taken from the left branch. For example, again summing the scores,
::

                               docs       12   17
                               scores    3.2  3.8 ...
                                 |
                                 |
                     Xapian::Query::OP_AND_NOT
                             /       \
                            /         \
                           /           \
                          /             \
       docs     1   12   16   17         1   8   16   18
       scores  3.1 3.2  3.1  3.8 ...    4.2 4.1 4.5  4.7 ...

A Xapian::Query::OP\_AND\_MAYBE node transmits the documents up the tree
from the left branch only, but adds in the score from the right branch
for documents which occur on both branches. For example,
::

                               docs       1    12   16   17
                               scores    7.3  3.2  7.6  3.8 ...
                                 |
                                 |
                    Xapian::Query::OP_AND_MAYBE
                             /       \
                            /         \
                           /           \
                          /             \
       docs     1   12   16   17         1   8   16   18
       scores  3.1 3.2  3.1  3.8 ...    4.2 4.1 4.5  4.7 ...

Xapian::Query::OP\_FILTER is like Xapian::Query::OP\_AND, but weights
are only transmitted from the left branch. For example,
::

                               docs       1   16
                               scores    3.1  3.1  ...
                                 |
                                 |
                      Xapian::Query::OP_FILTER
                             /       \
                            /         \
                           /           \
                          /             \
       docs     1   12   16   17         1   8   16   18
       scores  3.1 3.2  3.1  3.8 ...    4.2 4.1 4.5  4.7 ...

Xapian::Query::OP\_XOR is like Xapian::Query::OP\_OR, but documents on
both left and right branches are not transmitted up the tree. For
example,
::

                               docs       8    12    17    18
                               scores    4.1   3.2  3.8   4.7 ...
                                 |
                                 |
                          Xapian::Query::OP_XOR
                             /       \
                            /         \
                           /           \
                          /             \
       docs     1   12   16   17         1   8   16   18
       scores  3.1 3.2  3.1  3.8 ...    4.2 4.1 4.5  4.7 ...

A query can therefore be thought of as a process for generating an MSet
from the terms at the leaf nodes of the query. Each leaf node gives rise
to a posting list of documents with scores. Each higher level node gives
rise to a similar list, and the root node of the tree contains the final
set of documents with scores (or weights), which are candidates for
going into the MSet. The MSet contains the documents which get the
highest weights, and they are held in the MSet in weight order.

It is important to realise that within Xapian the structure of a query
is optimised for best performance, and it undergoes various
transformations as the query progresses. The precise way in which the
query is built up is therefore of little importance to Xapian - for
example, you can AND together terms pair-by-pair, or combine several
using AND on a std::vector of terms, and Xapian will build the same
structure internally.

Using queries
~~~~~~~~~~~~~

Probabilistic queries
^^^^^^^^^^^^^^^^^^^^^

A plain probabilistic query is created by connecting terms together with
Xapian::Query::OP\_OR operators. For example,
::

        Xapian::Query query("regulation");
        query = Xapian::Query(Xapian::Query::OP_OR, query, Xapian::Query("import"));
        query = Xapian::Query(Xapian::Query::OP_OR, query, Xapian::Query("export"));
        query = Xapian::Query(Xapian::Query::OP_OR, query, Xapian::Query("canned"));
        query = Xapian::Query(Xapian::Query::OP_OR, query, Xapian::Query("fish"));

This creates a probabilistic query with terms \`regulation', \`import',
\`export', \`canned' and \`fish'.

In fact this style of creation is so common that there is the shortcut
construction::

        vector<string> terms;
        terms.push_back("regulation");
        terms.push_back("import");
        terms.push_back("export");
        terms.push_back("canned");
        terms.push_back("fish");

        Xapian::Query query(Xapian::Query::OP_OR, terms.begin(), terms.end());

Boolean queries
^^^^^^^^^^^^^^^

Suppose now we have this Boolean query,
::

        ('EEC' - 'France') and ('1989' or '1991' or '1992') and 'Corporate_Law'

This could be built up as bquery like this,
::

        Xapian::Query bquery1(Xapian::Query::OP_AND_NOT, "EEC", "France");

        Xapian::Query bquery2("1989");
        bquery2 = Xapian::Query(Xapian::Query::OP_OR, bquery2, "1991");
        bquery2 = Xapian::Query(Xapian::Query::OP_OR, bquery2, "1992");

        Xapian::Query bquery3("Corporate_Law");

        Xapian::Query bquery(Xapian::Query::OP_AND, bquery1, Xapian::Query(Xapian::Query::OP_AND(bquery2, bquery3)));

and this can be attached as a filter to ``query`` to run the
probabilistic query with a Boolean filter,
::

        query = Xapian::Query(Xapian::Query::OP_FILTER, query, bquery);

If you want to run a pure boolean query, then set BoolWeight as the
weighting scheme (by calling Enquire::set\_weighting\_scheme() with
argument BoolWeight()).

Plus and minus terms
^^^^^^^^^^^^^^^^^^^^

A common requirement in search engine functionality is to run a
probabilistic query where some terms are required to index all the
retrieved documents (\`+' terms), and others are required to index none
of the retrieved documents (\`-' terms). For example,
::

        regulation import export +canned +fish -japan

the corresponding query can be set up by,
::

        vector<string> plus_terms;
        vector<string> minus_terms;
        vector<string> normal_terms;

        plus_terms.push_back("canned");
        plus_terms.push_back("fish");

        minus_terms.push_back("japan");

        normal_terms.push_back("regulation");
        normal_terms.push_back("import");
        normal_terms.push_back("export");

        Xapian::Query query(Xapian::Query::OP_AND_MAYBE,
                      Xapian::Query(Xapian::Query::OP_AND, plus_terms.begin(), plus_terms.end());
                      Xapian::Query(Xapian::Query::OP_OR, normal_terms.begin(), normal_terms.end()));

        query = Xapian::Query(Xapian::Query::OP_AND_NOT,
                        query,
                        Xapian::Query(Xapian::Query::OP_OR, minus_terms.begin(), minus_terms.end()));

Undefined queries
~~~~~~~~~~~~~~~~~

Performing a match with an undefined query matches nothing, which is
sometimes useful. Composing an undefined query with operators behaves
just as it would for any subquery which matches nothing.

Retrieving the results of a query
---------------------------------

To get the results of the query, call the ``Enquire::get_mset()`` method::

    Xapian::MSet Xapian::Enquire::get_mset(Xapian::doccount first,
                               Xapian::doccount maxitems,
                               const Xapian::RSet * rset = 0,
                               const Xapian::MatchDecider * mdecider = 0) const

When asking for the results, you must specify (in ``first``) the first
item in the result set to return, where the numbering starts at zero (so
a value of zero corresponds to the first item returned being that with
the highest score, and a value of 10 corresponds to the first 10 items
being ignored, and the returned items starting at the eleventh).

You must also specify (in ``maxitems``) the maximum number of items to
return. Unless there are not enough matching items, precisely this
number of items will be returned. If ``maxitems`` is zero, no items will
be returned, but the usual statistics (such as the maximum possible
weight which a document could be assigned by the query) will be
calculated. (See "The Xapian::MSet" below).

The Xapian::MSet
~~~~~~~~~~~~~~~~

Query results are returned in an
`Xapian::MSet <apidoc/html/classXapian_1_1MSet.html>`_ object. The
results can be accessed using a
`Xapian::MSetIterator <apidoc/html/classXapian_1_1MSetIterator.html>`_
which returns the matches in descending sorted order of relevance (so
the most relevant document is first in the list). Each ``Xapian::MSet``
entry comprises a document id, and the weight calculated for that
document.

An ``Xapian::MSet`` also contains various information about the search
result:

firstitem
    The index of the first item in the result which was put into the MSet.
    (Corresponding to ``first`` in ``Xapian::Enquire::get_mset()``)
max_attained
    The greatest weight which is attained in the full results of the search.
max_possible
    The maximum possible weight in the MSet.
docs_considered
    The number of documents matching the query considered for the MSet. This
    provides a lower bound on the number of documents in the database which
    have a weight greater than zero. Note that this value may change if the
    search is recalculated with different values for ``first`` or
    ``max_items``.

See the `automatically extracted
documentation <apidoc/html/classXapian_1_1MSet.html>`_ for more details
of these fields.

The ``Xapian::MSet`` also provides methods for converting the score
calculated for a given document into a percentage value, suitable for
displaying to a user. This may be done using the
`convert_to_percent() <apidoc/html/classXapian_1_1MSet.html>`_
methods::

         int Xapian::MSet::convert_to_percent(const Xapian::MSetIterator & item) const
         int Xapian::MSet::convert_to_percent(Xapian::weight wt) const

These methods return a value in the range 0 to 100, which will be 0 if
and only if the item did not match the query at all.

Accessing a document
~~~~~~~~~~~~~~~~~~~~

A document in the database is accessed via a
`Xapian::Document <apidoc/html/classXapian_1_1Document.html>`_
object. This can be obtained by calling
`Xapian::Database::get_document() <apidoc/html/classXapian_1_1Database.html>`_.
The returned ``Xapian::Document`` is a reference counted handle so
copying is cheap.

Each document can have the following types of information associated
with it:

-  document data - this is an arbitrary block of data accessed using
   `Xapian::Document::get_data() <apidoc/html/classXapian_1_1Document.html>`_.
   The contents of the document data can be whatever you want and in
   whatever format. Often it contains fields such as a URL or other
   external UID, a document title, and an excerpt from the document
   text. If you wish to interoperate with Omega, it should contain
   name=value pairs, one per line (recent versions of Omega also support
   one field value per line, and can assign names to line numbers in the
   query template).
-  terms and positional information - terms index the document (like
   index entries in the back of a book); positional information records
   the word offset into the document of each occurrence of a particular
   term. This is used to implement phrase searching and the NEAR
   operator.
-  document values - these are arbitrary pieces of data which are stored
   so they can be accessed rapidly during the match process (to allow
   sorting collapsing of duplicates, etc). Each value is stored in a
   numbered slot so you can have several for each document. There's
   currently no length limit, but you should keep them short for
   efficiency.

There's some overlap in what you can do with terms and with values. A
simple boolean operator (e.g. document language) is definitely better
done using a term and OP\_FILTER.

Using a value allows you to do things you can't do with terms, such as
"sort by price", or "show only the best match for each website". You can
also perform filtering with a value which is more sophisticated than can
easily be achieved with terms, for example: find matches with a price
between $100 and $900. Omega uses boolean terms to perform date range
filtering, but this might actually be better done using a value (the
code in Omega was written before values were added to Xapian).

Specifying a relevance set
--------------------------

Xapian supports the idea of relevance feedback: that is, of allowing the
user to mark documents as being relevant to the search, and using this
information to modify the search. This is supported by means of
relevance sets, which are simply sets of document ids which are marked
as relevant. These are held in
`Xapian::RSet <apidoc/html/classXapian_1_1RSet.html>`_ objects, one
of which may optionally be supplied to Xapian in the ``rset``
parameter when calling ``Xapian::Enquire::get_mset()``.

Match options
~~~~~~~~~~~~~

There are various additional options which may be specified when
performing the query. These are specified by calling `various methods of
the Xapian::Enquire object <apidoc/html/classXapian_1_1Enquire.html>`_.
The options are as follows.

collapse key
    Each document in a database may have a set of numbered value slots. The
    contents of each value slot is a string of arbitrary length. The
    ``set_collapse_key(Xapian::valueno collapse_key)`` method specifies a
    value slot number upon which to remove duplicates. Only the most
    recently set duplicate removal key is active at any time, and the
    default is to perform no duplicate removal.
percentage cutoff
    It may occasionally be desirable to exclude any documents which have a
    weight less than a given percentage value. This may be done using
    ``set_cutoff(Xapian::percent percent_cutoff)``.
sort direction
    Some weighting functions may frequently result in several documents being
    returned with the same weight. In this case, by default, the documents will
    be returned in ascending document id order. This can be changed by using
    ``set_docid_order()`` to set the sort direction.

    ``set_docid_order(Xapian::Enquire::DESCENDING)`` may be useful, for
    example, when it would be best to return the newest documents, and new
    documents are being added to the end of the database (which is what happens
    by default).

Match decision functors
~~~~~~~~~~~~~~~~~~~~~~~

Sometimes it may be useful to return only documents matching criteria
which can't be easily represented by queries. This can be done using a
match decision functor. To set such a condition, derive a class from
``Xapian::MatchDecider`` and override the function operator,
``operator()(const Xapian::Document &doc)``. The operator can make a
decision based on the document values via
``Xapian::Document::get_value(Xapian::valueno)``.

The functor will also have access to the document data stored in the
database (via ``Xapian::Document::get_data()``), but beware that for
most database backends, this is an expensive operation to be calling
for a lot of documents, so doing that is likely to slow down the search
considerably.

Expand - Suggesting new terms for the query
-------------------------------------------

Xapian also supports the idea of calculating terms to add to the query,
based on the relevant documents supplied. A set of such terms, together
with their weights, may be returned by:
::

    Xapian::ESet Xapian::Enquire::get_eset(Xapian::termcount maxitems,
                               const Xapian::RSet & rset,
                   bool exclude_query_terms = true,
                   bool use_exact_termfreq = false,
                   double k = 1.0,
                   const Xapian::ExpandDecider * edecider = 0) const;
    Xapian::ESet Xapian::Enquire::get_eset(Xapian::termcount maxitems,
                               const Xapian::RSet & rset,
                               const Xapian::ExpandDecider * edecider) const

As for ``get_mset``, up to ``maxitems`` expand terms will be returned,
with fewer being returned if and only if no more terms could be found.

The expand terms are returned in sorted weight order in an
`Xapian::ESet <apidoc/html/classXapian_1_1ESet.html>`_ item.

exclude\_query\_terms
~~~~~~~~~~~~~~~~~~~~~

By default terms which are already in the query will never be returned
by ``get_eset()``. If ``exclude_query_terms`` is ``false``) then query
terms may be returned.

use\_exact\_termfreq
~~~~~~~~~~~~~~~~~~~~

By default, Xapian uses an approximation to the term frequency when
``get_eset()`` is called when searching over multiple databases. This
approximation improves performance, and usually still returns good
results. If you're willing to pay the performance penalty, you can get
Xapian to calculate the exact term frequencies by passing ``true`` for
``use_exact_termfreq``.

Expand decision functors
~~~~~~~~~~~~~~~~~~~~~~~~

It is often useful to allow only certain classes of term to be returned
in the expand set. For example, there may be special terms in the
database with various prefixes, which should be removed from the expand
set. This is accomplished by providing a decision functor. To do this,
derive a class from ``Xapian::ExpandDecider`` and override the function
operator, ``operator()(const string &)``. The functor is called with
each term before it is added to the set, and it may accept (by returning
``true``) or reject (by returning ``false``) the term as appropriate.

Thread safety
-------------

There's no pthread specific code in Xapian. If you want to use the same
object concurrently from different threads, it's up to you to police
access (with a mutex or in some other way) to ensure only one method is
being executed at once. The reason for this is to avoid adding the
overhead of locking and unlocking mutexes when they aren't required. It
also makes the Xapian code easier to maintain, and simplifies building
it.

For most applications, this is unlikely to be an issue - generally the
calls to Xapian are likely to be from a single thread. And if they
aren't, you can just create an entirely separate Xapian::Database object
in each thread - this is no different to accessing the same database
from two different processes.

Examples
--------

Examples of usage of Xapian are available in the examples
subdirectory of xapian-core.
