/* copydatabase.cc: Document-by-document copy of quartz db
 *
 * ----START-LICENCE----
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002 Ananova Ltd
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
    // We take two arguments - the source databse pathname and the 
    // pathname to the database to create
    if (argc != 3) {
	cout << "usage: " << argv[0] << " <path to source database> <path to destination database>" << endl;
	exit(1);
    }
    
    try {
	// Create the directory for the database, if it doesn't exist already
	if (mkdir(argv[2], 0755) == -1) {
	    // Check if mkdir failed because there's already a directory there
	    // or for some other reason.  EEXIST can also mean there's a file
	    // with that name already.
	    if (errno == EEXIST) {
		struct stat sb;
		if (stat(argv[2], &sb) == 0 && S_ISDIR(sb.st_mode))
		    errno = 0;
		else
		    errno = EEXIST;
	    }
	    if (errno) {
		cerr << argv[0] << ": couldn't create directory `"
		     << argv[2] << "': " << strerror(errno) << endl;
		exit(1);
	    }
	}

	// Create the databases
	OmSettings src_settings;
	src_settings.set("backend", "quartz");
	src_settings.set("database_create", true);
	src_settings.set("quartz_dir", argv[1]);
	OmDatabase src_database(src_settings);

	OmSettings dest_settings;
	dest_settings.set("backend", "quartz");
	dest_settings.set("database_create", true);
	dest_settings.set("quartz_dir", argv[2]);
	OmWritableDatabase dest_database(dest_settings);

	// Now start copying mate!
	om_doccount sofar=0;
	om_doccount count=src_database.get_doccount();
	for(om_docid docid=1; docid<2000000000 && sofar<count;docid++) {
	  try {
	    OmDocument document=src_database.get_document(docid);
	    if (dest_database.add_document(document) && ! (++sofar % 1000)) dest_database.flush();
	    cout << sofar << "/" << count << endl;
	  } catch (const OmDocNotFoundError &error) {
cout << "No " << docid << endl;
	    continue;
	  }
	}
	dest_database.flush();
    }
    catch (const OmError &error) {
	cerr << argv[0] << ": " << error.get_msg() << endl;
	exit(1);
    }
    return 0;
}
