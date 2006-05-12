#!/usr/bin/env python
#
# $Id$
# Simple command-line query expand program
#
# Copyright 2003 James Aylett
# Copyright 2004,2006 Olly Betts
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

if len(sys.argv) < 3:
    print >> sys.stderr, "usage: %s <path to database> [<search terms>] [-- <relevant docids>]" % sys.argv[0]
    sys.exit(1)

try:
    database = xapian.Database(sys.argv[1])

    enquire = xapian.Enquire(database)
    stemmer = xapian.Stem("english")
    terms = []
    index = 2
    while index < len(sys.argv):
        term = sys.argv[index]
        index += 1
        if term == '--':
            # passed marker, move to relevant docids
            break
        terms.append(stemmer.stem_word(term.lower()))
    query = xapian.Query(xapian.Query.OP_OR, terms)

    # Prepare relevant document set (RSet)
    reldocs = xapian.RSet()
    if index < len(sys.argv):
        for index in xrange(index,len(sys.argv)):
            rdid = int(sys.argv[index])
            if rdid!=0:
                reldocs.add_document(rdid)

    matches = xapian.MSet()
    if not query.empty():
        print "Performing query `%s' against rset `%s'" % (query.get_description(),reldocs.get_description())

        enquire.set_query(query)
        matches = enquire.get_mset(0, 10, reldocs)

        print "%i results found" % matches.get_matches_estimated()
        for match in matches:
            print "ID %i %i%% [%s]" % (match[xapian.MSET_DID], match[xapian.MSET_PERCENT], match[xapian.MSET_DOCUMENT].get_data())

    # Put the top 5 (at most) docs into the rset if rset is empty
    if reldocs.is_empty():
        i = matches.begin()
        for j in xrange(1, 5):
            reldocs.add_document(i.get_docid())
            i.next()
            if i == matches.end():
                break

    # Get the suggested expand terms
    eterms = enquire.get_eset(10, reldocs)
    print "%i suggested additional terms" % eterms.size()
    k = eterms.begin()
    while k!=eterms.end():
        print "Term `%s'\t (weight %i)" % (k.get_termname(), k.get_weight())
        k.next()

except Exception, e:
    print >> sys.stderr, "Exception: %s" % str(e)
    sys.exit(1)
