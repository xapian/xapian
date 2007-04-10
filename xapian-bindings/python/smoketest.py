# Simple test to ensure that we can load the xapian module and exercise basic
# functionality successfully.
#
# Copyright (C) 2004,2005,2006 Olly Betts
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

class TestFail(Exception): pass

def checkeq(l, r):
    if l != r:
        raise TestFail("Expected equality: got %r and %r" % (l, r))

def checkquery(query, expected):
    desc = query.get_description()
    if desc != expected:
        raise TestFail("Unexpected query.get_description(): got %r, expected %r" % (desc, expected))

try:
    # Test the version number reporting functions give plausible results.
    v = "%d.%d.%d" % (xapian.major_version(),
                      xapian.minor_version(),
                      xapian.revision())
    v2 = xapian.version_string()
    if v != v2:
        print >> sys.stderr, "Unexpected version output (%s != %s)" % (v, v2)
        sys.exit(1)

    stem = xapian.Stem("english")
    if stem.get_description() != "Xapian::Stem(english)":
        print >> sys.stderr, "Unexpected stem.get_description()"
        sys.exit(1)

    doc = xapian.Document()
    doc.set_data("a\0b")
    if doc.get_data() == "a":
        print >> std.err, "get_data+set_data truncates at a zero byte"
        sys.exit(1)
    if doc.get_data() != "a\0b":
        print >> std.err, "get_data+set_data doesn't transparently handle a zero byte"
        sys.exit(1)
    doc.set_data("is there anybody out there?")
    doc.add_term("XYzzy")
    doc.add_posting(stem.stem_word("is"), 1)
    doc.add_posting(stem.stem_word("there"), 2)
    doc.add_posting(stem.stem_word("anybody"), 3)
    doc.add_posting(stem.stem_word("out"), 4)
    doc.add_posting(stem.stem_word("there"), 5)

    db = xapian.inmemory_open()
    db.add_document(doc)
    if db.get_doccount() != 1:
        print >> sys.stderr, "Unexpected db.get_doccount()"
        sys.exit(1)
    terms = ["smoke", "test", "terms"]
    checkquery(xapian.Query(xapian.Query.OP_OR, terms),
               "Xapian::Query((smoke OR test OR terms))")
    query1 = xapian.Query(xapian.Query.OP_PHRASE, ("smoke", "test", "tuple"))
    query2 = xapian.Query(xapian.Query.OP_XOR, (xapian.Query("smoke"), query1, "string"))
    checkquery(query1, "Xapian::Query((smoke PHRASE 3 test PHRASE 3 tuple))")
    checkquery(query2, "Xapian::Query((smoke XOR (smoke PHRASE 3 test PHRASE 3 tuple) XOR string))")
    subqs = ["a", "b"]
    checkquery(xapian.Query(xapian.Query.OP_OR, subqs), "Xapian::Query((a OR b))")

    # Feature test for Query.__iter__
    term_count = 0
    for term in query2:
        term_count += 1
    if term_count != 4:
        print >> sys.stderr, "Unexpected number of terms in query2 (%d)" % term_count
        sys.exit(1)

    enq = xapian.Enquire(db)
    enq.set_query(xapian.Query(xapian.Query.OP_OR, "there", "is"))
    mset = enq.get_mset(0, 10)
    if mset.size() != 1:
        print >> sys.stderr, "Unexpected mset.size()"
        sys.exit(1)

    # Feature test for Enquire.matching_terms(docid)
    term_count = 0
    for term in enq.matching_terms(mset.get_hit(0)):
        term_count += 1
    if term_count != 2:
        print >> sys.stderr, "Unexpected number of matching terms"
        sys.exit(1)

    # Feature test for MSet.__iter__
    msize = 0
    for match in mset:
        msize += 1
    if msize != mset.size():
        print >> sys.stderr, "Unexpected number of entries in mset (%d != %d)" % (msize, mset.size())
        sys.exit(1)

    terms = " ".join(enq.get_matching_terms(mset.get_hit(0)))
    if terms != "is there":
        print >> sys.stderr, "Unexpected terms"
        sys.exit(1)

    # Feature test for ESet.__iter__
    rset = xapian.RSet()
    rset.add_document(1)
    eset = enq.get_eset(10, rset)
    term_count = 0
    for term in eset:
        term_count += 1
    if term_count != 3:
        print >> sys.stderr, "Unexpected number of expand terms"
        sys.exit(1)

    # Feature test for Database.__iter__
    term_count = 0
    for term in db:
        term_count += 1
    if term_count != 5:
        print >> sys.stderr, "Unexpected number of terms in db (%d)" % term_count
        sys.exit(1)

    # Feature test for Database.allterms
    term_count = 0
    for term in db.allterms():
        term_count += 1
    if term_count != 5:
        print >> sys.stderr, "Unexpected number of terms in db.allterms (%d)" % term_count
        sys.exit(1)

    # Feature test for Database.postlist
    count = 0
    for posting in db.postlist("there"):
        count += 1
    if count != 1:
        print >> sys.stderr, "Unexpected number of entries in db.postlist (%d)" % count
        sys.exit(1)

    # Feature test for Database.postlist with empty term (alldocspostlist)
    count = 0
    for posting in db.postlist(""):
        count += 1
    if count != 1:
        print >> sys.stderr, "Unexpected number of entries in db.postlist('') (%d)" % count
        sys.exit(1)

    # Feature test for Database.termlist
    count = 0
    for term in db.termlist(1):
        count += 1
    if count != 5:
        print >> sys.stderr, "Unexpected number of entries in db.termlist (%d)" % count
        sys.exit(1)

    # Feature test for Database.positionlist
    count = 0
    for term in db.positionlist(1, "there"):
        count += 1
    if count != 2:
        print >> sys.stderr, "Unexpected number of entries in db.positionlist (%d)" % count
        sys.exit(1)

    # Feature test for Document.termlist
    count = 0
    for term in doc.termlist():
        count += 1
    if count != 5:
        print >> sys.stderr, "Unexpected number of entries in doc.termlist (%d)" % count
        sys.exit(1)

    # Feature test for TermIter.skip_to
    term = doc.termlist()
    term.skip_to('n')
    while True:
        try:
            x = term.next()
        except StopIteration:
            break
        if x[0] < 'n':
            print >> sys.stderr, "TermIter.skip_to didn't skip term '%s'" % x[0]
            sys.exit(1)

    # Feature test for Document.values
    count = 0
    for term in doc.values():
        count += 1
    if count != 0:
        print >> sys.stderr, "Unexpected number of entries in doc.values (%d)" % count
        sys.exit(1)

    # Check exception handling for Xapian::DocNotFoundError
    try:
        doc2 = db.get_document(2)
        print >> sys.stderr, "Retrieved non-existent document"
        sys.exit(1)
    except xapian.DocNotFoundError, e:
        if str(e) != "Docid 2 not found":
            print "Exception string not as expected, got: '%s'\n" % str(e)
            sys.exit(1)

    if xapian.Query.OP_ELITE_SET != 10:
        print >> sys.stderr, "OP_ELITE_SET is %d not 10" % xapian.Query.OP_ELITE_SET
        sys.exit(1)

    # Feature test for MatchDecider
    doc = xapian.Document()
    doc.set_data("Two")
    doc.add_posting(stem.stem_word("out"), 1)
    doc.add_posting(stem.stem_word("outside"), 1)
    doc.add_posting(stem.stem_word("source"), 2)
    doc.add_value(0, "yes")
    db.add_document(doc)

    class testmatchdecider(xapian.MatchDecider):
        def __call__(self, doc):
            return doc.get_value(0) == "yes"

    query = xapian.Query(stem.stem_word("out"))
    enquire = xapian.Enquire(db)
    enquire.set_query(query)
    mset = enquire.get_mset(0, 10, None, testmatchdecider())
    if mset.size() != 1:
        print >> sys.stderr, "MatchDecider found %d documents, expected 1" % mset.size()
        sys.exit(1)
    if mset.get_docid(0) != 2:
        print >> sys.stderr, "MatchDecider mset has wrong docid in"
        sys.exit(1)

    # Feature test for ExpandDecider
    class testexpanddecider(xapian.ExpandDecider):
        def __call__(self, term):
            return (not term.startswith('a'))

    enquire = xapian.Enquire(db)
    rset = xapian.RSet()
    rset.add_document(1)
    eset = enquire.get_eset(10, rset, 0, 1.0, testexpanddecider())
    eset_terms = [term[xapian.ESET_TNAME] for term in eset.items]
    if len(eset_terms) != eset.size():
        print >> sys.stderr, "ESet.size() mismatched number of terms in eset.items"
    if filter(lambda t: t.startswith('a'), eset_terms):
        print >> sys.stderr, "ExpandDecider was not used"

    # Check QueryParser parsing error.
    qp = xapian.QueryParser()
    try:
        qp.parse_query("test AND")
        print >> sys.stderr, "QueryParser doesn't report errors"
        sys.exit(1)
    except xapian.QueryParserError, e:
        if str(e) != "Syntax: <expression> AND <expression>":
            print "Exception string not as expected, got: '%s'\n" % str(e)
            sys.exit(1)

    # Check QueryParser pure NOT option
    qp = xapian.QueryParser()
    checkquery(qp.parse_query("NOT test", qp.FLAG_BOOLEAN + qp.FLAG_PURE_NOT),
               "Xapian::Query((<alldocuments> AND_NOT test:(pos=1)))")

    # Check QueryParser partial option
    qp = xapian.QueryParser()
    qp.set_database(db)
    qp.set_default_op(xapian.Query.OP_AND)
    qp.set_stemming_strategy(qp.STEM_SOME)
    qp.set_stemmer(xapian.Stem('en'))
    checkquery(qp.parse_query("foo o", qp.FLAG_PARTIAL),
               "Xapian::Query((foo:(pos=1) AND (out:(pos=2) OR outsid:(pos=2))))")

    checkquery(qp.parse_query("foo outside", qp.FLAG_PARTIAL),
               "Xapian::Query((foo:(pos=1) AND outsid:(pos=2)))")

    # Test supplying unicode strings
    checkquery(xapian.Query(xapian.Query.OP_OR, (u'foo', u'bar')),
               'Xapian::Query((foo OR bar))')
    checkquery(xapian.Query(xapian.Query.OP_OR, ('foo', u'bar\xa3')),
               'Xapian::Query((foo OR bar\xc2\xa3))')
    checkquery(xapian.Query(xapian.Query.OP_OR, ('foo', 'bar\xc2\xa3')),
               'Xapian::Query((foo OR bar\xc2\xa3))')
    checkquery(xapian.Query(xapian.Query.OP_OR, u'foo', u'bar'),
               'Xapian::Query((foo OR bar))')

    checkquery(qp.parse_query(u"NOT t\xe9st", qp.FLAG_BOOLEAN + qp.FLAG_PURE_NOT),
               "Xapian::Query((<alldocuments> AND_NOT t\xc3\xa9st:(pos=1)))")

    doc = xapian.Document()
    doc.set_data(u"Unicode with an acc\xe9nt")
    doc.add_posting(stem.stem_word(u"out\xe9r"), 1)
    checkeq(doc.get_data(), u"Unicode with an acc\xe9nt".encode('utf-8'))
    term = doc.termlist().next()[0]
    checkeq(term, u"out\xe9r".encode('utf-8'))

    # Check simple stopper
    stop = xapian.SimpleStopper()
    qp.set_stopper(stop)
    checkeq(stop('a'), False)
    checkquery(qp.parse_query(u"foo bar a", qp.FLAG_BOOLEAN),
               "Xapian::Query((foo:(pos=1) AND bar:(pos=2) AND a:(pos=3)))")

    stop.add('a')
    checkeq(stop('a'), True)
    checkquery(qp.parse_query(u"foo bar a", qp.FLAG_BOOLEAN),
               "Xapian::Query((foo:(pos=1) AND bar:(pos=2)))")

    # Feature test for custom Stopper
    class my_b_stopper(xapian.Stopper):
        def __call__(self, term):
            return term == "b"

        def get_description(self):
            return u"my_b_stopper"

    stop = my_b_stopper()
    checkeq(stop.get_description(), u"my_b_stopper")
    qp.set_stopper(stop)
    checkeq(stop('a'), False)
    checkquery(qp.parse_query(u"foo bar a", qp.FLAG_BOOLEAN),
               "Xapian::Query((foo:(pos=1) AND bar:(pos=2) AND a:(pos=3)))")

    checkeq(stop('b'), True)
    checkquery(qp.parse_query(u"foo bar b", qp.FLAG_BOOLEAN),
               "Xapian::Query((foo:(pos=1) AND bar:(pos=2)))")



except xapian.Error, e:
    print >> sys.stderr, "Xapian Error: %s" % str(e)
    raise
    sys.exit(1)

except Exception, e:
    print >> sys.stderr, "Exception: %s" % str(e)
    raise
    sys.exit(1)

except:
    print >> sys.stderr, "Unexpected exception"
    sys.exit(1)

# vim:syntax=python:set expandtab:
