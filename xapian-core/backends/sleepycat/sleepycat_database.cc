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
#include "sleepy_postlist.h"
#include "sleepy_termlist.h"
#include "sleepy_database.h"
#include "sleepy_database_internals.h"
#include "sleepy_list.h"


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
	throw (OmDatabaseError(string("Database error on close: ") + e.what()));
    }
    Assert((opened = false) == false);
}

om_doccount
SleepyDatabase::get_doccount() const
{
    Assert(opened);
    return 1;
}

om_doclength
SleepyDatabase::get_avlength() const
{
    Assert(opened);
    return 1;
}

om_doclength
SleepyDatabase::get_doclength(om_docid did) const
{
    Assert(opened);
    return 1;
}

om_doccount
SleepyDatabase::get_termfreq(const om_termname &tname) const
{   
    PostList *pl = open_post_list(tname);
    om_doccount freq = 0;
    if(pl) freq = pl->get_termfreq();
    delete pl;
    return freq;
}

bool
SleepyDatabase::term_exists(const om_termname &tname) const
{
    if(termcache->term_name_to_id(tname)) return true;
    return false;
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
	throw (OmOpeningError(string("Database error on open: ") + e.what()));
    }
    Assert((opened = true) == true);
}

LeafPostList *
SleepyDatabase::open_post_list(const om_termname & tname) const
{
    Assert(opened);
    om_termid tid = termcache->term_name_to_id(tname);
    Assert(tid != 0);

    Dbt key(&tid, sizeof(tid));
    Dbt data;

    // FIXME - should use DB_DBT_USERMEM and DB_DBT_PARTIAL eventually
    data.set_flags(DB_DBT_MALLOC);

    // Get, no transactions, no flags
    try {
	int found = internals->postlist_db->get(NULL, &key, &data, 0);
	if(found == DB_NOTFOUND) throw OmDatabaseError("Termid not found");

	// Any other errors should cause an exception.
	Assert(found == 0);
    }
    catch (DbException e) {
	throw OmDatabaseError("PostlistDb error:" + string(e.what()));
    }

    return new SleepyPostList(tname, (om_docid *)data.get_data(),
			      data.get_size() / sizeof(om_docid));
}

LeafTermList *
SleepyDatabase::open_term_list(om_docid did) const {
    Assert(opened);
    Dbt key(&did, sizeof(did));
    Dbt data;

    // FIXME - should use DB_DBT_USERMEM and DB_DBT_PARTIAL eventually
    data.set_flags(DB_DBT_MALLOC);

    // Get, no transactions, no flags
    try {
	int found = internals->termlist_db->get(NULL, &key, &data, 0);
	if(found == DB_NOTFOUND) throw OmDocNotFoundError("Docid not found");

	// Any other errors should cause an exception.
	Assert(found == 0);
    }
    catch (DbException e) {
	throw OmDatabaseError("TermlistDb error:" + string(e.what()));
    }

    return new SleepyTermList(termcache,
			      (om_termid *)data.get_data(),
			      data.get_size() / sizeof(om_termid),
			      get_doccount());
}

LeafDocument *
SleepyDatabase::open_document(om_docid did) const {
    Assert(opened);
    throw OmUnimplementedError(
	"SleepyDatabase.open_document() not implemented");
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

#if MUS_DEBUG_VERBOSE
	DebugMsg("New postlist: (");
	for(docid *pos = postlist;
	    pos != postlist + postlist_size / sizeof(docid);
	    pos++) {
	    DebugMsg(*pos << " ");
	}
	DebugMsg(")" << endl);
#endif

	free(postlist);
    }
    catch (DbException e) {
	throw OmError("PostlistDb error:" + string(e.what()));
    }
}
*/
