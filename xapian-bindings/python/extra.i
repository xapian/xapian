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

        *args holds the list of properties or property names to be returned, in
        the order they are returned by the sequence API.
        
        If an item in the list is a string, it is considered to be a property
        name; otherwise, it is considered to be a property value, and is
        returned without doing an attribute lookup.  (Yes, this is a nasty
        hack.  No, I don't care, because this is only a temporary piece of
        internal code.)

        """
        self._sequence_items = args

    def __len__(self):
        """Get the length of the sequence.

        Doesn't evaluate any of the lazily evaluated properties.

        """
        return len(self._sequence_items)

    def _get_single_item(self, index):
        """Get a single item.

        Used by __getitem__ to get individual items.

        """
        if not isinstance(index, basestring):
             return index
        return getattr(self, index)

    def __getitem__(self, key):
        """Get an item, or a slice of items, from the sequence.

        If any of the items are lazily evaluated properties, they will be
        evaluated here.

        """
        if isinstance(key, slice):
            return [self._get_single_item(i) for i in self._sequence_items[key]]
        return self._get_single_item(self._sequence_items[key])

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

    The item supports access to the following attributes and properties:

     - `docid`: The Xapian document ID corresponding to this MSet item.
     - `weight`: The weight corresponding to this MSet item.
     - `rank`: The rank of this MSet item.  The rank is the position in the
       total set of matching documents of this item.  The highest document is
       given a rank of 0.  If the MSet did not start at the highest matching
       document, because a non-zero `start` parameter was supplied to
       get_mset(), the first document in the MSet will have a rank greater than
       0 (in fact, it will be equal to the value of `start` supplied to
       get_mset()).
     - `percent`: The percentage score assigned to this MSet item.
     - `document`: The document for this MSet item.  This can be used to access
       the document data, or any other information stored in the document (such
       as term lists).  It is lazily evaluated.
     - `collapse_key`: The value of the key which was used for collapsing.
     - `collapse_count`: An estimate of the number of documents that have been
       collapsed into this one.

    The collapse count estimate will always be less than or equal to the actual
    number of other documents satisfying the match criteria with the same
    collapse key as this document.  If may be 0 even though there are other
    documents with the same collapse key which satisfying the match criteria.
    However if this method returns non-zero, there definitely are other such
    documents.  So this method may be used to inform the user that there are
    "at least N other matches in this group", or to control whether to offer a
    "show other documents in this group" feature (but note that it may not
    offer it in every case where it would show other documents).

    """

    __slots__ = ('_iter', '_mset', '_firstitem', 'docid', 'weight', 'rank',
                 'percent', 'collapse_key', 'collapse_count', '_document', )

    def __init__(self, iter, mset):
        self._iter = iter
        self._mset = mset
        self._firstitem = self._mset.get_firstitem()
        self.docid = iter.get_docid()
        self.weight = iter.get_weight()
        self.rank = iter.get_rank()
        self.percent = iter.get_percent()
        self.collapse_key = iter.get_collapse_key()
        self.collapse_count = iter.get_collapse_count()
        self._document = None
        _SequenceMixIn.__init__(self, 'docid', 'weight', 'rank', 'percent', 'document')

    def _get_document(self):
        if self._document is None:
            self._document = self._mset.get_hit(self.rank - self._firstitem).get_document()
        return self._document

    document = property(_get_document, doc="The document object corresponding to this MSet item.")

class MSetIter(object):
    """An iterator over the items in an MSet.

    The iterator will return MSetItem objects, which will be evaluated lazily
    where appropriate.

    """
    __slots__ = ('_iter', '_end', '_mset')
    def __init__(self, mset):
        self._iter = mset.begin()
        self._end = mset.end()
        self._mset = mset

    def __iter__(self):
        return self

    def next(self):
        if self._iter == self._end:
            raise StopIteration
        else:
            r = MSetItem(self._iter, self._mset)
            self._iter.next()
            return r


# Modify the MSet to allow access to the python iterators, and have other
# convenience methods.

def _mset_gen_iter(self):
    """Return an iterator over the MSet.

    The iterator will return MSetItem objects, which will be evaluated lazily
    where appropriate.

    """
    return MSetIter(self)
MSet.__iter__ = _mset_gen_iter

MSet.__len__ = MSet.size

def _mset_getitem(self, index):
    """Get an item from the MSet.

    The supplied index is relative to the start of the MSet, not the absolute
    rank of the item.

    """
    return MSetItem(self.get_hit(index), self)
MSet.__getitem__ = _mset_getitem

def _mset_contains(self, index):
    """Check if the Mset contains an item at the given index

    The supplied index is relative to the start of the MSet, not the absolute
    rank of the item.

    """
    return key >= 0 and key < len(self)
MSet.__contains__ = _mset_contains


##################################
# Support for iteration of ESets #
##################################

class ESetItem(_SequenceMixIn):
    """An item returned from iteration of the ESet.

    The item supports access to the following attributes:

     - `term`: The term corresponding to this ESet item.
     - `weight`: The weight corresponding to this ESet item.

    """
    __slots__ = ('term', 'weight')

    def __init__(self, iter):
        self.term = iter.get_term()
        self.weight = iter.get_weight()
        _SequenceMixIn.__init__(self, 'termname', 'weight')

class ESetIter(object):
    """An iterator over the items in an ESet.

    The iterator will return ESetItem objects.

    """
    __slots__ = ('_iter', '_end')
    def __init__(self, eset):
        self._iter = eset.begin()
        self._end = eset.end()

    def __iter__(self):
        return self

    def next(self):
        if self._iter == self._end:
            raise StopIteration
        else:
            r = ESetItem(self._iter)
            self._iter.next()
            return r

# Modify the ESet to allow access to the python iterators, and have other
# convenience methods.

def _eset_gen_iter(self):
    """Return an iterator over the ESet.
    
    The iterator will return ESetItem objects.

    """
    return ESetIter(self)
ESet.__iter__ = _eset_gen_iter

ESet.__len__ = ESet.size


#######################################
# Support for iteration of term lists #
#######################################

class TermListItem(_SequenceMixIn):
    """An item returned from iteration of a term list.

    """
    __slots__ = ('_iter', 'term', '_wdf', '_termfreq')

    def __init__(self, iter, term):
        self._iter = iter
        self.term = term
        self._wdf = None
        self._termfreq = None

        # Support for sequence API
        sequence = ['term', 'wdf', 'termfreq', 'positer']
        if not (iter._has & TermIter.HAS_WDF):
            sequence[1] = 0
        if not (iter._has & TermIter.HAS_TERMFREQS):
            sequence[2] = 0
        if not (iter._has & TermIter.HAS_POSITIONS):
            sequence[3] = PositionIter()
        _SequenceMixIn.__init__(self, *sequence)

    def _get_wdf(self):
        """Get the within document frequency.

        """
        if self._wdf is None:
            if not (self._iter._has & TermIter.HAS_WDF):
                raise InvalidOperationError("Iterator does not support wdfs")
            self._wdf = self._iter._iter.get_wdf()
        return self._wdf
    wdf = property(_get_wdf, doc=
    """The within-document-frequency of the current term (if meaningful).

    This will raise a InvalidOperationError exception if the iterator
    this item came from doesn't support within-document-frequencies.

    """)

    def _get_termfreq(self):
        """Get the term frequency.

        """
        if self._termfreq is None:
            if not (self._iter._has & TermIter.HAS_TERMFREQS):
                raise InvalidOperationError("Iterator does not support term frequencies")
            self._termfreq = self._iter._iter.get_termfreq()
        return self._termfreq
    termfreq = property(_get_termfreq, doc=
    """The term frequency of the current term (if meaningful).

    This will raise a InvalidOperationError exception if the iterator
    this item came from doesn't support term frequencies.

    """)

    def _get_positer(self):
        """Get a position list iterator.

        """
        if not (self._iter._has & TermIter.HAS_POSITIONS):
            raise InvalidOperationError("Iterator does not support position lists")
        return PositionIter(self._iter._iter.positionlist_begin(),
                            self._iter._iter.positionlist_end())
    positer = property(_get_positer, doc=
    """A position iterator for the current term (if meaningful).

    This will raise a InvalidOperationError exception if the iterator
    this item came from doesn't support position lists.

    """)


class TermIter(object):
    """An iterator over a term list.

    The iterator will return TermListItem objects, which will be evaluated
    lazily where appropriate.

    """
    __slots__ = ('_iter', '_end', '_has', '_lastterm', '_moved')

    HAS_NOTHING = 0
    HAS_TERMFREQS = 1
    HAS_POSITIONS = 2
    HAS_WDF = 4

    def __init__(self, start, end, has=HAS_NOTHING):
        self._iter = start
        self._end = end
        self._has = has
        self._lastterm = None # Used to test if the iterator has moved
        self._moved = True # True if we've moved onto the next item.

    def __iter__(self):
        return self

    def next(self):
        """Move the iterator to the next item, and return it.

        Raises StopIteration if there are no more items.

        """
        if not self._moved:
            self._iter.next()
            self._moved = True

        if self._iter == self._end:
            raise StopIteration
        else:
            newterm = self._iter.get_term()
            r = TermListItem(self, newterm)
            self._lastterm = newterm
            self._moved = False
            return r

    def skip_to(self, term):
        """Skip the iterator forward.

        The iterator is advanced to the first term at or after the current
        position which is greater than or equal to the supplied term.

        """
        self._iter.skip_to(term)

        # Update self._lastterm if the iterator has moved.
        # TermListItems compare a saved value of lastterm with self._lastterm
        # with the object identity comparator, so it is important to ensure
        # that it does not get modified if the new term compares equal.
        newterm = self._iter.get_term()
        if newterm != self._lastterm:
            self._lastterm = newterm
        self._moved = True

# Modify Enquire to add a "matching_terms()" method.
def _enquire_gen_iter(self, which):
    """Get an iterator over the terms which match a given match set item.

    The match set item to consider is specified by the `which` parameter, which
    may be a document ID, or an MSetItem object.

    The iterator will return TermListItem objects, but these will not support
    access to term frequency, wdf, or position information.

    """
    if isinstance(which, MSetItem):
        which = which.docid
    return TermIter(self.get_matching_terms_begin(which),
                    self.get_matching_terms_end(which))
Enquire.matching_terms = _enquire_gen_iter


##########################################
# Support for iteration of posting lists #
##########################################

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


###########################################
# Support for iteration of position lists #
###########################################

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


########################################
# Support for iteration of value lists #
########################################

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

def _query_gen_iter(self):
    # The C++ VectorTermList always returns 1 for wdf, but there's a FIXME
    # suggesting we make it throw Xapian::InvalidOperationError instead.
    return TermIter(self.get_terms_begin(), self.get_terms_end())
Query.__iter__ = _query_gen_iter

def _database_gen_allterms_iter(self):
    return TermIter(self.allterms_begin(), self.allterms_end(),
                    TermIter.HAS_TERMFREQS)
Database.__iter__ = _database_gen_allterms_iter
Database.allterms = _database_gen_allterms_iter

def _database_gen_postlist_iter(self, tname):
    if len(tname) != 0:
        return PostingIter(self.postlist_begin(tname), self.postlist_end(tname), PostingIter.HAS_POSITIONS)
    else:
        return PostingIter(self.postlist_begin(tname), self.postlist_end(tname))
Database.postlist = _database_gen_postlist_iter

def _database_gen_termlist_iter(self, docid):
    return TermIter(self.termlist_begin(docid), self.termlist_end(docid), TermIter.HAS_TERMFREQS|TermIter.HAS_POSITIONS|TermIter.HAS_WDF)
Database.termlist = _database_gen_termlist_iter

def _database_gen_positionlist_iter(self, docid, tname):
    return PositionIter(self.positionlist_begin(docid, tname), self.positionlist_end(docid, tname))
Database.positionlist = _database_gen_positionlist_iter

def _document_gen_termlist_iter(self):
    return TermIter(self.termlist_begin(), self.termlist_end(), TermIter.HAS_POSITIONS|TermIter.HAS_WDF)
Document.__iter__ = _document_gen_termlist_iter
Document.termlist = _document_gen_termlist_iter

def _document_gen_values_iter(self):
    return ValueIter(self.values_begin(), self.values_end())
Document.values = _document_gen_values_iter

def _queryparser_gen_stoplist_iter(self):
    # The C++ VectorTermList always returns 1 for wdf, but there's a FIXME
    # suggesting we make it throw Xapian::InvalidOperationError instead.
    return TermIter(self.stoplist_begin(), self.stoplist_end())
QueryParser.stoplist = _queryparser_gen_stoplist_iter

def _queryparser_gen_unstemlist_iter(self, tname):
    # The C++ VectorTermList always returns 1 for wdf, but there's a FIXME
    # suggesting we make it throw Xapian::InvalidOperationError instead.
    return TermIter(self.unstem_begin(tname), self.unstem_end(tname))
QueryParser.unstemlist = _queryparser_gen_unstemlist_iter

%}
/* vim:syntax=python:set expandtab: */
