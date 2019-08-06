#!/usr/bin/env python
#
# Simple example script demonstrating how to re-rank using the trained model.
# Copyright (C) 2004,2005,2006,2007,2008,2009,2010,2015 Olly Betts
# Copyright (C) 2011 Parth Gupta
# Copyright (C) 2016 Ayush Tomar
# Copyright (C) 2019 Vaibhav Kansagara
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
import xapianletor

# We require three command line arguments.
if len(sys.argv) != 4:
    print("Usage: %s DATABASE MSIZE QUERY" % sys.argv[0],
          "NB: QUERY should be quoted to protect it from the shell.",
          sep='\n', file=sys.stderr)
    sys.exit(1)

try:
    db_path = sys.argv[1]
    msize = sys.argv[2]
    query_string = sys.argv[3]

    db = xapian.Database(db_path)

    parser = xapian.QueryParser()
    parser.add_prefix("title", "S")
    parser.add_prefix("subject", "S")
    parser.set_database(db)
    parser.set_default_op(xapian.Query.OP_OR)
    parser.set_stemmer(xapian.Stem("en"))
    parser.set_stemming_strategy(xapian.QueryParser.STEM_SOME)

    query_no_prefix = parser.parse_query(query_string,
                                         parser.FLAG_DEFAULT|
                                         parser.FLAG_SPELLING_CORRECTION)
    # query with title as default prefix
    query_default_prefix = parser.parse_query(query_string,
                                              parser.FLAG_DEFAULT|
                                              parser.FLAG_SPELLING_CORRECTION,
                                              "S")
    # Combine queries
    query = xapian.Query(xapian.Query.OP_OR, query_no_prefix, query_default_prefix)

    enquire = xapian.Enquire(db)
    enquire.set_query(query)

    mset = enquire.get_mset(0, msize)

    if mset.empty():
        print("Empty MSet. No documents could be retrieved with the given Query.")
        sys.exit(0)

    print("Docids before re-ranking by LTR model:\n")
    for m in mset:
        print("%i: docid=%i [%s]" % (m.rank + 1, m.docid, m.document.get_data().decode('utf-8')))

    # Initialise Ranker object with ListNETRanker instance, db path and query.
    # See Ranker documentation for available Ranker subclass options.
    ranker = xapianletor.ListNETRanker()
    ranker.set_database_path(db_path)
    ranker.set_query(query)

    # Re-rank the existing mset using the letor model.
    ranker.rank(mset)

    print("Docids after re-ranking by LTR model:\n")

    for m in mset:
        print("%i: docid=%i [%s]" % (m.rank + 1, m.docid, m.document.get_data().decode('utf-8')))

except xapian.QueryParserError as e:
    print("Couldn't parse query: %s" % str(e), file=sys.stderr)
    sys.exit(1)

except Exception as e:
    print("Exception: %s" % str(e), file=sys.stderr)
    sys.exit(1)
