
.. Copyright (C) 2009 Lemur Consulting Ltd
.. Copyright (C) 2009 Olly Betts

======================================
Serialisation of Queries and Documents
======================================

.. contents:: Table of contents

Introduction
============

In order to pass Query and Document objects to or from remote databases, Xapian
includes support for serialising these objects to strings, and then converting
these strings back into objects.  This support may be accessed directly, and
used for storing persistent representations of Query and Document objects.

Be aware that the serialised representation may occasionally change between
releases.  This will be clearly noted in the release notes.

Serialising Documents
=====================

To get a serialised document, simply call the ``Document::serialise()`` method
on the instance of the document::

    std::string serialise() const;

Documents are often lazily fetched from databases: this method will first force
the full document contents to be fetched from the database, in order to
serialise them.  The serialised document will have identical contents (data,
terms, positions, values) to the original document.

To get a document from a serialised form, call the static
``Document::unserialise()`` method, passing it the string returned from
``serialise()``::

    static Document unserialise(const std::string &s);

Serialising Queries
===================

Serialisation of queries is very similar to serialisation of documents: there
is a ``Query::serialise()`` method to produce a serialised Query, and a
corresponding ``Query::unserialise()`` method to produce a Query from a
serialised representation::

    std::string serialise() const;
    static Query unserialise(const std::string &s);

However, there is a wrinkle.  Queries can contain arbitrary user-defined
PostingSource subqueries.  In order to serialise and unserialise such queries,
all the PostingSource subclasses used in the query must implement the
``name()``, ``serialise()`` and ``unserialise()`` methods (see the
`postingsource topic document <postingsource.html>`_ for details of these).
In addition, a special form of unserialise must be used::

    static Query unserialise(const std::string & s, const Registry & registry);

The ``Registry`` passed to this method must know about all the
custom posting sources used in the query.  You can tell a Registry
about a custom posting source using the
``Registry::register_posting_source`` method::

    void register_posting_source(const Xapian::PostingSource &source);

Note that Registry objects always know about built-in posting sources
(such as ``ValueWeightPostingSource``), so you don't need to call
``register_posting_source()`` for them.
