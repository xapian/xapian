/* database_builder.cc: Builder for creating databases
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

/** Type of a database */
enum om_database_type {
    OM_DBTYPE_NULL,
    OM_DBTYPE_AUTO, // autodetect database type
    OM_DBTYPE_MUSCAT36_DA_F,
    OM_DBTYPE_MUSCAT36_DA_H,
    OM_DBTYPE_MUSCAT36_DB_F,
    OM_DBTYPE_MUSCAT36_DB_H,
    OM_DBTYPE_INMEMORY,
    OM_DBTYPE_SLEEPY,
    OM_DBTYPE_MULTI,
    OM_DBTYPE_NET
};

// Translation of types as strings to types as enum om_database_type

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
    { "auto",			OM_DBTYPE_AUTO		},
    { "",			OM_DBTYPE_NULL		}  // End
};

IRDatabase *
DatabaseBuilder::create(const DatabaseBuilderParams & params)
{
    IRDatabase * database = NULL;

    // Convert type into an om_database_type
    om_database_type dbtype =
	stringToTypeMap<om_database_type>::get_type(params.type);

    // Create database of correct type, and open it
    switch (dbtype) {
	case OM_DBTYPE_NULL:
	    throw OmInvalidArgumentError("Unspecified database type");
	    break;
	case OM_DBTYPE_AUTO:
	    // Check validity of parameters
	    if (params.paths.size() != 1) {
		throw OmInvalidArgumentError("OM_DBTYPE_AUTO requires 1 parameter.");
	    }
#ifdef MUS_BUILD_BACKEND_MUSCAT36
	    if (file_exists(params.paths[0] + "/R") &&
		file_exists(params.paths[0] + "/T")) {
		// can't easily tell flimsy from heavyduty so assume hd
		// database = new DADatabase(params, 0);
		database = new DADatabase(params, 1);
                break;
            }
	    if (file_exists(params.paths[0] + "/DB")) {
		DatabaseBuilderParams myparams = params;
		myparams.paths[0] += "/DB";
		// can't easily tell flimsy from heavyduty so assume hd
		// database = new DBDatabase(myparams, 0);
		database = new DBDatabase(myparams, 1);
                break;
            }
	    if (file_exists(params.paths[0] + "/DB.da")) {
		DatabaseBuilderParams myparams = params;
		myparams.paths[0] += "/DB.da";
		// can't easily tell flimsy from heavyduty so assume hd
		// database = new DBDatabase(myparams, 0);
		database = new DBDatabase(myparams, 1);
                break;
            }
#endif
#ifdef MUS_BUILD_BACKEND_SLEEPY
            // SleepyDatabase has lots of files so just default to it for now
//#define FILENAME_POSTLIST "postlist.db"
//#define FILENAME_TERMLIST "termlist.db"
//#define FILENAME_TERMTOID "termid.db"
//#define FILENAME_IDTOTERM "termname.db"
//#define FILENAME_DOCUMENT "document.db"
//#define FILENAME_DOCKEYDB "dockey.db"
//#define FILENAME_STATS_DB "stats.db"
            database = new SleepyDatabase(params);
#endif
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
	throw OmOpeningError("Couldn't create database: support for specified "
			     "database type not available.");
    }

    return database;
}
