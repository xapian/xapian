.. Copyright (C) 2007 Olly Betts

========================================
Xapian 1.0 Term Indexing/Querying Scheme
========================================

.. contents:: Table of contents

Introduction
============

In Xapian 1.0, the default indexing scheme has been changed significantly, to address
lessons learned from observing the old scheme in real world use.  This document
describes the new scheme, with references to differences from the old.

Stemming
========

The most obvious difference is the handling of stemmed forms.

Previously all words were indexed stemmed without a prefix, and capitalised words were
indexed unstemmed (but lower cased) with an 'R' prefix.  The rationale for doing this was
that people want to be able to search for exact proper nouns (e.g. the English stemmer
conflates ``Tony`` and ``Toni``).  But of course this also indexes words at the start
of sentences, words in titles, and in German all nouns are capitalised so will be indexed.
Both the normal and R-prefixed terms were indexed with positional information.

Now we index all words lowercased with positional information, and also stemmed with a
'Z' prefix (unless they start with a digit), but without positional information.  By default
a Xapian::Stopper is used to avoid indexed stemmed forms of stopwords (tests show this shaves
around 1% off the database size).

The new scheme allows exact phrase searching (which the old scheme didn't).  ``NEAR``
now has to operate on unstemmed forms, but that's reasonable enough.  We can also disable
stemming of words which are capitalised in the query, to achieve good results for
proper nouns.  And Omega's $topterms will now always suggest unstemmed forms!

The main rationale for prefixing the stemmed forms is that there are simply fewer of
them!  As a side benefit, it opens the way for storing stemmed forms for multiple
languages (e.g. Z:en:, Z:fr: or something like that).

The special handling of a trailing ``.`` in the QueryParser (which would often
mistakenly trigger for pasted text) has been removed.  This feature was there to
support Omega's topterms adding stemmed forms, but Omega no longer needs to do this
as it can suggest unstemmed forms instead.

Word Characters
===============

By default, Unicode characters of category CONNECTOR_PUNCTUATION (``_`` and a
handful of others) are now word characters, which provides better indexing of
identifiers, without much degradation of other cases.  Previously cases like
``time_t`` required a phrase search.

Trailing ``+`` and ``#`` are still included on terms (up to 3 characters at most), but
``-`` no longer is by default.  The examples it benefits aren't compelling
(``nethack--``, ``Cl-``) and it tends to glue hyphens on to terms.

A single embedded ``'`` (apostrophe) is now included in a term.
Previously this caused a slow phrase search, and added junk terms to the index
(``didn't`` -> ``didn`` and ``t``, etc).  Various Unicode characters used for apostrophes
are all mapped to the ASCII representation.

A few other characters (taken from the Unicode definition of a word) are included
in terms if they occur between two word characters, and ``.``, ``,`` and a
few others are included in terms if they occur between two decimal digit characters.
