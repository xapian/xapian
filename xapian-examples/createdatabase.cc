/* createdatabase.cc: Create an empty Xapian database.
 *
 * ----START-LICENCE----
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002 Ananova Ltd
 * Copyright 2002,2003 Olly Betts
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

#include <xapian.h>

#include <iostream>

using namespace Xapian;
using namespace std;

int
main(int argc, char **argv)
{
    // We take one argument - the pathname to the database to create
    if (argc != 2) {
	cout << "usage: " << argv[0] << " <path to database>" << endl;
	exit(1);
    }
    
    try {
	// Create the database
	WritableDatabase database(Auto::open(argv[1], DB_CREATE));
    } catch (const Error &error) {
	cerr << argv[0] << ": " << error.get_msg() << endl;
	exit(1);
    }
}
