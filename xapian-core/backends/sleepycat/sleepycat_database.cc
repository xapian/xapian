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
#include <unistd.h>
#include <fcntl.h>
#include <string>
#include <algorithm>

#include <om/omdocument.h>

#include "utils.h"
#include "omassert.h"

// Sleepycat database stuff
#include <db_cxx.h>

#include "sleepy_postlist.h"
#include "sleepy_termlist.h"
#include "sleepy_database.h"
#include "sleepy_database_internals.h"
#include "sleepy_list.h"
#include "sleepy_document.h"
#include "sleepy_termcache.h"

#include "om/omerror.h"

SleepyDatabase::SleepyDatabase(const DatabaseBuilderParams &params)
{
    // Check validity of parameters
    if(params.paths.size() != 1) {
	throw OmInvalidArgumentError("SleepyDatabase requires 1 path parameter.");
    }
    if(params.subdbs.size() != 0) {
	throw OmInvalidArgumentError("SleepyDatabase cannot have sub databases.");
    }

    // FIXME - these should be autopointers, so that if an exception is
    // thrown, memory isn't leaked.
    internals = new SleepyDatabaseInternals();
    termcache = new SleepyDatabaseTermCache(internals);

    // Check that path is to a valid extant directory
    struct stat buf;
    int err_num = stat(params.paths[0].c_str(), &buf);
    if (err_num != 0) {
	throw OmOpeningError(string("SleepyDatabase: can't stat `") +
			     params.paths[0] + "'");
    }
    if (!S_ISDIR(buf.st_mode)) {
	throw OmOpeningError(string("SleepyDatabase: `") + params.paths[0] +
			     "' is not a directory.");
    }
    
    // Open database with specified path
    // May throw an OmOpeningError exception
    internals->open(params.paths[0], params.readonly);
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
    return internals->get_doccount();
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

    return new SleepyPostList(tid, internals, tname);
}

LeafTermList *
SleepyDatabase::open_term_list(om_docid did) const
{
    return new SleepyTermList(did, this, internals, termcache);
}

LeafDocument *
SleepyDatabase::open_document(om_docid did) const
{
    return new SleepyDocument(internals->document_db, did);
}


om_docid
SleepyDatabase::add_document(const struct OmDocumentContents & document)
{
    // FIXME - this method needs to be atomic
    
    // Make a new document to store the data, and use the document id returned
    om_docid did = make_new_document(document.data);

    // Build list of terms, sorted by termID
    map<om_termid, OmDocumentTerm> terms;
    OmDocumentContents::document_terms::const_iterator i;
    for(i = document.terms.begin(); i != document.terms.end(); i++) {
	om_termid tid = termcache->assign_new_termid(i->second.tname);
	terms.insert(make_pair(tid, i->second));
    }

    // Add this document to the postlist for each of its terms
    map<om_termid, OmDocumentTerm>::iterator term;
    for(term = terms.begin(); term != terms.end(); term++) {
	om_doccount newtermfreq;
	newtermfreq = add_entry_to_postlist(term->first,
					    did,
					    term->second.wdf,
					    term->second.positions);
	term->second.termfreq = newtermfreq;
    }

    // Make a new termlist for this document
    make_new_termlist(did, terms);

    // FIXME - need to go through each term in the termlist, open the
    // appropriate postlist, and for each document in each of these postlists
    // update the documents termlist to have the correct wdf for this document.
    // EEEK!

    return did;
}

om_doccount
SleepyDatabase::add_entry_to_postlist(om_termid tid,
				       om_docid did,
				       om_termcount wdf,
				       const vector<om_termpos> & positions)
{
// FIXME: suggest refactoring most of this method into a constructor of
// SleepyPostList, followed by adding an item to the postlist
    SleepyList mylist(internals->postlist_db,
		      reinterpret_cast<void *>(&tid),
		      sizeof(tid));

    // Term frequency isn't used for postlists: give 0
    SleepyListItem myitem(did, 0, wdf, positions);
    mylist.add_item(myitem);

    return mylist.get_item_count();
}

om_docid
SleepyDatabase::make_new_document(const OmData & docdata) 
{
    map<om_keyno, OmKey> omkeys;

    SleepyDocument document(internals->document_db, docdata, omkeys);
    return document.get_docid();
}

void
SleepyDatabase::make_new_termlist(om_docid did,
				  const map<om_termid, OmDocumentTerm> & terms)
{
// FIXME: suggest refactoring this method into a constructor of SleepyTermList
    SleepyList mylist(internals->termlist_db,
		      reinterpret_cast<void *>(&did),
		      sizeof(did));

    Assert(mylist.get_item_count() == 0);

    map<om_termid, OmDocumentTerm>::const_iterator term;
    for(term = terms.begin(); term != terms.end(); term++) {
	SleepyListItem myitem(term->first,
			      term->second.termfreq,
			      term->second.wdf,
			      term->second.positions);
	mylist.add_item(myitem);
    }
}
