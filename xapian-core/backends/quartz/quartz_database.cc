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
#include "om/autoptr.h"
#include "om/omerror.h"

#include "quartz_record.h"
#include "quartz_attributes.h"

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
    return settings.get_bool("quartz_block_size", QUARTZ_BTREE_DEF_BLOCK_SIZE);
}

void
QuartzDatabase::do_begin_session(om_timeout timeout)
{
    throw OmInvalidOperationError(
	"Cannot begin a modification session: database opened readonly.");
}

void
QuartzDatabase::do_end_session()
{
    Assert(false);
}

void
QuartzDatabase::do_flush()
{
    Assert(false);
}

void
QuartzDatabase::do_begin_transaction()
{
    Assert(false);
}

void
QuartzDatabase::do_commit_transaction()
{
    Assert(false);
}

void
QuartzDatabase::do_cancel_transaction()
{
    Assert(false);
}

om_docid
QuartzDatabase::do_add_document(const OmDocumentContents & document)
{
    Assert(false);
}

void
QuartzDatabase::do_delete_document(om_docid did)
{
    Assert(false);
}

void
QuartzDatabase::do_replace_document(om_docid did,
				    const OmDocumentContents & document)
{
    Assert(false);
}

OmDocumentContents
QuartzDatabase::do_get_document(om_docid did)
{
    OmLockSentry sentry(quartz_mutex);

    return do_get_document_internal(did);
}

OmDocumentContents
QuartzDatabase::do_get_document_internal(om_docid did)
{
    OmDocumentContents document;

    document.data = QuartzRecordManager::get_record(
			*(tables->get_record_table()), did);

    QuartzAttributesManager::get_all_attributes(
			*(tables->get_attribute_table()),
			document.keys,
			did);

    return document;
}


om_doccount 
QuartzDatabase::get_doccount() const
{
    OmLockSentry sentry(quartz_mutex);

    return QuartzRecordManager::get_doccount(*(tables->get_record_table()));
}

om_doclength
QuartzDatabase::get_avlength() const
{
    OmLockSentry sentry(quartz_mutex);
    throw OmUnimplementedError("QuartzDatabase::get_avlength() not yet implemented");
}

om_doclength
QuartzDatabase::get_doclength(om_docid did) const
{
    OmLockSentry sentry(quartz_mutex);
    throw OmUnimplementedError("QuartzDatabase::get_doclength() not yet implemented");
}

om_doccount
QuartzDatabase::get_termfreq(const om_termname & tname) const
{
    OmLockSentry sentry(quartz_mutex);
    throw OmUnimplementedError("QuartzDatabase::get_termfreq() not yet implemented");
}

bool
QuartzDatabase::term_exists(const om_termname & tname) const
{
    Assert(tname.size() != 0);
    OmLockSentry sentry(quartz_mutex);
    throw OmUnimplementedError("QuartzDatabase::term_exists() not yet implemented");
}


LeafPostList *
QuartzDatabase::do_open_post_list(const om_termname& tname) const
{
    OmLockSentry sentry(quartz_mutex);
    throw OmUnimplementedError("QuartzDatabase::do_open_post_list() not yet implemented");
}

LeafTermList *
QuartzDatabase::open_term_list(om_docid did) const
{
    OmLockSentry sentry(quartz_mutex);
    throw OmUnimplementedError("QuartzDatabase::open_term_list() not yet implemented");
}

LeafDocument *
QuartzDatabase::open_document(om_docid did) const
{
    OmLockSentry sentry(quartz_mutex);
    throw OmUnimplementedError("QuartzDatabase::open_document() not yet implemented");
}





QuartzWritableDatabase::QuartzWritableDatabase(const OmSettings & settings)
	: buffered_tables(new QuartzBufferedTableManager(
				QuartzDatabase::get_db_dir(settings),
				QuartzDatabase::get_log_filename(settings),
				QuartzDatabase::get_perform_recovery(settings),
				QuartzDatabase::get_block_size(settings))),
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
QuartzWritableDatabase::do_begin_session(om_timeout timeout)
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

    bool success = buffered_tables->apply();

    // FIXME - release write lock on the database

    if (!success) {
	throw OmDatabaseError("Unable to modify database - modifications lost.");
    }
}

void
QuartzWritableDatabase::do_flush()
{
    OmLockSentry sentry(database_ro.quartz_mutex);
    Assert(buffered_tables != 0);

    buffered_tables->apply();

    bool success = buffered_tables->apply();

    if (!success) {
	throw OmDatabaseError("Unable to modify database - modifications lost.");   
    }
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
QuartzWritableDatabase::do_add_document(const OmDocumentContents & document)
{
    OmLockSentry sentry(database_ro.quartz_mutex);

    Assert(buffered_tables != 0);

    om_doclength doclen = 0;

    OmDocumentContents::document_terms::const_iterator i;
    for (i = document.terms.begin(); i != document.terms.end(); i++) {
	const OmDocumentTerm & t = i->second;
	doclen += t.wdf;
    }

    om_docid did = QuartzRecordManager::add_record(
			*(buffered_tables->get_record_table()),
			document.data,
			doclen);

    for (i = document.terms.begin(); i != document.terms.end(); i++) {
#if 0
	QuartzPostList::add_posting(*(buffered_tables.get_postlist_table()),
				    i->second.tname,
				    did,
				    i->second.wdf);
	QuartzPositionList::add_positionlist(
				    *(buffered_tables.get_positionlist_table()),
				    did,
				    i->second.tname,
				    i->second.positions);
#endif
    }

    return did;
}

void
QuartzWritableDatabase::do_delete_document(om_docid did)
{
    OmLockSentry sentry(database_ro.quartz_mutex);
    Assert(buffered_tables != 0);

#if 0
    OmDocumentContents document(database_ro.do_get_document_internal(did));

    OmDocumentContents::document_terms::const_iterator i;
    for (i = document.terms.begin(); i != document.terms.end(); i++) {
	QuartzPostList::delete_posting(*(buffered_tables.get_postlist_table()),
				       i->second.tname,
				       did,
				       i->second.wdf);
	QuartzPositionList::delete_positionlist(
				    *(buffered_tables.get_positionlist_table()),
				    did,
				    i->second.tname,
				    i->second.positions);
    }
#endif

    QuartzRecordManager::delete_record(*(buffered_tables->get_record_table()),
				       did);
}

void
QuartzWritableDatabase::do_replace_document(om_docid did,
				    const OmDocumentContents & document)
{
    OmLockSentry sentry(database_ro.quartz_mutex);

    Assert(buffered_tables != 0);
    //return modifications->replace_document(did, document);
    throw OmUnimplementedError("QuartzWritableDatabase::do_replace_document() not yet implemented");
}

OmDocumentContents
QuartzWritableDatabase::do_get_document(om_docid did)
{
    OmLockSentry sentry(database_ro.quartz_mutex);

    return database_ro.do_get_document_internal(did);
}

om_doccount 
QuartzWritableDatabase::get_doccount() const
{
    return database_ro.get_doccount();
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

bool
QuartzWritableDatabase::term_exists(const om_termname & tname) const
{
    return database_ro.term_exists(tname);
}


LeafPostList *
QuartzWritableDatabase::do_open_post_list(const om_termname& tname) const
{
    return database_ro.do_open_post_list(tname);
}

LeafTermList *
QuartzWritableDatabase::open_term_list(om_docid did) const
{
    return database_ro.open_term_list(did);
}

LeafDocument *
QuartzWritableDatabase::open_document(om_docid did) const
{
    return database_ro.open_document(did);
}

