/* omenquire.cc: External interface for running queries
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

#include "config.h"
#include "utils.h"
#include "omlocks.h"
#include "omdatabaseinternal.h"
#include "omwritabledbinternal.h"

#include <vector>

// Table of names of database types
stringToType<om_database_type> stringToTypeMap<om_database_type>::types[] = {
    { "da_flimsy",		OM_DBTYPE_MUSCAT36_DA_F	},
    { "da_heavy",		OM_DBTYPE_MUSCAT36_DA_H	},
    { "db_flimsy",		OM_DBTYPE_MUSCAT36_DB_F	},
    { "db_heavy",		OM_DBTYPE_MUSCAT36_DB_H	},
    { "inmemory",		OM_DBTYPE_INMEMORY	},
    { "multidb",		OM_DBTYPE_MULTI		},
    { "sleepycat",		OM_DBTYPE_SLEEPY	},
    { "net",			OM_DBTYPE_NET		},
    { "",			OM_DBTYPE_NULL		}  // End
};

OmWritableDatabase::Internal::Internal(const string & type,
				       const vector<string> & paths)
{
    // Convert type into an om_database_type
    om_database_type dbtype = OM_DBTYPE_NULL;
    dbtype = stringToTypeMap<om_database_type>::get_type(type);

    // Prepare parameters to build database with (open it writable)
    DatabaseBuilderParams params(dbtype, false);
    params.paths = paths;

    // Open database
    mydb = DatabaseBuilder::create(params);
}


////////////////////////////////
// Methods of OmDatabaseGroup //
////////////////////////////////

OmDatabaseGroup::OmDatabaseGroup() {
    internal = new OmDatabaseGroup::Internal();
}

OmDatabaseGroup::~OmDatabaseGroup() {
    delete internal;
}

OmDatabaseGroup::OmDatabaseGroup(const OmDatabaseGroup &other)
	: internal(0)
{
    OmLockSentry locksentry(other.internal->mutex);

    internal = new Internal(*other.internal);
}

void
OmDatabaseGroup::operator=(const OmDatabaseGroup &other)
{
    // we get these locks in a defined order to avoid deadlock
    // should two threads try to assign two databases to each
    // other at the same time.
    OmLockSentry locksentry1(min(internal, other.internal)->mutex);
    OmLockSentry locksentry2(max(internal, other.internal)->mutex);
    
    Internal *newinternal = new Internal(*other.internal);

    swap(internal, newinternal);

    delete newinternal;
}

void
OmDatabaseGroup::add_database(const string &type,
			      const vector<string> &params)
{
    OmLockSentry locksentry(internal->mutex);

    internal->add_database(type, params);
}


//////////////////////////////////////////
// Methods of OmDatabaseGroup::Internal //
//////////////////////////////////////////

void
OmDatabaseGroup::Internal::add_database(const string & type,
					const vector<string> & paths)
{
    // Convert type into an om_database_type
    om_database_type dbtype = OM_DBTYPE_NULL;
    dbtype = stringToTypeMap<om_database_type>::get_type(type);

    // Prepare parameters to build database with (opening it readonly)
    DatabaseBuilderParams dbparam(dbtype, true);
    dbparam.paths = paths;

    // Add dbparam to the vector of database parameters
    params.push_back(dbparam);
}

OmRefCntPtr<MultiDatabase>
OmDatabaseGroup::Internal::get_multi_database()
{
#if 0
    DatabaseBuilderParams multiparams(OM_DBTYPE_MULTI);
    multiparams.subdbs = dbdesc.internal->params;
    auto_ptr<IRDatabase> tempdb(DatabaseBuilder::create(multiparams));

    // FIXME: we probably need a better way of getting a
    // MultiDatabase to avoid the dynamic_cast.
    database = dynamic_cast<MultiDatabase *>(tempdb.get());

    if (database == 0) {
	// it wasn't really a MultiDatabase...
	Assert(false);
    } else {
	// it was good - don't let the auto_ptr delete it.
	tempdb.release();
    }

    return multidb;
#endif
}
