/* sleepy_database.cc: interface to sleepycat database routines
 *
 * ----START-LICENCE----
 * Copyright 1999,2000 Dialog Corporation
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


#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string>
#include <algorithm>

// Sleepycat database stuff
#include <db_cxx.h>

#include "omassert.h"
#include "om/omerror.h"
#include "sleepy_database_internals.h"

// Standard names of files within database directory
#define FILENAME_POSTLIST "postlist.db"
#define FILENAME_TERMLIST "termlist.db"
#define FILENAME_TERMTOID "termid.db"
#define FILENAME_IDTOTERM "termname.db"

SleepyDatabaseInternals::SleepyDatabaseInternals() {
    postlist_db = NULL;
    termlist_db = NULL;
    termid_db = NULL;
    termname_db = NULL;
    opened = false;
}

SleepyDatabaseInternals::~SleepyDatabaseInternals(){
    close();
}

void
SleepyDatabaseInternals::open(const string &pathname, bool readonly)
{
    try {
	// Set up environment
	u_int32_t flags = DB_INIT_CDB;
	int mode = 0;

	if(readonly) {
	    flags = DB_RDONLY;
	} else {
	    flags = DB_CREATE;
	    mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH;
	}
	dbenv.appinit(pathname.c_str(), NULL, flags);
	opened = true;

	Db::open(FILENAME_POSTLIST, DB_BTREE, flags, mode,
		 &dbenv, NULL, &postlist_db);

	Db::open(FILENAME_TERMLIST, DB_BTREE, flags, mode,
		 &dbenv, NULL, &termlist_db);

	Db::open(FILENAME_TERMTOID, DB_HASH, flags, mode,
		 &dbenv, NULL, &termid_db);

	Db::open(FILENAME_IDTOTERM, DB_RECNO, flags, mode,
		 &dbenv, NULL, &termname_db);
    }
    catch (DbException e) {
	throw (OmOpeningError(string("Database error on open: ") + e.what()));
    }
}

void
SleepyDatabaseInternals::close()
{
    try {
	if(postlist_db) postlist_db->close(0);
	postlist_db = NULL;
	if(termlist_db) termlist_db->close(0);
	termlist_db = NULL;
	if(termid_db) termid_db->close(0);
	termid_db = NULL;
	if(termname_db) termname_db->close(0);
	termname_db = NULL;

	if(opened) dbenv.appexit();
	opened = false;
    }
    catch (DbException e) {
	throw (OmDatabaseError(string("Database error on close: ") + e.what()));
    }
}
