/* indexfile.cc: Simple indexer
 *
 * ----START-LICENCE----
 * Copyright 1999,2000 BrightStation PLC
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

int main(int argc, char *argv[])
{
    // Simplest possible options parsing: we just require three or more
    // parameters.
    if(argc != 3) {
	cout << "usage: " << argv[0] <<
		" <path to database> <file to index>" << endl;
	exit(1);
    }
    
    // Catch any OmError exceptions thrown
    try {
	// Make the database
	OmSettings db_parameters;
	settings.set("backend", "sleepycat");
	settings.set("sleepycat_dir", argv[1]);
	OmWritableDatabase database(settings);

	// Make the indexer
	OmIndexerBuilder builder;
	AutoPtr<OmIndexer> indexer = builder.build_from_file("indexfile.xml");

	// Make the document by invoking the indexer
	OmIndexerMessage filename(new OmIndexerData(argv[2]));
	indexer->set_input(filename);
	OmDocumentContents newdocument = indexer->get_output();

	cout << "Document data: " << newdocument.data.value << endl;
	cout << "Document keys: " << endl;
	for (OmDocumentContents::document_keys::const_iterator i =
	     newdocument.keys.begin();
	     i != newdocument.keys.end();
	     ++i) {
	    cout << "\t" << i->first << "\t" << i->second << endl;
	}

	cout << "Document terms: " << endl;
	for (OmDocumentContents::document_terms::const_iterator i =
	     newdocument.terms.begin();
	     i != newdocument.terms.end();
	     ++i) {
	    cout << "\t" << i->first << "\t" << i->second << endl;
	}

	// Add the document to the database
	database.add_document(newdocument);
    }
    catch(OmError &error) {
	cout << "Exception: "  << error.get_msg() << endl;
    }
}
