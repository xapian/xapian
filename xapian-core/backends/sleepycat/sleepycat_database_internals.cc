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
#define FILENAME_DOCKEYDB "dockey.db"
#define FILENAME_STATS_DB "stats.db"

enum {
    RECNUM_DOCCOUNT = 1,
    RECNUM_TOTALLEN = 2
};

SleepyDatabaseInternals::SleepyDatabaseInternals()
	: opened(false),
	  postlist_db(0),
	  termlist_db(0),
	  termid_db(0),
	  termname_db(0),
	  document_db(0),
	  key_db(0),
	  stats_db(0)
{
}

SleepyDatabaseInternals::~SleepyDatabaseInternals()
{
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

	Db::open(FILENAME_DOCKEYDB, DB_HASH, dbflags, mode,
		 &dbenv, 0, &key_db);

	Db::open(FILENAME_STATS_DB, DB_RECNO, dbflags, mode,
		 &dbenv, 0, &stats_db);
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
	if(key_db) key_db->close(0);
	key_db = 0;
	if(stats_db) stats_db->close(0);
	stats_db = 0;

	if(opened) dbenv.appexit();
	opened = false;
    }
    catch (DbException e) {
	throw (OmDatabaseError(string("Database error on close: ") + e.what()));
    }
}

om_doccount
SleepyDatabaseInternals::get_doccount() const
{
    const db_recno_t doccount_recnum = RECNUM_DOCCOUNT;

    Assert(opened);
    om_doccount doccount = 0;
    try {
	Dbt key(const_cast<db_recno_t *>(&doccount_recnum),
		sizeof(doccount_recnum));

	Dbt data;
	// FIXME - this flag results in extra copying - more inefficiency.
	// We should use DB_DBT_USERMEM and DB_DBT_PARTIAL
	data.set_flags(DB_DBT_MALLOC);

	int err_num = stats_db->get(0, &key, &data, 0);
	if(err_num == DB_NOTFOUND) {
	    // Document count hasn't been written: we have an empty database
	} else {
	    // Any other errors should cause an exception.
	    Assert(err_num == 0);

	    // FIXME: endian safety, check length, etc
	    doccount = *(reinterpret_cast<om_doccount *>(data.get_data()));

	    free(data.get_data());
	}
    }
    catch (DbException e) {
	throw (OmDatabaseError(string("Database error retrieving document count: ") + e.what()));
    }
    return doccount;
}

om_totlength
SleepyDatabaseInternals::get_totlength() const
{
    const db_recno_t doccount_totallen = RECNUM_TOTALLEN;

    Assert(opened);
    om_doclength totlength = 0;
    try {
	Dbt key(const_cast<db_recno_t *>(&doccount_totallen),
		sizeof(doccount_totallen));

	Dbt data;
	// FIXME - this flag results in extra copying - more inefficiency.
	// We should use DB_DBT_USERMEM and DB_DBT_PARTIAL
	data.set_flags(DB_DBT_MALLOC);

	int err_num = stats_db->get(0, &key, &data, 0);
	if(err_num == DB_NOTFOUND) {
	    // Document count hasn't been written: we have an empty database
	} else {
	    // Any other errors should cause an exception.
	    Assert(err_num == 0);

	    // FIXME: endian safety, check length, etc
	    totlength = *(reinterpret_cast<om_doclength *>(data.get_data()));

	    free(data.get_data());
	}
    }
    catch (DbException e) {
	throw (OmDatabaseError(string("Database error retrieving document lengths: ") + e.what()));
    }
    return totlength;
}


void
SleepyDatabaseInternals::set_doccount(om_doccount doccount)
{
    const db_recno_t doccount_recnum = RECNUM_DOCCOUNT;

    Assert(opened);
    try {
	Dbt key(const_cast<db_recno_t *>(&doccount_recnum),
		sizeof(doccount_recnum));
	Dbt data(const_cast<om_doccount *>(&doccount),
		 sizeof(doccount));

#ifdef MUS_DEBUG
	int err_num =
#endif
		stats_db->put(0, &key, &data, 0);

	// Any errors should cause an exception.
	Assert(err_num == 0);
    }
    catch (DbException e) {
	throw (OmDatabaseError(string("Database error setting document count: ") + e.what()));
    }
}

void
SleepyDatabaseInternals::set_totlength(om_totlength totlength)
{
    const db_recno_t doccount_totallen = RECNUM_TOTALLEN;

    Assert(opened);
    try {
	Dbt key(const_cast<db_recno_t *>(&doccount_totallen),
		sizeof(doccount_totallen));
	Dbt data(const_cast<om_doclength *>(&totlength),
		 sizeof(totlength));

#ifdef MUS_DEBUG
	int err_num =
#endif
		stats_db->put(0, &key, &data, 0);

	// Any errors should cause an exception.
	Assert(err_num == 0);
    }
    catch (DbException e) {
	throw (OmDatabaseError(string("Database error setting total document length: ") + e.what()));
    }
}

