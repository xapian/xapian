::

    /* quickstartindex.cc: Simplest possible indexer
     *
     * ----START-LICENCE----
     * Copyright 1999,2000,2001 BrightStation PLC
     * Copyright 2003,2004 Olly Betts
     *
     * This program is free software; you can redistribute it and/or
     * modify it under the terms of the GNU General Public License as
     * published by the Free Software Foundation; either version 2 of the
     * License, or (at your option) any later version.
     *
     * This program is distributed in the hope that it will be useful,
     * but WITHOUT ANY WARRANTY; without even the implied warranty of
     * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
     * GNU General Public License for more details.
     *
     * You should have received a copy of the GNU General Public License
     * along with this program; if not, write to the Free Software
     * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301
     * USA
     */

    #include <xapian.h>
    #include <iostream>
    using namespace std;

    int main(int argc, char **argv)
    {
        // Simplest possible options parsing: we just require three or more
        // parameters.
        if(argc < 4) {
            cout << "usage: " << argv[0] <<
                    " <path to database> <document data> <document terms>" << endl;
            exit(1);
        }

        // Catch any Xapian::Error exceptions thrown
        try {
            // Make the database
            Xapian::WritableDatabase database(argv[1], Xapian::DB_CREATE_OR_OPEN);

            // Make the document
            Xapian::Document newdocument;

            // Put the data in the document
            newdocument.set_data(string(argv[2]));

            // Put the terms into the document
            for (int i = 3; i < argc; ++i) {
                newdocument.add_posting(argv[i], i - 2);
            }

            // Add the document to the database
            database.add_document(newdocument);
        } catch(const Xapian::Error &error) {
            cout << "Exception: "  << error.get_msg() << endl;
        }
    }

