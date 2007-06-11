#!/usr/bin/env python
#
# Index each paragraph of a text file as a Xapian document.
#
# Copyright (C) 2003 James Aylett
# Copyright (C) 2004,2007 Olly Betts
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
# Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301
# USA

import sys
import xapian
import string


if len(sys.argv) != 2:
    print >> sys.stderr, "Usage: %s PATH_TO_DATABASE" % sys.argv[0]
    sys.exit(1)

try:
    # Open the database for update, creating a new database if necessary.
    database = xapian.WritableDatabase(sys.argv[1], xapian.DB_CREATE_OR_OPEN)

    indexer = xapian.TermGenerator()
    stemmer = xapian.Stem("english")
    indexer.set_stemmer(stemmer)

    para = ''
    try:
        for line in sys.stdin:
            line = string.strip(line)
            if line == '':
                if para != '':
                    # We've reached the end of a paragraph, so index it.
                    doc = xapian.Document()
                    doc.set_data(para)

                    indexer.set_document(doc)
                    indexer.index_text(para)

                    # Add the document to the database.
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
