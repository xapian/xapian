/* database_builder.cc: Builder for creating databases
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
#include "database_builder.h"

// Include headers for all the database types
#ifdef MUS_BUILD_BACKEND_MUSCAT36
#include "muscat36/da_database.h"
#include "muscat36/db_database.h"
#endif
#ifdef MUS_BUILD_BACKEND_INMEMORY
#include "inmemory/inmemory_database.h"
#endif
#ifdef MUS_BUILD_BACKEND_SLEEPY
#include "sleepy/sleepy_database.h"
#endif
#ifdef MUS_BUILD_BACKEND_MULTI
// multi_database.h is in common/
#include "multi_database.h"
#endif
#ifdef MUS_BUILD_BACKEND_NET
// net_database.h is in common/
#include "net_database.h"
#endif
#include "database.h"

IRDatabase *
DatabaseBuilder::create(const DatabaseBuilderParams & params)
{
    IRDatabase * database = NULL;

    // Create database of correct type, and 
    switch(params.type) {
	case OM_DBTYPE_NULL:
	    throw OmInvalidArgumentError("Unspecified database type");
	    break;
	case OM_DBTYPE_MUSCAT36_DA_F:
#ifdef MUS_BUILD_BACKEND_MUSCAT36
	    database = new DADatabase(params, 0);
#endif
	    break;
	case OM_DBTYPE_MUSCAT36_DA_H:
#ifdef MUS_BUILD_BACKEND_MUSCAT36
	    database = new DADatabase(params, 1);
#endif
	    break;
	case OM_DBTYPE_MUSCAT36_DB_F:
#ifdef MUS_BUILD_BACKEND_MUSCAT36
	    database = new DBDatabase(params, 0);
#endif
	    break;
	case OM_DBTYPE_MUSCAT36_DB_H:
#ifdef MUS_BUILD_BACKEND_MUSCAT36
	    database = new DBDatabase(params, 1);
#endif
	    break;
	case OM_DBTYPE_INMEMORY:
#ifdef MUS_BUILD_BACKEND_INMEMORY
	    database = new InMemoryDatabase(params);
#endif
	    break;
	case OM_DBTYPE_SLEEPY:
#ifdef MUS_BUILD_BACKEND_SLEEPY
	    database = new SleepyDatabase(params);
#endif
	    break;
	case OM_DBTYPE_MULTI:
#ifdef MUS_BUILD_BACKEND_MULTI
	    database = new MultiDatabase(params);
#endif
	    break;
	case OM_DBTYPE_NET:
#ifdef MUS_BUILD_BACKEND_NET
	    database = new NetworkDatabase(params);
#endif
	    break;
	default:
	    throw OmInvalidArgumentError("Unknown database type");
    }

    // Check that we have a database
    if(database == NULL) {
	throw OmOpeningError("Couldn't create database");
    }

    return database;
}
