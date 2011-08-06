
.. Copyright (C) 2007,2010,2011 Olly Betts
.. Copyright (C) 2009 Lemur Consulting Ltd
.. Copyright (C) 2011 Richard Boulton

=======================
Xapian Faceting Support
=======================

.. contents:: Table of contents

Introduction
============

Xapian provides functionality which allows you to dynamically generate complete
lists of category values which feature in matching documents.  There are
numerous potential uses this can be put to, but a common one is to offer the
user the ability to narrow down their search by filtering it to only include
documents with a particular value of a particular category.  This is often
referred to as ``faceted search``.

You may have many multiple facets (for example colour, manufacturer, product
type) so Xapian allows you to handle multiple facets at once.

How to use Faceting
===================

Indexing
--------

When indexing a document, you need to add each facet in a different numbered
value slot.  As described elsewhere in the documentation, each Xapian document
has a set of "value slots", each of which is addressed by a number, and can
contain a value which is an arbitrary string.

The ``Xapian::Document::add_value()`` method can be used to put values into a
particular slot.  So, if you had a database of books, you might put "price"
facet values in slot 0, say (serialised to strings using
``Xapian::sortable_serialise``, or some similar function), "author" facet
values in slot 1, "publisher" facet values in slot 2 and "publication type"
(eg, hardback, softback, etc) values in slot 3.

Searching
---------

Finding Facets
~~~~~~~~~~~~~~

At search time, for each facet you want to consider, you need to get a count of
the number of times each facet value occurs in each slot; for the example
above, if you wanted to get facets for "price", "author" and "publication type"
you'd want to get the counts from slots 0, 1 and 3.

This can be done by calling ``Xapian::Enquire::add_matchspy()`` with a pointer
to a ``Xapian::ValueCountMatchSpy`` object for each value slot you want to
get facet counts for, like so::

    Xapian::ValueCountMatchSpy spy0(0);
    Xapian::ValueCountMatchSpy spy1(1);
    Xapian::ValueCountMatchSpy spy3(3);

    Xapian::Enquire enq(db);
    enq.add_matchspy(&spy0);
    enq.add_matchspy(&spy1);
    enq.add_matchspy(&spy3);

    enq.set_query(query);

    Xapian::MSet mset = enq.get_mset(0, 10, 10000);

The ``10000`` in the call to ``get_mset()`` tells Xapian to check at least
10000 documents, so the MatchSpy objects will be passed at least 10000
documents to tally facet information from (unless fewer than 10000 documents
match the query, in which case they will see all of them).  Setting this to
``db.get_doccount()`` will make the facet counts exact, but Xapian will have to
do more work for most queries so searches will be slower.

The ``spy`` objects now contain the facet information.  You can find out how
many documents they looked at by calling ``spy0.get_total()``.  (All the spies
will have looked at the same number of documents.)  You can read the values
from, say, ``spy0`` like this::

    Xapian::TermIterator i;
    for (i = spy0.values_begin(); i != spy0.values_end(); ++i) {
        cout << *i << ": " << i.get_termfreq() << endl;
    }

Restricting by Facet Values
~~~~~~~~~~~~~~~~~~~~~~~~~~~

If you're using the facets to offer the user choices for narrowing down
their search results, you then need to be able to apply a suitable filter.

For a single value, you could use ``Xapian::Query::OP_VALUE_RANGE`` with the
same start and end, or ``Xapian::MatchDecider``, but it's probably most
efficient to also index the categories as suitably prefixed boolean terms and
use those for filtering.
