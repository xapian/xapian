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

#include "sleepy_postlist.h"
#include "sleepy_termlist.h"
#include "sleepy_database.h"
#include "sleepy_database_internals.h"
#include "sleepy_list.h"

// Sleepycat database stuff
#include <db_cxx.h>

#include "utils.h"
#include "omassert.h"
#include "sleepy_termcache.h"

SleepyDatabase::SleepyDatabase(const DatabaseBuilderParams &params)
{
    // Check validity of parameters
    Assert(params.paths.size() == 1);
    Assert(params.subdbs.size() == 0);

    internals = new SleepyDatabaseInternals();
    termcache = new SleepyDatabaseTermCache(internals);

    // Open database with specified path
    try {
	internals->open(params.paths[0], params.readonly);
    }
    catch (DbException e) {
	throw (OmOpeningError(string("Database error on open: ") + e.what()));
    }
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
}

om_doccount
SleepyDatabase::get_doccount() const
{
    // FIXME
    return 1;
}

om_doclength
SleepyDatabase::get_avlength() const
{
    // FIXME
    return 1;
}

om_doclength
SleepyDatabase::get_doclength(om_docid did) const
{
    // FIXME
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

LeafPostList *
SleepyDatabase::open_post_list(const om_termname & tname) const
{
    om_termid tid = termcache->term_name_to_id(tname);
    if(tid == 0) throw OmRangeError("Termid not found");

    // FIXME - specify which of termfreqs, wdfs, and positional info
    // should be stored.
    return new SleepyPostList(tid, internals, tname);
}

LeafTermList *
SleepyDatabase::open_term_list(om_docid did) const
{
    throw OmUnimplementedError(
	"SleepyDatabase.open_term_list() not implemented");
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
SleepyDatabase::open_document(om_docid did) const
{
    throw OmUnimplementedError(
	"SleepyDatabase.open_document() not implemented");
}


om_docid
SleepyDatabase::add_document(const struct OmDocumentContents & document)
{
    // FIXME - this method is incomplete
    om_docid did = get_doccount() + 1;

    OmDocumentContents::document_terms::const_iterator i;
    for(i = document.terms.begin(); i != document.terms.end(); i++) {
	om_termid tid = termcache->assign_new_termid(i->second.tname);
	make_entry_in_postlist(tid, did, i->second.wdf, i->second.positions);
    }

    return did;
}

void
SleepyDatabase::make_entry_in_postlist(om_termid tid,
				       om_docid did,
				       om_termcount wdf,
				       const vector<om_termpos> & positions)
{
    SleepyList mylist(internals->postlist_db,
		      reinterpret_cast<void *>(&tid),
		      sizeof(tid));
    SleepyListItem myitem(did, 0, wdf, positions);
    mylist.add_item(myitem);
}
