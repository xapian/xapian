/* quartz_database.cc: quartz database
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

#include "config.h"

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
#include "quartz_lexicon.h"
#include "quartz_record.h"
#include "quartz_attributes.h"
#include "quartz_document.h"
#include "quartz_alltermslist.h"

#include <string>

//
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
// quartz_perform_recovery - Boolean.  If true, and the database needs a
//                 recovery step to be performed, and the database is not
//                 being opened readonly, a recovery step will be performed
//                 before opening the database.  If false, and the database
//                 is not being opened readonly, and a recovery step needs to
//                 be performed, an OmNeedRecoveryError exception will be
//                 thrown.  If this is true, partially applied modifications
//                 will be thrown away silently - a typical usage would be
//                 to open the database with this false, catch any
//                 OmNeedRecoveryError exceptions, and give a warning message
//                 before reopening with this true.  A recovery step does
//                 not need to be performed before readonly access to the
//                 database is allowed.
//
// quartz_block_size - Integer.  This is the size of the blocks to use in
//                 the tables, in bytes.  Acceptable values are powers of
//                 two in the range 2048 to 65536.  The default is 8192.
//                 This setting is only used when creating databases.  If
//                 the database already exists, it is completely ignored.
//
QuartzDatabase::QuartzDatabase(const OmSettings & settings)
{
    // Open database manager
    tables.reset(new QuartzDiskTableManager(get_db_dir(settings),
					    get_log_filename(settings),
					    true,
					    false,
					    0u));
}

QuartzDatabase::QuartzDatabase(AutoPtr<QuartzTableManager> tables_)
	: tables(tables_)
{
}

QuartzDatabase::~QuartzDatabase()
{
    try {
	internal_end_session();
    } catch (...) {
	// Ignore any exceptions, since we may be being called due to an
	// exception anyway.  internal_end_session() should have already
	// been called, in the normal course of events.
	DEBUGLINE(DB, "Ignoring exception in QuartzDatabase destructor.");
    }
}

std::string
QuartzDatabase::get_db_dir(const OmSettings & settings)
{
    return settings.get("quartz_dir");
}

std::string
QuartzDatabase::get_log_filename(const OmSettings & settings)
{
    return settings.get("quartz_logfile", "");
}

bool
QuartzDatabase::get_perform_recovery(const OmSettings & settings)
{
    return settings.get_bool("quartz_perform_recovery", false);
}

unsigned int
QuartzDatabase::get_block_size(const OmSettings & settings)
{
    return settings.get_int("quartz_block_size", QUARTZ_BTREE_DEF_BLOCK_SIZE);
}

void
QuartzDatabase::do_begin_session()
{
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
QuartzDatabase::do_add_document(const OmDocument & document)
{ Assert(false); }

void
QuartzDatabase::do_delete_document(om_docid did)
{ Assert(false); }

void
QuartzDatabase::do_replace_document(om_docid did,
				    const OmDocument & document)
{ Assert(false); }

om_doccount 
QuartzDatabase::get_doccount() const
{
    OmLockSentry sentry(quartz_mutex);

    return get_doccount_internal();
}

om_doccount
QuartzDatabase::get_doccount_internal() const
{
    return QuartzRecordManager::get_doccount(*(tables->get_record_table()));
}

om_doclength
QuartzDatabase::get_avlength() const
{
    OmLockSentry sentry(quartz_mutex);

    return get_avlength_internal();
}

om_doclength
QuartzDatabase::get_avlength_internal() const
{
    // FIXME: probably want to cache this value (but not miss updates)
    om_doccount docs = get_doccount_internal();
    if (docs == 0) return 0;
    om_totlength totlen = QuartzRecordManager::get_total_length(*(tables->get_record_table()));

    return (double) totlen / docs;
}

om_doclength
QuartzDatabase::get_doclength(om_docid did) const
{
    Assert(did != 0);
    OmLockSentry sentry(quartz_mutex);
    throw OmUnimplementedError("QuartzDatabase::get_doclength() not yet implemented");
}

om_doccount
QuartzDatabase::get_termfreq(const om_termname & tname) const
{
    Assert(tname.size() != 0);
    OmLockSentry sentry(quartz_mutex);

    om_doccount termfreq = 0; // If not found, this value will be unchanged.
    QuartzLexicon::get_entry(tables->get_lexicon_table(),
			     tname,
			     &termfreq);
    return termfreq;
}

om_termcount
QuartzDatabase::get_collection_freq(const om_termname & tname) const
{
    Assert(tname.size() != 0);
    OmLockSentry sentry(quartz_mutex);

    om_termcount collfreq = 0; // If not found, this value will be unchanged.
    QuartzPostList pl(0,
		      tables->get_postlist_table(),
		      tables->get_positionlist_table(),
		      tname);
    collfreq = pl.get_collection_freq();
    return collfreq;
}

bool
QuartzDatabase::term_exists(const om_termname & tname) const
{
    Assert(tname.size() != 0);
    OmLockSentry sentry(quartz_mutex);
    return QuartzLexicon::get_entry(tables->get_lexicon_table(),
				    tname, 0);
}


LeafPostList *
QuartzDatabase::do_open_post_list(const om_termname& tname) const
{
    OmLockSentry sentry(quartz_mutex);

    RefCntBase::RefCntPtrToThis tmp;
    RefCntPtr<const QuartzDatabase> ptrtothis(tmp, this);

    return open_post_list_internal(tname, ptrtothis);
}

LeafPostList *
QuartzDatabase::open_post_list_internal(const om_termname& tname,
				RefCntPtr<const Database> ptrtothis) const
{
    Assert(tname.size() != 0);
    return(new QuartzPostList(ptrtothis,
			      tables->get_postlist_table(),
			      tables->get_positionlist_table(),
			      tname));
}

LeafTermList *
QuartzDatabase::open_term_list_internal(om_docid did,
				RefCntPtr<const Database> ptrtothis) const
{
    Assert(did != 0);
    return(new QuartzTermList(ptrtothis,
			      tables->get_termlist_table(),
			      tables->get_lexicon_table(),
			      did,
			      get_doccount_internal()));
}

LeafTermList *
QuartzDatabase::open_term_list(om_docid did) const
{
    OmLockSentry sentry(quartz_mutex);

    RefCntBase::RefCntPtrToThis tmp;
    RefCntPtr<const QuartzDatabase> ptrtothis(tmp, this);

    return open_term_list_internal(did, ptrtothis);
}

Document *
QuartzDatabase::open_document(om_docid did) const
{
    Assert(did != 0);
    OmLockSentry sentry(quartz_mutex);

    RefCntBase::RefCntPtrToThis tmp;
    RefCntPtr<const QuartzDatabase> ptrtothis(tmp, this);

    return new QuartzDocument(ptrtothis,
			      tables->get_attribute_table(),
			      tables->get_record_table(),
			      did);
}

AutoPtr<PositionList> 
QuartzDatabase::open_position_list(om_docid did,
				   const om_termname & tname) const
{
    Assert(did != 0);
    OmLockSentry sentry(quartz_mutex);

    AutoPtr<QuartzPositionList> poslist(new QuartzPositionList());
    poslist->read_data(tables->get_positionlist_table(), did, tname);

    return AutoPtr<PositionList>(poslist.release());
}

void
QuartzDatabase::do_reopen()
{
    tables->reopen();
}

TermList *
QuartzDatabase::open_allterms() const
{
    AutoPtr<QuartzCursor> pl_cursor(tables->get_postlist_table()->cursor_get());
    return new QuartzAllTermsList(RefCntPtr<const QuartzDatabase>(RefCntPtrToThis(), this),
				  pl_cursor);
}


QuartzWritableDatabase::QuartzWritableDatabase(const OmSettings & settings)
	: buffered_tables(new QuartzBufferedTableManager(
				QuartzDatabase::get_db_dir(settings),
				QuartzDatabase::get_log_filename(settings),
				QuartzDatabase::get_perform_recovery(settings),
				QuartzDatabase::get_block_size(settings))),
	  changecount(0),
	  database_ro(AutoPtr<QuartzTableManager>(buffered_tables))
{
}

QuartzWritableDatabase::~QuartzWritableDatabase()
{
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
    OmLockSentry sentry(database_ro.quartz_mutex);
    Assert(buffered_tables != 0);

    // FIXME - get a write lock on the database
}

void
QuartzWritableDatabase::do_end_session()
{
    OmLockSentry sentry(database_ro.quartz_mutex);
    Assert(buffered_tables != 0);

    buffered_tables->apply();

    // FIXME - release write lock on the database (even if an apply() throws)
}

void
QuartzWritableDatabase::do_flush()
{
    OmLockSentry sentry(database_ro.quartz_mutex);
    Assert(buffered_tables != 0);

    buffered_tables->apply();
}

void
QuartzWritableDatabase::do_begin_transaction()
{
    OmLockSentry sentry(database_ro.quartz_mutex);
    throw OmUnimplementedError("QuartzDatabase::do_begin_transaction() not yet implemented");
}

void
QuartzWritableDatabase::do_commit_transaction()
{
    OmLockSentry sentry(database_ro.quartz_mutex);
    throw OmUnimplementedError("QuartzDatabase::do_commit_transaction() not yet implemented");
}

void
QuartzWritableDatabase::do_cancel_transaction()
{
    OmLockSentry sentry(database_ro.quartz_mutex);
    throw OmUnimplementedError("QuartzDatabase::do_cancel_transaction() not yet implemented");
}

om_docid
QuartzWritableDatabase::do_add_document(const OmDocument & document)
{
    DEBUGCALL(DB, om_docid,
	      "QuartzWritableDatabase::do_add_document", document);

    OmLockSentry sentry(database_ro.quartz_mutex);

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
		document.get_data(),
		new_doclen);
	Assert(did != 0);

	// Set the attributes.
	{
	    OmKeyListIterator key = document.keylist_begin();
	    OmKeyListIterator key_end = document.keylist_end();
	    for ( ; key != key_end; ++key) {
		QuartzAttributesManager::add_attribute(
		    *(buffered_tables->get_attribute_table()),
		    *key, did, key.get_keyno());
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

	{
	    OmTermIterator term = document.termlist_begin();
	    OmTermIterator term_end = document.termlist_end();    
	    for ( ; term != term_end; ++term) {
		QuartzLexicon::increment_termfreq(
		    buffered_tables->get_lexicon_table(),
		    *term);
		QuartzPostList::add_entry(buffered_tables->get_postlist_table(),
					  *term, did, term.get_wdf(),
					  new_doclen);
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
    if (++changecount > 1000) {
	buffered_tables->apply();
	changecount = 0;
    }

    RETURN(did);
}

void
QuartzWritableDatabase::do_delete_document(om_docid did)
{
    Assert(did != 0);
    OmLockSentry sentry(database_ro.quartz_mutex);
    Assert(buffered_tables != 0);

    try {
	QuartzDatabase::RefCntPtrToThis tmp;
	RefCntPtr<const QuartzWritableDatabase> ptrtothis(tmp, this);

	QuartzTermList termlist(ptrtothis,
				database_ro.tables->get_termlist_table(),
				database_ro.tables->get_lexicon_table(),
				did,
				database_ro.get_doccount_internal());

	termlist.next();
	while (!termlist.at_end()) {
	    om_termname tname = termlist.get_termname();
	    QuartzPostList::delete_entry(buffered_tables->get_postlist_table(),
		tname, did);
	    QuartzPositionList::delete_positionlist(
		buffered_tables->get_positionlist_table(),
		did, tname);
	    QuartzLexicon::decrement_termfreq(
		buffered_tables->get_lexicon_table(),
		tname);
	    termlist.next();
	}

	// Set the document length.
	// (New doclen is always zero, since we're deleting the document.)
	quartz_doclen_t old_doclen = termlist.get_doclength();
	QuartzRecordManager::modify_total_length(
		*(buffered_tables->get_record_table()),
		old_doclen,
		0);

	// Remove the attributes
	// FIXME: implement

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
	buffered_tables->apply();
	changecount = 0;
    }
}

void
QuartzWritableDatabase::do_replace_document(om_docid did,
				    const OmDocument & document)
{
    Assert(did != 0);
    OmLockSentry sentry(database_ro.quartz_mutex);

    Assert(buffered_tables != 0);
    //return modifications->replace_document(did, document);

    // Note: If an error occurs while adding a document, or doing any other
    // transaction, the modifications so far must be cleared before
    // returning control to the user - otherwise partial modifications will
    // persist in memory, and eventually get written to disk.

    throw OmUnimplementedError("QuartzWritableDatabase::do_replace_document() not yet implemented");

    // FIXME: this should be configurable
    // FIXME: this should be done by checking memory usage, not the number of
    // changes.
    if (++changecount > 1000) {
	buffered_tables->apply();
	changecount = 0;
    }
}

om_doccount 
QuartzWritableDatabase::get_doccount() const
{
    OmLockSentry sentry(database_ro.quartz_mutex);
    return database_ro.get_doccount_internal();
}

om_doclength
QuartzWritableDatabase::get_avlength() const
{
    return database_ro.get_avlength();
}

om_doclength
QuartzWritableDatabase::get_doclength(om_docid did) const
{
    return database_ro.get_doclength(did);
}

om_doccount
QuartzWritableDatabase::get_termfreq(const om_termname & tname) const
{
    return database_ro.get_termfreq(tname);
}

om_termcount
QuartzWritableDatabase::get_collection_freq(const om_termname & tname) const
{
    return database_ro.get_collection_freq(tname);
}

bool
QuartzWritableDatabase::term_exists(const om_termname & tname) const
{
    return database_ro.term_exists(tname);
}


LeafPostList *
QuartzWritableDatabase::do_open_post_list(const om_termname& tname) const
{
    OmLockSentry sentry(database_ro.quartz_mutex);

    RefCntBase::RefCntPtrToThis tmp;
    RefCntPtr<const QuartzWritableDatabase> ptrtothis(tmp, this);

    return database_ro.open_post_list_internal(tname, ptrtothis);
}

LeafTermList *
QuartzWritableDatabase::open_term_list(om_docid did) const
{
    OmLockSentry sentry(database_ro.quartz_mutex);

    RefCntBase::RefCntPtrToThis tmp;
    RefCntPtr<const QuartzWritableDatabase> ptrtothis(tmp, this);

    return database_ro.open_term_list_internal(did, ptrtothis);
}

Document *
QuartzWritableDatabase::open_document(om_docid did) const
{
    Assert(did != 0);
    OmLockSentry sentry(database_ro.quartz_mutex);

    RefCntBase::RefCntPtrToThis tmp;
    RefCntPtr<const QuartzWritableDatabase> ptrtothis(tmp, this);

    return new QuartzDocument(ptrtothis,
			      buffered_tables->get_attribute_table(),
			      buffered_tables->get_record_table(),
			      did);
}

AutoPtr<PositionList> 
QuartzWritableDatabase::open_position_list(om_docid did,
				   const om_termname & tname) const
{
    Assert(did != 0);
    OmLockSentry sentry(database_ro.quartz_mutex);

    AutoPtr<QuartzPositionList> poslist(new QuartzPositionList());
    poslist->read_data(buffered_tables->get_positionlist_table(), did, tname);

    return AutoPtr<PositionList>(poslist.release());
}

void
QuartzWritableDatabase::do_reopen()
{
    /* Do nothing - we're the only writer, and so must be up to date. */
}

TermList *
QuartzWritableDatabase::open_allterms() const
{
    AutoPtr<QuartzCursor> pl_cursor(buffered_tables->get_postlist_table()->cursor_get());
    return new QuartzAllTermsList(RefCntPtr<const QuartzWritableDatabase>(RefCntPtrToThis(), this),
				  pl_cursor);
}
