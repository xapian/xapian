/* database.cc: Methods for Database
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
#include "database.h"
#include <om/omerror.h>

Database::Database()
	: session_in_progress(false),
	  transaction_in_progress(false)
{
}

Database::~Database()
{
    // Can't end_session() here because derived class destructors have already
    // run, and the derived classes therefore don't exist.  Thus, the
    // derived classes have responsibility for ending outstanding sessions
    // (by calling internal_end_session()):  let's check they did their job.
    Assert(!session_in_progress);
}

void
Database::begin_session(om_timeout timeout)
{
    OmLockSentry sentry(mutex);
    if (session_in_progress)
	throw OmInvalidOperationError("Cannot begin session - session already in progress");
    do_begin_session(timeout);
    session_in_progress = true;
}

void
Database::end_session()
{
    if (!session_in_progress)
	throw OmInvalidOperationError("Cannot end session - no session currently in progress");

    internal_end_session();
}

void
Database::internal_end_session()
{
    OmLockSentry sentry(mutex);
    if (!session_in_progress) return;

    if (transaction_in_progress) {
	try {
	    transaction_in_progress = false;
	    do_cancel_transaction();
	} catch (...) {
	    session_in_progress = false;
	    try {
		do_end_session();
	    } catch (...) {
		// Discard exception - we want to re-throw the first error
		// which occured
	    }
	    throw;
	}
    }

    session_in_progress = false;
    do_end_session();
}

void
Database::flush()
{
    OmLockSentry sentry(mutex);
    if (!session_in_progress)
	throw OmInvalidOperationError("Cannot flush modifications - no session in progress");
    do_flush();
}

void
Database::begin_transaction()
{
    OmLockSentry sentry(mutex);
    if (!session_in_progress)
	throw OmInvalidOperationError("Cannot begin transaction - no session currently in progress");
    if (transaction_in_progress)
	throw OmInvalidOperationError("Cannot begin transaction - transaction already in progress");
    do_begin_transaction();
    transaction_in_progress = true;
}

void
Database::commit_transaction()
{
    OmLockSentry sentry(mutex);
    if (!transaction_in_progress)
	throw OmInvalidOperationError("Cannot commit transaction - no transaction currently in progress");
    Assert(session_in_progress);
    transaction_in_progress = false;
    do_commit_transaction();
}

void
Database::cancel_transaction()
{
    OmLockSentry sentry(mutex);
    if (!transaction_in_progress)
	throw OmInvalidOperationError("Cannot cancel transaction - no transaction currently in progress");
    Assert(session_in_progress);
    transaction_in_progress = false;
    do_cancel_transaction();
}

om_docid
Database::add_document(const OmDocumentContents & document,
			 om_timeout timeout)
{
    OmLockSentry sentry(mutex);

    bool implicit_session = !session_in_progress;
    if (implicit_session) {
	do_begin_session(timeout);
    }

    om_docid did = 0;

    try {
	did = do_add_document(document);
    } catch(...) {
	if(implicit_session) {
	    try {
		do_end_session();
	    } catch (...) {
		// Discard exception - we want to re-throw the first error
		// which occured
	    }
	}
	throw;
    }
    do_end_session();

    return did;
}

void
Database::delete_document(om_docid did, om_timeout timeout)
{
    OmLockSentry sentry(mutex);

    bool implicit_session = !session_in_progress;
    if (implicit_session) {
	do_begin_session(timeout);
    }

    try {
	do_delete_document(did);
    } catch(...) {
	if(implicit_session) {
	    try {
		do_end_session();
	    } catch (...) {
		// Discard exception - we want to re-throw the first error
		// which occured
	    }
	}
	throw;
    }
    do_end_session();
}

void
Database::replace_document(om_docid did,
			     const OmDocumentContents & document,
			     om_timeout timeout)
{
    OmLockSentry sentry(mutex);

    bool implicit_session = !session_in_progress;
    if (implicit_session) {
	do_begin_session(timeout);
    }

    try {
	do_replace_document(did, document);
    } catch(...) {
	if(implicit_session) {
	    try {
		do_end_session();
	    } catch (...) {
		// Discard exception - we want to re-throw the first error
		// which occured
	    }
	}
	throw;
    }
    do_end_session();
}

OmDocumentContents
Database::get_document(om_docid did)
{
    return do_get_document(did);
}
