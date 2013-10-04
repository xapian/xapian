# Simple test to ensure that we can load the xapian module and exercise basic
# functionality successfully.
#
# Copyright (C) 2004,2005,2006,2007,2008,2010,2011,2012 Olly Betts
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

mystemmers = set()
mystemmer_id = 0
# Stemmer which strips English vowels.
class MyStemmer(xapian.StemImplementation):
    def __init__(self):
        global mystemmers
        global mystemmer_id
        super(MyStemmer, self).__init__()
        mystemmers.add(mystemmer_id)
        self._id = mystemmer_id
        mystemmer_id += 1

    def __call__(self, s):
        import re
        return re.sub(br'[aeiou]', b'', s)

    def __del__(self):
        global mystemmers
        if self._id not in mystemmers:
            raise TestFail("MyStemmer #%d deleted more than once" % self._id)
        mystemmers.remove(self._id)

def test_all():
    # Test the version number reporting functions give plausible results.
    v = "%d.%d.%d" % (xapian.major_version(),
                      xapian.minor_version(),
                      xapian.revision())
    v2 = xapian.version_string().decode('utf-8')
    expect(v2, v, "Unexpected version output")

    # A regexp check would be better, but seems to create a bogus "leak" of -1
    # objects in Python 3.
    expect(len(xapian.__version__.split('.')), 3, 'xapian.__version__ not X.Y.Z')
    expect((xapian.__version__.split('.'))[0], '1', 'xapian.__version__ not "1.Y.Z"')

    def access_cvar():
        return xapian.cvar

    # Check that SWIG isn't generating cvar (regression test for ticket#297).
    expect_exception(AttributeError, "'module' object has no attribute 'cvar'",
                     access_cvar)

    stem = xapian.Stem(b"english")
    expect(str(stem), "Xapian::Stem(english)", "Unexpected str(stem)")

    doc = xapian.Document()
    doc.set_data(b"a\0b")
    if doc.get_data() == b"a":
        raise TestFail("get_data+set_data truncates at a zero byte")
    expect(doc.get_data(), b"a\0b", "get_data+set_data doesn't transparently handle a zero byte")
    doc.set_data(b"is there anybody out there?")
    doc.add_term(b"XYzzy")
    doc.add_posting(stem(b"is"), 1)
    doc.add_posting(stem(b"there"), 2)
    doc.add_posting(stem(b"anybody"), 3)
    doc.add_posting(stem(b"out"), 4)
    doc.add_posting(stem(b"there"), 5)

    db = xapian.inmemory_open()
    db.add_document(doc)
    expect(db.get_doccount(), 1, "Unexpected db.get_doccount()")
    terms = ["smoke", "test", "terms"]
    expect_query(xapian.Query(xapian.Query.OP_OR, [t.encode('utf-8') for t in terms]),
                 "(smoke OR test OR terms)")
    query1 = xapian.Query(xapian.Query.OP_PHRASE, (b"smoke", b"test", b"tuple"))
    query2 = xapian.Query(xapian.Query.OP_XOR, (xapian.Query(b"smoke"), query1, b"string"))
    expect_query(query1, "(smoke PHRASE 3 test PHRASE 3 tuple)")
    expect_query(query2, "(smoke XOR (smoke PHRASE 3 test PHRASE 3 tuple) XOR string)")
    subqs = ["a", "b"]
    expect_query(xapian.Query(xapian.Query.OP_OR, [s.encode('utf-8') for s in subqs]), "(a OR b)")
    expect_query(xapian.Query(xapian.Query.OP_VALUE_RANGE, 0, b'1', b'4'),
                 "VALUE_RANGE 0 1 4")

    # Check database factory functions are wrapped as expected:

    expect_exception(xapian.DatabaseOpeningError, None,
                     xapian.open_stub, b"nosuchdir/nosuchdb")
    expect_exception(xapian.DatabaseOpeningError, None,
                     xapian.open_stub, b"nosuchdir/nosuchdb", xapian.DB_OPEN)

    expect_exception(xapian.DatabaseOpeningError, None,
                     xapian.brass_open, b"nosuchdir/nosuchdb")
    expect_exception(xapian.DatabaseCreateError, None,
                     xapian.brass_open, b"nosuchdir/nosuchdb", xapian.DB_CREATE)

    expect_exception(xapian.DatabaseOpeningError, None,
                     xapian.chert_open, b"nosuchdir/nosuchdb")
    expect_exception(xapian.DatabaseCreateError, None,
                     xapian.chert_open, b"nosuchdir/nosuchdb", xapian.DB_CREATE)

    expect_exception(xapian.NetworkError, None,
                     xapian.remote_open, b"/bin/false", b"")
    expect_exception(xapian.NetworkError, None,
                     xapian.remote_open_writable, b"/bin/false", b"")

    expect_exception(xapian.NetworkError, None,
                     xapian.remote_open, b"127.0.0.1", 0, 1)
    expect_exception(xapian.NetworkError, None,
                     xapian.remote_open_writable, b"127.0.0.1", 0, 1)

    # Check wrapping of MatchAll and MatchNothing:

    expect_query(xapian.Query.MatchAll, "<alldocuments>")
    expect_query(xapian.Query.MatchNothing, "")

    # Feature test for Query.__iter__
    term_count = 0
    for term in query2:
        term_count += 1
    expect(term_count, 4, "Unexpected number of terms in query2")

    enq = xapian.Enquire(db)
    enq.set_query(xapian.Query(xapian.Query.OP_OR, b"there", b"is"))
    mset = enq.get_mset(0, 10)
    expect(mset.size(), 1, "Unexpected mset.size()")
    expect(len(mset), 1, "Unexpected mset.size()")

    # Feature test for Enquire.matching_terms(docid)
    term_count = 0
    for term in enq.matching_terms(mset.get_hit(0)):
        term_count += 1
    expect(term_count, 2, "Unexpected number of matching terms")

    # Feature test for MSet.__iter__
    msize = 0
    for match in mset:
        msize += 1
    expect(msize, mset.size(), "Unexpected number of entries in mset")

    terms = b" ".join(enq.matching_terms(mset.get_hit(0)))
    expect(terms, b"is there", "Unexpected terms")

    # Feature test for ESet.__iter__
    rset = xapian.RSet()
    rset.add_document(1)
    eset = enq.get_eset(10, rset)
    term_count = 0
    for term in eset:
        term_count += 1
    expect(term_count, 3, "Unexpected number of expand terms")

    # Feature test for Database.__iter__
    term_count = 0
    for term in db:
        term_count += 1
    expect(term_count, 5, "Unexpected number of terms in db")

    # Feature test for Database.allterms
    term_count = 0
    for term in db.allterms():
        term_count += 1
    expect(term_count, 5, "Unexpected number of terms in db.allterms")

    # Feature test for Database.postlist
    count = 0
    for posting in db.postlist(b"there"):
        count += 1
    expect(count, 1, "Unexpected number of entries in db.postlist('there')")

    # Feature test for Database.postlist with empty term (alldocspostlist)
    count = 0
    for posting in db.postlist(b""):
        count += 1
    expect(count, 1, "Unexpected number of entries in db.postlist('')")

    # Feature test for Database.termlist
    count = 0
    for term in db.termlist(1):
        count += 1
    expect(count, 5, "Unexpected number of entries in db.termlist(1)")

    # Feature test for Database.positionlist
    count = 0
    for term in db.positionlist(1, b"there"):
        count += 1
    expect(count, 2, "Unexpected number of entries in db.positionlist(1, 'there')")

    # Feature test for Document.termlist
    count = 0
    for term in doc.termlist():
        count += 1
    expect(count, 5, "Unexpected number of entries in doc.termlist()")

    # Feature test for TermIter.skip_to
    term = doc.termlist()
    term.skip_to(b'n')
    while True:
        try:
            x = next(term)
        except StopIteration:
            break
        if x.term < b'n':
            raise TestFail("TermIter.skip_to didn't skip term '%s'" % x.term.decode('utf-8'))

    # Feature test for Document.values
    count = 0
    for term in list(doc.values()):
        count += 1
    expect(count, 0, "Unexpected number of entries in doc.values")

    # Check exception handling for Xapian::DocNotFoundError
    expect_exception(xapian.DocNotFoundError, "Docid 3 not found", db.get_document, 3)

    # Check value of OP_ELITE_SET
    expect(xapian.Query.OP_ELITE_SET, 10, "Unexpected value for OP_ELITE_SET")

    # Feature test for MatchDecider
    doc = xapian.Document()
    doc.set_data(b"Two")
    doc.add_posting(stem(b"out"), 1)
    doc.add_posting(stem(b"outside"), 1)
    doc.add_posting(stem(b"source"), 2)
    doc.add_value(0, b"yes")
    db.add_document(doc)

    class testmatchdecider(xapian.MatchDecider):
        def __call__(self, doc):
            return doc.get_value(0) == b"yes"

    query = xapian.Query(stem(b"out"))
    enquire = xapian.Enquire(db)
    enquire.set_query(query)
    mset = enquire.get_mset(0, 10, None, testmatchdecider())
    expect(mset.size(), 1, "Unexpected number of documents returned by match decider")
    expect(mset.get_docid(0), 2, "MatchDecider mset has wrong docid in")

    # Feature test for ExpandDecider
    class testexpanddecider(xapian.ExpandDecider):
        def __call__(self, term):
            return (not term.startswith(b'a'))

    enquire = xapian.Enquire(db)
    rset = xapian.RSet()
    rset.add_document(1)
    eset = enquire.get_eset(10, rset, xapian.Enquire.USE_EXACT_TERMFREQ, 1.0, testexpanddecider())
    eset_terms = [item.term for item in eset]
    expect(len(eset_terms), eset.size(), "Unexpected number of terms returned by expand")
    if [t for t in eset_terms if t.startswith(b'a')]:
        raise TestFail("ExpandDecider was not used")

    # Check min_wt argument to get_eset() works (new in 1.2.5).
    eset = enquire.get_eset(100, rset, xapian.Enquire.USE_EXACT_TERMFREQ)
    expect([i.weight for i in eset][-1] < 1.9, True, "test get_eset() without min_wt")
    eset = enquire.get_eset(100, rset, xapian.Enquire.USE_EXACT_TERMFREQ, 1.0, None, 1.9)
    expect([i.weight for i in eset][-1] >= 1.9, True, "test get_eset() min_wt")

    # Check QueryParser parsing error.
    qp = xapian.QueryParser()
    expect_exception(xapian.QueryParserError, "Syntax: <expression> AND <expression>", qp.parse_query, b"test AND")

    # Check QueryParser pure NOT option
    qp = xapian.QueryParser()
    expect_query(qp.parse_query(b"NOT test", qp.FLAG_BOOLEAN + qp.FLAG_PURE_NOT),
                 "(<alldocuments> AND_NOT test@1)")

    # Check QueryParser partial option
    qp = xapian.QueryParser()
    qp.set_database(db)
    qp.set_default_op(xapian.Query.OP_AND)
    qp.set_stemming_strategy(qp.STEM_SOME)
    qp.set_stemmer(xapian.Stem(b'en'))
    expect_query(qp.parse_query(b"foo o", qp.FLAG_PARTIAL),
                 "(Zfoo@1 AND ((out@2 SYNONYM outsid@2) OR Zo@2))")

    expect_query(qp.parse_query(b"foo outside", qp.FLAG_PARTIAL),
                 "(Zfoo@1 AND Zoutsid@2)")

    # Test supplying unicode strings
    expect_query(xapian.Query(xapian.Query.OP_OR, (b'foo', b'bar')),
                 '(foo OR bar)')
    expect_query(xapian.Query(xapian.Query.OP_OR, (b'foo', b'bar\xa3')),
                 '(foo OR bar\\xa3)')
    expect_query(xapian.Query(xapian.Query.OP_OR, (b'foo', b'bar\xc2\xa3')),
                 '(foo OR bar\u00a3)')
    expect_query(xapian.Query(xapian.Query.OP_OR, b'foo', b'bar'),
                 '(foo OR bar)')

    expect_query(qp.parse_query(b"NOT t\xe9st", qp.FLAG_BOOLEAN + qp.FLAG_PURE_NOT),
                 "(<alldocuments> AND_NOT Zt\u00e9st@1)")

    doc = xapian.Document()
    doc.set_data(b"Unicode with an acc\xe9nt")
    doc.add_posting(stem(b"out\xe9r"), 1)
    expect(doc.get_data(), b"Unicode with an acc\xe9nt")
    term = next(doc.termlist()).term
    expect(term, b"out\xe9r")

    # Check simple stopper
    stop = xapian.SimpleStopper()
    qp.set_stopper(stop)
    expect(stop(b'a'), False)
    expect_query(qp.parse_query(b"foo bar a", qp.FLAG_BOOLEAN),
                 "(Zfoo@1 AND Zbar@2 AND Za@3)")

    stop.add(b'a')
    expect(stop(b'a'), True)
    expect_query(qp.parse_query(b"foo bar a", qp.FLAG_BOOLEAN),
                 "(Zfoo@1 AND Zbar@2)")

    # Feature test for custom Stopper
    class my_b_stopper(xapian.Stopper):
        def __call__(self, term):
            return term == b"b"

        def get_description(self):
            return "my_b_stopper"

    stop = my_b_stopper()
    expect(stop.get_description(), "my_b_stopper")
    qp.set_stopper(stop)
    expect(stop(b'a'), False)
    expect_query(qp.parse_query(b"foo bar a", qp.FLAG_BOOLEAN),
                 "(Zfoo@1 AND Zbar@2 AND Za@3)")

    expect(stop(b'b'), True)
    expect_query(qp.parse_query(b"foo bar b", qp.FLAG_BOOLEAN),
                 "(Zfoo@1 AND Zbar@2)")

    # Test TermGenerator
    termgen = xapian.TermGenerator()
    doc = xapian.Document()
    termgen.set_document(doc)
    termgen.index_text(b'foo bar baz foo')
    expect([(item.term, item.wdf, [pos for pos in item.positer]) for item in doc.termlist()], [(b'bar', 1, [2]), (b'baz', 1, [3]), (b'foo', 2, [1, 4])])


    # Check DateValueRangeProcessor works
    context("checking that DateValueRangeProcessor works")
    qp = xapian.QueryParser()
    vrpdate = xapian.DateValueRangeProcessor(1, 1, 1960)
    qp.add_valuerangeprocessor(vrpdate)
    query = qp.parse_query(b'12/03/99..12/04/01')
    expect(str(query), 'Query(0 * VALUE_RANGE 1 19991203 20011204)')

    # Regression test for bug#193, fixed in 1.0.3.
    context("running regression test for bug#193")
    vrp = xapian.NumberValueRangeProcessor(0, b'$', True)
    a = '$10'
    b = '20'
    slot, a, b = vrp(a, b.encode('utf-8'))
    expect(slot, 0)
    expect(xapian.sortable_unserialise(a), 10)
    expect(xapian.sortable_unserialise(b), 20)

    # Regression tests copied from PHP (probably always worked in python, but
    # let's check...)
    context("running regression tests for issues which were found in PHP")

    # PHP overload resolution involving boolean types failed.
    enq.set_sort_by_value(1, True)

    # Regression test - fixed in 0.9.10.1.
    oqparser = xapian.QueryParser()
    oquery = oqparser.parse_query(b"I like tea")

    # Regression test for bug#192 - fixed in 1.0.3.
    enq.set_cutoff(100)

    # Test setting and getting metadata
    expect(db.get_metadata(b'Foo'), b'')
    db.set_metadata(b'Foo', b'Foo')
    expect(db.get_metadata(b'Foo'), b'Foo')
    expect_exception(xapian.InvalidArgumentError, "Empty metadata keys are invalid", db.get_metadata, b'')
    expect_exception(xapian.InvalidArgumentError, "Empty metadata keys are invalid", db.set_metadata, b'', b'Foo')
    expect_exception(xapian.InvalidArgumentError, "Empty metadata keys are invalid", db.get_metadata, b'')

    # Test OP_SCALE_WEIGHT and corresponding constructor
    expect_query(xapian.Query(xapian.Query.OP_SCALE_WEIGHT, xapian.Query(b'foo'), 5),
                 "5 * foo")

def test_userstem():
    mystem = MyStemmer()
    stem = xapian.Stem(mystem)
    expect(stem(b'test'), b'tst')
    stem2 = xapian.Stem(mystem)
    expect(stem2(b'toastie'), b'tst')

    indexer = xapian.TermGenerator()
    indexer.set_stemmer(xapian.Stem(MyStemmer()))

    doc = xapian.Document()
    indexer.set_document(doc)
    indexer.index_text(b'hello world')

    s = '/'
    for t in doc.termlist():
        s += t.term.decode('utf-8')
        s += '/'
    expect(s, '/Zhll/Zwrld/hello/world/')

    parser = xapian.QueryParser()
    parser.set_stemmer(xapian.Stem(MyStemmer()))
    parser.set_stemming_strategy(xapian.QueryParser.STEM_ALL)
    expect_query(parser.parse_query(b'color television'), '(clr@1 OR tlvsn@2)')

def test_zz9_check_leaks():
    import gc
    gc.collect()
    if len(mystemmers):
        raise TestFail("%d MyStemmer objects not deleted" % len(mystemmers))

# Run all tests (ie, callables with names starting "test_").
if not runtests(globals()):
    sys.exit(1)

# vim:syntax=python:set expandtab:
