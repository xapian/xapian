/* omwritabledb.cc
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
#include "omwritabledbinternal.h"
#include "omdebug.h"
#include <om/omoutput.h>

OmDatabase::OmDatabase(const std::string & type,
		       const std::vector<std::string> & params,
		       bool readonly)
	: internal(new OmDatabase::Internal(type, params, readonly))
{
    DEBUGAPICALL("OmDatabase::OmDatabase",
		 type << ", " << "[params]" << ", " << readonly);
}

OmDatabase::OmDatabase(const std::string & type,
		       const std::vector<std::string> & params)
	: internal(new OmDatabase::Internal(type, params, true))
{
    DEBUGAPICALL("OmDatabase::OmDatabase",
		 type << ", " << "[params]");
}

OmDatabase::OmDatabase(const OmDatabase &other)
	: internal(new Internal(*(other.internal)))
{
    DEBUGAPICALL("OmDatabase::OmDatabase", "OmDatabase");
}

void
OmDatabase::operator=(const OmDatabase &other)
{
    OmLockSentry locksentry(internal->mutex);
    DEBUGAPICALL("OmDatabase::operator=", "OmDatabase");
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


OmWritableDatabase::OmWritableDatabase(const std::string & type,
				       const std::vector<std::string> & params)
	: OmDatabase(type, params, false)
{
    DEBUGAPICALL("OmWritableDatabase::OmWritableDatabase",
		 type << ", [params]");
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
    internal->mutex.lock();
    IRDatabase * database = internal->mydb.get();
    internal->mutex.unlock();

    // FIXME - begin a session here
    database->lock(timeout);
}

void
OmWritableDatabase::end_session()
{
    DEBUGAPICALL("OmWritableDatabase::end_session", "");
    // Get the pointer while locked, in case someone is assigning to it.
    internal->mutex.lock();
    IRDatabase * database = internal->mydb.get();
    internal->mutex.unlock();

    database->unlock();
}

void
OmWritableDatabase::flush()
{
    throw OmUnimplementedError("OmWritableDatabase::flush() not yet implemented");
}

void
OmWritableDatabase::begin_transaction(om_timeout timeout = 0)
{
    throw OmUnimplementedError("OmWritableDatabase::begin_transaction() not yet implemented");
}

void
OmWritableDatabase::end_transaction()
{
    throw OmUnimplementedError("OmWritableDatabase::end_transaction() not yet implemented");
}


om_docid
OmWritableDatabase::add_document(const OmDocumentContents & document,
				 om_timeout timeout)
{
    DEBUGAPICALL("OmWritableDatabase::add_document", document);
    // Check the validity of the document
    OmDocumentContents::document_terms::const_iterator i;
    for(i = document.terms.begin(); i != document.terms.end(); i++) {
	if(i->second.tname.size() == 0) {
	    throw OmInvalidArgumentError(
		"Cannot add termnames of zero length to the database.");
	}
    }

    // Get the pointer while locked, in case someone is assigning to it.
    internal->mutex.lock();
    IRDatabase * database = internal->mydb.get();
    internal->mutex.unlock();

    // FIXME - have an implicit session here if not already in one,
    // and make sure we're exception safe.
    om_docid did = database->add_document(document);
    DEBUGAPIRETURN(did);
    return did;
}

void
OmWritableDatabase::delete_document(om_docid did, om_timeout timeout = 0)
{
    throw OmUnimplementedError("OmWritableDatabase::delete_document() not yet implemented");
}

void
OmWritableDatabase::replace_document(om_docid did,
				     const OmDocumentContents & document,
				     om_timeout timeout = 0)
{
    throw OmUnimplementedError("OmWritableDatabase::replace_document() not yet implemented");
}

OmDocumentContents
OmWritableDatabase::get_document(om_docid did)
{
    throw OmUnimplementedError("OmWritableDatabase::get_document() not yet implemented");
}

std::string
OmWritableDatabase::get_description() const
{
    /// \todo display contents of the writable database
    std::string description = "OmWritableDatabase()";
    DEBUGAPICALL("OmWritableDatabase::get_description", "");
    return description;
}

