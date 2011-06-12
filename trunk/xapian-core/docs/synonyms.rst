
.. Copyright (C) 2007,2008 Olly Betts

======================
Xapian Synonym Support
======================

.. contents:: Table of contents

Introduction
============

Xapian provides support for storing a synonym dictionary, or thesaurus.  This
can be used by the Xapian::QueryParser class to expand terms in user query
strings, either automatically, or when requested by the user with an explicit
synonym operator (``~``).

Note that Xapian doesn't offer automated generation of the synonym dictionary.

Model
=====

The model for the synonym dictionary is that a term or group of consecutive
terms can have one or more synonym terms.  A group of consecutive terms is
specified in the dictionary by simply joining them with a single space between
each one.

QueryParser Integration
=======================

In order for any of the synonym features of the QueryParser to work, you must
call ``QueryParser::set_database()`` to specify the database to use.

If ``FLAG_SYNONYM`` is passed to ``QueryParser::parse_query()`` then the
QueryParser will recognise ``~`` in front of a term as indicating a request for
synonym expansion.  If ``FLAG_LOVEHATE`` is also specified, you can use ``+``
and ``-`` before the ``~`` to indicate that you love or hate the synonym
expanded expression.

A synonym-expanded term becomes the term itself OR-ed with any listed synonyms,
so ``~truck`` might expand to ``truck OR lorry OR van``.  A group of terms is
handled in much the same way.

If a term to be synonym expanded will be stemmed by the QueryParser, then
synonyms will be checked for the unstemmed form first, and then for the stemmed
form, so you can provide different synonyms for particular unstemmed forms
if you want to.

If ``FLAG_AUTO_SYNONYMS`` is passed to ``QueryParser::parse_query()`` then the
QueryParser will automatically expand any term which has synonyms, unless the
term is in a phrase or similar.

If ``FLAG_AUTO_MULTIWORD_SYNONYMS`` is passed to ``QueryParser::parse_query()``
then the QueryParser will look at groups of terms separated only by whitespace
and try to expand them as term groups.  This is done in a "greedy" fashion, so
the first term which can start a group is expanded first, and the longest group
starting with that term is expanded.  After expansion, the QueryParser will
look for further possible expansions starting with the term after the last
term in the expanded group.

Current Limitations
===================

Explicit multi-word synonyms
----------------------------

There ought to be a way to explicitly request expansion of multi-term synonyms,
probably with the syntax ``~"stock market"``.  This hasn't been implemented
yet though.

Backend Support
---------------

Currently synonyms are only supported by chert, flint and brass databases.
They work
with a single database or multiple databases (use Database::add_database() as
usual).  We've no plans to support them for the InMemory backend, but we do
intend to support them for the remote backend in the future.
