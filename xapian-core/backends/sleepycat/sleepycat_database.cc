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
#include "sleepy_database.h"
#include "sleepy_list.h"


#define FILENAME_POSTLIST "postlist.db"
#define FILENAME_TERMLIST "termlist.db"
#define FILENAME_TERMTOID "termid.db"
#define FILENAME_IDTOTERM "termname.db"

/////////////////////////////
// Internal database state //
/////////////////////////////

class SleepyDatabaseInternals {
    private:
	DbEnv dbenv;
	bool opened;
    public:
	Db *postlist_db;
	Db *termlist_db;
	Db *termid_db;
	Db *termname_db;

	SleepyDatabaseInternals();
	~SleepyDatabaseInternals();
	void open(const string & pathname, bool readonly);
	void close();
};

inline
SleepyDatabaseInternals::SleepyDatabaseInternals() {
    postlist_db = NULL;
    termlist_db = NULL;
    termid_db = NULL;
    termname_db = NULL;
    opened = false;
}

inline
SleepyDatabaseInternals::~SleepyDatabaseInternals(){
    close();
}

inline void
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
	throw (OmError(string("Database error on open: ") + e.what()));
    }
}

inline void
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
	throw (OmError(string("Database error on close: ") + e.what()));
    }
}

///////////////
// Postlists //
///////////////

SleepyPostList::SleepyPostList(const termname &tn, docid *data_new, doccount tf)
	: pos(0), data(data_new), tname(tn), termfreq(tf)
{
}

SleepyPostList::~SleepyPostList() {
    free(data);
}

weight SleepyPostList::get_weight() const {
    Assert(!at_end());
    Assert(ir_wt != NULL);
    
    doccount wdf = 1;

    return ir_wt->get_weight(wdf, 1.0);
}

///////////////
// Termlists //
///////////////

SleepyTermList::SleepyTermList(const SleepyDatabaseTermCache *tc_new,
			       termid *data_new,
			       termcount terms_new,
			       doccount dbsize_new)
	: pos(0),
	  data(data_new),
	  terms(terms_new),
	  dbsize(dbsize_new),
	  termcache(tc_new)
{ return; }

SleepyTermList::~SleepyTermList() {
    free(data);
}

///////////////////////////
// Actual database class //
///////////////////////////

SleepyDatabase::SleepyDatabase() {
    internals = new SleepyDatabaseInternals();
    termcache = new SleepyDatabaseTermCache(internals);
    Assert((opened = false) == false);
}

SleepyDatabase::~SleepyDatabase() {
    // Close databases
    try {
	delete termcache;
	internals->close();
	delete internals;
    }
    catch (DbException e) {
	throw (OmError(string("Database error on close: ") + e.what()));
    }
    Assert((opened = false) == false);
}

void
SleepyDatabase::open(const DatabaseBuilderParams &params)
{
    Assert(!opened);

    // Check validity of parameters
    Assert(params.paths.size() == 1);
    Assert(params.subdbs.size() == 0);

    // Open database with specified path
    try {
	internals->open(params.paths[0], params.readonly);
    }
    catch (DbException e) {
	throw (OmError(string("Database error on open: ") + e.what()));
    }
    Assert((opened = true) == true);
}

DBPostList *
SleepyDatabase::open_post_list(const termname & tname, RSet *rset) const
{
    Assert(opened);
    termid tid = termcache->term_name_to_id(tname);
    Assert(tid != 0);

    Dbt key(&tid, sizeof(tid));
    Dbt data;

    // FIXME - should use DB_DBT_USERMEM and DB_DBT_PARTIAL eventually
    data.set_flags(DB_DBT_MALLOC);

    // Get, no transactions, no flags
    try {
	int found = internals->postlist_db->get(NULL, &key, &data, 0);
	if(found == DB_NOTFOUND) throw RangeError("Termid not found");

	// Any other errors should cause an exception.
	Assert(found == 0);
    }
    catch (DbException e) {
	throw OmError("PostlistDb error:" + string(e.what()));
    }

    return new SleepyPostList(tname, (docid *)data.get_data(),
			      data.get_size() / sizeof(docid));
}

DBTermList *
SleepyDatabase::open_term_list(docid did) const {
    Assert(opened);
    Dbt key(&did, sizeof(did));
    Dbt data;

    // FIXME - should use DB_DBT_USERMEM and DB_DBT_PARTIAL eventually
    data.set_flags(DB_DBT_MALLOC);

    // Get, no transactions, no flags
    try {
	int found = internals->termlist_db->get(NULL, &key, &data, 0);
	if(found == DB_NOTFOUND) throw RangeError("Docid not found");

	// Any other errors should cause an exception.
	Assert(found == 0);
    }
    catch (DbException e) {
	throw OmError("TermlistDb error:" + string(e.what()));
    }

    return new SleepyTermList(termcache,
			      (termid *)data.get_data(),
			      data.get_size() / sizeof(termid),
			      get_doccount());
}

IRDocument *
SleepyDatabase::open_document(docid did) const {
    Assert(opened);
    throw OmError("SleepyDatabase.open_document() not implemented");
}

/*
termid
SleepyDatabase::add_term(const termname &tname) {
    Assert(opened);

    termid newid;
    newid = term_name_to_id(tname);
    if(newid) return newid;

    try {
	// FIXME - currently no transactions
	Dbt key(&newid, sizeof(newid));
	Dbt data((void *)tname.data(), tname.size());
	int found;

	key.set_flags(DB_DBT_USERMEM);

	// Append to list of terms sorted by id - gets new id
	found = internals->termname_db->put(NULL, &key, &data, DB_APPEND);
	Assert(found == 0); // Any errors should cause an exception.
	
	// Put in termname to id database
	found = internals->termid_db->put(NULL, &data, &key, 0);
	Assert(found == 0); // Any errors should cause an exception.
    }
    catch (DbException e) {
	throw OmError("SleepyDatabase::add_term(): " + string(e.what()));
    }

    return newid;
}

void
SleepyDatabase::add(termid tid, docid did, termpos tpos) {
    Assert(opened);

    SleepyList pl(internals->postlist_db);
    pl.open(&tid, sizeof(tid));
    pl.add(did, 1);
    pl.close();
    
    // Add to Postlist
    try {
	// First see if appropriate postlist already exists
	Dbt key(&tid, sizeof(tid));
	Dbt data;
	data.set_flags(DB_DBT_MALLOC);
	docid * postlist = NULL;
	size_t postlist_size = 0;
	int found;

	found = internals->postlist_db->get(NULL, &key, &data, 0);
	if(found != DB_NOTFOUND) {
	    Assert(found == 0); // Any other errors should cause an exception.

	    postlist = (docid *)data.get_data();
	    postlist_size = data.get_size();
	}

	// Look through postlist for doc id
	docid * pos = postlist;
	docid * end = postlist + postlist_size / sizeof(docid);
	while(pos != end && *pos < did) pos++;
	if(pos != end && *pos == did) {
	    // Already there
	} else {
	    // Add doc id to postlist
	    postlist_size += sizeof(docid);
	    docid * postlist_new;
	    postlist_new = (docid *) realloc(postlist, postlist_size);
	    if(postlist_new == NULL) {
		free(postlist);
		throw std::bad_alloc();
	    }
	    pos = pos - postlist + postlist_new;
	    postlist = postlist_new;
	    memmove(pos + 1, pos,
		    postlist_size - (1 + (pos - postlist)) * sizeof(docid));
	    *pos = did;
	}

	// Save new postlist
	data.set_data(postlist);
	data.set_size(postlist_size);
	data.set_flags(0);
	found = internals->postlist_db->put(NULL, &key, &data, 0);
	Assert(found == 0); // Any errors should cause an exception.

#if 1
	cout << "New postlist: (";
	for(docid *pos = postlist;
	    pos != postlist + postlist_size / sizeof(docid);
	    pos++) {
	    cout << *pos << " ";
	}
	cout << ")" << endl;
#endif

	free(postlist);
    }
    catch (DbException e) {
	throw OmError("PostlistDb error:" + string(e.what()));
    }
}
*/

/////////////////
// Term  cache //
/////////////////

termid
SleepyDatabaseTermCache::term_name_to_id(const termname &tname) const {
    Dbt key((void *)tname.c_str(), tname.size());
    Dbt data;
    termid tid;

    data.set_flags(DB_DBT_USERMEM);
    data.set_ulen(sizeof(tid));
    data.set_data(&tid);

    // Get, no transactions, no flags
    try {
	int found = internals->termid_db->get(NULL, &key, &data, 0);
	if(found == DB_NOTFOUND) {
	    tid = 0;
	} else {
	    // Any other errors should cause an exception.
	    Assert(found == 0);

	    if(data.get_size() != sizeof(termid)) {
		throw OmError("TermidDb: found termname, but data is not a termid.");
	    }
	}
    }
    catch (DbException e) {
	throw OmError("TermidDb error: " + string(e.what()));
    }

    return tid;
}

termname
SleepyDatabaseTermCache::term_id_to_name(termid tid) const {
    if(tid == 0) throw RangeError("Termid 0 not valid");

    Dbt key(&tid, sizeof(tid));
    Dbt data;

    // Get, no transactions, no flags
    try {
	int found = internals->termname_db->get(NULL, &key, &data, 0);
	if(found == DB_NOTFOUND) throw RangeError("Termid not found");

	// Any other errors should cause an exception.
	Assert(found == 0);
    }
    catch (DbException e) {
	throw OmError("TermnameDb error:" + string(e.what()));
    }

    termname tname((char *)data.get_data(), data.get_size());
    return tname;
}
