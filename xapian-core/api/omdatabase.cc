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

#include "utils.h"
#include "omlocks.h"
#include "omdatabaseinternal.h"

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
    { "",			OM_DBTYPE_NULL		}  // End
};

void
OmDatabase::Internal::add_database(const string & type,
			const vector<string> & param)
{
    // Convert type into an om_database_type
    om_database_type dbtype = OM_DBTYPE_NULL;
    dbtype = stringToTypeMap<om_database_type>::get_type(type);

    // Prepare dbparams to build database with (open it readonly)
    DatabaseBuilderParams dbparam(dbtype, true);
    dbparam.paths = param;

    // Use dbparams to create database, and add it to the list of databases
    params.push_back(dbparam);
    //add_database(params);
}

///////////////////////////
// Methods of OmDatabase //
///////////////////////////

OmDatabase::OmDatabase() {
    internal = new OmDatabase::Internal();
}

OmDatabase::~OmDatabase() {
    delete internal;
}

OmDatabase::OmDatabase(const OmDatabase &other)
	: internal(0)
{
    OmLockSentry locksentry(other.internal->mutex);

    internal = new Internal(*other.internal);
}

void OmDatabase::operator=(const OmDatabase &other)
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

void OmDatabase::add_database(const string &type,
			      const vector<string> &params)
{
    OmLockSentry locksentry(internal->mutex);

    internal->add_database(type, params);
}
