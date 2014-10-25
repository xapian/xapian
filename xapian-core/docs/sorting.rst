
.. Copyright (C) 2007,2009,2011,2014 Olly Betts

=========================
Sorting of Search Results
=========================

.. contents:: Table of contents

Introduction
============

By default, Xapian orders search results by decreasing relevance score.
However, it also allows results to be ordered by other criteria, or
a mixture of other criteria and relevance score.

If two or more results compare equal by the sorting criteria, then their order
is decided by their document ids.  By default, the document ids sort in
ascending order (so a lower document id is "better"), but this can be set
to descending using ``enquire.set_docid_order(enquire.DESCENDING);``.  If you
have no preference, you can tell Xapian to use whatever order is most efficient
using ``enquire.set_docid_order(enquire.DONT_CARE);``.

Sorting by Relevance
====================

The BM25 weighting formula which Xapian uses by default has a number of
parameters.  We have picked some default parameter values which do a good job
in general.  The optimal values of these parameters depend on the data being
indexed and the type of queries being run, so you may be able to improve the
effectiveness of your search system by adjusting these values, but it's a
fiddly process to tune them so people tend not to bother.

See the `BM25 documentation <bm25.html>`_ for more details of BM25.

The other included weighting schemes are ``TradWeight`` and ``BoolWeight``.

TradWeight implements the original probabilistic weighting formula, which is
essentially a special case of BM25 (it's BM25 with k2 = 0, k3 = 0, b = 1, and
min_normlen = 0, except that the weights are scaled by a constant factor).

BoolWeight assigns a weight of 0 to all documents, so the ordering is
determined solely by other factors.

You can also implement your own weighting scheme, provided it can be expressed
in the form of a sum over the matching terms, plus an extra term which depends
on term-independent statistics (such as the normalised document length).

For example, here's an implementation of "coordinate matching" - each matching
term scores one point::

    class CoordinateWeight : public Xapian::Weight {
      public:
	CoordinateWeight * clone() const { return new CoordinateWeight; }
	CoordinateWeight() { }
	~CoordinateWeight() { }

	std::string name() const { return "Coord"; }
	std::string serialise() const { return ""; }
	CoordinateWeight * unserialise(const std::string &) const {
	    return new CoordinateWeight;
	}

	double get_sumpart(Xapian::termcount, Xapian::termcount, Xapian::termcount) const {
            return 1;
        }
	double get_maxpart() const { return 1; }

	double get_sumextra(Xapian::termcount, Xapian::termcount) const { return 0; }
	double get_maxextra() const { return 0; }

	bool get_sumpart_needs_doclength() const { return false; }
    };

.. FIXME: add a more complex example once user-defined weight classes can
   see the statistics.

Sorting by Other Properties
===========================

If you want to offer a "sort by date" feature, and can arrange for documents to
be indexed in date order (or a close-enough approximation), then you can
implement a very efficient "sort by date" feature by using a boolean search
(i.e. call ``enquire.set_weighting_scheme(Xapian::BoolWeight());``) with
``enquire.set_docid_order(Xapian::Enquire::DESCENDING);`` (for newest first) or
``enquire.set_docid_order(Xapian::Enquire::ASCENDING);`` (for oldest first).
There's no inherent reason why this technique can't be used for sorting by
something other than date, but it's usually much easier to arrange for new
documents to arrive in date order than in other orders.

Sorting by Value
----------------

You can order documents by comparing a specified document value.  Note that the
comparison used compares the byte values in the value (i.e. it's a string sort
ignoring locale), so ``1`` < ``10`` < ``2``.  If you want to encode the value
such that it sorts numerically, use ``Xapian::sortable_serialise()`` to encode
values at index time - this works equally will on integers and floating point
values::

    Xapian::Document doc;
    doc.add_value(0, Xapian::sortable_serialise(price));

There are three methods which are used to specify how the value is used to
sort, depending if/how you want relevance used in the ordering:

 * ``Enquire::set_sort_by_value()`` specifies the relevance doesn't affect the
   ordering at all.
 * ``Enquire::set_sort_by_value_then_relevance()`` specifies that relevance is
   used for ordering any groups of documents for which the value is the same.
 * ``Enquire::set_sort_by_relevance_then_value()`` specifies that documents are
   ordered by relevance, and the value is only used to order groups of documents
   with identical relevance values (note: the weight has to be the same, not
   just the rounded percentage score).  This method isn't very useful with the
   default BM25 weighting, which rarely assigns identical scores to
   different documents.

Sorting by Generated Key
------------------------

To allow more elaborate sorting schemes, Xapian allows you to provide a functor
object subclassed from ``Xapian::KeyMaker`` which generates a sort key for each
matching document which is under consideration.  This is called at most once
for each document, and then the generated sort keys are ordered by comparing
byte values (i.e. with a string sort ignoring locale).

There's a standard subclass ``Xapian::MultiValueKeyMaker`` which allows sorting
on more than one document value (so the first document value specified
determines the order except among groups which have the same value, when
the second document value specified is used, and so on).

``Xapian::KeyMaker`` can also be subclassed to offer features such as "sort by
geographical distance".  A subclass could take a coordinate pair - e.g.
(latitude, longitude) - for the user's location and sort results using
coordinates stored in a document value so that the nearest results ranked
highest.
