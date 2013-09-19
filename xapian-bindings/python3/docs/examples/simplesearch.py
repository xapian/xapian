#!/usr/bin/env python
#
# Simple command-line search script.
#
# Copyright (C) 2003 James Aylett
# Copyright (C) 2004,2007,2009 Olly Betts
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
    print >> sys.stderr, "Usage: %s PATH_TO_DATABASE QUERY" % sys.argv[0]
    sys.exit(1)

try:
    # Open the database for searching.
    database = xapian.Database(sys.argv[1])

    # Start an enquire session.
    enquire = xapian.Enquire(database)

    # Combine the rest of the command line arguments with spaces between
    # them, so that simple queries don't have to be quoted at the shell
    # level.
    query_string = str.join(' ', sys.argv[2:])

    # Parse the query string to produce a Xapian::Query object.
    qp = xapian.QueryParser()
    stemmer = xapian.Stem("english")
    qp.set_stemmer(stemmer)
    qp.set_database(database)
    qp.set_stemming_strategy(xapian.QueryParser.STEM_SOME)
    query = qp.parse_query(query_string)
    print "Parsed query is: %s" % str(query)

    # Find the top 10 results for the query.
    enquire.set_query(query)
    matches = enquire.get_mset(0, 10)

    # Display the results.
    print "%i results found." % matches.get_matches_estimated()
    print "Results 1-%i:" % matches.size()

    for m in matches:
        print "%i: %i%% docid=%i [%s]" % (m.rank + 1, m.percent, m.docid, m.document.get_data())

except Exception, e:
    print >> sys.stderr, "Exception: %s" % str(e)
    sys.exit(1)
