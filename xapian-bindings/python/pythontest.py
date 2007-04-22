# Tests of Python-specific parts of the xapian bindings.
#
# Copyright (C) 2007 Lemur Consulting Ltd
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

import sys
import xapian

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
    db.add_document(doc)
    doc.set_data("was it warm? three")
    doc.add_term("three", 3)
    db.add_document(doc)
    doc.set_data("was it warm? four")
    doc.add_term("four", 4)
    db.add_document(doc)

    expect(db.get_doccount(), 5)
    return db

def test_exception_base():
    """Check that xapian exceptions have Exception as a base class.

    """
    try:
        raise xapian.InvalidOperationError("Test exception")
    except Exception, e:
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

    context("testing returned item from mset")
    expect(items[2].docid, 4)
    expect(items[2].rank, 2)
    expect(items[2].percent, 81)
    expect(items[2].collapse_key, '')
    expect(items[2].collapse_count, 0)
    expect(items[2].document.get_data(), 'was it warm? three')

    context("testing deprecated sequence API")
    expect(len(items[2][:]), 5)
    expect(items[2][0], 4)
    expect(items[2][2], 2)
    expect(items[2][3], 81)

    # Check iterators for sub-msets against the whole mset.
    for start in xrange(0, 6):
        for maxitems in xrange(0, 6):
            context("checking iterators for sub-mset from %d, maxitems %d" % (start, maxitems))
            submset = enquire.get_mset(start, maxitems)
            num = 0
            for item in submset:
                context("testing hit %d for sub-mset from %d, maxitems %d" % (num, start, maxitems))
                expect(item.rank, num + start)

                context("comparing iterator item %d for sub-mset from %d, maxitems %d against hit" % (num, start, maxitems))
                hit = submset.get_hit(num)
                expect(hit.get_docid(), item.docid)
                expect(hit.get_rank(), item.rank)
                expect(hit.get_percent(), item.percent)
                expect(hit.get_document().get_data(), item.document.get_data())
                expect(hit.get_collapse_key(), item.collapse_key)
                expect(hit.get_collapse_count(), item.collapse_count)

                context("comparing iterator item %d for sub-mset mset from %d, maxitems %d against hit from whole mset" % (num, start, maxitems))
                hit = mset.get_hit(num + start)
                expect(hit.get_docid(), item.docid)
                expect(hit.get_rank(), item.rank)
                expect(hit.get_percent(), item.percent)
                expect(hit.get_document().get_data(), item.document.get_data())
                expect(hit.get_collapse_key(), item.collapse_key)
                expect(hit.get_collapse_count(), item.collapse_count)

                context("comparing iterator item %d for sub-mset mset from %d, maxitems %d against direct access with []" % (num, start, maxitems))
                expect(submset[num].docid, item.docid)
                expect(submset[num].rank, item.rank)
                expect(submset[num].percent, item.percent)
                expect(submset[num].document.get_data(), item.document.get_data())
                expect(submset[num].collapse_key, item.collapse_key)
                expect(submset[num].collapse_count, item.collapse_count)

                num += 1

            # Check that the item contents remain valid when the iterator has
            # moved on.
            saved_items = [item for item in submset]
            for num in xrange(len(saved_items)):
                item = saved_items[num]
                context("comparing iterator item %d for sub-mset mset from %d, maxitems %d against saved item" % (num, start, maxitems))
                expect(submset[num].docid, item.docid)
                expect(submset[num].rank, item.rank)
                expect(submset[num].percent, item.percent)
                expect(submset[num].document.get_data(), item.document.get_data())
                expect(submset[num].collapse_key, item.collapse_key)
                expect(submset[num].collapse_count, item.collapse_count)

            # Test deprecated sequence API.
            num = 0
            for item in submset:
                context("testing hit %d with deprecated sequence API for sub-mset from %d, maxitems %d" % (num, start, maxitems))
                hit = submset.get_hit(num)
                expect(len(item[:]), 5)
                expect(item[0], hit.get_docid())
                expect(item[1], hit.get_weight())
                expect(item[2], hit.get_rank())
                expect(item[3], hit.get_percent())
                expect(item[4].get_data(), hit.get_document().get_data())
                num += 1

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
    expect(len(items), 4)
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

    context("checking legacy sequence API for eset items")
    expect(items2[0][0], items[0].term)
    expect(items2[1][0], items[2].term)
    expect(items2[0][1], items[0].weight)
    expect(items2[1][1], items[2].weight)
    expect(items2[0][:], [items[0].term, items[0].weight])
    expect(items2[1][:], [items[2].term, items[2].weight])

def test_matchingterms_iter():
    """Test Enquire.matching_terms iterator.

    """
    db = setup_database()
    query = xapian.Query(xapian.Query.OP_OR, ("was", "it", "warm", "two"))

    enquire = xapian.Enquire(db)
    enquire.set_query(query)
    mset = enquire.get_mset(0, 10)
    for item in mset:

        # Make a list of the term names
        mterms = []
        for termitem in enquire.matching_terms(item.docid):
            mterms.append(termitem.term)
            expect_exception(xapian.InvalidOperationError, 'Iterator does not support wdfs', getattr, termitem, 'wdf')
            expect_exception(xapian.InvalidOperationError, 'Iterator does not support term frequencies', getattr, termitem, 'termfreq')
            expect_exception(xapian.InvalidOperationError, 'Iterator does not support position lists', getattr, termitem, 'positer')

        mterms2 = []
        for termitem in enquire.matching_terms(item):
            mterms2.append(termitem.term)
            expect_exception(xapian.InvalidOperationError, 'Iterator does not support wdfs', getattr, termitem, 'wdf')
            expect_exception(xapian.InvalidOperationError, 'Iterator does not support term frequencies', getattr, termitem, 'termfreq')
            expect_exception(xapian.InvalidOperationError, 'Iterator does not support position lists', getattr, termitem, 'positer')
        expect(mterms, mterms2)

        # Make a list of the match items (so we can test if they're still valid
        # once the iterator has moved on).
        termitems = []
        for termitem in enquire.matching_terms(item.docid):
            termitems.append(termitem)

        expect(len(termitems), len(mterms))
        for i in xrange(len(termitems)):
            expect(termitems[i].term, mterms[i])

def test_queryterms_iter():
    """Test Query term iterator.

    """
    db = setup_database()
    query = xapian.Query(xapian.Query.OP_OR, ("was", "it", "warm", "two"))

    # Make a list of the term names
    terms = []
    for termitem in query:
        terms.append(termitem.term)
        expect_exception(xapian.InvalidOperationError, 'Iterator does not support wdfs', getattr, termitem, 'wdf')
        expect_exception(xapian.InvalidOperationError, 'Iterator does not support term frequencies', getattr, termitem, 'termfreq')
        expect_exception(xapian.InvalidOperationError, 'Iterator does not support position lists', getattr, termitem, 'positer')

    # Make a list of the items (so we can test if they're still valid
    # once the iterator has moved on).
    termitems = []
    for termitem in query:
        termitems.append(termitem)

    expect(len(termitems), len(terms))
    for i in xrange(len(termitems)):
        expect(termitems[i].term, terms[i])

def test_queryparser_stoplist_iter():
    """Test QueryParser stoplist iterator.

    """
    db = setup_database()
    stemmer = xapian.Stem('en')

    # Check behaviour without having set a stoplist.
    queryparser = xapian.QueryParser()
    queryparser.set_stemmer(stemmer)
    queryparser.set_stemming_strategy(queryparser.STEM_SOME)
    expect([item.term for item in queryparser.stoplist()], [])
    query = queryparser.parse_query('to be or not to be is the questions')
    expect([item.term for item in queryparser.stoplist()], [])
    expect(str(query),
           'Xapian::Query((to:(pos=1) OR be:(pos=2) OR or:(pos=3) OR '
           'not:(pos=4) OR to:(pos=5) OR be:(pos=6) OR is:(pos=7) OR '
           'the:(pos=8) OR question:(pos=9)))')

    # Check behaviour with a stoplist, but no stemmer
    queryparser = xapian.QueryParser()
    stopper = xapian.SimpleStopper()
    stopper.add('to')
    stopper.add('not')
    stopper.add('question')
    queryparser.set_stopper(stopper)
    expect([item.term for item in queryparser.stoplist()], [])
    query = queryparser.parse_query('to be or not to be is the questions')

    expect([item.term for item in queryparser.stoplist()], ['to', 'not', 'to'])
    expect(str(query),
           'Xapian::Query((be:(pos=2) OR or:(pos=3) OR '
           'be:(pos=6) OR is:(pos=7) OR '
           'the:(pos=8) OR questions:(pos=9)))')

    # Check behaviour with a stoplist and a stemmer
    queryparser.set_stemmer(stemmer)
    queryparser.set_stemming_strategy(queryparser.STEM_SOME)
    expect([item.term for item in queryparser.stoplist()], ['to', 'not', 'to']) # Shouldn't have changed since previous query.
    query = queryparser.parse_query('to be or not to be is the questions')

    expect([item.term for item in queryparser.stoplist()], ['to', 'not', 'to', 'question'])
    expect(str(query),
           'Xapian::Query((be:(pos=2) OR or:(pos=3) OR '
           'be:(pos=6) OR is:(pos=7) OR '
           'the:(pos=8)))')

    # Make a list of the term names
    terms = []
    for termitem in queryparser.stoplist():
        terms.append(termitem.term)
        expect_exception(xapian.InvalidOperationError, 'Iterator does not support wdfs', getattr, termitem, 'wdf')
        expect_exception(xapian.InvalidOperationError, 'Iterator does not support term frequencies', getattr, termitem, 'termfreq')
        expect_exception(xapian.InvalidOperationError, 'Iterator does not support position lists', getattr, termitem, 'positer')

    # Make a list of the items (so we can test if they're still valid
    # once the iterator has moved on).
    termitems = []
    for termitem in queryparser.stoplist():
        termitems.append(termitem)

    expect(len(termitems), len(terms))
    for i in xrange(len(termitems)):
        expect(termitems[i].term, terms[i])

def test_queryparser_unstem_iter():
    """Test QueryParser unstemlist iterator.

    """
    db = setup_database()
    stemmer = xapian.Stem('en')

    queryparser = xapian.QueryParser()
    expect([item.term for item in queryparser.unstemlist('to')], [])
    expect([item.term for item in queryparser.unstemlist('question')], [])
    expect([item.term for item in queryparser.unstemlist('questions')], [])
    query = queryparser.parse_query('to question questions')

    expect([item.term for item in queryparser.unstemlist('to')], ['to'])
    expect([item.term for item in queryparser.unstemlist('question')], ['question'])
    expect([item.term for item in queryparser.unstemlist('questions')], ['questions'])
    expect(str(query),
           'Xapian::Query((to:(pos=1) OR question:(pos=2) OR questions:(pos=3)))')


    queryparser = xapian.QueryParser()
    queryparser.set_stemmer(stemmer)
    queryparser.set_stemming_strategy(queryparser.STEM_SOME)
    expect([item.term for item in queryparser.unstemlist('to')], [])
    expect([item.term for item in queryparser.unstemlist('question')], [])
    expect([item.term for item in queryparser.unstemlist('questions')], [])
    query = queryparser.parse_query('to question questions')

    expect([item.term for item in queryparser.unstemlist('to')], ['to'])
    expect([item.term for item in queryparser.unstemlist('question')], ['question', 'questions'])
    expect([item.term for item in queryparser.unstemlist('questions')], [])
    expect(str(query),
           'Xapian::Query((to:(pos=1) OR question:(pos=2) OR question:(pos=3)))')


    # Make a list of the term names
    terms = []
    for termitem in queryparser.unstemlist('question'):
        terms.append(termitem.term)
        expect_exception(xapian.InvalidOperationError, 'Iterator does not support wdfs', getattr, termitem, 'wdf')
        expect_exception(xapian.InvalidOperationError, 'Iterator does not support term frequencies', getattr, termitem, 'termfreq')
        expect_exception(xapian.InvalidOperationError, 'Iterator does not support position lists', getattr, termitem, 'positer')
    expect(terms, ['question', 'questions'])

    # Make a list of the items (so we can test if they're still valid
    # once the iterator has moved on).
    termitems = []
    for termitem in queryparser.unstemlist('question'):
        termitems.append(termitem)

    expect(len(termitems), len(terms))
    for i in xrange(len(termitems)):
        expect(termitems[i].term, terms[i])

def test_allterms_iter():
    """Test all-terms iterator on Database.

    """
    db = setup_database()

    # Make a list of the term names
    terms = []
    freqs = []
    for termitem in db:
        terms.append(termitem.term)
        expect_exception(xapian.InvalidOperationError, 'Iterator does not support wdfs', getattr, termitem, 'wdf')
        freqs.append(termitem.termfreq)
        expect_exception(xapian.InvalidOperationError, 'Iterator does not support position lists', getattr, termitem, 'positer')

    # Make a list of the items (so we can test if they're still valid
    # once the iterator has moved on).
    termitems = []
    for termitem in db:
        termitems.append(termitem)

    expect(len(termitems), len(terms))
    for i in xrange(len(termitems)):
        expect(termitems[i].term, terms[i])

    expect(len(termitems), len(freqs))
    for i in xrange(len(termitems)):
        expect_exception(xapian.InvalidOperationError, 'Iterator has moved, and does not support random access', getattr, termitem, 'termfreq')

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

    expect(terms, ['it', 'two', 'warm', 'was'])
    expect(wdfs, [1, 2, 1, 1])
    expect(freqs, [5, 3, 4, 4])
    expect(positers, [[2], [], [3], [1]])

    # Make a list of the terms (so we can test if they're still valid
    # once the iterator has moved on).
    termitems = []
    for termitem in db.termlist(3):
        termitems.append(termitem)

    expect(len(termitems), len(terms))
    for i in xrange(len(termitems)):
        expect(termitems[i].term, terms[i])

    expect(len(termitems), len(wdfs))
    for i in xrange(len(termitems)):
        expect(termitems[i].wdf, wdfs[i])

    expect(len(termitems), len(freqs))
    for i in xrange(len(termitems)):
        expect_exception(xapian.InvalidOperationError,
                         'Iterator has moved, and does not support random access',
                         getattr, termitem, 'termfreq')

    expect(len(termitems), len(freqs))
    for i in xrange(len(termitems)):
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

    expect(terms, ['it', 'two', 'warm', 'was'])
    expect(wdfs, [1, 2, 1, 1])
    expect(freqs, [5, 3, 4, 4])
    expect(positers, [[2], [], [3], [1]])

    # Make a list of the terms (so we can test if they're still valid
    # once the iterator has moved on).
    termitems = []
    for termitem in doc:
        termitems.append(termitem)

    expect(len(termitems), len(terms))
    for i in xrange(len(termitems)):
        expect(termitems[i].term, terms[i])

    expect(len(termitems), len(wdfs))
    for i in xrange(len(termitems)):
        expect(termitems[i].wdf, wdfs[i])

    expect(len(termitems), len(freqs))
    for i in xrange(len(termitems)):
        expect_exception(xapian.InvalidOperationError,
                         'Iterator has moved, and does not support random access',
                         getattr, termitem, 'termfreq')

    expect(len(termitems), len(freqs))
    for i in xrange(len(termitems)):
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
    freqs = []
    positers = []
    for termitem in doc:
        terms.append(termitem.term)
        wdfs.append(termitem.wdf)
        freqs.append(termitem.termfreq)
        positers.append([pos for pos in termitem.positer])

    expect(terms, ['it', 'two', 'warm', 'was'])
    expect(wdfs, [1, 2, 1, 1])
    expect(freqs, [0, 0, 0, 0])
    expect(positers, [[2], [], [3], [1]])

    # Make a list of the terms (so we can test if they're still valid
    # once the iterator has moved on).
    termitems = []
    for termitem in doc:
        termitems.append(termitem)

    expect(len(termitems), len(terms))
    for i in xrange(len(termitems)):
        expect(termitems[i].term, terms[i])

    expect(len(termitems), len(wdfs))
    for i in xrange(len(termitems)):
        expect(termitems[i].wdf, wdfs[i])

    expect(len(termitems), len(freqs))
    for i in xrange(len(termitems)):
        expect_exception(xapian.InvalidOperationError,
                         'Iterator has moved, and does not support random access',
                         getattr, termitem, 'termfreq')

    expect(len(termitems), len(freqs))
    for i in xrange(len(termitems)):
        expect_exception(xapian.InvalidOperationError,
                         'Iterator has moved, and does not support random access',
                         getattr, termitem, 'positer')




# Run all tests (ie, callables with names starting "test_").
if not runtests(globals()):
    sys.exit(1)

# vim:syntax=python:set expandtab:
