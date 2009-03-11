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

# Set the documentation format - this is used by tools like "epydoc" to decide
# how to format the documentation strings.
__docformat__ = "restructuredtext en"

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
       document, because a non-zero 'start' parameter was supplied to
       get_mset(), the first document in the MSet will have a rank greater than
       0 (in fact, it will be equal to the value of 'start' supplied to
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

    __slots__ = ('_mset', '_firstitem', 'docid', 'weight', 'rank',
                 'percent', 'collapse_key', 'collapse_count', '_document', )

    def __init__(self, iter, mset):
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
            self._document = self._mset._get_hit_internal(self.rank - self._firstitem).get_document()
        return self._document

    # Deprecated methods: to be removed in 1.1.0
    def get_docid(self):
        "Deprecated method: use the `docid` property instead."
        return self.docid
    def get_weight(self):
        "Deprecated method: use the `weight` property instead."
        return self.weight
    def get_rank(self):
        "Deprecated method: use the `rank` property instead."
        return self.rank
    def get_percent(self):
        "Deprecated method: use the `percent` property instead."
        return self.percent
    def get_collapse_key(self):
        "Deprecated method: use the `collapse_key` property instead."
        return self.collapse_key
    def get_collapse_count(self):
        "Deprecated method: use the `collapse_count` property instead."
        return self.collapse_count
    def get_document(self):
        "Deprecated method: use the `document` property instead."
        return self.document

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

# We replace the get_hit() method with one which returns an MSetItem.  We first
# have to copy the internal method, so that we can call it.
MSet._get_hit_internal = MSet.get_hit
def _mset_getitem(self, index):
    """Get an item from the MSet.

    The supplied index is relative to the start of the MSet, not the absolute
    rank of the item.

    Returns an MSetItem.

    """
    if index < 0:
        index += len(self)
    if index < 0 or index >= len(self):
        raise IndexError("Mset index out of range")
    return MSetItem(self._get_hit_internal(index), self)
MSet.__getitem__ = _mset_getitem
MSet.get_hit = _mset_getitem

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
        _SequenceMixIn.__init__(self, 'term', 'weight')

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

    The item supports access to the following attributes and properties:

     - `term`: The term corresponding to this TermListItem.
     - `wdf`: The within document frequency of this term.
     - `termfreq`: The number of documents in the collection which are indexed
       by the term
     - `positer`: An iterator over the positions which the term appears at in
       the document.  This is only available until the iterator which returned
       this item next moves.

    """
    __slots__ = ('_iter', 'term', '_wdf', '_termfreq')

    def __init__(self, iter, term):
        self._iter = iter
        self.term = term
        self._wdf = None
        self._termfreq = None

        if iter._has_wdf == TermIter.EAGER:
            self._wdf = iter._iter.get_wdf()
        if iter._has_termfreq == TermIter.EAGER:
            self._termfreq = iter._iter.get_termfreq()

        # Support for sequence API
        sequence = ['term', 'wdf', 'termfreq', 'positer']
        if iter._has_wdf == TermIter.INVALID:
            sequence[1] = 0
        if iter._has_termfreq == TermIter.INVALID:
            sequence[2] = 0
        if iter._has_positions == TermIter.INVALID:
            sequence[3] = PositionIter()
        _SequenceMixIn.__init__(self, *sequence)

    def _get_wdf(self):
        """Get the within-document-frequency of the current term.

        This will raise a InvalidOperationError exception if the iterator this
        item came from doesn't support within-document-frequencies.

        """
        if self._wdf is None:
            if self._iter._has_wdf == TermIter.INVALID:
                raise InvalidOperationError("Iterator does not support wdfs")
            if self.term is not self._iter._lastterm:
                raise InvalidOperationError("Iterator has moved, and does not support random access")
            self._wdf = self._iter._iter.get_wdf()
        return self._wdf
    wdf = property(_get_wdf, doc=
    """The within-document-frequency of the current term (if meaningful).

    This will raise a InvalidOperationError exception if the iterator
    this item came from doesn't support within-document-frequencies.

    """)

    def _get_termfreq(self):
        """Get the term frequency.

        This is the number of documents in the collection which are indexed by
        the term.

        This will raise a InvalidOperationError exception if the iterator this
        item came from doesn't support term frequencies.

        """
        if self._termfreq is None:
            if self._iter._has_termfreq == TermIter.INVALID:
                raise InvalidOperationError("Iterator does not support term frequencies")
            if self.term is not self._iter._lastterm:
                raise InvalidOperationError("Iterator has moved, and does not support random access")
            self._termfreq = self._iter._iter.get_termfreq()
        return self._termfreq
    termfreq = property(_get_termfreq, doc=
    """The term frequency of the current term (if meaningful).

    This is the number of documents in the collection which are indexed by the
    term.

    This will raise a InvalidOperationError exception if the iterator
    this item came from doesn't support term frequencies.

    """)

    def _get_positer(self):
        """Get a position list iterator.

        The iterator will return integers representing the positions that the
        term occurs at.

        This will raise a InvalidOperationError exception if the iterator this
        item came from doesn't support position lists, or if the iterator has
        moved on since the item was returned from it.

        """
        if self._iter._has_positions == TermIter.INVALID:
            raise InvalidOperationError("Iterator does not support position lists")
        # Access to position lists is always lazy, so we don't need to check
        # _has_positions.
        if self.term is not self._iter._lastterm:
            raise InvalidOperationError("Iterator has moved, and does not support random access")
        return PositionIter(self._iter._iter.positionlist_begin(),
                            self._iter._iter.positionlist_end())
    positer = property(_get_positer, doc=
    """A position iterator for the current term (if meaningful).

    The iterator will return integers representing the positions that the term
    occurs at.

    This will raise a InvalidOperationError exception if the iterator this item
    came from doesn't support position lists, or if the iterator has moved on
    since the item was returned from it.

    """)


class TermIter(object):
    """An iterator over a term list.

    The iterator will return TermListItem objects, which will be evaluated
    lazily where appropriate.

    """
    __slots__ = ('_iter', '_end', '_has_termfreq', '_has_wdf',
                 '_has_positions', '_return_strings', '_lastterm', '_moved')

    INVALID = 0
    LAZY = 1
    EAGER = 2

    def __init__(self, start, end, has_termfreq=INVALID,
                 has_wdf=INVALID, has_positions=INVALID,
                 return_strings=False):
        self._iter = start
        self._end = end
        self._has_termfreq = has_termfreq
        self._has_wdf = has_wdf
        self._has_positions = has_positions
        assert(has_positions != TermIter.EAGER) # Can't do eager access to position lists
        self._return_strings = return_strings
        self._lastterm = None # Used to test if the iterator has moved

        # _moved is True if we've moved onto the next item.  This is needed so
        # that the iterator doesn't have to move on until just before next() is
        # called: since the iterator starts by pointing at a valid item, we
        # can't just call self._iter.next() unconditionally at the start of our
        # next() method.
        self._moved = True

    def __iter__(self):
        return self

    def next(self):
        if not self._moved:
            self._iter.next()
            self._moved = True

        if self._iter == self._end:
            self._lastterm = None
            raise StopIteration
        else:
            self._lastterm = self._iter.get_term()
            self._moved = False
            if self._return_strings:
                return self._lastterm
            return TermListItem(self, self._lastterm)

    def skip_to(self, term):
        """Skip the iterator forward.

        The iterator is advanced to the first term at or after the current
        position which is greater than or equal to the supplied term.

        If there are no such items, this will raise StopIteration.

        This returns the item which the iterator is moved to.  The subsequent
        item will be returned the next time that next() is called (unless
        skip_to() is called again first).

        """
        if self._iter != self._end:
            self._iter.skip_to(term)

        if self._iter == self._end:
            self._lastterm = None
            self._moved = True
            raise StopIteration

        # Update self._lastterm if the iterator has moved.
        # TermListItems compare a saved value of lastterm with self._lastterm
        # with the object identity comparator, so it is important to ensure
        # that it does not get modified if the new term compares equal.
        newterm = self._iter.get_term()
        if newterm != self._lastterm:
            self._lastterm = newterm

        self._moved = False
        if self._return_strings:
            return self._lastterm
        return TermListItem(self, self._lastterm)

# Modify Enquire to add a "matching_terms()" method.
def _enquire_gen_iter(self, which):
    """Get an iterator over the terms which match a given match set item.

    The match set item to consider is specified by the `which` parameter, which
    may be a document ID, or an MSetItem object.

    The iterator will return string objects.

    """
    if isinstance(which, MSetItem):
        which = which.docid
    return TermIter(self.get_matching_terms_begin(which),
                    self.get_matching_terms_end(which),
                    return_strings=True)
Enquire.matching_terms = _enquire_gen_iter

# get_matching_terms() is deprecated, but does just the same as
# matching_terms()
Enquire.get_matching_terms = _enquire_gen_iter

# Modify Query to add an "__iter__()" method.
def _query_gen_iter(self):
    """Get an iterator over the terms in a query.

    The iterator will return string objects.

    """
    return TermIter(self.get_terms_begin(),
                    self.get_terms_end(),
                    return_strings=True)
Query.__iter__ = _query_gen_iter

# Modify Database to add an "__iter__()" method and an "allterms()" method.
def _database_gen_allterms_iter(self, prefix=None):
    """Get an iterator over all the terms in the database.

    The iterator will return TermListItem objects, but these will not support
    access to wdf, or position information.

    Access to term frequency information is only available until the iterator
    has moved on.

    If prefix is supplied, only terms which start with that prefix will be
    returned.

    """
    if prefix is None:
        return TermIter(self.allterms_begin(), self.allterms_end(),
                        has_termfreq=TermIter.LAZY)
    else:
        return TermIter(self.allterms_begin(prefix), self.allterms_end(prefix),
                        has_termfreq=TermIter.LAZY)
Database.__iter__ = _database_gen_allterms_iter
Database.allterms = _database_gen_allterms_iter

# Modify Database to add a "termlist()" method.
def _database_gen_termlist_iter(self, docid):
    """Get an iterator over all the terms which index a given document ID.

    The iterator will return TermListItem objects.

    Access to term frequency and position information is only available until
    the iterator has moved on.

    """
    # Note: has_termfreq is set to LAZY because most databases don't store term
    # frequencies in the termlist (because this would require updating many termlist
    # entries for every document update), so access to the term frequency requires a
    # separate lookup.
    return TermIter(self.termlist_begin(docid), self.termlist_end(docid),
                    has_termfreq=TermIter.LAZY,
                    has_wdf=TermIter.EAGER,
                    has_positions=TermIter.LAZY)
Database.termlist = _database_gen_termlist_iter

# Modify Database to add a "spellings()" method.
def _database_gen_spellings_iter(self):
    """Get an iterator which returns all the spelling correction targets

    The iterator will return TermListItem objects.  Only the term frequency is
    available; wdf and positions are not meaningful.

    """
    return TermIter(self.spellings_begin(), self.spellings_end(),
                    has_termfreq=TermIter.EAGER,
                    has_wdf=TermIter.INVALID,
                    has_positions=TermIter.INVALID)
Database.spellings = _database_gen_spellings_iter

# Modify Database to add a "synonyms()" method.
def _database_gen_synonyms_iter(self, term):
    """Get an iterator which returns all the synonyms for a given term.

    The term to return synonyms for is specified by the `term` parameter.

    The iterator will return string objects.

    """
    return TermIter(self.synonyms_begin(term),
                    self.synonyms_end(term),
                    return_strings=True)
Database.synonyms = _database_gen_synonyms_iter

# Modify Database to add a "synonym_keys()" method.
def _database_gen_synonym_keys_iter(self, prefix=""):
    """Get an iterator which returns all the terms which have synonyms.

    The iterator will return string objects.

    If `prefix` is non-empty, only terms with this prefix are returned.

    """
    return TermIter(self.synonym_keys_begin(prefix),
                    self.synonym_keys_end(prefix),
                    return_strings=True)
Database.synonym_keys = _database_gen_synonym_keys_iter

# Modify Database to add a "metadata_keys()" method, instead of direct access
# to metadata_keys_begin and metadata_keys_end.
def _database_gen_metadata_keys_iter(self, prefix=""):
    """Get an iterator which returns all the metadata keys.

    The iterator will return string objects.

    If `prefix` is non-empty, only metadata keys with this prefix are returned.

    """
    return TermIter(self._metadata_keys_begin(prefix),
                    self._metadata_keys_end(prefix),
                    return_strings=True)
Database.metadata_keys = _database_gen_metadata_keys_iter
Database._metadata_keys_begin = Database.metadata_keys_begin
del Database.metadata_keys_begin
Database._metadata_keys_end = Database.metadata_keys_end
del Database.metadata_keys_end


# Modify Document to add an "__iter__()" method and a "termlist()" method.
def _document_gen_termlist_iter(self):
    """Get an iterator over all the terms in a document.

    The iterator will return TermListItem objects.

    Access to term frequency and position information is only available until
    the iterator has moved on.

    Note that term frequency information is only meaningful for a document
    retrieved from a database.  If term frequency information is requested for
    a document which was freshly created, an InvalidOperationError will be
    raised.

    """
    # Note: document termlist iterators may be implemented entirely in-memory
    # (in which case access to all items could be allowed eagerly), but may
    # also be implemented by returning a database termlist (for documents which
    # are stored in a database, rather than freshly created).  We choose the
    # most conservative settings, to avoid doing eager access when lazy access
    # would be more appropriate.
    return TermIter(self.termlist_begin(), self.termlist_end(),
                    has_termfreq=TermIter.LAZY,
                    has_wdf=TermIter.EAGER,
                    has_positions=TermIter.LAZY)
Document.__iter__ = _document_gen_termlist_iter
Document.termlist = _document_gen_termlist_iter

# Modify QueryParser to add a "stoplist()" method.
def _queryparser_gen_stoplist_iter(self):
    """Get an iterator over all the stopped terms from the previous query.
    
    This returns an iterator over all the terms which were omitted from the
    previously parsed query due to being considered to be stopwords.  Each
    instance of a word omitted from the query is represented in the returned
    list, in the order in which the

    The iterator will return string objects.

    """
    return TermIter(self.stoplist_begin(), self.stoplist_end(),
                    return_strings=True)
QueryParser.stoplist = _queryparser_gen_stoplist_iter

# Modify QueryParser to add an "unstemlist()" method.
def _queryparser_gen_unstemlist_iter(self, tname):
    """Get an iterator over all the unstemmed forms of a stemmed term.
    
    This returns an iterator which returns all the unstemmed words which were
    stemmed to the stemmed form specifed by `tname` when parsing the previous
    query.  Each instance of a word which stems to `tname` is returned by the
    iterator in the order in which the words appeared in the query - an
    individual unstemmed word may thus occur multiple times.

    The iterator will return string objects.

    """
    return TermIter(self.unstem_begin(tname), self.unstem_end(tname),
                    return_strings=True)
QueryParser.unstemlist = _queryparser_gen_unstemlist_iter

# When we set a ValueRangeProcessor into the QueryParser, keep a python
# reference so it won't be deleted. This hack can probably be removed once
# xapian bug #186 is fixed.
__queryparser_add_valuerangeprocessor_orig = QueryParser.add_valuerangeprocessor
def _queryparser_add_valuerangeprocessor(self, vrproc):
    if not hasattr(self, '_vrps'):
        self._vrps = []
    self._vrps.append(vrproc)
    return __queryparser_add_valuerangeprocessor_orig(self, vrproc)
_queryparser_add_valuerangeprocessor.__doc__ = __queryparser_add_valuerangeprocessor_orig.__doc__
QueryParser.add_valuerangeprocessor = _queryparser_add_valuerangeprocessor
del _queryparser_add_valuerangeprocessor

# When we set a Stopper into the QueryParser, keep a python reference so it
# won't be deleted. This hack can probably be removed once xapian bug #186 is
# fixed.
__queryparser_set_stopper_orig = QueryParser.set_stopper
def _queryparser_set_stopper(self, stopper):
    self._stopper = stopper
    return __queryparser_set_stopper_orig(self, stopper)
_queryparser_set_stopper.__doc__ = __queryparser_set_stopper_orig.__doc__
QueryParser.set_stopper = _queryparser_set_stopper
del _queryparser_set_stopper

# When we set a Stopper into the TermGenerator, keep a python reference so it
# won't be deleted. This hack can probably be removed once xapian bug #186 is
# fixed.
__termgenerator_set_stopper_orig = TermGenerator.set_stopper
def _termgenerator_set_stopper(self, stopper):
    self._stopper = stopper
    return __termgenerator_set_stopper_orig(self, stopper)
_termgenerator_set_stopper.__doc__ = __termgenerator_set_stopper_orig.__doc__
TermGenerator.set_stopper = _termgenerator_set_stopper
del _termgenerator_set_stopper

def _enquire_check_deprec_args(reverse, kwargs, methodname):
    """Check the keyword arguments to one of the enquire set_sort_* methods.
    
    """
    if reverse is not None:
        if 'ascending' in kwargs:
            raise TypeError('Only one of "reverse" and "ascending" may be specified')
        if len(kwargs) != 0:
            raise TypeError('Only keyword arguments allowed are "reverse" and "ascending"')
    else:
        if 'ascending' in kwargs:
            reverse = kwargs.get('ascending')
            del kwargs['ascending']
        else:
            reverse = True
        if len(kwargs) != 0:
            raise TypeError('Only keyword arguments allowed are "reverse" and "ascending"')
    return reverse

# When we set a Sorter on enquire, keep a python reference so it won't be
# deleted.  This hack can probably be removed once xapian bug #186 is fixed.
__enquire_set_sort_by_key_orig = Enquire.set_sort_by_key
def _enquire_set_sort_by_key(self, sorter, reverse=None, **kwargs):
    self._sorter = sorter
    reverse = _enquire_check_deprec_args(reverse, kwargs, "set_sort_by_key")
    return __enquire_set_sort_by_key_orig(self, sorter, reverse)
_enquire_set_sort_by_key.__doc__ = __enquire_set_sort_by_key_orig.__doc__
Enquire.set_sort_by_key = _enquire_set_sort_by_key
del _enquire_set_sort_by_key

__enquire_set_sort_by_key_then_relevance_orig = Enquire.set_sort_by_key_then_relevance
def _enquire_set_sort_by_key_then_relevance(self, sorter, reverse=None, **kwargs):
    self._sorter = sorter
    reverse = _enquire_check_deprec_args(reverse, kwargs, "set_sort_by_key_then_relevance")
    return __enquire_set_sort_by_key_then_relevance_orig(self, sorter, reverse)
_enquire_set_sort_by_key_then_relevance.__doc__ = __enquire_set_sort_by_key_then_relevance_orig.__doc__
Enquire.set_sort_by_key_then_relevance = _enquire_set_sort_by_key_then_relevance
del _enquire_set_sort_by_key_then_relevance

__enquire_set_sort_by_relevance_then_key_orig = Enquire.set_sort_by_relevance_then_key
def _enquire_set_sort_by_relevance_then_key(self, sorter, reverse=None, **kwargs):
    self._sorter = sorter
    reverse = _enquire_check_deprec_args(reverse, kwargs, "set_sort_by_relevance_then_key")
    return __enquire_set_sort_by_relevance_then_key_orig(self, sorter, reverse)
_enquire_set_sort_by_relevance_then_key.__doc__ = __enquire_set_sort_by_relevance_then_key_orig.__doc__
Enquire.set_sort_by_relevance_then_key = _enquire_set_sort_by_relevance_then_key
del _enquire_set_sort_by_relevance_then_key

__enquire_set_sort_by_value_orig = Enquire.set_sort_by_value
def _enquire_set_sort_by_value(self, sort_key, reverse=None, **kwargs):
    reverse = _enquire_check_deprec_args(reverse, kwargs, "set_sort_by_value")
    return __enquire_set_sort_by_value_orig(self, sort_key, reverse)
_enquire_set_sort_by_value.__doc__ = __enquire_set_sort_by_value_orig.__doc__
Enquire.set_sort_by_value = _enquire_set_sort_by_value
del _enquire_set_sort_by_value

__enquire_set_sort_by_relevance_then_value_orig = Enquire.set_sort_by_relevance_then_value
def _enquire_set_sort_by_relevance_then_value(self, sort_key, reverse=None, **kwargs):
    reverse = _enquire_check_deprec_args(reverse, kwargs, "set_sort_by_relevance_then_value")
    return __enquire_set_sort_by_relevance_then_value_orig(self, sort_key, reverse)
_enquire_set_sort_by_relevance_then_value.__doc__ = __enquire_set_sort_by_relevance_then_value_orig.__doc__
Enquire.set_sort_by_relevance_then_value = _enquire_set_sort_by_relevance_then_value
del _enquire_set_sort_by_relevance_then_value

__enquire_set_sort_by_value_then_relevance_orig = Enquire.set_sort_by_value_then_relevance
def _enquire_set_sort_by_value_then_relevance(self, sort_key, reverse=None, **kwargs):
    reverse = _enquire_check_deprec_args(reverse, kwargs, "set_sort_by_value_then_relevance")
    return __enquire_set_sort_by_value_then_relevance_orig(self, sort_key, reverse)
_enquire_set_sort_by_value_then_relevance.__doc__ = __enquire_set_sort_by_value_then_relevance_orig.__doc__
Enquire.set_sort_by_value_then_relevance = _enquire_set_sort_by_value_then_relevance
del _enquire_set_sort_by_value_then_relevance


##########################################
# Support for iteration of posting lists #
##########################################

class PostingItem(_SequenceMixIn):
    """An item returned from iteration of a posting list.

    The item supports access to the following attributes and properties:

     - `docid`: The document ID corresponding to this PostingItem.
     - `doclength`: The length of the document corresponding to this
       PostingItem.
     - `wdf`: The within document frequency of the term which the posting list
       is for in the document corresponding to this PostingItem.
     - `positer`: An iterator over the positions which the term corresponing to
       this posting list occurs at in the document corresponding to this
       PostingItem.  This is only available until the iterator which returned
       this item next moves.

    """
    __slots__ = ('_iter', 'docid', 'doclength', 'wdf',)

    def __init__(self, iter):
        self._iter = iter
        self.docid = iter._iter.get_docid()
        self.doclength = iter._iter.get_doclength()
        self.wdf = iter._iter.get_wdf()

        # Support for sequence API
        sequence = ['docid', 'doclength', 'wdf', 'positer']
        if not iter._has_positions:
            sequence[3] = PositionIter()
        _SequenceMixIn.__init__(self, *sequence)

    def _get_positer(self):
        """Get a position list iterator.

        The iterator will return integers representing the positions that the
        term occurs at in the document corresponding to this PostingItem.

        This will raise a InvalidOperationError exception if the iterator this
        item came from doesn't support position lists, or if the iterator has
        moved on since the item was returned from it.

        """
        if not self._iter._has_positions:
            raise InvalidOperationError("Iterator does not support position lists")
        if self._iter._iter == self._iter._end or \
           self.docid != self._iter._iter.get_docid():
            raise InvalidOperationError("Iterator has moved, and does not support random access")
        return PositionIter(self._iter._iter.positionlist_begin(),
                            self._iter._iter.positionlist_end())
    positer = property(_get_positer, doc=
    """A position iterator for the current posting (if meaningful).

    The iterator will return integers representing the positions that the term
    occurs at.

    This will raise a InvalidOperationError exception if the iterator this item
    came from doesn't support position lists, or if the iterator has moved on
    since the item was returned from it.

    """)


class PostingIter(object):
    """An iterator over a posting list.

    The iterator will return PostingItem objects, which will be evaluated
    lazily where appropriate.

    """
    __slots__ = ('_iter', '_end', '_has_positions', '_moved')

    def __init__(self, start, end, has_positions=False):
        self._iter = start
        self._end = end
        self._has_positions = has_positions

        # _moved is True if we've moved onto the next item.  This is needed so
        # that the iterator doesn't have to move on until just before next() is
        # called: since the iterator starts by pointing at a valid item, we
        # can't just call self._iter.next() unconditionally at the start of our
        # next() method.
        self._moved = True

    def __iter__(self):
        return self

    def next(self):
        if not self._moved:
            self._iter.next()
            self._moved = True

        if self._iter == self._end:
            raise StopIteration
        else:
            self._moved = False
            return PostingItem(self)

    def skip_to(self, docid):
        """Skip the iterator forward.

        The iterator is advanced to the first document with a document ID
        which is greater than or equal to the supplied document ID.

        If there are no such items, this will raise StopIteration.

        This returns the item which the iterator is moved to.  The subsequent
        item will be returned the next time that next() is called (unless
        skip_to() is called again first).

        """
        if self._iter != self._end:
            self._iter.skip_to(docid)
        if self._iter == self._end:
            self._moved = True
            raise StopIteration
        self._moved = False
        return PostingItem(self)

def _database_gen_postlist_iter(self, tname):
    """Get an iterator over the postings which are indexed by a given term.

    If `tname` is empty, an iterator over all the documents will be returned
    (this will contain one entry for each document, will always return a wdf of
    1, and will not allow access to a position iterator).

    """
    if len(tname) != 0:
        return PostingIter(self.postlist_begin(tname), self.postlist_end(tname),
                           has_positions=True)
    else:
        return PostingIter(self.postlist_begin(tname), self.postlist_end(tname))
Database.postlist = _database_gen_postlist_iter


###########################################
# Support for iteration of position lists #
###########################################

class PositionIter(object):
    """An iterator over a position list.

    The iterator will return integers, in ascending order.

    """
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

# Modify Database to add a "positionlist()" method.
def _database_gen_positionlist_iter(self, docid, tname):
    """Get an iterator over all the positions in a given document of a term.
    
    The iterator will return integers, in ascending order.

    """
    return PositionIter(self.positionlist_begin(docid, tname), self.positionlist_end(docid, tname))
Database.positionlist = _database_gen_positionlist_iter

########################################
# Support for iteration of value lists #
########################################

class ValueItem(_SequenceMixIn):
    """An item returned from iteration of the values in a document.

    The item supports access to the following attributes:

     - `num`: The number of the value.
     - `value`: The contents of the value.

    """

    __slots__ = ('num', 'value', )

    def __init__(self, num, value):
        self.num = num
        self.value = value
        _SequenceMixIn.__init__(self, 'num', 'value')

class ValueIter(object):
    """An iterator over all the values stored in a document.

    The iterator will return ValueItem objects, in ascending order of value number.

    """
    def __init__(self, start, end):
        self.iter = start
        self.end = end

    def __iter__(self):
        return self

    def next(self):
        if self.iter==self.end:
            raise StopIteration
        else:
            r = ValueItem(self.iter.get_valueno(), self.iter.get_value())
            self.iter.next()
            return r

# Modify Document to add a "values()" method.
def _document_gen_values_iter(self):
    """Get an iterator over all the values stored in a document.

    The iterator will return ValueItem objects, in ascending order of value number.

    """
    return ValueIter(self.values_begin(), self.values_end())
Document.values = _document_gen_values_iter

# Set the list of names which should be public.
# Note that this needs to happen at the end of xapian.py.
__all__ = []
for item in dir():
    if item.startswith('_') or item.endswith('_swigregister') or item.endswith('Iterator'):
        continue
    __all__.append(item)
__all__ = tuple(__all__)


# Fix up ValueRangeProcessor by replacing its __call__ method (which doesn't
# work) with its __call() method (which we define with an %extend in util.i)
ValueRangeProcessor.__call__ = ValueRangeProcessor.__call

%}

/* vim:syntax=python:set expandtab: */
