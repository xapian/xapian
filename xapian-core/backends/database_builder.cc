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
#ifdef MUS_BUILD_BACKEND_QUARTZ
#include "quartz/quartz_database.h"
#endif
#ifdef MUS_BUILD_BACKEND_NET
// net_database.h is in common/
#include "net_database.h"
#endif
#include "database.h"

/** Type of a database */
enum om_database_type {
    DBTYPE_NULL,
    DBTYPE_AUTO, // autodetect database type
    DBTYPE_MUSCAT36_DA,
    DBTYPE_MUSCAT36_DB,
    DBTYPE_INMEMORY,
    DBTYPE_NETWORK,
    DBTYPE_QUARTZ,
    DBTYPE_SLEEPYCAT
};

// Translation of types as strings to types as enum om_database_type

/** The mapping from database type names to database type codes.
 *  This list must be in alphabetic order. */
static const StringAndValue database_strings[] = {
    { "auto",			DBTYPE_AUTO		},
    { "da",			DBTYPE_MUSCAT36_DA	},
    { "db",			DBTYPE_MUSCAT36_DB	},
    { "inmemory",		DBTYPE_INMEMORY		},
    { "network",		DBTYPE_NETWORK		},
    { "quartz",			DBTYPE_QUARTZ		},
    { "sleepycat",		DBTYPE_SLEEPYCAT	},
    { "",			DBTYPE_NULL		}  // End
};

IRDatabase *
DatabaseBuilder::create(const OmSettings & params, bool readonly)
{
    IRDatabase * database = NULL;

    // Convert type into an om_database_type
    om_database_type dbtype = static_cast<om_database_type> (
	map_string_to_value(database_strings, params.get_value("backend")));

    // Create database of correct type, and open it
    switch (dbtype) {
	case DBTYPE_NULL:
	    throw OmInvalidArgumentError("Unknown database type `" + 
					 params.get_value("backend") + "'");
	    break;
	case DBTYPE_AUTO: {
	    // Check validity of parameters
	    string path = params.get_value("auto_dir");
	    OmSettings myparams = params;
#ifdef MUS_BUILD_BACKEND_MUSCAT36
	    if (file_exists(path + "/R") && file_exists(path + "/T")) {
		// can't easily tell flimsy from heavyduty so assume hd
		myparams.set_value("m36_record_file", path + "/R");
		myparams.set_value("m36_term_file", path + "/T");
		myparams.set_value("m36_heavyduty", true);
                if (file_exists(path + "/keyfile"))
		    myparams.set_value("m36_key_file", path + "/keyfile");
		database = new DADatabase(myparams, readonly);
                break;
            }
	    if (file_exists(path + "/DB")) {
		myparams.set_value("m36_db_file", path + "/DB");
		// can't easily tell flimsy from heavyduty so assume hd
		myparams.set_value("m36_heavyduty", true);
                if (file_exists(path + "/keyfile"))
		    myparams.set_value("m36_key_file", path + "/keyfile");
		database = new DBDatabase(myparams, readonly);
                break;
            }
	    if (file_exists(path + "/DB.da")) {
		myparams.set_value("m36_db_file", path + "/DB.da");
		// can't easily tell flimsy from heavyduty so assume hd
		myparams.set_value("m36_heavyduty", true);
                if (file_exists(path + "/keyfile"))
		    myparams.set_value("m36_key_file", path + "/keyfile");
		database = new DBDatabase(myparams, readonly);
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
	    myparams.set_value("sleepy_dir", path);
            database = new SleepyDatabase(myparams, readonly);
#endif
            break;
        }
	case DBTYPE_MUSCAT36_DA:
#ifdef MUS_BUILD_BACKEND_MUSCAT36
	    database = new DADatabase(params, readonly);
#endif
	    break;
	case DBTYPE_MUSCAT36_DB:
#ifdef MUS_BUILD_BACKEND_MUSCAT36
	    database = new DBDatabase(params, readonly);
#endif
	    break;
	case DBTYPE_INMEMORY:
#ifdef MUS_BUILD_BACKEND_INMEMORY
	    database = new InMemoryDatabase(params, readonly);
#endif
	    break;
	case DBTYPE_SLEEPYCAT:
#ifdef MUS_BUILD_BACKEND_SLEEPY
	    database = new SleepyDatabase(params, readonly);
#endif
	    break;
	case DBTYPE_QUARTZ:
#ifdef MUS_BUILD_BACKEND_QUARTZ
	    database = new QuartzDatabase(params, readonly);
#endif
	    break;
	case DBTYPE_NETWORK:
#ifdef MUS_BUILD_BACKEND_NET
	    database = new NetworkDatabase(params, readonly);
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
