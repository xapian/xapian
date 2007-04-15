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

def test_mset_iter():
    db = setup_database()
    query = xapian.Query(xapian.Query.OP_OR, "was", "it")

    # Test test for MSet.__iter__
    enquire = xapian.Enquire(db)
    enquire.set_query(query)
    mset = enquire.get_mset(0, 10)
    items = [item for item in mset]
    expect(len(items), 5)

    context("testing returned item from mset")
    expect(items[2].docid, 4)
    expect(items[2].rank, 2)
    expect(items[2].percent, 81)
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
                hit = submset.get_hit(num)
                expect(hit.get_docid(), item.docid)
                expect(hit.get_rank(), item.rank)
                expect(hit.get_percent(), item.percent)
                expect(hit.get_document().get_data(), item.document.get_data())
                expect(hit.get_collapse_count(), item.collapse_count)

                hit = mset.get_hit(num + start)
                expect(hit.get_docid(), item.docid)
                expect(hit.get_rank(), item.rank)
                expect(hit.get_percent(), item.percent)
                expect(hit.get_document().get_data(), item.document.get_data())
                expect(hit.get_collapse_count(), item.collapse_count)

                expect(submset[num].docid, item.docid)
                expect(submset[num].rank, item.rank)
                expect(submset[num].percent, item.percent)
                expect(submset[num].document.get_data(), item.document.get_data())
                expect(submset[num].collapse_count, item.collapse_count)

                num += 1

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

            context("checking length of sub-mset from %d, maxitems %d" % (start, maxitems))
            items = [item for item in submset]
            expect(len(items), min(maxitems, 5 - start))
            expect(len(submset), min(maxitems, 5 - start))

# Run all tests (ie, callables with names starting "test_").
if not runtests(globals()):
    sys.exit(1)

# vim:syntax=python:set expandtab:
