/* indexfile.cc: Simple indexer
 *
 * ----START-LICENCE----
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002 Ananova Ltd
 * Copyright 2002 Olly Betts
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 * USA
 * -----END-LICENCE-----
 */

#include <om/om.h>
using std::cout;
using std::endl;

int main(int argc, char *argv[])
{
    // Simplest possible options parsing: we just require two parameters.
    if (argc != 3) {
	cout << "usage: " << argv[0] <<
		" <path to database> <file to index>" << endl;
	exit(1);
    }
    
    // Catch any OmError exceptions thrown
    try {
	// Create the database if needed
	OmWritableDatabase database(OmQuartz__open(argv[1], OM_DB_CREATE_OR_OPEN));

	// Make the indexer
	OmIndexerBuilder builder;
	OmIndexer indexer = builder.build_from_file("indexfile.xml");

	// Make the document by invoking the indexer
	OmIndexerMessage filename(argv[2]);
	indexer.set_input(filename);
	OmDocument newdocument = indexer.get_output();

	cout << "Document data: " << newdocument.get_data() << endl;
	cout << "Document values: " << endl;
	for (OmValueIterator i = newdocument.values_begin();
	     i != newdocument.values_end();
	     ++i) {
	    cout << "\t" << i.get_valueno() << "\t" << *i << endl;
	}

	cout << "Document terms: " << endl;
	for (OmTermIterator i = newdocument.termlist_begin();
	     i != newdocument.termlist_end();
	     ++i) {
	    cout << "\t" << *i << " [" << i.get_wdf() << "]" << endl;
	}

	// Add the document to the database
	database.add_document(newdocument);
    } catch (const OmError &error) {
	cout << error.get_type() << ": "  << error.get_msg() << endl;
	exit(1);
    }
}
