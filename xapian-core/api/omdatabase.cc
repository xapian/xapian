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
#include <om/omoutput.h>

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
	: internal(new Internal(*(other.internal)))
{
    DEBUGAPICALL("OmDatabase::OmDatabase", "OmDatabase");
}

void
OmDatabase::operator=(const OmDatabase &other)
{
    DEBUGAPICALL("OmDatabase::operator=", "OmDatabase");
    OmLockSentry locksentry(internal->mutex);
    // pointers are reference counted.
    internal->mydb = other.internal->mydb;
}

OmDatabase::~OmDatabase()
{
    DEBUGAPICALL("OmDatabase::~OmDatabase", "");
    delete internal;
    internal = 0;
}

std::string
OmDatabase::get_description() const
{
    DEBUGAPICALL("OmDatabase::get_description", "");
    /// \todo display contents of the database
    return "OmDatabase()";
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
OmWritableDatabase::operator=(const OmDatabase &other)
{
    DEBUGAPICALL("OmWritableDatabase::operator=", "OmDatabase");
    if(other.is_writable()) {
	OmLockSentry locksentry(internal->mutex);
	// pointers are reference counted.
	internal->mydb = other.internal->mydb;
    } else {
	throw OmInvalidArgumentError("Cannot assign a readonly database to a writable database");
    }
}

void
OmWritableDatabase::operator=(const OmWritableDatabase &other)
{
    DEBUGAPICALL("OmWritableDatabase::operator=", "OmWritableDatabase");
    OmLockSentry locksentry(internal->mutex);
    // pointers are reference counted.
    internal->mydb = other.internal->mydb;
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
	database = internal->mydb.get();
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
	database = internal->mydb.get();
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
	database = internal->mydb.get();
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
	database = internal->mydb.get();
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
	database = internal->mydb.get();
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
	database = internal->mydb.get();
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
	database = internal->mydb.get();
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
	database = internal->mydb.get();
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
	database = internal->mydb.get();
    }

    database->replace_document(did, document, timeout);
}

OmDocumentContents
OmWritableDatabase::get_document(om_docid did)
{
    DEBUGAPICALL("OmWritableDatabase::get_document", did);
    // Get the pointer while locked, in case someone is assigning to it.
    IRDatabase * database;
    {
	OmLockSentry locksentry(internal->mutex);
	database = internal->mydb.get();
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

////////////////////////////////
// Methods of OmDatabaseGroup //
////////////////////////////////

OmDatabaseGroup::OmDatabaseGroup()
{
    DEBUGAPICALL("OmDatabaseGroup::OmDatabaseGroup", "");
    internal = new OmDatabaseGroup::Internal();
}

OmDatabaseGroup::~OmDatabaseGroup() {
    DEBUGAPICALL("OmDatabaseGroup::~OmDatabaseGroup", "");
    delete internal;
}

OmDatabaseGroup::OmDatabaseGroup(const OmDatabaseGroup &other)
	: internal(0)
{
    DEBUGAPICALL("OmDatabaseGroup::OmDatabaseGroup", "OmDatabaseGroup");
    OmLockSentry locksentry(other.internal->mutex);

    internal = new Internal(*other.internal);
}

void
OmDatabaseGroup::operator=(const OmDatabaseGroup &other)
{
    DEBUGAPICALL("OmDatabaseGroup::operator=", "OmDatabaseGroup");
    if(this == &other) {
	DEBUGLINE(API, "OmDatabaseGroup assigned to itself");
	return;
    }

    // we get these locks in a defined order to avoid deadlock
    // should two threads try to assign two databases to each
    // other at the same time.
    Internal * newinternal;

    {
	OmLockSentry locksentry1(std::min(internal, other.internal)->mutex);
	OmLockSentry locksentry2(std::max(internal, other.internal)->mutex);

	newinternal = new Internal(*other.internal);

	std::swap(internal, newinternal);
    }

    delete newinternal;
}

void
OmDatabaseGroup::add_database(const OmSettings &params)
{
    DEBUGAPICALL("OmDatabaseGroup::add_database", params);
    internal->add_database(params);
}

void
OmDatabaseGroup::add_database(const OmDatabase & database)
{
    DEBUGAPICALL("OmDatabaseGroup::add_database", "OmDatabase");
    OmRefCntPtr<IRDatabase> dbptr;
    {
	OmLockSentry locksentry(database.internal->mutex);
	dbptr = database.internal->mydb;
    }

    internal->add_database(dbptr);
}

std::string
OmDatabaseGroup::get_description() const
{
    DEBUGAPICALL("OmDatabaseGroup::get_description", "");
    /// \todo display the contents of the database group
    std::string description = "OmDatabaseGroup()";
    DEBUGAPIRETURN(description);
    return description;
}
