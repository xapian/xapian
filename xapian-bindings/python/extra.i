%{
/* python/extra.i: Xapian scripting python interface additional code.
 *
 * Copyright (C) 2003,2004,2005 James Aylett
 * Copyright (C) 2005,2006,2007 Olly Betts
 * Copyright (C) 2007 Lemur Consulting Ltd
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301
 * USA
 */
%}

%pythoncode %{

class _SequenceMixIn(object):
    """Simple mixin class which provides a sequence API to a class.

    This is used to support the legacy API to iterators used for releases of
    Xapian earlier than 1.0.  It will be removed once this legacy API is
    removed in release 1.1.

    """

    __slots__ = ('_sequence_items', )
    def __init__(self, *args):
        """Initialise the sequence.

        *args holds the list of property names to be returned, in the order
        they are returned by the sequence API.

        """
        self._sequence_items = args

    def __len__(self):
        """Get the length of the sequence.

        Doesn't evaluate any of the lazily evaluated properties.

        """
        return len(self._sequence_items)

    def __getitem__(self, key):
        """Get an item, or a slice of items, from the sequence.

        If any of the items are lazily evaluated properties, they will be
        evaluated here.

        """
        if isinstance(key, slice):
            return [getattr(self, i) for i in self._sequence_items[key]]
        i = self._sequence_items[key]
        return getattr(self, i)

    def __iter__(self):
        """Make an iterator for over the sequence.

        This simply copies the items into a list, and returns an iterator over
        it.  Any lazily evaluated properties will be evaluated here.

        """
        return iter(self[:])


##################################
# Support for iteration of MSets #
##################################

class MSetItem(_SequenceMixIn):
    """An item returned from iteration of the MSet.

    The item support access to the following attributes and properties:
    `docid`, `weight`, `rank`, `percent`, `document`, `collapse_count`

    The document property is lazily evaluated when first requested.

    """

    __slots__ = ('_iter', '_mset', '_firstitem', 'docid', 'weight', 'rank',
                 'percent', 'collapse_count', '_document', )

    def __init__(self, iter, mset):
        self._iter = iter
        self._mset = mset
        self._firstitem = self._mset.get_firstitem()
        self.docid = iter.get_docid()
        self.weight = iter.get_weight()
        self.rank = iter.get_rank()
        self.percent = iter.get_percent()
        self.collapse_count = iter.get_collapse_count()
        self._document = None
        _SequenceMixIn.__init__(self, 'docid', 'weight', 'rank', 'percent', 'document')

    def _get_document(self):
        if self._document is None:
            self._document = self._mset.get_hit(self.rank - self._firstitem).get_document()
        return self._document

    document = property(_get_document, doc="Get the document ")

class MSetIter(object):
    """An iterator over the items in an MSet.

    The items returned are evaluated lazily where appropriate.

    """
    __slots__ = ('_iter', '_end', '_mset')
    def __init__(self, mset):
        self._iter = mset.begin()
        self._end = mset.end()
        self._mset = mset

    def __iter__(self):
        return self

    def next(self):
        if self._iter==self._end:
            raise StopIteration
        else:
            r = MSetItem(self._iter, self._mset)
            self._iter.next()
            return r


# Modify the MSet to allow access to the python iterators, and have other
# convenience methods.

def _mset_gen_iter(self):
    """Return an iterator over the MSet.

    """
    return MSetIter(self)
MSet.__iter__ = _mset_gen_iter

MSet.__len__ = MSet.size

def _mset_getitem(self, index):
    """Get and item from the MSet.

    The supplied index is relative to the start of the MSet, not the absolute
    rank of the item.

    """
    return MSetItem(self.get_hit(index), self)
MSet.__getitem__ = _mset_getitem

def _mset_contains(self, index):
    """Check if the mset contains an item at the given index

    The supplied index is relative to the start of the MSet, not the absolute
    rank of the item.

    """
    return key >= 0 and key < len(self)
MSet.__contains__ = _mset_contains


##################################
# Support for iteration of ESets #
##################################


class ESetIter(object):
    def __init__(self, start, end):
        self.iter = start
        self.end = end

    def __iter__(self):
        return self

    def next(self):
        if self.iter==self.end:
            raise StopIteration
        else:
            r = [self.iter.get_termname(), self.iter.get_weight()]
            self.iter.next()
            return r

class TermIter(object):
    HAS_NOTHING = 0
    HAS_TERMFREQS = 1
    HAS_POSITIONS = 2
    HAS_WDF = 4

    def __init__(self, start, end, has = HAS_NOTHING):
        self.iter = start
        self.end = end
        self.has = has

    def __iter__(self):
        return self

    def next(self):
        if self.iter==self.end:
            raise StopIteration
        else:
            termfreq = 0
            if self.has & TermIter.HAS_TERMFREQS:
                termfreq = self.iter.get_termfreq()
            if self.has & TermIter.HAS_POSITIONS:
                positer = PositionIter(self.iter.positionlist_begin(), self.iter.positionlist_end())
            else:
                positer = PositionIter()
            if self.has & TermIter.HAS_WDF:
                wdf = self.iter.get_wdf()
            else:
                wdf = 0
            r = [self.iter.get_term(), wdf, termfreq, positer]
            self.iter.next()
            return r

    def skip_to(self, term):
        self.iter.skip_to(term)

class PostingIter(object):
    HAS_NOTHING = 0
    HAS_POSITIONS = 1

    def __init__(self, start, end, has=HAS_NOTHING):
        self.iter = start
        self.end = end
        self.has = has

    def __iter__(self):
        return self

    def next(self):
        if self.iter==self.end:
            raise StopIteration
        else:
            if self.has & PostingIter.HAS_POSITIONS:
                r = [self.iter.get_docid(), self.iter.get_doclength(), self.iter.get_wdf(), PositionIter(self.iter.positionlist_begin(), self.iter.positionlist_end())]
            else:
                r = [self.iter.get_docid(), self.iter.get_doclength(), self.iter.get_wdf(), PositionIter()]
            self.iter.next()
            return r

class PositionIter(object):
    def __init__(self, start = 0, end = 0):
        self.iter = start
        self.end = end

    def __iter__(self):
        return self

    def next(self):
        if self.iter==self.end:
            raise StopIteration
        else:
            r = self.iter.get_termpos()
            self.iter.next()
            return r

class ValueIter(object):
    def __init__(self, start, end):
        self.iter = start
        self.end = end

    def __iter__(self):
        return self

    def next(self):
        if self.iter==self.end:
            raise StopIteration
        else:
            r = [self.iter.get_valueno(), self.iter.get_value()]
            self.iter.next()
            return r

# Bind the Python iterators into the shadow classes

def eset_gen_iter(self):
    return ESetIter(self.begin(), self.end())

ESet.__iter__ = eset_gen_iter

def enquire_gen_iter(self, which):
    # The C++ VectorTermList always returns 1 for wdf, but there's a FIXME
    # suggesting we make it throw Xapian::InvalidOperationError instead.
    return TermIter(self.get_matching_terms_begin(which), self.get_matching_terms_end(which))

Enquire.matching_terms = enquire_gen_iter

def query_gen_iter(self):
    # The C++ VectorTermList always returns 1 for wdf, but there's a FIXME
    # suggesting we make it throw Xapian::InvalidOperationError instead.
    return TermIter(self.get_terms_begin(), self.get_terms_end())

Query.__iter__ = query_gen_iter

def database_gen_allterms_iter(self):
    return TermIter(self.allterms_begin(), self.allterms_end(), TermIter.HAS_TERMFREQS)

Database.__iter__ = database_gen_allterms_iter

def database_gen_postlist_iter(self, tname):
    if len(tname) != 0:
        return PostingIter(self.postlist_begin(tname), self.postlist_end(tname), PostingIter.HAS_POSITIONS)
    else:
        return PostingIter(self.postlist_begin(tname), self.postlist_end(tname))
def database_gen_termlist_iter(self, docid):
    return TermIter(self.termlist_begin(docid), self.termlist_end(docid), TermIter.HAS_TERMFREQS|TermIter.HAS_POSITIONS|TermIter.HAS_WDF)
def database_gen_positionlist_iter(self, docid, tname):
    return PositionIter(self.positionlist_begin(docid, tname), self.positionlist_end(docid, tname))

Database.allterms = database_gen_allterms_iter
Database.postlist = database_gen_postlist_iter
Database.termlist = database_gen_termlist_iter
Database.positionlist = database_gen_positionlist_iter

def document_gen_termlist_iter(self):
    return TermIter(self.termlist_begin(), self.termlist_end(), TermIter.HAS_POSITIONS|TermIter.HAS_WDF)
def document_gen_values_iter(self):
    return ValueIter(self.values_begin(), self.values_end())

Document.__iter__ = document_gen_termlist_iter
Document.termlist = document_gen_termlist_iter
Document.values = document_gen_values_iter

def queryparser_gen_stoplist_iter(self):
    # The C++ VectorTermList always returns 1 for wdf, but there's a FIXME
    # suggesting we make it throw Xapian::InvalidOperationError instead.
    return TermIter(self.stoplist_begin(), self.stoplist_end())
def queryparser_gen_unstemlist_iter(self, tname):
    # The C++ VectorTermList always returns 1 for wdf, but there's a FIXME
    # suggesting we make it throw Xapian::InvalidOperationError instead.
    return TermIter(self.unstem_begin(tname), self.unstem_end(tname))

QueryParser.stoplist = queryparser_gen_stoplist_iter
QueryParser.unstemlist = queryparser_gen_unstemlist_iter

%}
/* vim:syntax=python:set expandtab: */
