# Simple test that we can load the xapian module and run a simple test
#
# Copyright (C) 2004,2005 Olly Betts
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
    stem = xapian.Stem("english")
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
    query1 = xapian.Query(xapian.Query.OP_PHRASE, ("smoke", "test", "tuple"))
    query2 = xapian.Query(xapian.Query.OP_XOR, (query, query1, xapian.Query("a")))
    subqs = ["a", "b"]
    query3 = xapian.Query(xapian.Query.OP_OR, subqs)
except:
    sys.exit(1)
