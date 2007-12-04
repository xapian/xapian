
.. Copyright (C) 2007 Olly Betts

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
--------------------

The BM25 weighting formula which Xapian uses by default has a number of parameters.
The optimal values of these depends on the data being indexed and the type of
queries being run, so we have picked some default parameter values which do a
good job in general.

.. Explain more about altering BM25 parameters

The other included weighting schemes are ``TradWeight`` and ``BoolWeight``.

.. Explain details of other included weighting scheme

You can also implement your own weighting scheme, provide it can be expressed in
a particular form.

.. Details of writing your own weighting scheme

Sorting by Value
----------------

If you want to offer a "sort by date" feature, and can arrange for documents to
be indexed in date order (or a close-enough approximation), then you can implement
a very efficient "sort by date" feature by using a boolean search (i.e. call
``enquire.set_weighting_scheme(Xapian::BoolWeight());``) with
``enquire.set_docid_order(Xapian::Enquire::DESCENDING);`` (for newest first) or
``enquire.set_docid_order(Xapian::Enquire::ASCENDING);`` (for oldest first).
There's no inherent reason why this technique can't be used for sorting by
something other than date, but it's usually much easier to arrange for new
documents to arrive in date order than in other orders.

.. set_sort_by_value
.. set_sort_by_value_then_relevance
.. set_sort_by_relevance_then_value

Sorting by Generated Key
------------------------

.. discuss

.. sort by geographical distance from coordinates
