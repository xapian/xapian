/* indexfile.cc: Simple indexer
 *
 * ----START-LICENCE----
 * Copyright 1999,2000,2001 BrightStation PLC
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
    // Simplest possible options parsing: we just require two parameters.
    if(argc != 3) {
	std::cout << "usage: " << argv[0] <<
		" <path to database> <file to index>" << std::endl;
	exit(1);
    }
    
    // Catch any OmError exceptions thrown
    try {
	// Create the database if needed
	OmSettings db_parameters;
	db_parameters.set("backend", "quartz");
	db_parameters.set("database_create", true);
	db_parameters.set("quartz_dir", argv[1]);
	try {
	    OmWritableDatabase database(db_parameters);
	} catch(const OmDatabaseCreateError &) {
	    // ok, database exists
	}
	// unset the "create" flag
	db_parameters.set("database_create", false);
	OmWritableDatabase database(db_parameters);

	// Make the indexer
	OmIndexerBuilder builder;
	OmIndexer indexer = builder.build_from_file("indexfile.xml");

	// Make the document by invoking the indexer
	OmIndexerMessage filename(argv[2]);
	indexer.set_input(filename);
	OmDocument newdocument = indexer.get_output();

	std::cout << "Document data: " << newdocument.get_data().value << std::endl;
	std::cout << "Document keys: " << std::endl;
	for (OmKeyListIterator i = newdocument.keylist_begin();
	     i != newdocument.keylist_end();
	     ++i) {
	    std::cout << "\t" << i.get_keyno() << "\t" << *i << std::endl;
	}

	std::cout << "Document terms: " << std::endl;
	for (OmTermIterator i = newdocument.termlist_begin();
	     i != newdocument.termlist_end();
	     ++i) {
	    std::cout << "\t" << *i << " [" << i.get_wdf() << "]" << std::endl;
	}

	// Add the document to the database
	database.add_document(newdocument);
    }
    catch(OmError &error) {
	std::cout << error.get_type() << ": "  << error.get_msg() << std::endl;
    }
}
