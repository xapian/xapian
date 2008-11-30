
.. Copyright (C) 2008 Olly Betts

=====================
Xapian::PostingSource
=====================

.. contents:: Table of contents

Introduction
============

Xapian::PostingSource is an API class which you can subclass to feed data to
Xapian's matcher.  This feature can be made use of in a number of ways - for
example:

As a filter - a subclass could return a stream of document ids to filter a
query against.

As a weight boost - a subclass could return every document, but with a
varying weight so that certain documents receive a weight boost.  This could
be used to prefer documents based on some external factor, such as age,
price, proximity to a physical location, link analysis score, etc.

As an alternative way of ranking documents - if the weighting scheme is set
to Xapian::BoolWeight, then the ranking will be entirely by the weight
returned by Xapian::PostingSource.

Anatomy
=======

Three methods return statistics independent of the iteration position.
These are upper and lower bounds for the number of documents which can
be returned, and an estimate of this number::

    virtual Xapian::doccount get_termfreq_min() const = 0;
    virtual Xapian::doccount get_termfreq_max() const = 0;
    virtual Xapian::doccount get_termfreq_est() const = 0;

These methods are pure-virtual in the base class, so you have to define
them when deriving your subclass.

It must always be true that::

    get_termfreq_min() <= get_termfreq_est() <= get_termfreq_max()

PostingSources must always return documents in increasing document ID order.

After construction, a PostingSource points to a position *before* the first
document id - so before a docid can be read, the position must be advanced.

Two methods return weight related information - ``get_weight()`` returns
the weight for the current document, while ``get_maxweight()`` returns an
upper bound on what ``get_weight()`` can return *from now on*.  The weights
must always be >= 0::

    virtual Xapian::weight get_maxweight() const;
    virtual Xapian::weight get_weight() const;

These methods have default implementations which always return 0, for
convenience when deriving "weight-less" subclasses.

The ``at_end()`` method checks if the current iteration position is past the
last entry::

    virtual bool at_end() const = 0;

The ``get_docid()`` method returns the document id at the current iteration
position::

    virtual Xapian::docid get_docid() const = 0;

There are three methods which advance the current position.  All of these take
a Xapian::Weight parameter ``min_wt``, which indicates the minimum weight
contribution which the matcher is interested in.  The matcher still checks
the weight of documents so it's OK to ignore this parameter completely, or to
use it to discard only some documents.  But it can be useful for optimising
in some cases.

The simplest of these three methods is ``next()``, which simply advances the
iteration position to the next document (possibly skipping documents with
weight contribution < min_wt)::

    virtual void next(Xapian::weight min_wt) = 0;

Then there's ``skip_to()``.  This advances the iteration position to the next
document with document id >= that specified (possibly also skipping documents
with weight contribution < min_wt)::

    virtual void skip_to(Xapian::docid did, Xapian::weight min_wt);

A default implementation of ``skip_to()`` is provided which just calls
``next()`` repeatedly.  This works but ``skip_to()`` can often be implemented
much more efficiently.

The final method of this group is ``check()``.  In some cases, it's fairly
cheap to check if a given document matches, but the requirement that
``skip_to()`` must leave the iteration position on the next document is
rather costly to implement (for example, it might require linear scanning
of document ids).  To avoid this where possible, the ``check()`` method
allows the matcher to just check if a given document matches::

    virtual bool check(Xapian::docid did, Xapian::weight min_wt);

The return value is ``true`` if the method leaves the iteration position valid,
and ``false`` if it doesn't.  In the latter case, ``next()`` will advance to
the first matching position after document id ``did``, and ``skip_to()`` will
act as it would if the iteration position was the first matching position
after ``did``.

The default implementation of ``check()`` is just a thin wrapper around
``skip_to()`` which returns true - you should use this if ``skip_to()`` incurs
only a small extra cost.

In order to cope with being used more than once, there needs to be a way to
reset a PostingSource to its freshly constructed state.  This is provided
by the ``reset()`` method::

    virtual void reset() = 0;

There's also a method to return a string describing this object::

    virtual std::string get_description() const;

The default implementation returns a generic answer.  This default it provided
to avoid forcing you to provide an implementation if you don't really care
what ``get_description()`` gives for your sub-class.

Examples
========

FIXME: Provide some!

Current Limitations
===================

Xapian::PostingSource doesn't currently work well with multiple databases.  The
issue is that the document ids refer to those in the sub-database, but there's
no hint as to which sub-database they are in, and generally the same document
id can appear in more than one sub-database.

The remote backend isn't supported either yet.
