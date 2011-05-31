::

    /* quickstartsearch.cc: Simplest possible searcher
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
        // Simplest possible options parsing: we just require two or more
        // parameters.
        if (argc < 3) {
            cout << "usage: " << argv[0] <<
                    " <path to database> <search terms>" << endl;
            exit(1);
        }

        // Catch any Xapian::Error exceptions thrown
        try {
            // Make the database
        Xapian::Database db(argv[1]);

            // Start an enquire session
            Xapian::Enquire enquire(db);

            // Build the query object
            Xapian::Query query(Xapian::Query::OP_OR, argv + 2, argv + argc);
            cout << "Performing query `" << query.get_description() << "'" << endl;

            // Give the query object to the enquire session
            enquire.set_query(query);

            // Get the top 10 results of the query
            Xapian::MSet matches = enquire.get_mset(0, 10);

            // Display the results
            cout << matches.size() << " results found" << endl;

            for (Xapian::MSetIterator i = matches.begin();
                 i != matches.end();
                 ++i) {
                Xapian::Document doc = i.get_document();
                cout << "Document ID " << *i << "\t" <<
                        i.get_percent() << "% [" <<
                        doc.get_data() << "]" << endl;
            }
        } catch(const Xapian::Error &error) {
            cout << "Exception: "  << error.get_msg() << endl;
        }
    }

