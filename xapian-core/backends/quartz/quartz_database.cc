/* quartz_database.cc: quartz database
 *
 * ----START-LICENCE----
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2001 Hein Ragas
 * Copyright 2002 Ananova Ltd
 * Copyright 2002 Olly Betts
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

#include <config.h>

#include "quartz_table_manager.h"

#include "quartz_database.h"
#include "utils.h"
#include "omdebug.h"
#include "autoptr.h"
#include "om/omerror.h"
#include "refcnt.h"

#include "quartz_postlist.h"
#include "quartz_termlist.h"
#include "quartz_positionlist.h"
#ifdef USE_LEXICON               
#include "quartz_lexicon.h"
#else
#include "quartz_utils.h"
#endif
#include "quartz_record.h"
#include "quartz_values.h"
#include "quartz_document.h"
#include "quartz_alltermslist.h"

#include <string>
using std::string;
using std::vector;

// Compulsory settings.
// quartz_dir    - Directory that the database is stored in.  Must be a full
//                 path.
//
// Optional settings.
// quartz_logfile - File in which to store log information regarding
//                 modifications and accesses made to the database.  If not
//                 specified, such log information will not be stored.
//                 If this is a relative path, it is taken to be relative
//                 to the quartz_dir directory.
//
// quartz_block_size - Integer.  This is the size of the blocks to use in
//                 the tables, in bytes.  Acceptable values are powers of
//                 two in the range 2048 to 65536.  The default is 8192.
//                 This setting is only used when creating databases.  If
//                 the database already exists, it is completely ignored.
//
QuartzDatabase::QuartzDatabase(const OmSettings & settings)
{
    DEBUGCALL(DB, void, "QuartzDatabase", settings);
    // Open database manager
    tables.reset(new QuartzDiskTableManager(get_db_dir(settings),
					    get_log_filename(settings),
					    true,
					    0u,
					    false,
					    false));
}

QuartzDatabase::QuartzDatabase(AutoPtr<QuartzTableManager> tables_)
	: tables(tables_)
{
    DEBUGCALL(DB, void, "QuartzDatabase", "[tables_]");
}

QuartzDatabase::~QuartzDatabase()
{
    DEBUGCALL(DB, void, "~QuartzDatabase", "");
    try {
	internal_end_session();
    } catch (...) {
	// Ignore any exceptions, since we may be being called due to an
	// exception anyway.  internal_end_session() should have already
	// been called, in the normal course of events.
	DEBUGLINE(DB, "Ignoring exception in QuartzDatabase destructor.");
    }
}

string
QuartzDatabase::get_db_dir(const OmSettings & settings)
{
    DEBUGCALL_STATIC(DB, string, "QuartzDatabase::get_db_dir", settings);
    RETURN(settings.get("quartz_dir"));
}

string
QuartzDatabase::get_log_filename(const OmSettings & settings)
{
    DEBUGCALL_STATIC(DB, string, "QuartzDatabase::get_log_filename", settings);
    RETURN(settings.get("quartz_logfile", ""));
}

unsigned int
QuartzDatabase::get_block_size(const OmSettings & settings)
{
    DEBUGCALL_STATIC(DB, unsigned int, "QuartzDatabase::get_block_size", settings);
    RETURN(settings.get_int("quartz_block_size", QUARTZ_BTREE_DEF_BLOCK_SIZE));
}

bool
QuartzDatabase::get_create(const OmSettings & settings)
{
    DEBUGCALL_STATIC(DB, bool, "QuartzDatabase::get_create", settings);
    RETURN(settings.get_bool("database_create", false));
}

bool
QuartzDatabase::get_allow_overwrite(const OmSettings & settings)
{
    DEBUGCALL_STATIC(DB, bool, "QuartzDatabase::get_allow_overwrite", settings);
    RETURN(settings.get_bool("database_allow_overwrite", false));
}

void
QuartzDatabase::do_begin_session()
{
    DEBUGCALL(DB, void, "QuartzDatabase::do_begin_session", "");
    throw OmInvalidOperationError(
	"Cannot begin a modification session: database opened readonly.");
}

void
QuartzDatabase::do_end_session()
{ Assert(false); }

void
QuartzDatabase::do_flush()
{ Assert(false); }

void
QuartzDatabase::do_begin_transaction()
{ Assert(false); }

void
QuartzDatabase::do_commit_transaction()
{ Assert(false); }

void
QuartzDatabase::do_cancel_transaction()
{ Assert(false); }

om_docid
QuartzDatabase::do_add_document(const OmDocument & /*document*/)
{
    DEBUGCALL(DB, om_docid, "QuartzDatabase::do_add_document", "");
    Assert(false);
    RETURN(0);
}

void
QuartzDatabase::do_delete_document(om_docid /*did*/)
{ Assert(false); }

void
QuartzDatabase::do_replace_document(om_docid /*did*/,
				    const OmDocument & /*document*/)
{ Assert(false); }

om_doccount 
QuartzDatabase::get_doccount() const
{
    DEBUGCALL(DB, om_doccount, "QuartzDatabase::get_doccount", "");
    RETURN(QuartzRecordManager::get_doccount(*(tables->get_record_table())));
}

om_doclength
QuartzDatabase::get_avlength() const
{
    DEBUGCALL(DB, om_doclength, "QuartzDatabase::get_avlength", "");
    RETURN(QuartzRecordManager::get_avlength(*(tables->get_record_table())));
}

om_doclength
QuartzDatabase::get_doclength(om_docid did) const
{
    DEBUGCALL(DB, om_doclength, "QuartzDatabase::get_doclength", did);
    Assert(did != 0);

    QuartzTermList termlist(0,
			    tables->get_termlist_table(),
#ifdef USE_LEXICON               
			    tables->get_lexicon_table(),
#else
			    tables->get_postlist_table(),
#endif
			    did,
			    0);
    RETURN(termlist.get_doclength());
}

om_doccount
QuartzDatabase::get_termfreq(const om_termname & tname) const
{
    DEBUGCALL(DB, om_doccount, "QuartzDatabase::get_termfreq", tname);
    Assert(!tname.empty());

    om_doccount termfreq = 0; // If not found, this value will be unchanged.
#ifdef USE_LEXICON               
    QuartzLexicon::get_entry(tables->get_lexicon_table(),
			     tname,
			     &termfreq);
#else
    QuartzPostList pl(NULL, tables->get_postlist_table(), NULL, tname);
    termfreq = pl.get_termfreq();
#endif
    RETURN(termfreq);
}

om_termcount
QuartzDatabase::get_collection_freq(const om_termname & tname) const
{
    DEBUGCALL(DB, om_termcount, "QuartzDatabase::get_collection_freq", tname);
    Assert(!tname.empty());

    om_termcount collfreq = 0; // If not found, this value will be unchanged.
    QuartzPostList pl(NULL, tables->get_postlist_table(), NULL, tname);
    collfreq = pl.get_collection_freq();
    RETURN(collfreq);
}

bool
QuartzDatabase::term_exists(const om_termname & tname) const
{
    DEBUGCALL(DB, bool, "QuartzDatabase::term_exists", tname);
    Assert(!tname.empty());
#ifdef USE_LEXICON
    return QuartzLexicon::get_entry(tables->get_lexicon_table(),
				    tname, 0);
#else
    const QuartzTable * table = tables->get_postlist_table();
    AutoPtr<QuartzCursor> cursor(table->cursor_get());
    // FIXME: nasty C&P from backends/quartz/quartz_postlist.cc
    string key = pack_string_preserving_sort(tname);
    return cursor->find_entry(key);
#endif
}


LeafPostList *
QuartzDatabase::do_open_post_list(const om_termname& tname) const
{
    DEBUGCALL(DB, LeafPostList *, "QuartzDatabase::do_open_post_list", tname);
    RefCntBase::RefCntPtrToThis tmp;
    RefCntPtr<const QuartzDatabase> ptrtothis(tmp, this);

    RETURN(open_post_list_internal(tname, ptrtothis));
}

LeafPostList *
QuartzDatabase::open_post_list_internal(const om_termname& tname,
				RefCntPtr<const Database> ptrtothis) const
{
    DEBUGCALL(DB, LeafPostList *, "QuartzDatabase::open_post_list_internal", tname << ", [ptrtothis]");
    Assert(!tname.empty());
    return(new QuartzPostList(ptrtothis,
			      tables->get_postlist_table(),
			      tables->get_positionlist_table(),
			      tname));
}

LeafTermList *
QuartzDatabase::open_term_list_internal(om_docid did,
				RefCntPtr<const Database> ptrtothis) const
{
    DEBUGCALL(DB, LeafTermList *, "QuartzDatabase::open_term_list_internal",
	      did << ", [ptrtothis]");
    Assert(did != 0);
    return(new QuartzTermList(ptrtothis,
			      tables->get_termlist_table(),
#ifdef USE_LEXICON
			      tables->get_lexicon_table(),
#else
			      tables->get_postlist_table(),
#endif
			      did,
			      get_doccount()));
}

LeafTermList *
QuartzDatabase::open_term_list(om_docid did) const
{
    DEBUGCALL(DB, LeafTermList *, "QuartzDatabase::open_term_list", did);
    RefCntBase::RefCntPtrToThis tmp;
    RefCntPtr<const QuartzDatabase> ptrtothis(tmp, this);

    RETURN(open_term_list_internal(did, ptrtothis));
}

Document *
QuartzDatabase::open_document(om_docid did, bool lazy) const
{
    DEBUGCALL(DB, Document *, "QuartzDatabase::open_document",
	      did << ", " << lazy);
    Assert(did != 0);
    RefCntBase::RefCntPtrToThis tmp;
    RefCntPtr<const QuartzDatabase> ptrtothis(tmp, this);

    return new QuartzDocument(ptrtothis,
			      tables->get_value_table(),
			      tables->get_record_table(),
			      did, lazy);
}

AutoPtr<PositionList> 
QuartzDatabase::open_position_list(om_docid did,
				   const om_termname & tname) const
{
    Assert(did != 0);
    AutoPtr<QuartzPositionList> poslist(new QuartzPositionList());
    poslist->read_data(tables->get_positionlist_table(), did, tname);
    if (poslist->get_size() == 0) {
	// Check that term / document combination exists.
	RefCntBase::RefCntPtrToThis tmp;
	RefCntPtr<const QuartzDatabase> ptrtothis(tmp, this);
	// If the doc doesn't exist, this will throw OmDocNotFound:
	AutoPtr<LeafTermList> ltl(open_term_list_internal(did, ptrtothis));
	ltl->skip_to(tname);
	if (ltl->at_end() || ltl->get_termname() != tname)
	    throw OmRangeError("Can't open position list: requested term is not present in document.");
    }

    return AutoPtr<PositionList>(poslist.release());
}

void
QuartzDatabase::do_reopen()
{
    DEBUGCALL(DB, void, "QuartzDatabase::do_reopen", "");
    tables->reopen();
}

TermList *
QuartzDatabase::open_allterms() const
{
    DEBUGCALL(DB, TermList *, "QuartzDatabase::open_allterms", "");
    QuartzTable *t = tables->get_postlist_table();
    AutoPtr<QuartzCursor> pl_cursor(t->cursor_get());
    RETURN(new QuartzAllTermsList(RefCntPtr<const QuartzDatabase>(RefCntPtrToThis(), this),
				  pl_cursor, t->get_entry_count()));
}


QuartzWritableDatabase::QuartzWritableDatabase(const OmSettings & settings)
	: buffered_tables(new QuartzBufferedTableManager(
				QuartzDatabase::get_db_dir(settings),
				QuartzDatabase::get_log_filename(settings),
				QuartzDatabase::get_block_size(settings),
				QuartzDatabase::get_create(settings),
				QuartzDatabase::get_allow_overwrite(settings))),
	  changecount(0),
	  database_ro(AutoPtr<QuartzTableManager>(buffered_tables))
{
    DEBUGCALL(DB, void, "QuartzWritableDatabase", settings);
}

QuartzWritableDatabase::~QuartzWritableDatabase()
{
    DEBUGCALL(DB, void, "~QuartzWritableDatabase", "");
    // FIXME - release write lock if held
    try {
	internal_end_session();
    } catch (...) {
	// Ignore any exceptions, since we may be being called due to an
	// exception anyway.  internal_end_session() should have already
	// been called, in the normal course of events.
	DEBUGLINE(DB, "Ignoring exception in QuartzWritableDatabase destructor.");
    }
}

void
QuartzWritableDatabase::do_begin_session()
{
    DEBUGCALL(DB, void, "QuartzWritableDatabase::do_begin_session", "");
    Assert(buffered_tables != 0);

    changecount = 0;
    // FIXME - get a write lock on the database
}

void
QuartzWritableDatabase::do_end_session()
{
    DEBUGCALL(DB, void, "QuartzWritableDatabase::do_end_session", "");
    Assert(buffered_tables != 0);

    buffered_tables->apply();

    // FIXME - release write lock on the database (even if an apply() throws)
}

void
QuartzWritableDatabase::do_flush()
{
    DEBUGCALL(DB, void, "QuartzWritableDatabase::do_flush", "");
    Assert(buffered_tables != 0);

    changecount = 0;
    buffered_tables->apply();
}

void
QuartzWritableDatabase::do_begin_transaction()
{
    DEBUGCALL(DB, void, "QuartzWritableDatabase::do_begin_transaction", "");
    throw OmUnimplementedError("QuartzDatabase::do_begin_transaction() not yet implemented");
}

void
QuartzWritableDatabase::do_commit_transaction()
{
    DEBUGCALL(DB, void, "QuartzWritableDatabase::do_commit_transaction", "");
    throw OmUnimplementedError("QuartzDatabase::do_commit_transaction() not yet implemented");
}

void
QuartzWritableDatabase::do_cancel_transaction()
{
    DEBUGCALL(DB, void, "QuartzWritableDatabase::do_cancel_transaction", "");
    throw OmUnimplementedError("QuartzDatabase::do_cancel_transaction() not yet implemented");
}

om_docid
QuartzWritableDatabase::do_add_document(const OmDocument & document)
{
    DEBUGCALL(DB, om_docid,
	      "QuartzWritableDatabase::do_add_document", document);
    Assert(buffered_tables != 0);

    // Calculate the new document length
    quartz_doclen_t new_doclen = 0;
    {
	OmTermIterator term = document.termlist_begin();
	OmTermIterator term_end = document.termlist_end();    
	for ( ; term != term_end; ++term) {
	    new_doclen += term.get_wdf();
	}
    }

    om_docid did;

    try {
	// Set the record, and get the document ID to use.
	did = QuartzRecordManager::add_record(
		*(buffered_tables->get_record_table()),
		document.get_data());
	Assert(did != 0);

	// Set the values.
	{
	    OmValueIterator value = document.values_begin();
	    OmValueIterator value_end = document.values_end();
	    for ( ; value != value_end; ++value) {
		QuartzValueManager::add_value(
		    *(buffered_tables->get_value_table()),
		    *value, did, value.get_valueno());
	    }
	}

	// Set the termlist.
	QuartzTermList::set_entries(buffered_tables->get_termlist_table(), did,
		document.termlist_begin(), document.termlist_end(),
		new_doclen, false);

	// Set the new document length
	// (Old doclen is always zero, since this is a new document)
	QuartzRecordManager::modify_total_length(
		*(buffered_tables->get_record_table()),
		0,
		new_doclen);

	OmTermIterator term = document.termlist_begin();
	OmTermIterator term_end = document.termlist_end();    
	for ( ; term != term_end; ++term) {
#ifdef USE_LEXICON
	    QuartzLexicon::increment_termfreq(
		buffered_tables->get_lexicon_table(),
		*term);
#endif
	    QuartzPostList::add_entry(buffered_tables->get_postlist_table(),
				      *term, did, term.get_wdf(),
				      new_doclen);
	    if (term.positionlist_begin() != term.positionlist_end()) {
		QuartzPositionList::set_positionlist(
		    buffered_tables->get_positionlist_table(), did,
		    *term, term.positionlist_begin(), term.positionlist_end());
	    }
	}

    } catch (...) {
	// If an error occurs while adding a document, or doing any other
	// transaction, the modifications so far must be cleared before
	// returning control to the user - otherwise partial modifications will
	// persist in memory, and eventually get written to disk.
	buffered_tables->cancel();
	throw;
    }

    // FIXME: this should be configurable
    // FIXME: this should be done by checking memory usage, not the number of
    // changes.
    if (++changecount >= 1000) {
	changecount = 0;
	buffered_tables->apply();
    }

    RETURN(did);
}

void
QuartzWritableDatabase::do_delete_document(om_docid did)
{
    DEBUGCALL(DB, void, "QuartzWritableDatabase::do_delete_document", did);
    Assert(did != 0);
    Assert(buffered_tables != 0);

    try {
	QuartzDatabase::RefCntPtrToThis tmp;
	RefCntPtr<const QuartzWritableDatabase> ptrtothis(tmp, this);

	QuartzTermList termlist(ptrtothis,
				database_ro.tables->get_termlist_table(),
#ifdef USE_LEXICON
				database_ro.tables->get_lexicon_table(),
#else
				database_ro.tables->get_postlist_table(),
#endif
				did,
				database_ro.get_doccount());

	termlist.next();
	while (!termlist.at_end()) {
	    om_termname tname = termlist.get_termname();
	    QuartzPostList::delete_entry(buffered_tables->get_postlist_table(),
		tname, did);
	    QuartzPositionList::delete_positionlist(
		buffered_tables->get_positionlist_table(),
		did, tname);
#ifdef USE_LEXICON
	    QuartzLexicon::decrement_termfreq(
		buffered_tables->get_lexicon_table(),
		tname);
#endif
	    termlist.next();
	}

	// Set the document length.
	// (New doclen is always zero, since we're deleting the document.)
	quartz_doclen_t old_doclen = termlist.get_doclength();
	QuartzRecordManager::modify_total_length(
		*(buffered_tables->get_record_table()),
		old_doclen,
		0);

	// Remove the values
	QuartzValueManager::delete_all_values(*(buffered_tables->get_value_table()),
					      did);

	// Remove the termlist.
	QuartzTermList::delete_termlist(buffered_tables->get_termlist_table(),
					did);

	// Remove the record.
	QuartzRecordManager::delete_record(*(buffered_tables->get_record_table()),
					   did);
    } catch (...) {
	// If an error occurs while deleting a document, or doing any other
	// transaction, the modifications so far must be cleared before
	// returning control to the user - otherwise partial modifications will
	// persist in memory, and eventually get written to disk.
	buffered_tables->cancel();

	throw;
    }

    // FIXME: this should be configurable
    // FIXME: this should be done by checking memory usage, not the number of
    // changes.
    if (++changecount > 1000) {
	changecount = 0;
	buffered_tables->apply();
    }
}

void
QuartzWritableDatabase::do_replace_document(om_docid did,
				    const OmDocument & document)
{
    DEBUGCALL(DB, void, "QuartzWritableDatabase::do_replace_document", did << ", " << document);
    Assert(did != 0);
    Assert(buffered_tables != 0);

    // Calculate the new document length
    quartz_doclen_t new_doclen = 0;
    {
	OmTermIterator term = document.termlist_begin();
	OmTermIterator term_end = document.termlist_end();    
	for ( ; term != term_end; ++term) {
	    new_doclen += term.get_wdf();
	}
    }

    try {
	// Replace the record
	QuartzRecordManager::replace_record(
		*(buffered_tables->get_record_table()),
		document.get_data(),
		did);

	// Replace the values.
	QuartzValueManager::delete_all_values(
		*(buffered_tables->get_value_table()),
		did);
	{
	    OmValueIterator value = document.values_begin();
	    OmValueIterator value_end = document.values_end();
	    for ( ; value != value_end; ++value) {
		QuartzValueManager::add_value(
		    *(buffered_tables->get_value_table()),
		    *value, did, value.get_valueno());
	    }
	}

	// Set the termlist.
	// We should detect what terms have been deleted, and which ones have
	// been added. Then we'll add/delete only those terms.
	quartz_doclen_t old_doclen;
	{
            vector<om_termname> delTerms;
            vector<om_termname> addTerms;
            vector<om_termname> posTerms;

	    // First, before we modify the Postlist, we should detect the old
	    // document length, since that seems to be of some importance later
	    // on.
	    QuartzDatabase::RefCntPtrToThis tmp;
	    RefCntPtr<const QuartzWritableDatabase> ptrtothis(tmp, this);
	    QuartzTermList termlist(ptrtothis,
	  			    database_ro.tables->get_termlist_table(),
#ifdef USE_LEXICON
				    database_ro.tables->get_lexicon_table(),
#else
				    database_ro.tables->get_postlist_table(),
#endif
				    did,
				    database_ro.get_doccount());
	    old_doclen = termlist.get_doclength();
            OmTermIterator tNewIter = document.termlist_begin();
            termlist.next();
            while (!termlist.at_end() && tNewIter != document.termlist_end()) {
              om_termname tname = termlist.get_termname();
              if (tname < (*tNewIter)) {
                // Deleted term exists in the old termlist, but not in the new one.
                delTerms.push_back(tname);
                termlist.next();
              } else {
                if (tname > (*tNewIter)) {
                  // Added term does not exist in the old termlist, but it does in the new one.
                  addTerms.push_back((*tNewIter));
                } else {
                  // Terms are equal, but perhaps its positionlist has been modified. Record it, and skip to the next.
                  posTerms.push_back(tname);
                  termlist.next();
                }
		++tNewIter;
              }
            }
            // One of the lists (or both!) has been processed. Check if any of the iterators are not at the end.
            while (!termlist.at_end()) {
              // Any term left in the old list must be removed.
              om_termname tname = termlist.get_termname();
              delTerms.push_back(tname);
              termlist.next();
            }
            while (tNewIter != document.termlist_end()) {
              // Any term left in the new list must be added.
              addTerms.push_back((*tNewIter));
              ++tNewIter;
            }
            // We now know which terms to add and which to remove. Let's get to work!
            // Delete the terms on our "hitlist"...
            vector<om_termname>::iterator vIter = delTerms.begin();
            while (vIter != delTerms.end()) {
	        om_termname tname = (*vIter);
	        QuartzPostList::delete_entry(buffered_tables->get_postlist_table(),
		    tname, did);
	        QuartzPositionList::delete_positionlist(
		    buffered_tables->get_positionlist_table(),
		    did, tname);
#ifdef USE_LEXICON
	        QuartzLexicon::decrement_termfreq(
		    buffered_tables->get_lexicon_table(),
		    tname);
#endif
                ++vIter;
	    }
            // Now add the terms that are new...
            vIter = addTerms.begin();
            while (vIter != addTerms.end()) {
                OmTermIterator tIter = document.termlist_begin();
                tIter.skip_to((*vIter));
#ifdef USE_LEXICON
		QuartzLexicon::increment_termfreq(
		    buffered_tables->get_lexicon_table(),
		    *tIter);
#endif
		QuartzPostList::add_entry(buffered_tables->get_postlist_table(),
					  *tIter, did, tIter.get_wdf(),
					  new_doclen);
		if (tIter.positionlist_begin() != tIter.positionlist_end()) {
		  QuartzPositionList::set_positionlist(
		      buffered_tables->get_positionlist_table(), did,
		      *tIter, tIter.positionlist_begin(), tIter.positionlist_end());
		}
                ++vIter;
	    }
            // Finally, update the positionlist of terms that are not new or removed.
            vIter = posTerms.begin();
            while (vIter != posTerms.end()) {
                OmTermIterator tIter = document.termlist_begin();
                tIter.skip_to((*vIter));
                if (tIter.positionlist_begin() == tIter.positionlist_end()) {
                  // In the new document, this term does not have any positions associated with it
                  QuartzPositionList qpl;
                  qpl.read_data(buffered_tables->get_positionlist_table(), did, *tIter);
                  if (qpl.get_size() != 0) {
                    // But there are positions associated with this term in the index. Delete the positionlist.
                    QuartzPositionList::delete_positionlist(buffered_tables->get_positionlist_table(), did, *tIter);
                  }
                } else {
                  // In the new document, this term has positions associated with it. Check whether we need to re-create
                  //   the positionlist.
                  QuartzPositionList qpl;
                  qpl.read_data(buffered_tables->get_positionlist_table(), did, *tIter);
                  qpl.next();
                  OmPositionListIterator pIter = tIter.positionlist_begin();
                  while (!qpl.at_end() && pIter != tIter.positionlist_end()) {
                    if (qpl.get_current_pos() != (*pIter)) {
                      // Position lists do not match, so create a new one.
  		      QuartzPositionList::set_positionlist(
		        buffered_tables->get_positionlist_table(), did,
		        *tIter, tIter.positionlist_begin(), tIter.positionlist_end());
                      break;
                    }
                    qpl.next();
                    ++pIter;
                  }
                  if (!qpl.at_end() || pIter != tIter.positionlist_end()) {
                    // One of the position lists has not reached yet the end -- which means they are different. Create a
                    //   new posisitionlist based on the one in the new OmDocument.
  		    QuartzPositionList::set_positionlist(
		      buffered_tables->get_positionlist_table(), did,
		      *tIter, tIter.positionlist_begin(), tIter.positionlist_end());
                  }
                }
                ++vIter;
	    }
            // All done!
	}
        // Set pointers from the document to the terms.
	QuartzTermList::set_entries(buffered_tables->get_termlist_table(), did,
		document.termlist_begin(), document.termlist_end(),
		new_doclen, false);
	// Set the new document length
	QuartzRecordManager::modify_total_length(
		*(buffered_tables->get_record_table()),
		old_doclen,
		new_doclen);

    } catch (...) {
	// If an error occurs while adding a document, or doing any other
	// transaction, the modifications so far must be cleared before
	// returning control to the user - otherwise partial modifications will
	// persist in memory, and eventually get written to disk.
	buffered_tables->cancel();
	throw;
    }

    // FIXME: this should be configurable
    // FIXME: this should be done by checking memory usage, not the number of
    // changes.
    if (++changecount > 1000) {
	changecount = 0;
	buffered_tables->apply();
    }
}

om_doccount 
QuartzWritableDatabase::get_doccount() const
{
    DEBUGCALL(DB, om_doccount, "QuartzWritableDatabase::get_doccount", "");
    RETURN(database_ro.get_doccount());
}

om_doclength
QuartzWritableDatabase::get_avlength() const
{
    DEBUGCALL(DB, om_doclength, "QuartzWritableDatabase::get_avlength", "");
    RETURN(database_ro.get_avlength());
}

om_doclength
QuartzWritableDatabase::get_doclength(om_docid did) const
{
    DEBUGCALL(DB, om_doclength, "QuartzWritableDatabase::get_doclength", did);
    RETURN(database_ro.get_doclength(did));
}

om_doccount
QuartzWritableDatabase::get_termfreq(const om_termname & tname) const
{
    DEBUGCALL(DB, om_doccount, "QuartzWritableDatabase::get_termfreq", tname);
    RETURN(database_ro.get_termfreq(tname));
}

om_termcount
QuartzWritableDatabase::get_collection_freq(const om_termname & tname) const
{
    DEBUGCALL(DB, om_termcount, "QuartzWritableDatabase::get_collection_freq", tname);
    RETURN(database_ro.get_collection_freq(tname));
}

bool
QuartzWritableDatabase::term_exists(const om_termname & tname) const
{
    DEBUGCALL(DB, bool, "QuartzWritableDatabase::term_exists", tname);
    RETURN(database_ro.term_exists(tname));
}


LeafPostList *
QuartzWritableDatabase::do_open_post_list(const om_termname& tname) const
{
    DEBUGCALL(DB, LeafPostList *, "QuartzWritableDatabase::do_open_post_list", tname);
    RefCntBase::RefCntPtrToThis tmp;
    RefCntPtr<const QuartzWritableDatabase> ptrtothis(tmp, this);

    RETURN(database_ro.open_post_list_internal(tname, ptrtothis));
}

LeafTermList *
QuartzWritableDatabase::open_term_list(om_docid did) const
{
    DEBUGCALL(DB, LeafTermList *, "QuartzWritableDatabase::open_term_list",
	      did);
    RefCntBase::RefCntPtrToThis tmp;
    RefCntPtr<const QuartzWritableDatabase> ptrtothis(tmp, this);

    RETURN(database_ro.open_term_list_internal(did, ptrtothis));
}

Document *
QuartzWritableDatabase::open_document(om_docid did, bool lazy) const
{
    DEBUGCALL(DB, Document *, "QuartzWritableDatabase::open_document",
	      did << ", " << lazy);
    Assert(did != 0);

    RefCntBase::RefCntPtrToThis tmp;
    RefCntPtr<const QuartzWritableDatabase> ptrtothis(tmp, this);

    RETURN(new QuartzDocument(ptrtothis,
			      buffered_tables->get_value_table(),
			      buffered_tables->get_record_table(),
			      did, lazy));
}

AutoPtr<PositionList> 
QuartzWritableDatabase::open_position_list(om_docid did,
				   const om_termname & tname) const
{
    Assert(did != 0);

    AutoPtr<QuartzPositionList> poslist(new QuartzPositionList());
    poslist->read_data(buffered_tables->get_positionlist_table(), did, tname);
    if (poslist->get_size() == 0) {
	// Check that term / document combination exists.
	RefCntBase::RefCntPtrToThis tmp;
	RefCntPtr<const QuartzWritableDatabase> ptrtothis(tmp, this);
	// If the doc doesn't exist, this will throw OmDocNotFound:
	AutoPtr<LeafTermList> ltl(database_ro.open_term_list_internal(did, ptrtothis));
	ltl->skip_to(tname);
	if (ltl->at_end() || ltl->get_termname() != tname)
	    throw OmRangeError("Can't open position list: requested term is not present in document.");
    }

    return AutoPtr<PositionList>(poslist.release());
}

void
QuartzWritableDatabase::do_reopen()
{
    DEBUGCALL(DB, void, "QuartzWritableDatabase::do_reopen", "");
    /* Do nothing - we're the only writer, and so must be up to date. */
}

TermList *
QuartzWritableDatabase::open_allterms() const
{
    DEBUGCALL(DB, TermList *, "QuartzWritableDatabase::open_allterms", "");
    QuartzTable *t = buffered_tables->get_postlist_table();
    AutoPtr<QuartzCursor> pl_cursor(t->cursor_get());
    RETURN(new QuartzAllTermsList(RefCntPtr<const QuartzWritableDatabase>(RefCntPtrToThis(), this),
				  pl_cursor, t->get_entry_count()));
}
