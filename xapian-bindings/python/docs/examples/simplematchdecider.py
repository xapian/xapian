#!/usr/bin/env python
#
# $Id$
# Simple command-line match decider example
#
# ----START-LICENCE----
# Copyright 2003 James Aylett
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

MAX_PROB_TERM_LENGTH = 64

if len(sys.argv) < 4:
    print >> sys.stderr, "usage: %s <path to database> <avoid-term> <search terms>" % sys.argv[0]
    sys.exit(1)

class mymatcher(xapian.MatchDecider):
    def __init__(self, avoidterm):
        xapian.MatchDecider.__init__(self)
        self.avoidterm = avoidterm
        
    def __call__(self, doc):
#        print "Called on document '%s'" % doc.get_description()
        ti = doc.termlist_begin()
        while ti!=doc.termlist_end():
#            print "Term '%s' (we are '%s')" % (ti.get_term(), self.avoidterm)
            if ti.get_term()==self.avoidterm:
                return 0
            ti.next()
        return 1

try:
    database = xapian.open(sys.argv[1])

    enquire = xapian.Enquire(database)
    stemmer = xapian.Stem("english")
#    subqs = []
    topquery = None
    for term in sys.argv[3:]:
        nextquery = xapian.Query(stemmer.stem_word(term.lower()))
        if topquery==None:
            topquery = nextquery
        else:
            topquery = xapian.Query(xapian.Query.OP_OR, topquery, nextquery)
#        subqs.append(xapian.Query(term))
#    query = xapian.Query(xapian.Query.OP_OR, subqs)
    query = topquery
    print "Performing query `%s'" % query.get_description()

    enquire.set_query(query)
    matcher = mymatcher(sys.argv[2])
    matches = enquire.get_mset(0, 10, None, matcher)

    print "%i results found" % matches.get_matches_estimated()
    i = matches.begin()
    while i!=matches.end():
        print "ID %i %i%% [%s]" % (i.get_docid(), i.get_percent(), i.get_document().get_data())
        i.next()

except:
    # FIXME: exception message
    print >> sys.stderr, "Exception"
    raise
