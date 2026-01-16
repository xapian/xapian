Xapian::QueryParser Syntax
==========================

This document describes the query syntax supported by the
Xapian::QueryParser class. The syntax is designed to be similar to other
web based search engines, so that users familiar with them don't have to
learn a whole new syntax.

Operators
---------

AND
~~~

*expression* AND *expression* matches documents which are matched by
both of the subexpressions.

OR
~~

*expression* OR *expression* matches documents which are matched by
either of the subexpressions.

NOT
~~~

*expression* NOT *expression* matches documents which are matched by
only the first subexpression. This can also be written as *expression*
AND NOT *expression*. If ``FLAG_PURE_NOT`` is enabled, then
NOT *expression* will match documents which don't match the
subexpression.

XOR
~~~

*expression* XOR *expression* matches documents which are matched by one
or other of the subexpressions, but not both. XOR is probably a bit
esoteric.

'+' and '-'
~~~~~~~~~~~

A group of terms with some marked with + and - will match documents
containing all of the + terms, but none of the - terms. Terms not marked
with + or - contribute towards the document rankings. You can also use +
and - on phrases and on bracketed expressions.

NEAR
~~~~

``one NEAR two NEAR three`` matches documents containing those words
within 10 words of each other. You can set the threshold to *n* by using
``NEAR/n`` like so: ``one NEAR/6 two``.

ADJ
~~~

``ADJ`` is like ``NEAR`` but only matches if the words appear in the
same order as in the query. So ``one ADJ two ADJ three`` matches
documents containing those three words in that order and within 10 words
of each other. You can set the threshold to *n* by using ``ADJ/n`` like
so: ``one ADJ/6 two``.

SYN
~~~

``SYN`` matches when any of its subqueries match (like ``OR`` does)
but the ranking is done assuming the subqueries are synonyms and so treats the
entire ``SYN`` subquery as a single term.

Bracketed expressions
~~~~~~~~~~~~~~~~~~~~~

You can control the precedence of operators using brackets.
In the query ``one OR two AND three`` the AND takes precedence, so this
is the same as ``one OR (two AND three)``. You can override the
precedence using ``(one OR two) AND three``.

The default precedence from highest to lowest is:

* SYN
* +, - (equal)
* NEAR, ADJ (equal)
* AND, NOT (equal)
* XOR
* OR


Phrase searches
~~~~~~~~~~~~~~~

A phrase surrounded with double quotes ("") matches documents containing
that exact phrase. Hyphenated words are also treated as phrases, as are
cases such as filenames and email addresses (e.g. ``/etc/passwd`` or
``president@whitehouse.gov``).

Searching within a free-text field
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

If the database has been indexed with prefixes on terms generated from
certain free-text fields, you can set up a prefix map so that the user can
search within those fields. For example ``author:dickens title:shop``
might find documents by dickens with shop in the title. You can also
specify a prefix on a quoted phrase (e.g. ``author:"charles dickens"``)
or on a bracketed subexpression (e.g. ``title:(mice men)``).

Searching for proper names
~~~~~~~~~~~~~~~~~~~~~~~~~~

If a query term is entered with a capitalised first letter, then it will
be searched for unstemmed.

Range searches
~~~~~~~~~~~~~~

The QueryParser `can be configured to support
range-searching <valueranges.html>`_ using document values.

The syntax for a range search is ``start..end`` - for example,
``01/03/2007..04/04/2007``, ``$10..100``, ``5..10kg``.

Open-ended ranges are also supported - an empty start or end is
interpreted as no limit, for example: ``..2010-06-17``, ``$10..``,
``$..100``, ``..5kg``.

Synonyms
~~~~~~~~

The QueryParser can be configured to support synonyms, which can either
be used when explicitly specified (using the syntax ``~term``) or
implicitly (synonyms will be used for all terms or groups of terms for
which they have been specified).

Wildcards
~~~~~~~~~

The QueryParser supports using wildcards, but this support is not
enabled by default.  Matching wildcard queries is inherently more
work which may be problematic for heavily used search systems.

Prior to Xapian 2.0.0, only a trailing ``*`` wildcard was supported.
This matches any number of trailing characters, so ``wildc*`` would match
wildcard, wildcarded, wildcards, wildcat, wildcats, etc.  This wildcard
mode is enabled by passing ``Xapian::QueryParser::FLAG_WILDCARD`` in the flags
argument of ``Xapian::QueryParser::parse_query(query_string, flags)``.

(In Xapian 1.2.x you also needed to tell the QueryParser which database to
expand wildcards from using the ``QueryParser::set_database(database)`` method.
Since Xapian 1.3.3 wildcards are only expanded when ``Enquire::get_mset()``
is called, and expansion now uses the database being searched.)

Xapian 2.0.0 added an "extended wildcard" feature, which supports
both ``*`` (matching zero or more characters) and ``?`` (matching
exactly one character).  These can be used anywhere in the term,
and can appear multiple times in a term.  Extended wildcards are
enabled using flag ``Xapian::QueryParser::FLAG_WILDCARD_GLOB``
(or ``Xapian::QueryParser::FLAG_WILDCARD_MULTI`` if you only
want to support ``*``, or ``Xapian::QueryParser::FLAG_WILDCARD_SINGLE`` if you
only want to support ``?``).  A term cannot consist entirely of wildcards.

You can specify a minimum length for the fixed initial portion in
wildcard pattern with ``QueryParser::set_min_wildcard_prefix()``, for example
to prevent users searching for ``e*`` which would expand to thousands of term
and be a fairly slow query.  By default there is no minimum length, so with
extended wildcards users can use wildcards at the start of a term.

You can limit the number of terms a wildcard will expand to by
calling ``Xapian::QueryParser::set_max_expansion()``.  This supports
several different modes, and can also be used to limit expansion
performed via ``FLAG_PARTIAL`` - see the API documentation for
details.  By default, there's no limit on wildcard expansion and
``FLAG_PARTIAL`` expands to the most frequent 100 terms.

Partially entered query matching
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The QueryParser also supports performing a search with a query which has
only been partially entered. This is intended for use with "incremental
search" systems, which don't wait for the user to finish typing their
search before displaying an initial set of results. For example, in such
a system a user would enter a search, and the system would display a new
set of results after each letter, or whenever the user pauses for a
short period of time (or some other similar strategy).

The problem with this kind of search is that the last word in a
partially entered query often has no semantic relation to the completed
word. For example, a search for "dynamic cat" would return a quite
different set of results to a search for "dynamic categorisation". This
results in the set of results displayed flicking rapidly as each new
character is entered. A much smoother result can be obtained if the
final word is treated as having an implicit terminating wildcard, so
that it matches all words starting with the entered characters - thus,
as each letter is entered, the set of results displayed narrows down to
the desired subject.

A similar effect could be obtained simply by enabling the wildcard
matching option, and appending a "\*" character to each query string.
However, this would be confused by searches which ended with punctuation
or other characters.

This feature is disabled by default - pass
``Xapian::QueryParser::FLAG_PARTIAL`` flag in the flags argument of
``Xapian::QueryParser::parse_query(query_string, flags)`` to enable it,
and tell the QueryParser which database to expand wildcards from using
the ``QueryParser::set_database(database)`` method.
