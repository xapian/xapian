
.. Copyright (C) 2007 Olly Betts
.. Copyright (C) 2009 Lemur Consulting Ltd

=============================
Xapian Categorisation Support
=============================

.. contents:: Table of contents

Introduction
============

Xapian provides functionality which allows you to dynamically generate complete
lists of category values which feature in matching documents.  There are
numerous potential uses this can be put to, but a common one is to offer the
user the ability to narrow down their search by filtering it to only include
documents with a particular value of a particular category.  This is often
referred to as ``faceted search``.

Some categories are numeric and can take many different values (examples
include price, width, and height).  The number of different values will often
be overwhelming, and users will generally be more interested in narrowing their
search to a range rather than a single value.  For these, Xapian can group the
results into ranges for you.

In some applications, you may have many different categories (for example
colour, price, width, height) but not always want to offer all of them
for every search.  If all the results are red, and none have width, it's
not useful to offer to narrow the search by colour or width.  Also, the
user interface may not have room to include every category, so you may
want to select the "best" few categories to show the user.

How to make use of the categorisation functionality
===================================================

Indexing
--------

When indexing a document, you need to add each category in a different
number value slot.  For numeric values which you want to be able to
group, you should encode the numeric value as a string using
``Xapian::sortable_serialise()``.

Searching
---------

At search time, you need to pass a ``Xapian::ValueCountMatchSpy`` object for
each category you want to look at to ``Xapian::Enquire::add_matchspy()``, like
so::

    Xapian::ValueCountMatchSpy spy0(0);
    Xapian::ValueCountMatchSpy spy1(1);
    Xapian::ValueCountMatchSpy spy3(3);

    Xapian::Enquire enq(db);
    enq.add_matchspy(spy0);
    enq.add_matchspy(spy1);
    enq.add_matchspy(spy3);

    enq.set_query(query);

    Xapian::MSet mset = enq.get_mset(0, 10, 10000);

The ``10000`` in the call to ``get_mset()`` tells Xapian to check at least
10000 documents, so the MatchSpies will be passed at least 10000 documents
to tally category information from (unless fewer than 10000 documents match the
query, in which case it will see all of them).  Setting this higher will make
the counts exact, but Xapian will have to do more work for most queries so
searches will be slower.

The ``spy`` objects now contain the category information.  You can find out how
many documents they looked at by calling ``spy0.get_total()``.  (All the spies
will have looked at the same number of documents.)  You can read the values
from, say, ``spy0`` like this::

    const map<string, size_t> & cat = spy0.get_values();
    map<string, size_t>::const_iterator i;
    for (i = cat.begin(); i != cat.end(); ++i) {
        cout << i->first << ": " << i->second << endl;
    }

You can build ranges from numeric values for the values returned from spy
``spy0``, asking for at most ``num_ranges`` ranges like so::

    std::map<Xapian::NumericRange, Xapian::doccount> result;
    Xapian::doccount values_seen;
    values_seen = build_numeric_ranges(result, spy0.get_values(), num_ranges);

Here, ``result`` will be filled with a set of numeric ranges (holding at most
``num_ranges`` ranges), and ``values_seen`` will be the count of the number of
values seen (note - this may be different from the number of documents seen by
the matchspy, since some may have no value stored in the slot).

If there are no values seen by the spy, ``result`` will be empty.  If all the
values seen by the spy are the same, ``result`` will contain a single entry,
with a single range with the same start and end points.

Restricting by category values
------------------------------

If you're using the categorisation to offer the user choices for narrowing down
their search results, you then need to be able to apply a suitable filter.

For a range, the easiest way is to use ``Xapian::Query::OP_VALUE_RANGE`` to
build a filter query, and then combine this with the user's query using
``Xapian::Query::OP_FILTER``.

For a single value, you could use ``Xapian::Query::OP_VALUE_RANGE`` with the
same start and end, or ``Xapian::MatchDecider``, but it's probably most
efficient to also index the categories as suitably prefixed boolean terms and
use those for filtering.

Current Limitations
===================

It's not currently possible to build logarithmic ranges with
``build_numeric_ranges``.
