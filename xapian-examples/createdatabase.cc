/* createdatabase.cc: Create an empty quartz database.
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
#include <sys/stat.h>

int
main(int argc, char **argv)
{
    // We take one argument - the pathname to the database to create
    if (argc != 2) {
	cout << "usage: " << argv[0] << " <path to database>" << endl;
	exit(1);
    }
    
    try {
	// Create the directory for the database, if it doesn't exist already
	if (mkdir(argv[1], 0755) == -1) {
	    // Check if mkdir failed because there's already a directory there
	    // or for some other reason.  EEXIST can also mean there's a file
	    // with that name already.
	    if (errno == EEXIST) {
		struct stat sb;
		if (stat(argv[1], &sb) == 0 && S_ISDIR(sb.st_mode))
		    errno = 0;
		else
		    errno = EEXIST;
	    }
	    if (errno) {
		if (errno < sys_nerr && sys_errlist[errno]) {
		    cerr << argv[0] << ": " << sys_errlist[errno] << " ("
			 << argv[1] << ")" << endl;
		} else {
		    cerr << argv[0] << ": couldn't create directory `"
			 << argv[1] << "'" << endl;
		}
		exit(1);
	    }
	}

	// Create the database
	OmSettings settings;
	settings.set("backend", "quartz");
	settings.set("database_create", true);
	settings.set("quartz_dir", argv[1]);
	OmWritableDatabase database(settings);
    }
    catch (const OmError &error) {
	cerr << "Exception: " << error.get_msg() << endl;
	exit(1);
    }
    return 0;
}
