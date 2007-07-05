
.. Copyright (C) 2007 Olly Betts

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
documents with a particular value of a particular category.

Some categories are numeric and can take many different values (examples
include price, width, and height).  The number of different values will often
be overwhelming, and users will generally be more interested in narrowing their
search to a range rather than a single value.  For these, Xapian can group the
results into ranges for you.

In some applications, you may have many different categories (for example
colour, price, width, height) but not always want to offer all of them
for every search.  If all the results are red, and none have width, it's
not useful to offer to narrow the search by colour or width.  Also, the
user interface may not have room to include every category, so you 

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

At search time, you need to pass a ``Xapian::MatchSpy`` object to
``Xapian::Enquire::get_mset()``, like so::

    Xapian::MatchSpy spy;

    spy.add_category(0);
    spy.add_category(1);
    spy.add_category(3);

    Xapian::Enquire enq(db);

    enq.set_query(query);

    Xapian::MSet mset = enq.get_mset(0, 10, 10000, NULL, NULL, &spy);

The ``10000`` in the call to ``get_mset`` tells Xapian to check at least
10000 documents, so the ``spy`` object will be passed at least 10000 documents
to tally category information from (unless less than 10000 documents match
the query, in which case it will see all of them).  Setting this higher will
make the counts exact, but Xapian will have to do more work for most queries
so searches will be slower.

The ``spy`` object now contains the category information.  You can find out
how many documents it looked at by calling ``spy.get_total()``.  You can
read the values for category ``cat_no`` like this::

    const map<string, size_t> & cat = spy.get_categories(cat_no);
    map<string, size_t>::const_iterator i;
    for (i = cat.begin(); i != cat.end(); ++i) {
        cout << i->first << ": " << i->second << endl;
    }

You calculate the score for category ``cat_no`` like so::

    double score = spy.score_categorisation(cat_num);

Or if you prefer categories with 4 or 5 values::

    double score = spy.score_categorisation(cat_num, 4.5);

The smaller the score, the better - a perfectly even split with exactly the
number of entries asked (or with no preference given for the number of entries)
scores 0.  You should experiment to find a suitable threshold for your
application, but to give you a rough idea, a suitable threshold is likely to be
less than one.

The scoring uses a sum of squared differences (currently that is - this should
probably be regarded as an implementation detail which could change in the
future if we find a better algorithm).

You would build ranges from numeric values for value ``cat_no``, asking for at
more ``num_ranges`` ranges like so::

    bool result = spy.build_numeric_ranges(cat_no, num_ranges);

If ranges could not be built (for example, because all documents have the
same value for ``cat_no``), ``false`` is returned.  Otherwise ``true`` is
returned, and the spy object's category map for value ``cat_no`` is modified
to consist of ranges.  Keys are now built of strings returned by
``Xapian::sortable_serialise()`` - either a single string if there is only
one number in a particular range, or for a range a string padded to 9 bytes
with zero bytes, with a second string appended.

Restricting by category values
------------------------------

If you're using the categorisation to offer the user choices for narrowing
down their search results, you then need to be able to apply a suitable
filter.

For a range, the best way is to use ``Xapian::Query::OP_VALUE_RANGE`` to
build a filter query, and then combine this with the user's query using
``Xapian::Query::OP_FILTER``.

For a single value, you could use ``Xapian::Query::OP_VALUE_RANGE`` with
the same start and end, or ``Xapian::MatchDecider``, but it's probably
most efficient to also index the categories as suitably prefixed boolean
terms and use those for filtering.

Current Limitations
===================

It's not currently possible to build logarithmic ranges without writing
your own subclass.

It's not possible to try building different ranges because the original
data is overwritten.  If it's actually useful to do this, the API needs
adjusting.
