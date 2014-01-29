.. Copyright (C) 2009,2011 Olly Betts

============================
Collapsing of Search Results
============================

.. contents:: Table of contents

Introduction
============

Xapian provides the ability to eliminate "duplicate" documents from the MSet.
This feature is known as "collapsing" - think of a pile of duplicates being
collapsed down to leave a single result (or a small number of results).

The collapsing always removes the worse ranked documents (if ranking by
relevance, those with the lowest weight; if ranking by sorting, those which
sort lowest).

Whether two documents count as duplicates of one another is determined by their
"collapse key".  If a document has an empty collapse key, it will never be
collapsed, but otherwise documents with the same collapse key will be collapsed
together.

Currently the collapse key is taken from a value slot you specify (via the
method ``Enquire::set_collapse_key()``), but in the future you should be able
to build collapse keys dynamically using ``Xapian::KeyMaker`` as you already
can for sort keys.

Performance
===========

The collapsing is performed during the match process, so is pretty efficient.
In particular, this approach is much better than generating a larger MSet and
post-processing it.

However, if the collapsing eliminates a lot of documents then the collapsed
search will typically take rather longer than the uncollapsed search because
the matcher has to consider many more potential matches.

API
===

To enable collapsing, call the method ``Enquire::set_collapse_key`` with the
value slot, and optionally the number of matches with each collapse key to keep
(this defaults to 1 if not specified), e.g.::

    // Collapse on value slot 4, leaving at most 2 documents with each
    // collapse key.
    enquire.set_collapse_key(4, 2);

Once you have the ``MSet`` object, you can read the collapse key for each
match with ``MSetIterator::get_collapse_key()``, and also the "collapse count"
with ``MSetIterator::get_collapse_count()``.  The latter is a lower bound on
the number of documents with the same collapse key which collapsing eliminated.

Beware that if you have a percentage cutoff active, then the collapse count
will (at least in the current implementation) will always be either 0 or 1
as it is hard to tell if the collapsed documents would have failed the cutoff.

Statistics
==========

As well as the usual bounds and estimate of the "full" MSet size (i.e. the
size if you'd asked for enough matches to get them all), the matcher also
calculates bounds and an estimate for what the MSet size would be if collapsing
had not been used - you can obtain these using these methods::

    Xapian::doccount get_uncollapsed_matches_lower_bound() const;
    Xapian::doccount get_uncollapsed_matches_estimated() const;
    Xapian::doccount get_uncollapsed_matches_upper_bound() const;

Examples
========

Here are some ways this feature can be used:

Duplicate Elimination
---------------------

If your document collection includes some identical documents, it's unhelpful
when these show up in the search results.  Sometimes it is possible to
eliminate them at index time, but this isn't always feasible.

If you store a checksum (e.g. SHA1 or MD5) of the document contents and store
this in a document value then you can collapse on this to eliminate such
duplicates.

If the document files will be identical, then the checksum can just be of the
file, but sometimes it makes sense to extract and normalise the text, then
calculate the checksum of this.

Restricting the Number of Matches per Source
--------------------------------------------

It's sometimes desirable to avoid one source dominating the results.  For
example, in a web search application, you might want to show at most three
matches from any website, in which case you could collapse on the hostname
with collapse_max set to 3.

When displaying the results, you can use the collapse count of each match
to inform the user that there are at least that many other matches for this
host (unless you are also using a percentage cutoff - see above).  If it is
non-zero it means you can usefully provide a "show all documents for host
<get_collapse_key()>" button which reruns the search without collapsing and
with a boolean filter for a prefixed term containing the hostname (though note
that this may not always give a button when there are collapsed documents
because the collapse count is a lower bound and may be zero when there are
collapsed matches with the same key).

This approach isn't just useful for web search - the "source" can be defined
usefully in many applications.  For example, a forum or mailing list search
could collapse on a topic or thread identifier, an index at the chapter level
could collapse on a book identifier (such as an ISBN), etc.
