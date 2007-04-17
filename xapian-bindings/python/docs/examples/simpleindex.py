#!/usr/bin/env python
#
# $Id$
# Index each paragraph of a textfile as a document
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
import string

MAX_PROB_TERM_LENGTH = 64

def p_alnum(c):
    return (c in string.ascii_letters or c in string.digits)

def p_notalnum(c):
    return not p_alnum(c)

def p_notplusminus(c):
    return c != '+' and c != '-'

def find_p(string, start, predicate):
    while start<len(string) and not predicate(string[start]):
        start += 1
    return start

if len(sys.argv) != 2:
    print >> sys.stderr, "usage: %s <path to database>" % sys.argv[0]
    sys.exit(1)

try:
    database = xapian.WritableDatabase(sys.argv[1], xapian.DB_CREATE_OR_OPEN)

    stemmer = xapian.Stem("english")
    para = ''
    try:
        for line in sys.stdin:
            line = string.strip(line)
            if line == '':
                if para != '':
                    doc = xapian.Document()
                    doc.set_data(para)
                    pos = 0
                    # At each point, find the next alnum character (i), then
                    # find the first non-alnum character after that (j). Find
                    # the first non-plusminus character after that (k), and if
                    # k is non-alnum (or is off the end of the para), set j=k.
                    # The term generation string is [i,j), so len = j-i
                    i = 0
                    while i < len(para):
                        i = find_p(para, i, p_alnum)
                        j = find_p(para, i, p_notalnum)
                        k = find_p(para, j, p_notplusminus)
                        if k == len(para) or not p_alnum(para[k]):
                            j = k
                        if (j - i) <= MAX_PROB_TERM_LENGTH and j > i:
                            term = stemmer(string.lower(para[i:j]))
                            doc.add_posting(term, pos)
                            pos += 1
                        i = j
                    database.add_document(doc)
                    para = ''
            else:
                if para != '':
                    para += ' '
                para += line
    except StopIteration:
        pass

except Exception, e:
    print >> sys.stderr, "Exception: %s" % str(e)
    sys.exit(1)
