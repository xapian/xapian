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

OmDatabase::OmDatabase(const string & type,
		       const vector<string> & params,
		       bool readonly)
	: internal(new OmDatabase::Internal(type, params, readonly))
{
}

OmDatabase::OmDatabase(const string & type,
		       const vector<string> & params)
	: internal(new OmDatabase::Internal(type, params, true))
{
}

OmDatabase::OmDatabase(const OmDatabase &other)
	: internal(new Internal(*(other.internal)))
{
}

void
OmDatabase::operator=(const OmDatabase &other)
{
    OmLockSentry locksentry(internal->mutex);
    // pointers are reference counted.
    internal->mydb = other.internal->mydb;
}

OmDatabase::~OmDatabase()
{
    delete internal;
    internal = 0;
}

string
OmDatabase::get_description() const
{
    /// \todo display contents of the database
    return "OmDatabase()";
}


OmWritableDatabase::OmWritableDatabase(const string & type,
				       const vector<string> & params)
	: OmDatabase(type, params, false)
{
}

OmWritableDatabase::OmWritableDatabase(const OmWritableDatabase &other)
	: OmDatabase(other)
{
}

void
OmWritableDatabase::operator=(const OmDatabase &other)
{
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
    OmLockSentry locksentry(internal->mutex);
    // pointers are reference counted.
    internal->mydb = other.internal->mydb;
}

OmWritableDatabase::~OmWritableDatabase()
{
    delete internal;
    internal = 0;
}

void
OmWritableDatabase::lock(om_timeout timeout)
{
    // Get the pointer while locked, in case someone is assigning to it.
    internal->mutex.lock();
    IRDatabase * database = internal->mydb.get();
    internal->mutex.unlock();

    database->lock(timeout);
}

void
OmWritableDatabase::unlock()
{
    // Get the pointer while locked, in case someone is assigning to it.
    internal->mutex.lock();
    IRDatabase * database = internal->mydb.get();
    internal->mutex.unlock();

    database->unlock();
}

om_docid
OmWritableDatabase::add_document(const OmDocumentContents & document)
{
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
 
    return database->add_document(document);
}

string
OmWritableDatabase::get_description() const
{
    /// \todo display contents of the writable database
    return "OmWritableDatabase()";
}

