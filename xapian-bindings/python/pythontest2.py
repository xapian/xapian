# Tests of Python-specific parts of the xapian bindings.
#
# Copyright (C) 2007 Lemur Consulting Ltd
# Copyright (C) 2008 Olly Betts
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
import shutil

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
    doc.set_data("was it warm? four it")
    doc.add_term("four", 4)
    doc.add_term("it", 6)
    doc.add_posting("it", 7)
    doc.add_value(5, 'five')
    doc.add_value(9, 'nine')
    doc.add_value(0, 'zero')
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
    expect(len(mset), len(items), "Expected number of items to be length of mset")

    context("testing returned item from mset")
    expect(items[2].docid, 4)
    expect(items[2].rank, 2)
    expect(items[2].percent, 86)
    expect(items[2].collapse_key, '')
    expect(items[2].collapse_count, 0)
    expect(items[2].document.get_data(), 'was it warm? three')

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
        for term in enquire.matching_terms(item.docid):
            mterms.append(term)

        mterms2 = []
        for term in enquire.matching_terms(item):
            mterms2.append(term)
        expect(mterms, mterms2)

    mterms = []
    for term in enquire.matching_terms(mset.get_hit(0)):
        mterms.append(term)
    expect(mterms, ['it', 'two', 'warm', 'was'])

def test_queryterms_iter():
    """Test Query term iterator.

    """
    db = setup_database()
    query = xapian.Query(xapian.Query.OP_OR, ("was", "it", "warm", "two"))

    # Make a list of the term names
    terms = []
    for term in query:
        terms.append(term)
    expect(terms, ['it', 'two', 'warm', 'was'])

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
           'Xapian::Query((Zto:(pos=1) OR Zbe:(pos=2) OR Zor:(pos=3) OR '
           'Znot:(pos=4) OR Zto:(pos=5) OR Zbe:(pos=6) OR Zis:(pos=7) OR '
           'Zthe:(pos=8) OR Zquestion:(pos=9)))')

    # Check behaviour with a stoplist, but no stemmer
    queryparser = xapian.QueryParser()
    stopper = xapian.SimpleStopper()
    stopper.add('to')
    stopper.add('not')
    stopper.add('question')
    queryparser.set_stopper(stopper)
    expect([term for term in queryparser.stoplist()], [])
    query = queryparser.parse_query('to be or not to be is the questions')

    expect([term for term in queryparser.stoplist()], ['to', 'not', 'to'])
    expect(str(query),
           'Xapian::Query((be:(pos=2) OR or:(pos=3) OR '
           'be:(pos=6) OR is:(pos=7) OR '
           'the:(pos=8) OR questions:(pos=9)))')

    # Check behaviour with a stoplist and a stemmer
    queryparser.set_stemmer(stemmer)
    queryparser.set_stemming_strategy(queryparser.STEM_SOME)
    expect([term for term in queryparser.stoplist()], ['to', 'not', 'to']) # Shouldn't have changed since previous query.
    query = queryparser.parse_query('to be or not to be is the questions')

    expect([term for term in queryparser.stoplist()], ['to', 'not', 'to'])
    expect(str(query),
           'Xapian::Query((Zbe:(pos=2) OR Zor:(pos=3) OR '
           'Zbe:(pos=6) OR Zis:(pos=7) OR '
           'Zthe:(pos=8) OR Zquestion:(pos=9)))')

def test_queryparser_unstem_iter():
    """Test QueryParser unstemlist iterator.

    """
    stemmer = xapian.Stem('en')

    queryparser = xapian.QueryParser()
    expect([term for term in queryparser.unstemlist('to')], [])
    expect([term for term in queryparser.unstemlist('question')], [])
    expect([term for term in queryparser.unstemlist('questions')], [])
    query = queryparser.parse_query('to question questions')

    expect([term for term in queryparser.unstemlist('to')], ['to'])
    expect([term for term in queryparser.unstemlist('question')], ['question'])
    expect([term for term in queryparser.unstemlist('questions')], ['questions'])
    expect(str(query),
           'Xapian::Query((to:(pos=1) OR question:(pos=2) OR questions:(pos=3)))')


    queryparser = xapian.QueryParser()
    queryparser.set_stemmer(stemmer)
    queryparser.set_stemming_strategy(queryparser.STEM_SOME)
    expect([term for term in queryparser.unstemlist('Zto')], [])
    expect([term for term in queryparser.unstemlist('Zquestion')], [])
    expect([term for term in queryparser.unstemlist('Zquestions')], [])
    query = queryparser.parse_query('to question questions')

    expect([term for term in queryparser.unstemlist('Zto')], ['to'])
    expect([term for term in queryparser.unstemlist('Zquestion')], ['question', 'questions'])
    expect([term for term in queryparser.unstemlist('Zquestions')], [])
    expect(str(query),
           'Xapian::Query((Zto:(pos=1) OR Zquestion:(pos=2) OR Zquestion:(pos=3)))')

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
    termitems = []
    for termitem in db:
        termitems.append(termitem)

    expect(len(termitems), len(terms))
    for i in range(len(termitems)):
        expect(termitems[i].term, terms[i])

    expect(len(termitems), len(freqs))
    for i in range(len(termitems)):
        expect_exception(xapian.InvalidOperationError, 'Iterator has moved, and does not support random access', getattr, termitem, 'termfreq')

    context("checking that restricting the terms iterated with a prefix works")
    prefix_terms = []
    prefix_freqs = []
    for i in range(len(terms)):
        if terms[i][0] == 't':
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

    expect(terms, ['it', 'two', 'warm', 'was'])
    expect(wdfs, [1, 2, 1, 1])
    expect(freqs, [5, 3, 4, 4])
    expect(positers, [[2], [], [3], [1]])

    # Test skip_to().
    tliter = db.termlist(3)

    # skip to an item before the first item.
    termitem = tliter.skip_to('a')
    expect((termitem.term, termitem.wdf, termitem.termfreq,
            [pos for pos in termitem.positer]), ('it', 1, 5, [2]))

    # skip forwards to an item.
    termitem = tliter.skip_to('two')
    expect((termitem.term, termitem.wdf, termitem.termfreq,
            [pos for pos in termitem.positer]), ('two', 2, 3, []))

    # skip to same place (should return same item)
    termitem = tliter.skip_to('two')
    expect((termitem.term, termitem.wdf, termitem.termfreq,
            [pos for pos in termitem.positer]), ('two', 2, 3, []))

    # next() after a skip_to(), should return next item.
    termitem = next(tliter)
    expect((termitem.term, termitem.wdf, termitem.termfreq,
            [pos for pos in termitem.positer]), ('warm', 1, 4, [3]))

    # skip to same place (should return same item)
    termitem = tliter.skip_to('warm')
    expect((termitem.term, termitem.wdf, termitem.termfreq,
            [pos for pos in termitem.positer]), ('warm', 1, 4, [3]))

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
    termitems = []
    for termitem in db.termlist(3):
        termitems.append(termitem)

    expect(len(termitems), len(terms))
    for i in range(len(termitems)):
        expect(termitems[i].term, terms[i])

    expect(len(termitems), len(wdfs))
    for i in range(len(termitems)):
        expect(termitems[i].wdf, wdfs[i])

    expect(len(termitems), len(freqs))
    for i in range(len(termitems)):
        expect_exception(xapian.InvalidOperationError,
                         'Iterator has moved, and does not support random access',
                         getattr, termitem, 'termfreq')

    expect(len(termitems), len(freqs))
    for i in range(len(termitems)):
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
    for i in range(len(termitems)):
        expect(termitems[i].term, terms[i])

    expect(len(termitems), len(wdfs))
    for i in range(len(termitems)):
        expect(termitems[i].wdf, wdfs[i])

    expect(len(termitems), len(freqs))
    for i in range(len(termitems)):
        expect_exception(xapian.InvalidOperationError,
                         'Iterator has moved, and does not support random access',
                         getattr, termitem, 'termfreq')

    expect(len(termitems), len(freqs))
    for i in range(len(termitems)):
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

    expect(terms, ['it', 'two', 'warm', 'was'])
    expect(wdfs, [1, 2, 1, 1])
    expect(positers, [[2], [], [3], [1]])

    # Make a list of the terms (so we can test if they're still valid
    # once the iterator has moved on).
    termitems = []
    for termitem in doc:
        termitems.append(termitem)

    expect(len(termitems), len(terms))
    for i in range(len(termitems)):
        expect(termitems[i].term, terms[i])

    expect(len(termitems), len(wdfs))
    for i in range(len(termitems)):
        expect(termitems[i].wdf, wdfs[i])

    for i in range(len(termitems)):
        expect_exception(xapian.InvalidOperationError,
                         'Iterator has moved, and does not support random access',
                         getattr, termitems[i], 'termfreq')

    expect(len(termitems), len(positers))
    for i in range(len(termitems)):
        expect_exception(xapian.InvalidOperationError,
                         'Iterator has moved, and does not support random access',
                         getattr, termitems[i], 'positer')

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
    postings = []
    for posting in db.postlist('it'):
        postings.append(posting)

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
    for i in range(len(postings)):
        expect_exception(xapian.InvalidOperationError,
                         'Iterator has moved, and does not support random access',
                         getattr, postings[i], 'positer')

def test_position_iter():
    """Test position iterator for a document in a database.

    """
    db = setup_database()

    doc = db.get_document(5)

    # Make lists of the item contents
    positions = []
    for position in db.positionlist(5, 'it'):
        positions.append(position)

    expect(positions, [2, 7])

def test_value_iter():
    """Test iterators over list of values in a document.

    """
    db = setup_database()
    doc = db.get_document(5)

    items = list(doc.values())
    expect(len(items), 3)
    expect(items[0].num, 0)
    expect(items[0].value, 'zero')
    expect(items[1].num, 5)
    expect(items[1].value, 'five')
    expect(items[2].num, 9)
    expect(items[2].value, 'nine')

def test_synonyms_iter():
    """Test iterators over list of synonyms in a database.

    """
    dbpath = 'db_test_synonyms_iter'
    db = xapian.WritableDatabase(dbpath, xapian.DB_CREATE_OR_OVERWRITE)

    db.add_synonym('hello', 'hi')
    db.add_synonym('hello', 'howdy')

    expect([item for item in db.synonyms('foo')], [])
    expect([item for item in db.synonyms('hello')], ['hi', 'howdy'])
    expect([item for item in db.synonym_keys()], ['hello'])
    expect([item for item in db.synonym_keys('foo')], [])
    expect([item for item in db.synonym_keys('he')], ['hello'])
    expect([item for item in db.synonym_keys('hello')], ['hello'])

    dbr=xapian.Database(dbpath)
    expect([item for item in dbr.synonyms('foo')], [])
    expect([item for item in dbr.synonyms('hello')], [])
    expect([item for item in dbr.synonym_keys()], [])
    expect([item for item in dbr.synonym_keys('foo')], [])
    expect([item for item in dbr.synonym_keys('he')], [])
    expect([item for item in dbr.synonym_keys('hello')], [])

    db.flush()

    expect([item for item in db.synonyms('foo')], [])
    expect([item for item in db.synonyms('hello')], ['hi', 'howdy'])
    expect([item for item in db.synonym_keys()], ['hello'])
    expect([item for item in db.synonym_keys('foo')], [])
    expect([item for item in db.synonym_keys('he')], ['hello'])
    expect([item for item in db.synonym_keys('hello')], ['hello'])

    dbr=xapian.Database(dbpath)
    expect([item for item in dbr.synonyms('foo')] , [])
    expect([item for item in dbr.synonyms('hello')], ['hi', 'howdy'])
    expect([item for item in dbr.synonym_keys()], ['hello'])
    expect([item for item in dbr.synonym_keys('foo')], [])
    expect([item for item in dbr.synonym_keys('he')], ['hello'])
    expect([item for item in dbr.synonym_keys('hello')], ['hello'])

    del db
    del dbr
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
           ['author', 'item1', 'item2', 'type'])
    expect([item for item in db.metadata_keys('foo')], [])
    expect([item for item in db.metadata_keys('item')], ['item1', 'item2'])
    expect([item for item in db.metadata_keys('it')], ['item1', 'item2'])
    expect([item for item in db.metadata_keys('type')], ['type'])

    dbr=xapian.Database(dbpath)
    expect([item for item in dbr.metadata_keys()], [])
    expect([item for item in dbr.metadata_keys('foo')], [])
    expect([item for item in dbr.metadata_keys('item')], [])
    expect([item for item in dbr.metadata_keys('it')], [])
    expect([item for item in dbr.metadata_keys('type')], [])

    db.flush()
    expect([item for item in db.metadata_keys()],
           ['author', 'item1', 'item2', 'type'])
    expect([item for item in db.metadata_keys('foo')], [])
    expect([item for item in db.metadata_keys('item')], ['item1', 'item2'])
    expect([item for item in db.metadata_keys('it')], ['item1', 'item2'])
    expect([item for item in db.metadata_keys('type')], ['type'])

    dbr=xapian.Database(dbpath)
    expect([item for item in dbr.metadata_keys()],
           ['author', 'item1', 'item2', 'type'])
    expect([item for item in dbr.metadata_keys('foo')], [])
    expect([item for item in dbr.metadata_keys('item')], ['item1', 'item2'])
    expect([item for item in dbr.metadata_keys('it')], ['item1', 'item2'])
    expect([item for item in dbr.metadata_keys('type')], ['type'])

    del db
    del dbr
    shutil.rmtree(dbpath)

def test_spell():
    """Test basic spelling correction features.

    """
    dbpath = 'db_test_spell'
    db = xapian.WritableDatabase(dbpath, xapian.DB_CREATE_OR_OVERWRITE)

    db.add_spelling('hello')
    db.add_spelling('mell', 2)
    expect(db.get_spelling_suggestion('hell'), 'mell')
    expect([(item.term, item.termfreq) for item in db.spellings()], [('hello', 1), ('mell', 2)])
    dbr=xapian.Database(dbpath)
    expect(dbr.get_spelling_suggestion('hell'), '')
    expect([(item.term, item.termfreq) for item in dbr.spellings()], [])
    db.flush()
    dbr=xapian.Database(dbpath)
    expect(db.get_spelling_suggestion('hell'), 'mell')
    expect(dbr.get_spelling_suggestion('hell'), 'mell')
    expect([(item.term, item.termfreq) for item in dbr.spellings()], [('hello', 1), ('mell', 2)])

    del db
    del dbr
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
           'Xapian::Query(VALUE_RANGE 7 A5 B8)')


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
            expected = [(item.weight * mult, item.docid) for item in mset1]
        expect([(item.weight, item.docid) for item in mset2], expected)

    context("checking queries with OP_SCALE_WEIGHT with a multipler of -1")
    query1 = xapian.Query("it")
    expect_exception(xapian.InvalidArgumentError,
                     "Xapian::Query: SCALE_WEIGHT requires a non-negative parameter.",
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

        def reset(self):
            self.current = -1

        def get_termfreq_min(self): return 0
        def get_termfreq_est(self): return self.max / 2
        def get_termfreq_max(self): return self.max
        def next(self, minweight):
            self.current += 2
        def at_end(self): return self.current > self.max
        def get_docid(self): return self.current

    dbpath = 'db_test_postingsource'
    db = xapian.WritableDatabase(dbpath, xapian.DB_CREATE_OR_OVERWRITE)
    for id in range(10):
        doc = xapian.Document()
        db.add_document(doc)

    source = OddPostingSource(10)
    query = xapian.Query(source)

    enquire = xapian.Enquire(db)
    enquire.set_query(query)
    mset = enquire.get_mset(0, 10)

    expect([item.docid for item in mset], [1, 3, 5, 7, 9])

    del db
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

    source = xapian.ValueWeightPostingSource(db, 1)
    query = xapian.Query(source)
    # del source # Check that query keeps a reference to it.

    enquire = xapian.Enquire(db)
    enquire.set_query(query)
    mset = enquire.get_mset(0, 10)

    expect([item.docid for item in mset], [2, 1, 5, 3, 4, 8, 9, 6, 7, 10])

    del db
    shutil.rmtree(dbpath)

def test_value_stats():
    """Simple test of being able to get value statistics.

    """
    dbpath = 'db_test_value_stats'
    db = xapian.chert_open(dbpath, xapian.DB_CREATE_OR_OVERWRITE)

    vals = (6, 9, 4.5, 4.4, 4.6, 2, 1, 4, 3, 0)
    for id in range(10):
        doc = xapian.Document()
        doc.add_value(1, xapian.sortable_serialise(vals[id]))
        db.add_document(doc)

    expect(db.get_value_freq(0), 0)
    expect(db.get_value_lower_bound(0), "")
    expect(db.get_value_upper_bound(0), "")
    expect(db.get_value_freq(1), 10)
    expect(db.get_value_lower_bound(1), xapian.sortable_serialise(0))
    expect(db.get_value_upper_bound(1), xapian.sortable_serialise(9))
    expect(db.get_value_freq(2), 0)
    expect(db.get_value_lower_bound(2), "")
    expect(db.get_value_upper_bound(2), "")

    del db
    shutil.rmtree(dbpath)

def test_director_exception():
    """Test handling of an exception raised in a director.

    """
    dbpath = 'db_test_value_stats'
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

# Run all tests (ie, callables with names starting "test_").
if not runtests(globals(), sys.argv[1:]):
    sys.exit(1)

# vim:syntax=python:set expandtab:
