/* copydatabase.cc: Document-by-document copy of one or more Xapian databases
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

#include <iostream>

using namespace std;

#include <errno.h>
#include <string.h>
#include <sys/stat.h>

int
main(int argc, char **argv)
{
    // We take at least two arguments - one or more paths to source databases
    // and the path to the database to create
    if (argc < 3) {
	cout << "usage: " << argv[0]
	     << " <path to source database>... <path to destination database>"
	     << endl;
	exit(1);
    }

    const char *dest = argv[argc - 1];
    try {
	// Create the directory for the database, if it doesn't exist already
	if (mkdir(dest, 0755) == -1) {
	    // Check if mkdir failed because there's already a directory there
	    // or for some other reason.  EEXIST can also mean there's a file
	    // with that name already.
	    if (errno == EEXIST) {
		struct stat sb;
		if (stat(dest, &sb) == 0 && S_ISDIR(sb.st_mode))
		    errno = 0;
		else
		    errno = EEXIST; // stat might have changed it
	    }
	    if (errno) {
		cerr << argv[0] << ": couldn't create directory `"
		     << dest << "': " << strerror(errno) << endl;
		exit(1);
	    }
	}

	// Create the destination database
	OmSettings dest_settings;
	dest_settings.set("backend", "auto");
	dest_settings.set("database_create", true);
	dest_settings.set("auto_dir", dest);
	OmWritableDatabase dest_database(dest_settings);

	for (int i = 1; i < argc - 1; ++i) {
	    // Open the source database
	    OmSettings src_settings;
	    src_settings.set("backend", "auto");
	    src_settings.set("auto_dir", argv[i]);
	    OmDatabase src_database(src_settings);

	    // Copy each document across

	    // At present there's no way to iterate across all documents
	    // so we have to test each docid in turn until we've found all
	    // the documents
	    om_doccount count = src_database.get_doccount();
	    om_docid docid = 1;
	    while (count) {
		try {
		    OmDocument document = src_database.get_document(docid);
		    dest_database.add_document(document);
		    --count;
		    cout << '\r' << argv[i] << ": " << count
			 << " docs to go" << flush;
		} catch (const OmDocNotFoundError &/*error*/) {
		    // that document must have been deleted
		}
		if (docid == (om_docid)-1) break;
		++docid;
	    }
	    cout << '\r' << argv[i] << ": Done                  " << endl;
	}
    }
    catch (const OmError &error) {
	cerr << argv[0] << ": " << error.get_msg() << endl;
	exit(1);
    }
    return 0;
}
