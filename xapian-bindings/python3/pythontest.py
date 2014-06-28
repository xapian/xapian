# Tests of Python-specific parts of the xapian bindings.
#
# Copyright (C) 2007 Lemur Consulting Ltd
# Copyright (C) 2008,2009,2010,2011,2013,2014 Olly Betts
# Copyright (C) 2010,2011 Richard Boulton
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation; either version 2 of the
# License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301
# USA

import os
import random
import shutil
import sys
import tempfile
import xapian

try:
    import threading
    have_threads = True
except ImportError:
    have_threads = False

from testsuite import *

def setup_database():
    """Set up and return an inmemory database with 5 documents.

    """
    db = xapian.inmemory_open()

    doc = xapian.Document()
    doc.set_data("is it cold?")
    doc.add_term("is")
    doc.add_posting("it", 1)
    doc.add_posting("cold", 2)
    db.add_document(doc)

    doc = xapian.Document()
    doc.set_data("was it warm?")
    doc.add_posting("was", 1)
    doc.add_posting("it", 2)
    doc.add_posting("warm", 3)
    db.add_document(doc)
    doc.set_data("was it warm? two")
    doc.add_term("two", 2)
    doc.add_value(0, xapian.sortable_serialise(2))
    db.add_document(doc)
    doc.set_data("was it warm? three")
    doc.add_term("three", 3)
    doc.add_value(0, xapian.sortable_serialise(1.5))
    db.add_document(doc)
    doc.set_data("was it warm? four it")
    doc.add_term("four", 4)
    doc.add_term("it", 6)
    doc.add_posting("it", 7)
    doc.add_value(5, 'five')
    doc.add_value(9, 'nine')
    doc.add_value(0, xapian.sortable_serialise(2))
    db.add_document(doc)

    expect(db.get_doccount(), 5)

    # Test that str is rejected by sortable_unserialise().
    try:
        xapian.sortable_unserialise("unicode")
    except TypeError as e:
        expect(str(e), 'expected bytes, str found')

    return db

def test_exception_base():
    """Check that xapian exceptions have Exception as a base class.

    """
    try:
        raise xapian.InvalidOperationError("Test exception")
    except Exception as e:
        pass

def test_mset_iter():
    """Test iterators over MSets.

    """
    db = setup_database()
    query = xapian.Query(xapian.Query.OP_OR, "was", "it")

    enquire = xapian.Enquire(db)
    enquire.set_query(query)
    mset = enquire.get_mset(0, 10)
    items = [item for item in mset]
    expect(len(items), 5)
    expect(len(mset), len(items), "Expected number of items to be length of mset")

    context("testing returned item from mset")
    expect(items[2].docid, 4)
    expect(items[2].rank, 2)
    expect(items[2].percent, 86)
    expect(items[2].collapse_key, b'')
    expect(items[2].collapse_count, 0)
    expect(items[2].document.get_data(), b'was it warm? three')

    # Check iterators for sub-msets against the whole mset.
    for start in range(0, 6):
        for maxitems in range(0, 6):
            context("checking iterators for sub-mset from %d, maxitems %d" % (start, maxitems))
            submset = enquire.get_mset(start, maxitems)
            num = 0
            for item in submset:
                context("testing hit %d for sub-mset from %d, maxitems %d" % (num, start, maxitems))
                expect(item.rank, num + start)

                context("comparing iterator item %d for sub-mset from %d, maxitems %d against hit" % (num, start, maxitems))
                hit = submset.get_hit(num)
                expect(hit.docid, item.docid)
                expect(hit.rank, item.rank)
                expect(hit.percent, item.percent)
                expect(hit.document.get_data(), item.document.get_data())
                expect(hit.collapse_key, item.collapse_key)
                expect(hit.collapse_count, item.collapse_count)

                context("comparing iterator item %d for sub-mset from %d, maxitems %d against hit from whole mset" % (num, start, maxitems))
                hit = mset.get_hit(num + start)
                expect(hit.docid, item.docid)
                expect(hit.rank, item.rank)
                expect(hit.percent, item.percent)
                expect(hit.document.get_data(), item.document.get_data())
                expect(hit.collapse_key, item.collapse_key)
                expect(hit.collapse_count, item.collapse_count)

                context("comparing iterator item %d for sub-mset from %d, maxitems %d against direct access with []" % (num, start, maxitems))
                expect(submset[num].docid, item.docid)
                expect(submset[num].rank, item.rank)
                expect(submset[num].percent, item.percent)
                expect(submset[num].document.get_data(), item.document.get_data())
                expect(submset[num].collapse_key, item.collapse_key)
                expect(submset[num].collapse_count, item.collapse_count)

                num += 1

            context("Checking out of range access to mset, for sub-mset from %d, maxitems %d" % (start, maxitems))
            # Test out-of-range access to mset:
            expect_exception(IndexError, 'Mset index out of range',
                             submset.__getitem__, -10)
            expect_exception(IndexError, 'Mset index out of range',
                             submset.__getitem__, 10)
            expect_exception(IndexError, 'Mset index out of range',
                             submset.__getitem__, -1-len(submset))
            expect_exception(IndexError, 'Mset index out of range',
                             submset.__getitem__, len(submset))

            # Check that the item contents remain valid when the iterator has
            # moved on.
            saved_items = [item for item in submset]
            for num in range(len(saved_items)):
                item = saved_items[num]
                context("comparing iterator item %d for sub-mset mset from %d, maxitems %d against saved item" % (num, start, maxitems))
                expect(submset[num].docid, item.docid)
                expect(submset[num].rank, item.rank)
                expect(submset[num].percent, item.percent)
                expect(submset[num].document.get_data(), item.document.get_data())
                expect(submset[num].collapse_key, item.collapse_key)
                expect(submset[num].collapse_count, item.collapse_count)

            # Check that the right number of items exist in the mset.
            context("checking length of sub-mset from %d, maxitems %d" % (start, maxitems))
            items = [item for item in submset]
            expect(len(items), min(maxitems, 5 - start))
            expect(len(submset), min(maxitems, 5 - start))

def test_eset_iter():
    """Test iterators over ESets.

    """
    db = setup_database()
    query = xapian.Query(xapian.Query.OP_OR, "was", "it")
    rset = xapian.RSet()
    rset.add_document(3)

    context("getting eset items without a query")
    enquire = xapian.Enquire(db)
    eset = enquire.get_eset(10, rset)
    items = [item for item in eset]
    expect(len(items), 3)
    expect(len(items), len(eset))

    context("getting eset items with a query")
    enquire = xapian.Enquire(db)
    enquire.set_query(query)
    eset = enquire.get_eset(10, rset)
    items2 = [item for item in eset]
    expect(len(items2), 2)
    expect(len(items2), len(eset))

    context("comparing eset items with a query to those without")
    expect(items2[0].term, items[0].term)
    expect(items2[1].term, items[2].term)

    context("comparing eset weights with a query to those without")
    expect(items2[0].weight, items[0].weight)
    expect(items2[1].weight, items[2].weight)

def test_matchingterms_iter():
    """Test Enquire.matching_terms iterator.

    """
    db = setup_database()
    query = xapian.Query(xapian.Query.OP_OR, ("was", "it", "warm", "two"))

    # Prior to 1.2.4 Enquire.matching_terms() leaked references to its members.

    enquire = xapian.Enquire(db)
    enquire.set_query(query)
    mset = enquire.get_mset(0, 10)

    for item in mset:
        # Make a list of the term names
        mterms = [term for term in enquire.matching_terms(item.docid)]
        mterms2 = [term for term in enquire.matching_terms(item)]
        expect(mterms, mterms2)

    mterms = [term for term in enquire.matching_terms(mset.get_hit(0))]
    expect(mterms, [b'it', b'two', b'warm', b'was'])

def test_queryterms_iter():
    """Test Query term iterator.

    """
    db = setup_database()
    query = xapian.Query(xapian.Query.OP_OR, ("was", "it", "warm", "two"))

    # Make a list of the term names
    terms = [term for term in query]
    expect(terms, [b'it', b'two', b'warm', b'was'])

def test_queryparser_stoplist_iter():
    """Test QueryParser stoplist iterator.

    """
    stemmer = xapian.Stem('en')

    # Check behaviour without having set a stoplist.
    queryparser = xapian.QueryParser()
    queryparser.set_stemmer(stemmer)
    queryparser.set_stemming_strategy(queryparser.STEM_SOME)
    expect([term for term in queryparser.stoplist()], [])
    query = queryparser.parse_query('to be or not to be is the questions')
    expect([term for term in queryparser.stoplist()], [])
    expect(str(query),
           'Query((Zto@1 OR Zbe@2 OR Zor@3 OR Znot@4 OR Zto@5 OR Zbe@6 OR '
           'Zis@7 OR Zthe@8 OR Zquestion@9))')

    # Check behaviour with a stoplist, but no stemmer
    queryparser = xapian.QueryParser()
    stopper = xapian.SimpleStopper()
    stopper.add('to')
    stopper.add('not')
    stopper.add('question')
    queryparser.set_stopper(stopper)
    expect([term for term in queryparser.stoplist()], [])
    query = queryparser.parse_query('to be or not to be is the questions')

    expect([term for term in queryparser.stoplist()], [b'to', b'not', b'to'])
    expect(str(query),
           'Query((be@2 OR or@3 OR be@6 OR is@7 OR the@8 OR questions@9))')

    # Check behaviour with a stoplist and a stemmer
    queryparser.set_stemmer(stemmer)
    queryparser.set_stemming_strategy(queryparser.STEM_SOME)
    expect([term for term in queryparser.stoplist()], [b'to', b'not', b'to']) # Shouldn't have changed since previous query.
    query = queryparser.parse_query('to be or not to be is the questions')

    expect([term for term in queryparser.stoplist()], [b'to', b'not', b'to'])
    expect(str(query),
           'Query((Zbe@2 OR Zor@3 OR Zbe@6 OR Zis@7 OR Zthe@8 OR Zquestion@9))')

def test_queryparser_unstem_iter():
    """Test QueryParser unstemlist iterator.

    """
    stemmer = xapian.Stem('en')

    queryparser = xapian.QueryParser()
    expect([term for term in queryparser.unstemlist('to')], [])
    expect([term for term in queryparser.unstemlist('question')], [])
    expect([term for term in queryparser.unstemlist('questions')], [])
    query = queryparser.parse_query('to question questions')

    expect([term for term in queryparser.unstemlist('to')], [b'to'])
    expect([term for term in queryparser.unstemlist('question')], [b'question'])
    expect([term for term in queryparser.unstemlist('questions')], [b'questions'])
    expect(str(query),
           'Query((to@1 OR question@2 OR questions@3))')


    queryparser = xapian.QueryParser()
    queryparser.set_stemmer(stemmer)
    queryparser.set_stemming_strategy(queryparser.STEM_SOME)
    expect([term for term in queryparser.unstemlist('Zto')], [])
    expect([term for term in queryparser.unstemlist('Zquestion')], [])
    expect([term for term in queryparser.unstemlist('Zquestions')], [])
    query = queryparser.parse_query('to question questions')

    expect([term for term in queryparser.unstemlist('Zto')], [b'to'])
    expect([term for term in queryparser.unstemlist('Zquestion')], [b'question', b'questions'])
    expect([term for term in queryparser.unstemlist('Zquestions')], [])
    expect(str(query),
           'Query((Zto@1 OR Zquestion@2 OR Zquestion@3))')

def test_allterms_iter():
    """Test all-terms iterator on Database.

    """
    db = setup_database()

    context("making a list of the term names and frequencies")
    terms = []
    freqs = []
    for termitem in db:
        terms.append(termitem.term)
        expect_exception(xapian.InvalidOperationError, 'Iterator does not support wdfs', getattr, termitem, 'wdf')
        freqs.append(termitem.termfreq)
        expect_exception(xapian.InvalidOperationError, 'Iterator does not support position lists', getattr, termitem, 'positer')

    context("checking that items are no longer valid once the iterator has moved on");
    termitems = [termitem for termitem in db]

    expect(len(termitems), len(terms))
    for i in range(len(termitems)):
        expect(termitems[i].term, terms[i])

    expect(len(termitems), len(freqs))
    for termitem in termitems:
        expect_exception(xapian.InvalidOperationError, 'Iterator has moved, and does not support random access', getattr, termitem, 'termfreq')

    context("checking that restricting the terms iterated with a prefix works")
    prefix_terms = []
    prefix_freqs = []
    for i in range(len(terms)):
        if terms[i].startswith(b't'):
            prefix_terms.append(terms[i])
            prefix_freqs.append(freqs[i])
    i = 0
    for termitem in db.allterms('t'):
        expect(termitem.term, prefix_terms[i])
        expect(termitem.termfreq, prefix_freqs[i])
        i += 1
    expect(len(prefix_terms), i)

def test_termlist_iter():
    """Test termlist iterator on Database.

    """
    db = setup_database()

    # Make lists of the item contents
    terms = []
    wdfs = []
    freqs = []
    positers = []
    for termitem in db.termlist(3):
        terms.append(termitem.term)
        wdfs.append(termitem.wdf)
        freqs.append(termitem.termfreq)
        positers.append([pos for pos in termitem.positer])

    expect(terms, [b'it', b'two', b'warm', b'was'])
    expect(wdfs, [1, 2, 1, 1])
    expect(freqs, [5, 3, 4, 4])
    expect(positers, [[2], [], [3], [1]])

    # Test skip_to().
    tliter = db.termlist(3)

    # skip to an item before the first item.
    termitem = tliter.skip_to('a')
    expect((termitem.term, termitem.wdf, termitem.termfreq,
            [pos for pos in termitem.positer]), (b'it', 1, 5, [2]))

    # skip forwards to an item.
    termitem = tliter.skip_to('two')
    expect((termitem.term, termitem.wdf, termitem.termfreq,
            [pos for pos in termitem.positer]), (b'two', 2, 3, []))

    # skip to same place (should return same item)
    termitem = tliter.skip_to('two')
    expect((termitem.term, termitem.wdf, termitem.termfreq,
            [pos for pos in termitem.positer]), (b'two', 2, 3, []))

    # next() after a skip_to(), should return next item.
    termitem = next(tliter)
    expect((termitem.term, termitem.wdf, termitem.termfreq,
            [pos for pos in termitem.positer]), (b'warm', 1, 4, [3]))

    # skip to same place (should return same item)
    termitem = tliter.skip_to('warm')
    expect((termitem.term, termitem.wdf, termitem.termfreq,
            [pos for pos in termitem.positer]), (b'warm', 1, 4, [3]))

    # skip backwards (should return same item)
    termitem = tliter.skip_to('a')

    # skip to after end.
    expect_exception(StopIteration, '', tliter.skip_to, 'zoo')
    # skip backwards (should still return StopIteration).
    expect_exception(StopIteration, '', tliter.skip_to, 'a')
    # next should continue to return StopIteration.
    expect_exception(StopIteration, '', next, tliter)


    # Make a list of the terms (so we can test if they're still valid
    # once the iterator has moved on).
    termitems = [termitem for termitem in db.termlist(3)]

    expect(len(termitems), len(terms))
    for i in range(len(termitems)):
        expect(termitems[i].term, terms[i])

    expect(len(termitems), len(wdfs))
    for i in range(len(termitems)):
        expect(termitems[i].wdf, wdfs[i])

    expect(len(termitems), len(freqs))
    for termitem in termitems:
        expect_exception(xapian.InvalidOperationError,
                         'Iterator has moved, and does not support random access',
                         getattr, termitem, 'termfreq')

    expect(len(termitems), len(freqs))
    for termitem in termitems:
        expect_exception(xapian.InvalidOperationError,
                         'Iterator has moved, and does not support random access',
                         getattr, termitem, 'positer')

def test_dbdocument_iter():
    """Test document terms iterator for document taken from a database.

    """
    db = setup_database()

    doc = db.get_document(3)

    # Make lists of the item contents
    terms = []
    wdfs = []
    freqs = []
    positers = []
    for termitem in doc:
        terms.append(termitem.term)
        wdfs.append(termitem.wdf)
        freqs.append(termitem.termfreq)
        positers.append([pos for pos in termitem.positer])

    expect(terms, [b'it', b'two', b'warm', b'was'])
    expect(wdfs, [1, 2, 1, 1])
    expect(freqs, [5, 3, 4, 4])
    expect(positers, [[2], [], [3], [1]])

    # Make a list of the terms (so we can test if they're still valid
    # once the iterator has moved on).
    termitems = [termitem for termitem in doc]

    expect(len(termitems), len(terms))
    for i in range(len(termitems)):
        expect(termitems[i].term, terms[i])

    expect(len(termitems), len(wdfs))
    for i in range(len(termitems)):
        expect(termitems[i].wdf, wdfs[i])

    expect(len(termitems), len(freqs))
    for termitem in termitems:
        expect_exception(xapian.InvalidOperationError,
                         'Iterator has moved, and does not support random access',
                         getattr, termitem, 'termfreq')

    expect(len(termitems), len(freqs))
    for termitem in termitems:
        expect_exception(xapian.InvalidOperationError,
                         'Iterator has moved, and does not support random access',
                         getattr, termitem, 'positer')

def test_newdocument_iter():
    """Test document terms iterator for newly created document.

    """
    doc = xapian.Document()
    doc.set_data("was it warm? two")
    doc.add_posting("was", 1)
    doc.add_posting("it", 2)
    doc.add_posting("warm", 3)
    doc.add_term("two", 2)

    # Make lists of the item contents
    terms = []
    wdfs = []
    positers = []
    for termitem in doc:
        terms.append(termitem.term)
        wdfs.append(termitem.wdf)
        expect_exception(xapian.InvalidOperationError,
                         "Can't get term frequency from a document termlist "
                         "which is not associated with a database.",
                         getattr, termitem, 'termfreq')
        positers.append([pos for pos in termitem.positer])

    expect(terms, [b'it', b'two', b'warm', b'was'])
    expect(wdfs, [1, 2, 1, 1])
    expect(positers, [[2], [], [3], [1]])

    # Make a list of the terms (so we can test if they're still valid
    # once the iterator has moved on).
    termitems = [termitem for termitem in doc]

    expect(len(termitems), len(terms))
    for i in range(len(termitems)):
        expect(termitems[i].term, terms[i])

    expect(len(termitems), len(wdfs))
    for i in range(len(termitems)):
        expect(termitems[i].wdf, wdfs[i])

    for termitem in termitems:
        expect_exception(xapian.InvalidOperationError,
                         'Iterator has moved, and does not support random access',
                         getattr, termitem, 'termfreq')

    expect(len(termitems), len(positers))
    for termitem in termitems:
        expect_exception(xapian.InvalidOperationError,
                         'Iterator has moved, and does not support random access',
                         getattr, termitem, 'positer')

def test_postinglist_iter():
    """Test postinglist iterator on Database.

    """
    db = setup_database()

    # Make lists of the item contents
    docids = []
    doclengths = []
    wdfs = []
    positers = []
    for posting in db.postlist('it'):
        docids.append(posting.docid)
        doclengths.append(posting.doclength)
        wdfs.append(posting.wdf)
        positers.append([pos for pos in posting.positer])

    expect(docids, [1, 2, 3, 4, 5])
    expect(doclengths, [3, 3, 5, 8, 19])
    expect(wdfs, [1, 1, 1, 1, 8])
    expect(positers, [[1], [2], [2], [2], [2, 7]])

    # Test skip_to().
    pliter = db.postlist('it')

    # skip to an item before the first item.
    posting = pliter.skip_to(0)
    expect((posting.docid, posting.doclength, posting.wdf,
            [pos for pos in posting.positer]), (1, 3, 1, [1]))

    # skip forwards to an item.
    posting = pliter.skip_to(3)
    expect((posting.docid, posting.doclength, posting.wdf,
            [pos for pos in posting.positer]), (3, 5, 1, [2]))

    # skip to same place (should return same item)
    posting = pliter.skip_to(3)
    expect((posting.docid, posting.doclength, posting.wdf,
            [pos for pos in posting.positer]), (3, 5, 1, [2]))

    # next() after a skip_to(), should return next item.
    posting = next(pliter)
    expect((posting.docid, posting.doclength, posting.wdf,
            [pos for pos in posting.positer]), (4, 8, 1, [2]))

    # skip to same place (should return same item)
    posting = pliter.skip_to(4)
    expect((posting.docid, posting.doclength, posting.wdf,
            [pos for pos in posting.positer]), (4, 8, 1, [2]))

    # skip backwards (should return same item)
    posting = pliter.skip_to(2)
    expect((posting.docid, posting.doclength, posting.wdf,
            [pos for pos in posting.positer]), (4, 8, 1, [2]))

    # skip to after end.
    expect_exception(StopIteration, '', pliter.skip_to, 6)
    # skip backwards (should still return StopIteration).
    expect_exception(StopIteration, '', pliter.skip_to, 6)
    # next should continue to return StopIteration.
    expect_exception(StopIteration, '', next, pliter)


    # Make a list of the postings (so we can test if they're still valid once
    # the iterator has moved on).
    postings = [posting for posting in db.postlist('it')]

    expect(len(postings), len(docids))
    for i in range(len(postings)):
        expect(postings[i].docid, docids[i])

    expect(len(postings), len(doclengths))
    for i in range(len(postings)):
        expect(postings[i].doclength, doclengths[i])

    expect(len(postings), len(wdfs))
    for i in range(len(postings)):
        expect(postings[i].wdf, wdfs[i])

    expect(len(postings), len(positers))
    for posting in postings:
        expect_exception(xapian.InvalidOperationError,
                         'Iterator has moved, and does not support random access',
                         getattr, posting, 'positer')

def test_valuestream_iter():
    """Test a valuestream iterator on Database.

    """
    db = setup_database()

    # Check basic iteration
    expect([(item.docid, item.value) for item in db.valuestream(0)],
           [(3, b'\xa4'), (4, b'\xa2'), (5, b'\xa4')])
    expect([(item.docid, item.value) for item in db.valuestream(1)], [])
    expect([(item.docid, item.value) for item in db.valuestream(5)],
           [(5, b"five")])
    expect([(item.docid, item.value) for item in db.valuestream(9)],
           [(5, b"nine")])

    # Test skip_to() on iterator with no values, and behaviours when called
    # after already returning StopIteration.
    i = db.valuestream(1)
    expect_exception(StopIteration, "", i.skip_to, 1)
    expect_exception(StopIteration, "", i.skip_to, 1)
    i = db.valuestream(1)
    expect_exception(StopIteration, "", i.skip_to, 1)
    expect_exception(StopIteration, "", i.__next__)
    i = db.valuestream(1)
    expect_exception(StopIteration, "", i.__next__)
    expect_exception(StopIteration, "", i.skip_to, 1)

    # Test that skipping to a value works, and that skipping doesn't have to
    # advance.
    i = db.valuestream(0)
    item = i.skip_to(4)
    expect((item.docid, item.value), (4, b'\xa2'))
    item = i.skip_to(4)
    expect((item.docid, item.value), (4, b'\xa2'))
    item = i.skip_to(1)
    expect((item.docid, item.value), (4, b'\xa2'))
    item = i.skip_to(5)
    expect((item.docid, item.value), (5, b'\xa4'))
    expect_exception(StopIteration, "", i.skip_to, 6)

    # Test that alternating skip_to() and next() works.
    i = db.valuestream(0)
    item = next(i)
    expect((item.docid, item.value), (3, b'\xa4'))
    item = i.skip_to(4)
    expect((item.docid, item.value), (4, b'\xa2'))
    item = next(i)
    expect((item.docid, item.value), (5, b'\xa4'))
    expect_exception(StopIteration, "", i.skip_to, 6)

    # Test that next works correctly after skip_to() called with an earlier
    # item.
    i = db.valuestream(0)
    item = i.skip_to(4)
    expect((item.docid, item.value), (4, b'\xa2'))
    item = i.skip_to(1)
    expect((item.docid, item.value), (4, b'\xa2'))
    item = next(i)
    expect((item.docid, item.value), (5, b'\xa4'))

    # Test that next works correctly after skipping to last item
    i = db.valuestream(0)
    item = i.skip_to(5)
    expect((item.docid, item.value), (5, b'\xa4'))
    expect_exception(StopIteration, "", i.__next__)

def test_position_iter():
    """Test position iterator for a document in a database.

    """
    db = setup_database()

    doc = db.get_document(5)

    # Make lists of the item contents
    positions = [position for position in db.positionlist(5, 'it')]

    expect(positions, [2, 7])

def test_value_iter():
    """Test iterators over list of values in a document.

    """
    db = setup_database()
    doc = db.get_document(5)

    items = list(doc.values())
    expect(len(items), 3)
    expect(items[0].num, 0)
    expect(items[0].value, xapian.sortable_serialise(2))
    expect(items[1].num, 5)
    expect(items[1].value, b'five')
    expect(items[2].num, 9)
    expect(items[2].value, b'nine')

def test_synonyms_iter():
    """Test iterators over list of synonyms in a database.

    """
    dbpath = 'db_test_synonyms_iter'
    db = xapian.WritableDatabase(dbpath, xapian.DB_CREATE_OR_OVERWRITE)

    db.add_synonym('hello', 'hi')
    db.add_synonym('hello', 'howdy')

    expect([item for item in db.synonyms('foo')], [])
    expect([item for item in db.synonyms('hello')], [b'hi', b'howdy'])
    expect([item for item in db.synonym_keys()], [b'hello'])
    expect([item for item in db.synonym_keys('foo')], [])
    expect([item for item in db.synonym_keys('he')], [b'hello'])
    expect([item for item in db.synonym_keys('hello')], [b'hello'])

    dbr=xapian.Database(dbpath)
    expect([item for item in dbr.synonyms('foo')], [])
    expect([item for item in dbr.synonyms('hello')], [])
    expect([item for item in dbr.synonym_keys()], [])
    expect([item for item in dbr.synonym_keys('foo')], [])
    expect([item for item in dbr.synonym_keys('he')], [])
    expect([item for item in dbr.synonym_keys('hello')], [])

    db.commit()

    expect([item for item in db.synonyms('foo')], [])
    expect([item for item in db.synonyms('hello')], [b'hi', b'howdy'])
    expect([item for item in db.synonym_keys()], [b'hello'])
    expect([item for item in db.synonym_keys('foo')], [])
    expect([item for item in db.synonym_keys('he')], [b'hello'])
    expect([item for item in db.synonym_keys('hello')], [b'hello'])

    dbr=xapian.Database(dbpath)
    expect([item for item in dbr.synonyms('foo')] , [])
    expect([item for item in dbr.synonyms('hello')], [b'hi', b'howdy'])
    expect([item for item in dbr.synonym_keys()], [b'hello'])
    expect([item for item in dbr.synonym_keys('foo')], [])
    expect([item for item in dbr.synonym_keys('he')], [b'hello'])
    expect([item for item in dbr.synonym_keys('hello')], [b'hello'])

    db.close()
    expect(xapian.Database.check(dbpath), 0);
    dbr.close()
    shutil.rmtree(dbpath)

def test_metadata_keys_iter():
    """Test iterators over list of metadata keys in a database.

    """
    dbpath = 'db_test_metadata_iter'
    db = xapian.WritableDatabase(dbpath, xapian.DB_CREATE_OR_OVERWRITE)

    db.set_metadata('author', 'richard')
    db.set_metadata('item1', 'hello')
    db.set_metadata('item1', 'hi')
    db.set_metadata('item2', 'howdy')
    db.set_metadata('item3', '')
    db.set_metadata('item4', 'goodbye')
    db.set_metadata('item4', '')
    db.set_metadata('type', 'greeting')

    expect([item for item in db.metadata_keys()],
           [b'author', b'item1', b'item2', b'type'])
    expect([item for item in db.metadata_keys('foo')], [])
    expect([item for item in db.metadata_keys('item')], [b'item1', b'item2'])
    expect([item for item in db.metadata_keys('it')], [b'item1', b'item2'])
    expect([item for item in db.metadata_keys('type')], [b'type'])

    dbr=xapian.Database(dbpath)
    expect([item for item in dbr.metadata_keys()], [])
    expect([item for item in dbr.metadata_keys('foo')], [])
    expect([item for item in dbr.metadata_keys('item')], [])
    expect([item for item in dbr.metadata_keys('it')], [])
    expect([item for item in dbr.metadata_keys('type')], [])

    db.commit()
    expect([item for item in db.metadata_keys()],
           [b'author', b'item1', b'item2', b'type'])
    expect([item for item in db.metadata_keys('foo')], [])
    expect([item for item in db.metadata_keys('item')], [b'item1', b'item2'])
    expect([item for item in db.metadata_keys('it')], [b'item1', b'item2'])
    expect([item for item in db.metadata_keys('type')], [b'type'])

    dbr=xapian.Database(dbpath)
    expect([item for item in dbr.metadata_keys()],
           [b'author', b'item1', b'item2', b'type'])
    expect([item for item in dbr.metadata_keys('foo')], [])
    expect([item for item in dbr.metadata_keys('item')], [b'item1', b'item2'])
    expect([item for item in dbr.metadata_keys('it')], [b'item1', b'item2'])
    expect([item for item in dbr.metadata_keys('type')], [b'type'])

    db.close()
    expect(xapian.Database.check(dbpath), 0);
    dbr.close()
    shutil.rmtree(dbpath)

def test_spell():
    """Test basic spelling correction features.

    """
    dbpath = 'db_test_spell'
    db = xapian.WritableDatabase(dbpath, xapian.DB_CREATE_OR_OVERWRITE)

    db.add_spelling('hello')
    db.add_spelling('mell', 2)
    expect(db.get_spelling_suggestion('hell'), b'mell')
    expect([(item.term, item.termfreq) for item in db.spellings()], [(b'hello', 1), (b'mell', 2)])
    dbr=xapian.Database(dbpath)
    expect(dbr.get_spelling_suggestion('hell'), b'')
    expect([(item.term, item.termfreq) for item in dbr.spellings()], [])
    db.commit()
    dbr=xapian.Database(dbpath)
    expect(db.get_spelling_suggestion('hell'), b'mell')
    expect(dbr.get_spelling_suggestion('hell'), b'mell')
    expect([(item.term, item.termfreq) for item in dbr.spellings()], [(b'hello', 1), (b'mell', 2)])

    db.close()
    expect(xapian.Database.check(dbpath), 0);
    dbr.close()
    shutil.rmtree(dbpath)

def test_queryparser_custom_vrp():
    """Test QueryParser with a custom (in python) ValueRangeProcessor.

    """
    class MyVRP(xapian.ValueRangeProcessor):
        def __init__(self):
            xapian.ValueRangeProcessor.__init__(self)

        def __call__(self, begin, end):
            return (7, "A"+begin, "B"+end)

    queryparser = xapian.QueryParser()
    myvrp = MyVRP()

    queryparser.add_valuerangeprocessor(myvrp)
    query = queryparser.parse_query('5..8')

    expect(str(query),
           'Query(0 * VALUE_RANGE 7 A5 B8)')

def test_queryparser_custom_vrp_deallocation():
    """Test that QueryParser doesn't delete ValueRangeProcessors too soon.

    """
    class MyVRP(xapian.ValueRangeProcessor):
        def __init__(self):
            xapian.ValueRangeProcessor.__init__(self)

        def __call__(self, begin, end):
            return (7, "A"+begin, "B"+end)

    def make_parser():
        queryparser = xapian.QueryParser()
        myvrp = MyVRP()
        queryparser.add_valuerangeprocessor(myvrp)
        return queryparser

    queryparser = make_parser()
    query = queryparser.parse_query('5..8')

    expect(str(query),
           'Query(0 * VALUE_RANGE 7 A5 B8)')

def test_scale_weight():
    """Test query OP_SCALE_WEIGHT feature.

    """
    db = setup_database()
    for mult in (0, 1, 2.5):
        context("checking queries with OP_SCALE_WEIGHT with a multipler of %r" %
                mult)
        query1 = xapian.Query("it")
        query2 = xapian.Query(xapian.Query.OP_SCALE_WEIGHT, query1, mult)

        enquire = xapian.Enquire(db)
        enquire.set_query(query1)
        mset1 = enquire.get_mset(0, 10)
        enquire.set_query(query2)
        mset2 = enquire.get_mset(0, 10)
        if mult <= 0:
            expected = [(0, item.docid) for item in mset1]
            expected.sort()
        else:
            expected = [(int(item.weight * mult * 1000000), item.docid) for item in mset1]
        expect([(int(item.weight * 1000000), item.docid) for item in mset2], expected)

    context("checking queries with OP_SCALE_WEIGHT with a multipler of -1")
    query1 = xapian.Query("it")
    expect_exception(xapian.InvalidArgumentError,
                     "OP_SCALE_WEIGHT requires factor >= 0",
                     xapian.Query,
                     xapian.Query.OP_SCALE_WEIGHT, query1, -1)


def test_weight_normalise():
    """Test normalising of query weights using the OP_SCALE_WEIGHT feature.

    This test first runs a search (asking for no results) to get the maximum
    possible weight for a query, and then checks that the results of
    MSet.get_max_possible() match this.

    This tests that the get_max_possible() value is correct (though it isn't
    guaranteed to be at a tight bound), and that the SCALE_WEIGHT query can
    compensate correctly.

    """
    db = setup_database()
    for query in (
                  "it",
                  "was",
                  "it was",
                  "it was four",
                  "it was four five",
                  "\"was it warm\" four notpresent",
                  "notpresent",
    ):
        context("checking query %r using OP_SCALE_WEIGHT to normalise the weights" % query)
        qp = xapian.QueryParser()
        query1 = qp.parse_query(query)
        enquire = xapian.Enquire(db)
        enquire.set_query(query1)
        mset1 = enquire.get_mset(0, 0)

        # Check the max_attained value is 0 - this gives us some reassurance
        # that the match didn't actually do the work of calculating any
        # results.
        expect(mset1.get_max_attained(), 0)

        max_possible = mset1.get_max_possible()
        if query == "notpresent":
            expect(max_possible, 0)
            continue
        mult = 1.0 / max_possible
        query2 = xapian.Query(xapian.Query.OP_SCALE_WEIGHT, query1, mult)

        enquire = xapian.Enquire(db)
        enquire.set_query(query2)
        mset2 = enquire.get_mset(0, 10)
        # max_possible should be 1 (excluding rounding errors) for mset2
        expect(int(mset2.get_max_possible() * 1000000.0 + 0.5), 1000000)
        for item in mset2:
            expect(item.weight > 0, True)
            expect(item.weight <= 1, True)


def test_valuesetmatchdecider():
    """Simple tests of the ValueSetMatchDecider class

    """
    md = xapian.ValueSetMatchDecider(0, True)
    doc = xapian.Document()
    expect(md(doc), False)

    md.add_value('foo')
    doc.add_value(0, 'foo')
    expect(md(doc), True)

    md.remove_value('foo')
    expect(md(doc), False)

    md = xapian.ValueSetMatchDecider(0, False)
    expect(md(doc), True)

    md.add_value('foo')
    expect(md(doc), False)


def test_postingsource():
    """Simple test of the PostingSource class.

    """
    class OddPostingSource(xapian.PostingSource):
        def __init__(self, max):
            xapian.PostingSource.__init__(self)
            self.max = max

        def init(self, db):
            self.current = -1

        def get_termfreq_min(self): return 0
        def get_termfreq_est(self): return int(self.max / 2)
        def get_termfreq_max(self): return self.max
        def __next__(self, minweight):
            self.current += 2
        def at_end(self): return self.current > self.max
        def get_docid(self): return self.current

    dbpath = 'db_test_postingsource'
    db = xapian.WritableDatabase(dbpath, xapian.DB_CREATE_OR_OVERWRITE)
    for id in range(10):
        doc = xapian.Document()
        db.add_document(doc)

    # Do a dance to check that the posting source doesn't get dereferenced too
    # soon in various cases.
    def mkenq(db):
        # First - check that it's kept when the source goes out of scope.
        def mkquery():
            source = OddPostingSource(10)
            return xapian.Query(xapian.Query.OP_OR, xapian.Query(source))

        # Check that it's kept when the query goes out of scope.
        def submkenq():
            query = mkquery()
            enquire = xapian.Enquire(db)
            enquire.set_query(query)
            return enquire

        # Check it's kept when the query is retrieved from enquire and put into
        # a new enquire.
        def submkenq2():
            enq1 = submkenq()
            enquire = xapian.Enquire(db)
            enquire.set_query(enq1.get_query())
            return enquire

        return submkenq2()

    enquire = mkenq(db)
    mset = enquire.get_mset(0, 10)

    expect([item.docid for item in mset], [1, 3, 5, 7, 9])

    db.close()
    expect(xapian.Database.check(dbpath), 0);
    shutil.rmtree(dbpath)

def test_postingsource2():
    """Simple test of the PostingSource class.

    """
    dbpath = 'db_test_postingsource2'
    db = xapian.WritableDatabase(dbpath, xapian.DB_CREATE_OR_OVERWRITE)
    vals = (6, 9, 4.5, 4.4, 4.6, 2, 1, 4, 3, 0)
    for id in range(10):
        doc = xapian.Document()
        doc.add_value(1, xapian.sortable_serialise(vals[id]))
        db.add_document(doc)

    source = xapian.ValueWeightPostingSource(1)
    query = xapian.Query(source)
    del source # Check that query keeps a reference to it.

    enquire = xapian.Enquire(db)
    enquire.set_query(query)
    mset = enquire.get_mset(0, 10)

    expect([item.docid for item in mset], [2, 1, 5, 3, 4, 8, 9, 6, 7, 10])

    db.close()
    shutil.rmtree(dbpath)

def test_value_stats():
    """Simple test of being able to get value statistics.

    """
    dbpath = 'db_test_value_stats'
    db = xapian.WritableDatabase(dbpath, xapian.DB_CREATE_OR_OVERWRITE)

    vals = (6, 9, 4.5, 4.4, 4.6, 2, 1, 4, 3, 0)
    for id in range(10):
        doc = xapian.Document()
        doc.add_value(1, xapian.sortable_serialise(vals[id]))
        db.add_document(doc)

    expect(db.get_value_freq(0), 0)
    expect(db.get_value_lower_bound(0), b"")
    expect(db.get_value_upper_bound(0), b"")
    expect(db.get_value_freq(1), 10)
    expect(db.get_value_lower_bound(1), xapian.sortable_serialise(0))
    expect(db.get_value_upper_bound(1), xapian.sortable_serialise(9))
    expect(db.get_value_freq(2), 0)
    expect(db.get_value_lower_bound(2), b"")
    expect(db.get_value_upper_bound(2), b"")

    db.close()
    expect(xapian.Database.check(dbpath), 0);
    shutil.rmtree(dbpath)

def test_get_uuid():
    """Test getting UUIDs from databases.

    """
    dbpath = 'db_test_get_uuid'
    db1 = xapian.WritableDatabase(dbpath + "1", xapian.DB_CREATE_OR_OVERWRITE)
    db2 = xapian.WritableDatabase(dbpath + "2", xapian.DB_CREATE_OR_OVERWRITE)
    dbr1 = xapian.Database(dbpath + "1")
    dbr2 = xapian.Database(dbpath + "2")
    expect(db1.get_uuid() != db2.get_uuid(), True)
    expect(db1.get_uuid(), dbr1.get_uuid())
    expect(db2.get_uuid(), dbr2.get_uuid())

    db = xapian.Database()
    db.add_database(db1)
    expect(db1.get_uuid(), db.get_uuid())

    db1.close()
    db2.close()
    dbr1.close()
    dbr2.close()
    db.close()
    shutil.rmtree(dbpath + "1")
    shutil.rmtree(dbpath + "2")

def test_director_exception():
    """Test handling of an exception raised in a director.

    """
    db = setup_database()
    query = xapian.Query('it')
    enq = xapian.Enquire(db)
    enq.set_query(query)
    class TestException(Exception):
        def __init__(self, a, b):
            Exception.__init__(self, a + b)

    rset = xapian.RSet()
    rset.add_document(1)
    class EDecider(xapian.ExpandDecider):
        def __call__(self, term):
            raise TestException("foo", "bar")
    edecider = EDecider()
    expect_exception(TestException, "foobar", edecider, "foo")
    expect_exception(TestException, "foobar", enq.get_eset, 10, rset, edecider)

    class MDecider(xapian.MatchDecider):
        def __call__(self, doc):
            raise TestException("foo", "bar")
    mdecider = MDecider()
    expect_exception(TestException, "foobar", mdecider, xapian.Document())
    expect_exception(TestException, "foobar", enq.get_mset, 0, 10, None, mdecider)

def check_vals(db, vals):
    """Check that the values in slot 1 are as in vals.

    """
    for docid in range(1, db.get_lastdocid() + 1):
        val = db.get_document(docid).get_value(1)
        expect(val, vals[docid], "Expected stored value in doc %d" % docid)

def test_value_mods():
    """Test handling of modifications to values.

    """
    dbpath = 'db_test_value_mods'
    db = xapian.WritableDatabase(dbpath, xapian.DB_CREATE_OR_OVERWRITE)
    random.seed(42)
    doccount = 1000
    vals = {}

    # Add a value to all the documents
    for num in range(1, doccount):
        doc=xapian.Document()
        val = ('val%d' % num).encode('utf-8')
        doc.add_value(1, val)
        db.add_document(doc)
        vals[num] = val
    db.commit()
    check_vals(db, vals)

    # Modify one of the values (this is a regression test which failed with the
    # initial implementation of streaming values).
    doc = xapian.Document()
    val = b'newval0'
    doc.add_value(1, val)
    db.replace_document(2, doc)
    vals[2] = val
    db.commit()
    check_vals(db, vals)

    # Do some random modifications.
    for count in range(1, doccount * 2):
        docid = random.randint(1, doccount)
        doc = xapian.Document()

        if count % 5 == 0:
            val = b''
        else:
            val = ('newval%d' % count).encode('utf-8')
            doc.add_value(1, val)
        db.replace_document(docid, doc)
        vals[docid] = val

    # Check the values before and after modification.
    check_vals(db, vals)
    db.commit()
    check_vals(db, vals)

    # Delete all the values which are non-empty, in a random order.
    keys = [key for key, val in vals.items() if val != '']
    random.shuffle(keys)
    for key in keys:
        doc = xapian.Document()
        db.replace_document(key, doc)
        vals[key] = b''
    check_vals(db, vals)
    db.commit()
    check_vals(db, vals)

    db.close()
    expect_exception(xapian.DatabaseError, "Database has been closed", check_vals, db, vals)
    shutil.rmtree(dbpath)

def test_serialise_document():
    """Test serialisation of documents.

    """
    doc = xapian.Document()
    doc.add_term('foo', 2)
    doc.add_value(1, b'bar')
    doc.set_data('baz')
    s = doc.serialise()
    doc2 = xapian.Document.unserialise(s)
    expect(len(list(doc.termlist())), len(list(doc2.termlist())))
    expect(len(list(doc.termlist())), 1)
    expect([(item.term, item.wdf) for item in doc.termlist()],
           [(item.term, item.wdf) for item in doc2.termlist()])
    expect([(item.num, item.value) for item in list(doc.values())],
           [(item.num, item.value) for item in list(doc2.values())])
    expect(doc.get_data(), doc2.get_data())
    expect(doc.get_data(), b'baz')

    db = setup_database()
    doc = db.get_document(1)
    s = doc.serialise()
    doc2 = xapian.Document.unserialise(s)
    expect(len(list(doc.termlist())), len(list(doc2.termlist())))
    expect(len(list(doc.termlist())), 3)
    expect([(item.term, item.wdf) for item in doc.termlist()],
           [(item.term, item.wdf) for item in doc2.termlist()])
    expect([(item.num, item.value) for item in list(doc.values())],
           [(item.num, item.value) for item in list(doc2.values())])
    expect(doc.get_data(), doc2.get_data())
    expect(doc.get_data(), b'is it cold?')

def test_serialise_query():
    """Test serialisation of queries.

    """
    q = xapian.Query()
    q2 = xapian.Query.unserialise(q.serialise())
    expect(str(q), str(q2))
    expect(str(q), 'Query()')

    q = xapian.Query('hello')
    q2 = xapian.Query.unserialise(q.serialise())
    expect(str(q), str(q2))
    expect(str(q), 'Query(hello)')

    q = xapian.Query(xapian.Query.OP_OR, ('hello', b'world'))
    q2 = xapian.Query.unserialise(q.serialise())
    expect(str(q), str(q2))
    expect(str(q), 'Query((hello OR world))')

def test_preserve_query_parser_stopper():
    """Test preservation of stopper set on query parser.

    """
    def make_qp():
        queryparser = xapian.QueryParser()
        stopper = xapian.SimpleStopper()
        stopper.add('to')
        stopper.add('not')
        queryparser.set_stopper(stopper)
        del stopper
        return queryparser
    queryparser = make_qp()
    query = queryparser.parse_query('to be')
    expect([term for term in queryparser.stoplist()], [b'to'])

def test_preserve_term_generator_stopper():
    """Test preservation of stopper set on term generator.

    """
    def make_tg():
        termgen = xapian.TermGenerator()
        termgen.set_stemmer(xapian.Stem('en'))
        stopper = xapian.SimpleStopper()
        stopper.add('to')
        stopper.add('not')
        termgen.set_stopper(stopper)
        del stopper
        return termgen
    termgen = make_tg()

    termgen.index_text('to be')
    doc = termgen.get_document()
    terms = [term.term for term in doc.termlist()]
    terms.sort()
    expect(terms, [b'Zbe', b'be', b'to'])

def test_preserve_enquire_sorter():
    """Test preservation of sorter set on enquire.

    """
    db = xapian.inmemory_open()
    doc = xapian.Document()
    doc.add_term('foo')
    doc.add_value(1, '1')
    db.add_document(doc)
    db.add_document(doc)

    def make_enq1(db):
        enq = xapian.Enquire(db)
        sorter = xapian.MultiValueKeyMaker()
        enq.set_sort_by_key(sorter, False)
        del sorter
        return enq
    enq = make_enq1(db)
    enq.set_query(xapian.Query('foo'))
    enq.get_mset(0, 10)

    def make_enq2(db):
        enq = xapian.Enquire(db)
        sorter = xapian.MultiValueKeyMaker()
        enq.set_sort_by_key_then_relevance(sorter, False)
        del sorter
        return enq
    enq = make_enq2(db)
    enq.set_query(xapian.Query('foo'))
    enq.get_mset(0, 10)

    def make_enq3(db):
        enq = xapian.Enquire(db)
        sorter = xapian.MultiValueKeyMaker()
        enq.set_sort_by_relevance_then_key(sorter, False)
        del sorter
        return enq
    enq = make_enq3(db)
    enq.set_query(xapian.Query('foo'))
    enq.get_mset(0, 10)

def test_matchspy():
    """Test use of matchspies.

    """
    db = setup_database()
    query = xapian.Query(xapian.Query.OP_OR, "was", "it")
    enq = xapian.Enquire(db)
    enq.set_query(query)

    def set_matchspy_deref(enq):
        """Set a matchspy, and then drop the reference, to check that it
        doesn't get deleted too soon.
        """
        spy = xapian.ValueCountMatchSpy(0)
        enq.add_matchspy(spy)
        del spy
    set_matchspy_deref(enq)
    mset = enq.get_mset(0, 10)
    expect(len(mset), 5)

    spy = xapian.ValueCountMatchSpy(0)
    enq.add_matchspy(spy)
    # Regression test for clear_matchspies() - used to always raise an
    # exception due to a copy and paste error in its definition.
    enq.clear_matchspies()
    mset = enq.get_mset(0, 10)
    expect([item for item in list(spy.values())], [])

    enq.add_matchspy(spy)
    mset = enq.get_mset(0, 10)
    expect(spy.get_total(), 5)
    expect([(item.term, item.termfreq) for item in list(spy.values())], [
           (xapian.sortable_serialise(1.5), 1),
           (xapian.sortable_serialise(2), 2),
    ])
    expect([(item.term, item.termfreq) for item in spy.top_values(10)], [
           (xapian.sortable_serialise(2), 2),
           (xapian.sortable_serialise(1.5), 1),
    ])

def test_import_star():
    """Test that "from xapian import *" works.

    This is a regression test - this failed in the 1.2.0 release.
    It's not normally good style to use it, but it should work anyway!

    """
    import test_xapian_star

def test_latlongcoords_iter():
    """Test LatLongCoordsIterator wrapping.

    """
    coords = xapian.LatLongCoords()
    expect([c for c in coords], [])
    coords.append(xapian.LatLongCoord(0, 0))
    coords.append(xapian.LatLongCoord(0, 1))
    expect([str(c) for c in coords], ['Xapian::LatLongCoord(0, 0)',
                                      'Xapian::LatLongCoord(0, 1)'])


def test_compactor():
    """Test that xapian.Compactor works.

    """
    tmpdir = tempfile.mkdtemp()
    db1 = db2 = db3 = None
    try:
        db1path = os.path.join(tmpdir, 'db1')
        db2path = os.path.join(tmpdir, 'db2')
        db3path = os.path.join(tmpdir, 'db3')

        # Set up a couple of sample input databases
        db1 = xapian.WritableDatabase(db1path, xapian.DB_CREATE_OR_OVERWRITE)
        doc1 = xapian.Document()
        doc1.add_term('Hello')
        doc1.add_term('Hello1')
        doc1.add_value(0, 'Val1')
        db1.set_metadata('key', '1')
        db1.set_metadata('key1', '1')
        db1.add_document(doc1)
        db1.flush()

        db2 = xapian.WritableDatabase(db2path, xapian.DB_CREATE_OR_OVERWRITE)
        doc2 = xapian.Document()
        doc2.add_term('Hello')
        doc2.add_term('Hello2')
        doc2.add_value(0, 'Val2')
        db2.set_metadata('key', '2')
        db2.set_metadata('key2', '2')
        db2.add_document(doc2)
        db2.flush()

        # Compact with the default compactor
        # Metadata conflicts are resolved by picking the first value
        c = xapian.Compactor()
        c.add_source(db1path)
        c.add_source(db2path)
        c.set_destdir(db3path)
        c.compact()

        db3 = xapian.Database(db3path)
        expect([(item.term, item.termfreq) for item in db3.allterms()],
               [(b'Hello', 2), (b'Hello1', 1), (b'Hello2', 1)])
        expect(db3.get_document(1).get_value(0), b'Val1')
        expect(db3.get_document(2).get_value(0), b'Val2')
        expect(db3.get_metadata('key'), b'1')
        expect(db3.get_metadata('key1'), b'1')
        expect(db3.get_metadata('key2'), b'2')

        context("testing a custom compactor which merges duplicate metadata")
        class MyCompactor(xapian.Compactor):
            def __init__(self):
                xapian.Compactor.__init__(self)
                self.log = []

            def set_status(self, table, status):
                if len(status) == 0:
                    self.log.append('Starting %s' % table.decode('utf-8'))
                else:
                    self.log.append('%s: %s' % (table.decode('utf-8'), status.decode('utf-8')))

            def resolve_duplicate_metadata(self, key, vals):
                return b','.join(vals)

        c = MyCompactor()
        c.add_source(db1path)
        c.add_source(db2path)
        c.set_destdir(db3path)
        c.compact()
        log = '\n'.join(c.log)
        # Check we got some messages in the log
        expect('Starting postlist' in log, True)

        db3 = xapian.Database(db3path)
        expect([(item.term, item.termfreq) for item in db3.allterms()],
               [(b'Hello', 2), (b'Hello1', 1), (b'Hello2', 1)])
        expect(db3.get_metadata('key'), b'1,2')
        expect(db3.get_metadata('key1'), b'1')
        expect(db3.get_metadata('key2'), b'2')

    finally:
        if db1 is not None:
            db1.close()
        if db2 is not None:
            db2.close()
        if db3 is not None:
            db3.close()

        shutil.rmtree(tmpdir)

def test_custom_matchspy():
    class MSpy(xapian.MatchSpy):
        def __init__(self):
            xapian.MatchSpy.__init__(self)
            self.count = 0

        def __call__(self, doc, weight):
            self.count += 1

    mspy = MSpy()

    db = setup_database()
    query = xapian.Query(xapian.Query.OP_OR, "was", "it")

    enquire = xapian.Enquire(db)
    enquire.add_matchspy(mspy)
    enquire.set_query(query)
    mset = enquire.get_mset(0, 1)
    expect(len(mset), 1)
    expect(mspy.count >= 1, True)

    expect(db.get_doccount(), 5)

def test_removed_features():
    ok = True
    db = xapian.inmemory_open()
    doc = xapian.Document()
    enq = xapian.Enquire(db)
    eset = xapian.ESet()
    mset = xapian.MSet()
    query = xapian.Query()
    qp = xapian.QueryParser()
    titer = xapian._TermIterator()
    postiter = xapian._PostingIterator()

    def check_missing(obj, attr):
        expect_exception(AttributeError, None, getattr, obj, attr)

    check_missing(xapian, 'Stem_get_available_languages')
    check_missing(xapian, 'TermIterator')
    check_missing(xapian, 'PositionIterator')
    check_missing(xapian, 'PostingIterator')
    check_missing(xapian, 'ValueIterator')
    check_missing(xapian, 'MSetIterator')
    check_missing(xapian, 'ESetIterator')
    check_missing(db, 'allterms_begin')
    check_missing(db, 'allterms_end')
    check_missing(db, 'metadata_keys_begin')
    check_missing(db, 'metadata_keys_end')
    check_missing(db, 'synonym_keys_begin')
    check_missing(db, 'synonym_keys_end')
    check_missing(db, 'synonyms_begin')
    check_missing(db, 'synonyms_end')
    check_missing(db, 'spellings_begin')
    check_missing(db, 'spellings_end')
    check_missing(db, 'positionlist_begin')
    check_missing(db, 'positionlist_end')
    check_missing(db, 'postlist_begin')
    check_missing(db, 'postlist_end')
    check_missing(db, 'termlist_begin')
    check_missing(db, 'termlist_end')
    check_missing(doc, 'termlist_begin')
    check_missing(doc, 'termlist_end')
    check_missing(doc, 'values_begin')
    check_missing(doc, 'values_end')
    check_missing(enq, 'get_matching_terms_begin')
    check_missing(enq, 'get_matching_terms_end')
    check_missing(eset, 'begin')
    check_missing(eset, 'end')
    check_missing(mset, 'begin')
    check_missing(mset, 'end')
    check_missing(postiter, 'positionlist_begin')
    check_missing(postiter, 'positionlist_end')
    check_missing(query, 'get_terms_begin')
    check_missing(query, 'get_terms_end')
    check_missing(qp, 'stoplist_begin')
    check_missing(qp, 'stoplist_end')
    check_missing(qp, 'unstem_begin')
    check_missing(qp, 'unstem_end')
    check_missing(titer, 'positionlist_begin')
    check_missing(titer, 'positionlist_end')

result = True

# Run all tests (ie, callables with names starting "test_").
def run():
    global result
    if not runtests(globals(), sys.argv[1:]):
        result = False

print("Running tests without threads")
run()

if have_threads:
    print("Running tests with threads")

    # This testcase seems to just block when run in a thread, so just remove
    # it before running tests in a thread.
    del test_import_star

    t = threading.Thread(name='test runner', target=run)
    t.start()
    # Block until the thread has completed so the thread gets a chance to exit
    # with error status.
    t.join()

if not result:
    sys.exit(1)

# vim:syntax=python:set expandtab:
