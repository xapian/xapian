#!/usr/bin/env python
#
# Simple example script demonstrating query expansion.
#
# Copyright (C) 2003 James Aylett
# Copyright (C) 2004,2006,2007,2012,2013 Olly Betts
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

# We require at least two command line arguments.
if len(sys.argv) < 3:
    print >> sys.stderr, "Usage: %s PATH_TO_DATABASE QUERY [-- [DOCID...]]" % sys.argv[0]
    sys.exit(1)

try:
    # Open the database for searching.
    database = xapian.Database(sys.argv[1])

    # Start an enquire session.
    enquire = xapian.Enquire(database)

    # Combine command line arguments up to "--" with spaces between
    # them, so that simple queries don't have to be quoted at the shell
    # level.
    query_string = sys.argv[2]
    index = 3
    while index < len(sys.argv):
        arg = sys.argv[index]
        index += 1
        if arg == '--':
            # Passed marker, move to parsing relevant docids.
            break
        query_string += ' '
        query_string += arg

    # Create an RSet with the listed docids in.
    reldocs = xapian.RSet()
    for index in xrange(index, len(sys.argv)):
        reldocs.add_document(int(sys.argv[index]))

    # Parse the query string to produce a Xapian::Query object.
    qp = xapian.QueryParser()
    stemmer = xapian.Stem("english")
    qp.set_stemmer(stemmer)
    qp.set_database(database)
    qp.set_stemming_strategy(xapian.QueryParser.STEM_SOME)
    query = qp.parse_query(query_string)

    if not query.empty():
        print "Parsed query is: %s" % str(query)

        # Find the top 10 results for the query.
        enquire.set_query(query)
        matches = enquire.get_mset(0, 10, reldocs)

        # Display the results.
        print "%i results found." % matches.get_matches_estimated()
        print "Results 1-%i:" % matches.size()

        for m in matches:
            print "%i: %i%% docid=%i [%s]" % (m.rank + 1, m.percent, m.docid, m.document.get_data())

    # Put the top 5 (at most) docs into the rset if rset is empty
    if reldocs.empty():
        rel_count = 0
        for m in matches:
            reldocs.add_document(m.docid)
            rel_count += 1
            if rel_count == 5:
                break

    # Get the suggested expand terms
    eterms = enquire.get_eset(10, reldocs)
    print "%i suggested additional terms" % eterms.size()
    for k in eterms:
        print "%s: %f" % (k.term, k.weight)

except Exception, e:
    print >> sys.stderr, "Exception: %s" % str(e)
    sys.exit(1)
