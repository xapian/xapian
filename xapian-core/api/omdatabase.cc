/* omdatabase.cc: External interface for running queries
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

#include "om/omerror.h"
#include "omdatabaseinternal.h"
#include "omdebug.h"
#include "om/ompostlistiterator.h"
#include "ompostlistiteratorinternal.h"
#include "om/omtermlistiterator.h"
#include "omtermlistiteratorinternal.h"
#include "om/omoutput.h"

OmDatabase::OmDatabase()
{
    DEBUGAPICALL("OmDatabase::OmDatabase", "");
    internal = new OmDatabase::Internal();
}

OmDatabase::OmDatabase(const OmSettings & params, bool readonly)
	: internal(new OmDatabase::Internal(params, readonly))
{
    DEBUGAPICALL("OmDatabase::OmDatabase", params << ", " << readonly);
}

OmDatabase::OmDatabase(const OmSettings & params)
	: internal(new OmDatabase::Internal(params, true))
{
    DEBUGAPICALL("OmDatabase::OmDatabase", params);
}

OmDatabase::OmDatabase(const OmDatabase &other)
	: internal(0)
{
    DEBUGAPICALL("OmDatabase::OmDatabase", "OmDatabase");
    OmLockSentry locksentry(other.internal->mutex);

    internal = new Internal(*(other.internal));
}

void
OmDatabase::operator=(const OmDatabase &other)
{
    DEBUGAPICALL("OmDatabase::operator=", "OmDatabase");
    if (this == &other) {
	DEBUGLINE(API, "OmDatabase assigned to itself");
	return;
    }

    // we get these locks in a defined order to avoid deadlock
    // should two threads try to assign two databases to each
    // other at the same time.
    Internal * newinternal;

    {
	OmLockSentry locksentry1(std::min(internal, other.internal)->mutex);
	OmLockSentry locksentry2(std::max(internal, other.internal)->mutex);

	newinternal = new Internal(*(other.internal));

	std::swap(internal, newinternal);
    }

    delete newinternal;
}

OmDatabase::~OmDatabase()
{
    DEBUGAPICALL("OmDatabase::~OmDatabase", "");
    delete internal;
    internal = 0;
}

void
OmDatabase::add_database(const OmSettings &params)
{
    DEBUGAPICALL("OmDatabase::add_database", params);
    internal->add_database(params);
}

void
OmDatabase::add_database(const OmDatabase & database)
{
    DEBUGAPICALL("OmDatabase::add_database", "OmDatabase");
    if (this == &database) {
	DEBUGLINE(API, "OmDatabase added to itself");
	throw OmInvalidArgumentError("Can't add an OmDatabase to itself");
	return;
    }
    std::vector<RefCntPtr<IRDatabase> >::iterator i;
    OmLockSentry locksentry(database.internal->mutex);
    for (i = database.internal->databases.begin();
	 i != database.internal->databases.end(); i++) {
	internal->add_database(*i);
    }
}

OmPostListIterator
OmDatabase::postlist_begin(const om_termname &tname) const
{
    DEBUGAPICALL("OmDatabase::postlist_begin", tname);
    if (tname.empty()) throw OmInvalidArgumentError("Zero length terms are invalid");
    DEBUGAPIRETURN("OmPostListIterator");    
    return OmPostListIterator(new OmPostListIterator::Internal(internal->get_multi_database()->open_post_list(tname)));
}

OmPostListIterator
OmDatabase::postlist_end(const om_termname &tname) const
{
    DEBUGAPICALL("OmDatabase::postlist_end", tname);
    if (tname.empty()) throw OmInvalidArgumentError("Zero length terms are invalid");
    DEBUGAPIRETURN("OmPostListIterator");
    return OmPostListIterator(NULL);
}

OmTermListIterator
OmDatabase::termlist_begin(om_docid did) const
{
    DEBUGAPICALL("OmDatabase::termlist_begin", did);
    DEBUGAPIRETURN("OmTermListIterator");
    return OmTermListIterator(new OmTermListIterator::Internal(internal->get_multi_database()->open_term_list(did)));
}

OmTermListIterator
OmDatabase::termlist_end(om_docid did) const
{
    DEBUGAPICALL("OmDatabase::termlist_end", did);
    DEBUGAPIRETURN("OmTermListIterator");
    return OmTermListIterator(NULL);
}

std::string
OmDatabase::get_description() const
{
    DEBUGAPICALL("OmDatabase::get_description", "");
    /// \todo display contents of the database
    std::string description = "OmDatabase()";
    DEBUGAPIRETURN(description);
    return description;
}


OmWritableDatabase::OmWritableDatabase(const OmSettings & params)
	: OmDatabase(params, false)
{
    DEBUGAPICALL("OmWritableDatabase::OmWritableDatabase", params);
}

OmWritableDatabase::OmWritableDatabase(const OmWritableDatabase &other)
	: OmDatabase(other)
{
    DEBUGAPICALL("OmWritableDatabase::OmWritableDatabase", "OmWritableDatabase");
}

void
OmWritableDatabase::operator=(const OmWritableDatabase &other)
{
    DEBUGAPICALL("OmWritableDatabase::operator=", "OmWritableDatabase");
    OmDatabase::operator=(other);
}

OmWritableDatabase::~OmWritableDatabase()
{
    DEBUGAPICALL("OmWritableDatabase::~OmWritableDatabase", "");
    delete internal;
    internal = 0;
}

void
OmWritableDatabase::begin_session(om_timeout timeout)
{
    DEBUGAPICALL("OmWritableDatabase::begin_session", timeout);
    // Get the pointer while locked, in case someone is assigning to it.
    IRDatabase * database;
    {
	OmLockSentry locksentry(internal->mutex);
	database = internal->databases[0].get();
    }

    database->begin_session(timeout);
}

void
OmWritableDatabase::end_session()
{
    DEBUGAPICALL("OmWritableDatabase::end_session", "");
    // Get the pointer while locked, in case someone is assigning to it.
    IRDatabase * database;
    {
	OmLockSentry locksentry(internal->mutex);
	database = internal->databases[0].get();
    }

    database->end_session();
}

void
OmWritableDatabase::flush()
{
    DEBUGAPICALL("OmWritableDatabase::flush", "");
    // Get the pointer while locked, in case someone is assigning to it.
    IRDatabase * database;
    {
	OmLockSentry locksentry(internal->mutex);
	database = internal->databases[0].get();
    }

    database->flush();
}

void
OmWritableDatabase::begin_transaction()
{
    DEBUGAPICALL("OmWritableDatabase::begin_transaction", "");
    // Get the pointer while locked, in case someone is assigning to it.
    IRDatabase * database;
    {
	OmLockSentry locksentry(internal->mutex);
	database = internal->databases[0].get();
    }

    database->begin_transaction();
}

void
OmWritableDatabase::commit_transaction()
{
    DEBUGAPICALL("OmWritableDatabase::commit_transaction", "");
    // Get the pointer while locked, in case someone is assigning to it.
    IRDatabase * database;
    {
	OmLockSentry locksentry(internal->mutex);
	database = internal->databases[0].get();
    }

    database->commit_transaction();
}

void
OmWritableDatabase::cancel_transaction()
{
    DEBUGAPICALL("OmWritableDatabase::cancel_transaction", "");
    // Get the pointer while locked, in case someone is assigning to it.
    IRDatabase * database;
    {
	OmLockSentry locksentry(internal->mutex);
	database = internal->databases[0].get();
    }

    database->cancel_transaction();
}


om_docid
OmWritableDatabase::add_document(const OmDocumentContents & document,
				 om_timeout timeout)
{
    DEBUGAPICALL("OmWritableDatabase::add_document",
		 document << ", " << timeout);
    // Check the validity of the document
    OmDocumentContents::document_terms::const_iterator i;
    for(i = document.terms.begin(); i != document.terms.end(); i++) {
	if(i->second.tname.size() == 0) {
	    throw OmInvalidArgumentError(
		"Cannot add termnames of zero length to the database.");
	}
    }

    // Get the pointer while locked, in case someone is assigning to it.
    IRDatabase * database;
    {
	OmLockSentry locksentry(internal->mutex);
	database = internal->databases[0].get();
    }

    om_docid did = database->add_document(document, timeout);
    DEBUGAPIRETURN(did);
    return did;
}

void
OmWritableDatabase::delete_document(om_docid did, om_timeout timeout)
{
    DEBUGAPICALL("OmWritableDatabase::delete_document",
		 did << ", " << timeout);
    // Get the pointer while locked, in case someone is assigning to it.
    IRDatabase * database;
    {
	OmLockSentry locksentry(internal->mutex);
	database = internal->databases[0].get();
    }

    database->delete_document(did, timeout);
}

void
OmWritableDatabase::replace_document(om_docid did,
				     const OmDocumentContents & document,
				     om_timeout timeout)
{
    DEBUGAPICALL("OmWritableDatabase::replace_document",
		 did << ", " << document << ", " << timeout);
    // Get the pointer while locked, in case someone is assigning to it.
    IRDatabase * database;
    {
	OmLockSentry locksentry(internal->mutex);
	database = internal->databases[0].get();
    }

    database->replace_document(did, document, timeout);
}

OmDocumentContents
OmWritableDatabase::get_document(om_docid did) const
{
    DEBUGAPICALL("OmWritableDatabase::get_document", did);
    // Get the pointer while locked, in case someone is assigning to it.
    IRDatabase * database;
    {
	OmLockSentry locksentry(internal->mutex);
	database = internal->databases[0].get();
    }

    return database->get_document(did);
}

std::string
OmWritableDatabase::get_description() const
{
    /// \todo display contents of the writable database
    std::string description = "OmWritableDatabase()";
    DEBUGAPICALL("OmWritableDatabase::get_description", "");
    return description;
}
