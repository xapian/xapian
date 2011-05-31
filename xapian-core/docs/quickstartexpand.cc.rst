::

    /* quickstartexpand.cc: Simplest possible query expansion
     *
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
                    " <path to database> <search terms> -- <relevant docids>" <<
                    endl;
            exit(1);
        }

        // Catch any Xapian::Error exceptions thrown
        try {
            // Open the database
            Xapian::Database database(argv[1]);

            // Start an enquire session
            Xapian::Enquire enquire(database);

            // Prepare the query terms
            vector<string> queryterms;
            int optpos;
            for (optpos = 2; optpos < argc; optpos++) {
                if(string(argv[optpos]) == "--") {
                    optpos++;
                    break;
                }
                queryterms.push_back(argv[optpos]);
            }

            // Prepare the relevant document list
            Xapian::RSet reldocs;
            for (; optpos < argc; optpos++) {
                Xapian::docid rdid = atoi(argv[optpos]);
                if (rdid != 0) {
                    reldocs.add_document(rdid);
                }
            }

            // Build the query object
            Xapian::Query query(Xapian::Query::OP_OR, queryterms.begin(), queryterms.end());

            Xapian::MSet matches;
            if (query.is_defined()) {
                cout << "Performing query `" << query.get_description() <<
                        "'" << endl;

                // Give the query object to the enquire session
                enquire.set_query(query);

                // Get the top 10 results of the query
                matches = enquire.get_mset(0, 10);

                // Display the results
                cout << matches.items.size() << " results found" << endl;

                for (Xapian::MSetIterator i = matches.begin();
                     i != matches.end();
                     ++i) {
                    Xapian::Document doc = i.get_document();
                    cout << "Document ID " << *i << "\t" <<
                            i.get_percent() << "% [" <<
                            doc.get_data() << "]" << endl;
                }
            }

            // Put the top 5 into the rset if rset is empty
            if (reldocs.empty()) {
                Xapian::MSetIterator i;
                int j;
                for (i = matches.begin(),
                     j = 0;
                     (i != matches.end()) && (j < 5);
                     ++i, ++j) {
                    reldocs.add_document(*i);
                }
            }
            
            // Get the suggested expand terms
            Xapian::ESet eterms = enquire.get_eset(10, reldocs);

            // Display the expand terms
            cout << eterms.size() << " suggested additional terms" << endl;

            for (Xapian::ESetIterator k = eterms.begin();
                 k != eterms.end();
                 ++k) {
                cout << "Term `" << *k << "'\t " <<
                        "(weight " << k.get_weight() << ")" << endl;
            }
        } catch(const Xapian::Error &error) {
            cout << "Exception: "  << error.get_msg() << endl;
        }
    }

