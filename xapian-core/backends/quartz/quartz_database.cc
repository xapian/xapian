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
#include "quartz_modifications.h"

#include "quartz_database.h"
#include "autoptr.h"
#include "utils.h"
#include <om/omerror.h>
#include <string>

//
// Compulsory settings.
// quartz_dir    - Directory that the database is stored in.  Must be a full
//                 path.
//
// Optional settings.
// quartz_tmpdir - Directory in which to store temporary files.  If not
//                 specified, the database directory will be used.  If this
//                 is a relative path, it is taken to be relative to the
//                 quartz_dir directory.
//
// quartz_use_transactions - Boolean, true if transactions should be used.
//		   Defaults to false.  If false, begin_transaction,
//		   cancel_transaction, and commit_transaction will all
//		   throw an exception if called.  Also, all updates will
//		   require a modified copy of the entire database to be
//		   created, which will then replace the old database.
//		   This does have the advantage that databases may be
//		   stored on NFS partitions.
//
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
QuartzDatabase::QuartzDatabase(const OmSettings & settings, bool readonly)
	: modifications(0),
	  use_transactions(false),
	  readonly(readonly)
{
    // Read parameters
    string db_dir  = settings.get("quartz_dir");
    string tmp_dir = settings.get("quartz_tmpdir", db_dir);
    string log_filename = settings.get("quartz_logfile", "");
    use_transactions = settings.get_bool("quartz_use_transactions", false);
    bool perform_recovery = settings.get_bool("quartz_perform_recovery", false);

    // Open database manager
    table_manager.reset(new QuartzTableManager(db_dir,
					       tmp_dir,
					       log_filename,
					       readonly,
					       perform_recovery));
}

QuartzDatabase::~QuartzDatabase()
{
    try {
	internal_end_session();
    } catch (...) {
	// Ignore any exceptions, since we may be being called due to an
	// exception anyway.  internal_end_session() should have already
	// been called, in the normal course of events.
    }
}


void
QuartzDatabase::do_begin_session(om_timeout timeout)
{
    OmLockSentry sentry(quartz_mutex);

    if(readonly) {
	throw OmInvalidOperationError("Cannot begin a modification session: "
				      "database opened readonly.");
    }
    modifications.reset(new QuartzModifications(table_manager.get()));
    Assert(modifications.get() != 0);
}

void
QuartzDatabase::do_end_session()
{
    OmLockSentry sentry(quartz_mutex);

    Assert(!readonly);
    Assert(modifications.get() != 0);
    modifications->apply();
    modifications.reset();
}

void
QuartzDatabase::do_flush()
{
    OmLockSentry sentry(quartz_mutex);

    Assert(!readonly);
    Assert(modifications.get() != 0);
    modifications->apply();
}


void
QuartzDatabase::do_begin_transaction()
{
    OmLockSentry sentry(quartz_mutex);
    if (!use_transactions) {
	throw OmInvalidOperationError("Database is not opened with transaction support.");
    }

    // Start a new modifications object, which must be able to
    // work while there's a quiescent other modifications object.
    // Flush should flush the quiescent one.
    throw OmUnimplementedError("QuartzDatabase::do_begin_transaction() not yet implemented");
}

void
QuartzDatabase::do_commit_transaction()
{
    OmLockSentry sentry(quartz_mutex);

    throw OmUnimplementedError("QuartzDatabase::do_commit_transaction() not yet implemented");
}

void
QuartzDatabase::do_cancel_transaction()
{
    OmLockSentry sentry(quartz_mutex);

    throw OmUnimplementedError("QuartzDatabase::do_cancel_transaction() not yet implemented");
}


om_docid
QuartzDatabase::do_add_document(const OmDocumentContents & document)
{
    OmLockSentry sentry(quartz_mutex);

    Assert(modifications.get() != 0);
    return modifications->add_document(document);
}

void
QuartzDatabase::do_delete_document(om_docid did)
{
    OmLockSentry sentry(quartz_mutex);

    Assert(modifications.get() != 0);
    return modifications->delete_document(did);
}

void
QuartzDatabase::do_replace_document(om_docid did,
				    const OmDocumentContents & document)
{
    OmLockSentry sentry(quartz_mutex);

    Assert(modifications.get() != 0);
    return modifications->replace_document(did, document);
}


OmDocumentContents
QuartzDatabase::do_get_document(om_docid did)
{
    OmLockSentry sentry(quartz_mutex);


    throw OmUnimplementedError("QuartzDatabase::do_get_document() not yet implemented");
}



om_doccount 
QuartzDatabase::get_doccount() const
{
    OmLockSentry sentry(quartz_mutex);

    // FIXME: check that the sizes of these types (om_doccount and
    // quartz_tablesize_t) are compatible.
    return table_manager->record_table.get_entry_count() - 1;
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

