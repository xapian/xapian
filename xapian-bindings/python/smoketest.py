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

try:
    # Test the version number reporting functions give plausible results.
    v = "%d.%d.%d" % (xapian.xapian_major_version(),
                      xapian.xapian_minor_version(),
                      xapian.xapian_revision())
    if v != xapian.xapian_version_string():
        sys.exit(1)
    stem = xapian.Stem("english")
    if stem.get_description() != "Xapian::Stem(english)":
        sys.exit(1)
    doc = xapian.Document()
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
        sys.exit(1)
    terms = ["smoke", "test", "terms"]
    query = xapian.Query(xapian.Query.OP_OR, terms)
    if query.get_description() != "Xapian::Query((smoke OR test OR terms))":
        sys.exit(1)
    query1 = xapian.Query(xapian.Query.OP_PHRASE, ("smoke", "test", "tuple"))
    if query1.get_description() != "Xapian::Query((smoke PHRASE 3 test PHRASE 3 tuple))":
        sys.exit(1)
    query2 = xapian.Query(xapian.Query.OP_XOR, (xapian.Query("smoke"), query1, "string"))
    if query2.get_description() != "Xapian::Query((smoke XOR (smoke PHRASE 3 test PHRASE 3 tuple) XOR string))":
        sys.exit(1)
    subqs = ["a", "b"]
    query3 = xapian.Query(xapian.Query.OP_OR, subqs)
    if query3.get_description() != "Xapian::Query((a OR b))":
        sys.exit(1)
    enq = xapian.Enquire(db)
    enq.set_query(xapian.Query(xapian.Query.OP_OR, "there", "is"))
    mset = enq.get_mset(0, 10)
    if mset.size() != 1:
        sys.exit(1)
    terms = " ".join(enq.get_matching_terms(mset.get_hit(0)))
    if terms != "is there":
        sys.exit(1)
except:
    sys.exit(1)

# vim:syntax=python:set expandtab:
