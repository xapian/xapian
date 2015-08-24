
.. Copyright (C) 2008,2009,2010,2011,2013 Olly Betts
.. Copyright (C) 2008,2009 Lemur Consulting Ltd

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

When first constructed, a PostingSource is not tied to a particular database.
Before Xapian can get any postings (or statistics) from the source, it needs to
be supplied with a database.  This is performed by the init() method, which is
passed a single parameter holding the database to use.  This method will always
be called before asking for any information about the postings in the list.  If
a posting source is used for multiple searches, the init() method will be
called before each search; implementations must cope with init() being called
multiple times, and should always use the database provided in the most recent
call::

    virtual void init(const Xapian::Database & db) = 0;

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
document id - so before a docid can be read, the position must be advanced
by calling ``next()``, ``skip_to()`` or ``check()``.

The ``get_weight()`` method returns the weight that you want to contribute
to the current document.  This weight must always be >= 0::

    virtual Xapian::weight get_weight() const;

The default implementation of ``get_weight()`` returns 0, for convenience when
deriving "weight-less" subclasses.

You also need to specify an upper bound on the value which ``get_weight()`` can
return, which is used by the matcher to perform various optimisations.  You
should try hard to find a bound for efficiency, but if there really isn't one
then you can set ``DBL_MAX``::

    void get_maxweight(Xapian::weight max_weight);

This method specifies an upper bound on what ``get_weight()`` will return *from
now on* (until the next call to ``init()``).  So if you know that the upper
bound has decreased, you should call ``set_maxweight()`` with the new reduced
bound.

One thing to be aware of is that currently calling ``set_maxweight()`` during
the match triggers an recursion through the postlist tree to recalculate the
new overall maxweight, which takes a comparable amount of time to calculating
the weight for a matching document.  If your maxweight reduces for nearly
every document, you may want to profile to see if it's beneficial to notify
every single change.  Experiments with a modified ``FixedWeightPostingSource``
which forces a pointless recalculation for every document suggest a worst case
overhead in search times of about 37%, but reports of profiling results for
real world examples are most welcome.  In real cases, this overhead could
easily be offset by the extra scope for matcher optimisations which a tighter
maxweight bound allows.

A simple approach to reducing the number of calculations is only to do it every
N documents.  If it's cheap to calculate the maxweight in your posting source,
a more sophisticated strategy might be to decide an absolute maximum number of
times to update the maxweight (say 100) and then to call it whenever::

    last_notified_maxweight - new_maxweight >= original_maxweight / 100.0

This ensures that only reasonably significant drops result in a recalculation
of the maxweight.

Since ``get_weight()`` must always return >= 0, the upper bound must clearly
also always be >= 0 too.  If you don't call ``get_maxweight()`` then the
bound defaults to 0, to match the default implementation of ``get_weight()``.

If you want to read the currently set upper bound, you can call::

    Xapian::weight get_maxweight() const;

This is just a getter method for a member variable in the
``Xapian::PostingSource`` class, and is inlined from the API headers, so
there's no point storing this yourself in your subclass - it should be just as
efficient to call ``get_maxweight()`` whenever you want to use it.

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

There's also a method to return a string describing this object::

    virtual std::string get_description() const;

The default implementation returns a generic answer.  This default is provided
to avoid forcing you to provide an implementation if you don't really care
what ``get_description()`` gives for your sub-class.

Examples
========

Here is an example of a Python PostingSource which contributes additional
weight from some external source (note that in Python, you call ``next()``
on an iterator to get each item, including the first, which is exactly
the semantics we need to implement here)::

    class ExternalWeightPostingSource(xapian.PostingSource):
	"""
	A Xapian posting source returning weights from an external source.
	"""
	def __init__(self, db, wtsource):
	    xapian.PostingSource.__init__(self)
	    self.db = db
	    self.wtsource = wtsource

	def init(self, db):
	    self.alldocs = db.postlist('')

	def get_termfreq_min(self): return 0
	def get_termfreq_est(self): return self.db.get_doccount()
	def get_termfreq_max(self): return self.db.get_doccount()

	def next(self, minweight):
	    try:
		self.current = self.alldocs.next()
	    except StopIteration:
		self.current = None

	def skip_to(self, docid, minweight):
	    try:
		self.current = self.alldocs.skip_to(docid)
	    except StopIteration:
		self.current = None

	def at_end(self):
	    return self.current is None

	def get_docid(self):
	    return self.current.docid

	def get_maxweight(self):
	    return self.wtsource.get_maxweight()

	def get_weight(self):
	    doc = self.db.get_document(self.current.docid)
	    return self.wtsource.get_weight(doc)

ExternalWeightPostingSource doesn't restrict which documents match - it's
intended to be combined with an existing query using `OP_AND_MAYBE` like so::

    extwtps = xapian.ExternalWeightPostingSource(db, wtsource)
    query = xapian.Query(query.OP_AND_MAYBE, query, xapian.Query(extwtps))

The wtsource would be a class like this one::

    class WeightSource(object):
	def get_maxweight(self):
	    return 12.34;

	def get_weight(self, doc):
	    return some_func(doc.get_docid())

.. FIXME: Provide some more examples!
.. FIXME "why you might want to do this" (e.g. scenario) too

Multiple databases, and remote databases
========================================

In order to work with searches across multiple databases, or in remote
databases, some additional methods need to be implemented in your
Xapian::PostingSource subclass.  The first of these is ``clone()``, which is
used for multi database searches.  This method should just return a newly
allocated instance of the same posting source class, initialised in the same
way as the source that clone() was called on.  The returned source will be
deallocated by the caller (using "delete" - so you should allocate it with
"new").

If you don't care about supporting searches across multiple databases, you can
simply return NULL from this method.  In fact, the default implementation does
this, so you can just leave the default implementation in place.  If
``clone()`` returns NULL, an attempt to perform a search with multiple
databases will raise an exception::

    virtual PostingSource * clone() const;

To work with searches across remote databases, you need to implement a few more
methods.  Firstly, you need to implement the ``name()`` method.  This simply
returns the name of your posting source (fully qualified with any namespace)::

    virtual std::string name() const;

Next, you need to implement the serialise and unserialise methods.  The
``serialise()`` method converts all the settings of the PostingSource to a
string, and the ``unserialise()`` method converts one of these strings back
into a PostingSource.  Note that the serialised string doesn't need to include
any information about the current iteration position of the PostingSource::

    virtual std::string serialise() const;
    virtual PostingSource * unserialise(const std::string &s) const;

Finally, you need to make a remote server which knows about your PostingSource.
Currently, the only way to do this is to modify the source slightly, and
compile your own xapian-tcpsrv.  To do this, you need to edit
``xapian-core/bin/xapian-tcpsrv.cc`` and find the
``register_user_weighting_schemes()`` function.  If ``MyPostingSource`` is your
posting source, at the end of this function, add these lines::

    Xapian::Registry registry;
    registry.register_postingsource(MyPostingSource());
    server.set_registry(registry);
