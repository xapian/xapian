/* omindexer.cc
 *
 * ----START-LICENCE----
 * Copyright 1999,2000 Dialog Corporation
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

#include "omassert.h"
#include "om/omerror.h"
#include "om/omindexer.h"
#include "omwritabledbinternal.h"

OmWritableDatabase::OmWritableDatabase(const string & type,
				       const vector<string> & params)
	: internal(new OmWritableDatabase::Internal(type, params))
{
}

OmWritableDatabase::~OmWritableDatabase()
{
    delete internal;
}

OmWritableDatabase::OmWritableDatabase(const OmWritableDatabase &other)
{
    internal = new Internal(*(other.internal));
}

void
OmWritableDatabase::operator=(const OmWritableDatabase &other)
{
    OmLockSentry locksentry(internal->mutex);
    // pointers are reference counted.
    internal->mydb = other.internal->mydb;
}

om_docid
OmWritableDatabase::add_document(const OmDocumentContents & document)
{
    // Get the pointer while locked, in case someone is assigning to it.
    internal->mutex.lock();
    IRDatabase * database = internal->mydb.get();
    internal->mutex.unlock();
 
    database->add_document(document);
}
