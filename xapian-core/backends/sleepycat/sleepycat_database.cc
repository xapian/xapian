/* sleepycat_database.cc: interface to sleepycat database routines
 *
 * ----START-LICENCE----
 * Copyright 1999,2000 BrightStation PLC
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
#include <unistd.h>
#include <fcntl.h>
#include <string>
#include <algorithm>

#include <om/omdocument.h>

#include "utils.h"
#include "omdebug.h"

// Sleepycat database stuff
#include <db_cxx.h>

#include "sleepycat_postlist.h"
#include "sleepycat_termlist.h"
#include "sleepycat_database.h"
#include "sleepycat_database_internals.h"
#include "sleepycat_list.h"
#include "sleepycat_document.h"
#include "sleepycat_termcache.h"

#include "om/omerror.h"

SleepycatDatabase::SleepycatDatabase(const OmSettings &params, bool readonly)
{
    string path = params.get("sleepycat_dir");

    // FIXME: misuse of auto_ptr - should be refcnt
    std::auto_ptr<SleepycatDatabaseInternals> tempptr1(new SleepycatDatabaseInternals());
    internals = tempptr1;
    std::auto_ptr<SleepycatDatabaseTermCache>
	tempptr2(new SleepycatDatabaseTermCache(internals.get()));
    termcache = tempptr2;

    // Open database with specified path
    // May throw an OmOpeningError exception
    internals->open(path, readonly);
}

SleepycatDatabase::~SleepycatDatabase()
{
    try {
	internal_end_session();
    } catch (...) {
	// Ignore any exceptions, since we may be being called due to an
	// exception anyway.  internal_end_session() should have already
	// been called, in the normal course of events.
    }

    // Close databases
    try {
	internals->close();
    }
    catch (DbException e) {
	throw (OmDatabaseError(std::string("Database error on close: ") +
			       e.what()));
    }
}

om_doccount
SleepycatDatabase::get_doccount() const
{
    return internals->get_doccount();
}

om_doclength
SleepycatDatabase::get_avlength() const
{
    om_doccount doccount = internals->get_doccount();
    if (doccount == 0) return 0;
    return internals->get_totlength() / doccount;
}

om_doclength
SleepycatDatabase::get_doclength(om_docid did) const
{
    std::auto_ptr<SleepycatTermList> tl(
	new SleepycatTermList(did, RefCntPtr<const SleepycatDatabase>(RefCntPtrToThis(), this),
			      internals.get(), termcache.get()));
    return tl->get_doclength();
}

om_doccount
SleepycatDatabase::get_termfreq(const om_termname &tname) const
{
    if(!term_exists(tname)) return 0;
    PostList *pl = do_open_post_list(tname);
    om_doccount freq = 0;
    if(pl) freq = pl->get_termfreq();
    delete pl;
    return freq;
}

bool
SleepycatDatabase::term_exists(const om_termname &tname) const
{
    DEBUGLINE(DB, "termcache->term_name_to_id(tname) = " <<
	      termcache->term_name_to_id(tname));
    Assert(tname.size() != 0);
    if (termcache->term_name_to_id(tname) != 0) return true;
    return false;
}

LeafPostList *
SleepycatDatabase::do_open_post_list(const om_termname & tname) const
{
    om_termid tid = termcache->term_name_to_id(tname);
    if (tid == 0) throw OmRangeError("Termid " + om_tostring(tid) +
				     " not found; can't open postlist");

    return new SleepycatPostList(tid, internals.get(), tname,
				 RefCntPtr<const SleepycatDatabase>(RefCntPtrToThis(), this));
}

LeafTermList *
SleepycatDatabase::open_term_list(om_docid did) const
{
    if (did == 0) throw OmInvalidArgumentError("Docid 0 invalid");
    return new SleepycatTermList(did, RefCntPtr<const SleepycatDatabase>(RefCntPtrToThis(), this),
				 internals.get(), termcache.get());
}

LeafDocument *
SleepycatDatabase::open_document(om_docid did) const
{
    return new SleepycatDocument(internals->document_db,
				 internals->key_db,
				 did);
}

void
SleepycatDatabase::do_begin_session(om_timeout timeout)
{
}

void
SleepycatDatabase::do_end_session()
{
}

void
SleepycatDatabase::do_flush()
{
}

void
SleepycatDatabase::do_begin_transaction()
{
    throw OmUnimplementedError("Transactions not implemented for SleepycatDatabase");
}

void
SleepycatDatabase::do_commit_transaction()
{
    throw OmUnimplementedError("Transactions not implemented for SleepycatDatabase");
}

void
SleepycatDatabase::do_cancel_transaction()
{
    throw OmUnimplementedError("Transactions not implemented for SleepycatDatabase");
}


void
SleepycatDatabase::do_delete_document(om_docid did)
{
    throw OmUnimplementedError("SleepycatDatabase::do_delete_document() not implemented");  
}

void
SleepycatDatabase::do_replace_document(om_docid did,
			 const OmDocumentContents & document)
{
    throw OmUnimplementedError("SleepycatDatabase::do_replace_document() not implemented");  
}

OmDocumentContents
SleepycatDatabase::do_get_document(om_docid did)
{
    throw OmUnimplementedError("SleepycatDatabase::do_get_document() not implemented");  
}

om_docid
SleepycatDatabase::do_add_document(const struct OmDocumentContents & document)
{
    // Remember whether database was already locked, so we can leave it
    // in the lock state we found it in.

    om_docid did;

    // FIXME - this method needs to be atomic

    // Make a new document to store the data, and use the document id
    // returned
    did = make_new_document(document);

    om_doclength doclength = 0;

    // Build list of terms, sorted by termID
    std::map<om_termid, OmDocumentTerm> terms;
    OmDocumentContents::document_terms::const_iterator i;
    for(i = document.terms.begin(); i != document.terms.end(); i++) {
	Assert(i->second.tname.size() != 0);
	om_termid tid = termcache->assign_new_termid(i->second.tname);
	terms.insert(std::make_pair(tid, i->second));
	doclength += i->second.wdf;
    }

    // Add this document to the postlist for each of its terms
    std::map<om_termid, OmDocumentTerm>::iterator term;
    for(term = terms.begin(); term != terms.end(); term++) {
	om_doccount newtermfreq;
	newtermfreq = add_entry_to_postlist(term->first,
					    did,
					    term->second.wdf,
					    term->second.positions,
					    doclength);
	term->second.termfreq = newtermfreq;
    }

    // Make a new termlist for this document
    make_new_termlist(did, terms);

    // Now: to store the termfrequency in termlists we would need to go
    // through each term in the termlist, open the appropriate postlist,
    // and for each document in each of these postlists update the
    // documents termlist to have the correct termfreq for this term.
    //
    // This can't be done efficiently, so we don't store the termfreq in
    // the termlist for dynamically updatable databases such as these.

    // Increase the document count and total length
    internals->set_doccount(get_doccount() + 1);
    internals->set_totlength(internals->get_totlength() + doclength);
    DEBUGLINE(DB, "New doccount and total length is: " <<
	      internals->get_doccount() << ", " <<
	      internals->get_totlength());

    return did;
}

om_doccount
SleepycatDatabase::add_entry_to_postlist(om_termid tid,
					 om_docid did,
					 om_termcount wdf,
					 const OmDocumentTerm::term_positions & positions,
					 om_doclength doclength)
{
// FIXME: suggest refactoring most of this method into a constructor of
// SleepycatPostList, followed by adding an item to the postlist
    SleepycatList mylist(internals->postlist_db,
			 reinterpret_cast<void *>(&tid),
			 sizeof(tid), false);

    // Term frequency isn't used for postlists: give 0.
    SleepycatListItem myitem(did, wdf, positions, 0, doclength);
    mylist.add_item(myitem);

    return mylist.get_item_count();
}

om_docid
SleepycatDatabase::make_new_document(const OmDocumentContents & doccontents)
{
    SleepycatDocument document(internals->document_db,
			       internals->key_db,
			       doccontents);
    return document.get_docid();
}

void
SleepycatDatabase::make_new_termlist(om_docid did,
				  const std::map<om_termid, OmDocumentTerm> & terms)
{
// FIXME: suggest refactoring this method into a constructor of SleepycatTermList
    SleepycatList mylist(internals->termlist_db,
			 reinterpret_cast<void *>(&did),
			 sizeof(did), false, false);

    Assert(mylist.get_item_count() == 0);

    std::map<om_termid, OmDocumentTerm>::const_iterator term;
    for (term = terms.begin(); term != terms.end(); term++) {
	// Document length is not used in termlists: use 0.
	SleepycatListItem myitem(term->first,
				 term->second.wdf,
				 term->second.positions,
				 term->second.termfreq,
				 0);
	mylist.add_item(myitem);
    }
}
