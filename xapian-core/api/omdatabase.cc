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

#include "config.h"
#include "utils.h"
#include "omlocks.h"
#include "omdatabaseinternal.h"
#include "omwritabledbinternal.h"
#include "omdatabaseinterface.h"
#include "multi_database.h"

#include "omdebug.h"
#include <om/omoutput.h>
#include <vector>

OmDatabase::Internal::Internal(const std::string & type,
			       const std::vector<std::string> & paths,
			       bool readonly)
{
    // Prepare parameters to build database with (open it writable)
    DatabaseBuilderParams params(type, readonly);
    params.paths = paths;

    // Open database
    mydb = DatabaseBuilder::create(params);
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
OmDatabaseGroup::add_database(const std::string &type,
			      const std::vector<std::string> &params)
{
    // FIXME: describe the params
    DEBUGAPICALL("OmDatabaseGroup::add_database", type << ", " << "[params]");
    internal->add_database(type, params);
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

//////////////////////////////////////////
// Methods of OmDatabaseGroup::Internal //
//////////////////////////////////////////

void
OmDatabaseGroup::Internal::add_database(const std::string & type,
					const std::vector<std::string> & paths)
{
    OmLockSentry locksentry(mutex);

    // Forget existing multidatabase
    multi_database = 0;

    // Prepare parameters to build database with (opening it readonly)
    DatabaseBuilderParams dbparam(type, true);
    dbparam.paths = paths;

    // Open database and add it to the list
    OmRefCntPtr<IRDatabase> newdb(DatabaseBuilder::create(dbparam));
    databases.push_back(newdb);
}

void
OmDatabaseGroup::Internal::add_database(OmRefCntPtr<IRDatabase> newdb)
{
    OmLockSentry locksentry(mutex);

    // Forget existing multidatabase
    multi_database = 0;

    // Add database to the list
    databases.push_back(newdb);
}

OmRefCntPtr<MultiDatabase>
OmDatabaseGroup::Internal::get_multidatabase()
{
    OmLockSentry locksentry(mutex);

    if (multi_database.get() == 0) {
	multi_database = new MultiDatabase(databases);
    }

    return multi_database;
}

///////////////////////////////////////////////////
// Methods of OmDatabaseGroup::InternalInterface //
///////////////////////////////////////////////////

OmRefCntPtr<MultiDatabase>
OmDatabaseGroup::InternalInterface::get_multidatabase(
						const OmDatabaseGroup &dbg)
{
    return dbg.internal->get_multidatabase();
}
