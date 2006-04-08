#!/usr/bin/env python
#
# $Id$
# Simple command-line match decider example
#
# ----START-LICENCE----
# Copyright 2003 James Aylett
# Copyright 2004 Olly Betts
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
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
# USA
# -----END-LICENCE-----

import sys
import xapian

if len(sys.argv) < 4:
    print >> sys.stderr, "usage: %s <path to database> <avoid value> <search terms>" % sys.argv[0]
    sys.exit(1)

class mymatcher(xapian.MatchDecider):
    def __init__(self, avoidvalue):
        xapian.MatchDecider.__init__(self)
        self.avoidvalue = avoidvalue
        
    def __call__(self, doc):
        return doc.get_value(0) != self.avoidvalue

try:
    database = xapian.Database(sys.argv[1])

    enquire = xapian.Enquire(database)
    stemmer = xapian.Stem("english")
    terms = []
    for term in sys.argv[3:]:
        terms.append(stemmer.stem_word(term.lower()))
    query = xapian.Query(xapian.Query.OP_OR, terms)
    print "Performing query `%s'" % query.get_description()

    enquire.set_query(query)
    matcher = mymatcher(sys.argv[2])
    matches = enquire.get_mset(0, 10, None, matcher)

    print "%i results found" % matches.get_matches_estimated()
    for match in matches:
        print "ID %i %i%% [%s]" % (match[xapian.MSET_DID], match[xapian.MSET_PERCENT], match[xapian.MSET_DOCUMENT].get_data())

except Exception, e:
    print >> sys.stderr, "Exception: %s" % str(e)
    sys.exit(1)
