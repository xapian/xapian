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
#define FILENAME_DOCUMENT "document.db"

SleepyDatabaseInternals::SleepyDatabaseInternals() {
    postlist_db = 0;
    termlist_db = 0;
    termid_db = 0;
    termname_db = 0;
    document_db = 0;
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
	u_int32_t envflags = 0;
	u_int32_t dbflags = 0;
	int mode = 0;

	if(readonly) {
	    dbflags = DB_RDONLY;
	    envflags = 0;
	} else {
	    envflags = DB_CREATE;
	    dbflags = DB_CREATE;
	    mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH;
	}
	dbenv.appinit(pathname.c_str(), 0, envflags);
	opened = true;

	Db::open(FILENAME_POSTLIST, DB_BTREE, dbflags, mode,
		 &dbenv, 0, &postlist_db);

	Db::open(FILENAME_TERMLIST, DB_BTREE, dbflags, mode,
		 &dbenv, 0, &termlist_db);

	Db::open(FILENAME_TERMTOID, DB_HASH, dbflags, mode,
		 &dbenv, 0, &termid_db);

	Db::open(FILENAME_IDTOTERM, DB_RECNO, dbflags, mode,
		 &dbenv, 0, &termname_db);

	Db::open(FILENAME_DOCUMENT, DB_RECNO, dbflags, mode,
		 &dbenv, 0, &document_db);
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
	postlist_db = 0;
	if(termlist_db) termlist_db->close(0);
	termlist_db = 0;
	if(termid_db) termid_db->close(0);
	termid_db = 0;
	if(termname_db) termname_db->close(0);
	termname_db = 0;
	if(document_db) document_db->close(0);
	document_db = 0;

	if(opened) dbenv.appexit();
	opened = false;
    }
    catch (DbException e) {
	throw (OmDatabaseError(string("Database error on close: ") + e.what()));
    }
}
