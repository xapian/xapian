/* database_builder.cc: Builder for creating databases
 *
 * ----START-LICENCE----
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002 Ananova Ltd
 * Copyright 2002 Olly Betts
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

#include <config.h>
#include "database_builder.h"

// Include headers for all the database types
#ifdef MUS_BUILD_BACKEND_MUSCAT36
#include "muscat36/da_database.h"
#include "muscat36/db_database.h"
#endif
#ifdef MUS_BUILD_BACKEND_INMEMORY
#include "inmemory/inmemory_database.h"
#endif
#ifdef MUS_BUILD_BACKEND_QUARTZ
#include "quartz/quartz_database.h"
#endif
#ifdef MUS_BUILD_BACKEND_REMOTE
// net_database.h is in common/
#include "net_database.h"
#endif
#include "database.h"

#include <stdio.h>

using std::string;

/** Type of a database */
enum om_database_type {
    DBTYPE_NULL,
    DBTYPE_AUTO, // autodetect database type
    DBTYPE_MUSCAT36_DA,
    DBTYPE_MUSCAT36_DB,
    DBTYPE_INMEMORY,
    DBTYPE_REMOTE,
    DBTYPE_QUARTZ
};

// Translation of types as strings to types as enum om_database_type

/** The mapping from database type names to database type codes.
 *  This list must be in alphabetic order. */
static const StringAndValue database_strings[] = {
    { "auto",			DBTYPE_AUTO		},
    { "da",			DBTYPE_MUSCAT36_DA	},
    { "db",			DBTYPE_MUSCAT36_DB	},
    { "inmemory",		DBTYPE_INMEMORY		},
    { "remote",			DBTYPE_REMOTE		},
    { "quartz",			DBTYPE_QUARTZ		},
    { "",			DBTYPE_NULL		}  // End
};

static string
read_file(const string path)
{
    FILE *stubfd = fopen(path.c_str(), "r");
    if (stubfd == 0) {
	throw OmOpeningError("Can't open stub database file: " + path);
    }

    struct stat st;
    if (fstat(fileno(stubfd), &st) != 0 || !S_ISREG(st.st_mode)) {
	throw OmOpeningError("Can't get size of stub database file: " + path);
    }
    char *buf = (char*)malloc(st.st_size);
    if (buf == 0) { 
	throw OmOpeningError("Can't allocate space to read stub database file: "
			     + path);
    }

    int bytes = fread(buf, 1, st.st_size, stubfd);
    if (bytes != st.st_size) {
	throw OmOpeningError("Can't read stub database file: " + path);
    }
    string result = string(buf, st.st_size);
    free(buf);
    return result;
}

static void
read_stub_database(OmSettings & params, bool readonly)
{
    // Check validity of parameters
    string buf = read_file(params.get("auto_dir"));

    string::size_type linestart = 0;
    string::size_type lineend = 0;
    int linenum = 0;
    while (linestart != buf.size()) {
	lineend = buf.find_first_of("\n\r", linestart);
	if (lineend == string::npos) lineend = buf.size();
	linenum++;

	string::size_type eqpos;
	eqpos = buf.find('=', linestart);
	if (eqpos >= lineend) {
	    throw OmOpeningError("Invalid entry in stub database file, at line " + om_tostring(linenum));
	}
	params.set(buf.substr(linestart, eqpos - linestart),
		   buf.substr(eqpos + 1, lineend - eqpos - 1));
	linestart = buf.find_first_not_of("\n\r", lineend);
	if (linestart == string::npos) linestart = buf.size();
    }
}

RefCntPtr<Database>
DatabaseBuilder::create(const OmSettings & params, bool readonly)
{
    RefCntPtr<Database> database;

    // Convert type into an om_database_type
    om_database_type dbtype = static_cast<om_database_type> (
	map_string_to_value(database_strings, params.get("backend")));

    // Copy params so we can modify them if we have a stub database, or an
    // auto database.
    OmSettings myparams = params;

    // Check for an auto database which is pointing at a stub database.
    if (dbtype == DBTYPE_AUTO) {
	string path = myparams.get("auto_dir");

	// Check for path actually being a file - if so, assume it to be
	// a stub database.
	if (file_exists(path)) {
	    read_stub_database(myparams, readonly);
	}

	// Recalculate the database type: it may well have changed.
	dbtype = static_cast<om_database_type> (
		map_string_to_value(database_strings, myparams.get("backend")));
    }

    // Create database of correct type, and open it
    switch (dbtype) {
	case DBTYPE_NULL:
	    throw OmInvalidArgumentError("Unknown database type `" + 
					 myparams.get("backend") + "'");
	    break;
	case DBTYPE_AUTO: {
	    // Check validity of parameters
	    string path = myparams.get("auto_dir");

#ifdef MUS_BUILD_BACKEND_MUSCAT36
	    if (file_exists(path + "/R") && file_exists(path + "/T")) {
		// can't easily tell flimsy from heavyduty so assume hd
		myparams.set("m36_record_file", path + "/R");
		myparams.set("m36_term_file", path + "/T");
		myparams.set("m36_heavyduty", true);
                if (file_exists(path + "/keyfile"))
		    myparams.set("m36_key_file", path + "/keyfile");
		database = new DADatabase(myparams, readonly);
                break;
            }
	    if (file_exists(path + "/DB")) {
		myparams.set("m36_db_file", path + "/DB");
		// can't easily tell flimsy from heavyduty so assume hd
		myparams.set("m36_heavyduty", true);
                if (file_exists(path + "/keyfile"))
		    myparams.set("m36_key_file", path + "/keyfile");
		database = new DBDatabase(myparams, readonly);
                break;
            }
	    if (file_exists(path + "/DB.da")) {
		myparams.set("m36_db_file", path + "/DB.da");
		// can't easily tell flimsy from heavyduty so assume hd
		myparams.set("m36_heavyduty", true);
                if (file_exists(path + "/keyfile"))
		    myparams.set("m36_key_file", path + "/keyfile");
		database = new DBDatabase(myparams, readonly);
                break;
            }
#endif
#ifdef MUS_BUILD_BACKEND_QUARTZ
	    // FIXME: Quartz has lots of files, and the names may change
	    // during development.  Make sure this stays up to date.

	    if (file_exists(path + "/record_DB")) {
		myparams.set("quartz_dir", path);
		if (readonly) {
		    database = new QuartzDatabase(myparams);
		} else {
		    database = new QuartzWritableDatabase(myparams);
		}
		break;
	    }
#endif
	    // OK, we didn't detect a known database.  If we're constructing
	    // a writable database, this may mean it doesn't exist, so try
	    // defaulting to a backend which supports writing and is actually
	    // built in...
	    if (!readonly) {
#ifdef MUS_BUILD_BACKEND_QUARTZ
		myparams.set("quartz_dir", path);
		database = new QuartzWritableDatabase(myparams);
		break;
#endif
	    }
            break;
        }
	case DBTYPE_MUSCAT36_DA:
#ifdef MUS_BUILD_BACKEND_MUSCAT36
	    database = new DADatabase(myparams, readonly);
#endif
	    break;
	case DBTYPE_MUSCAT36_DB:
#ifdef MUS_BUILD_BACKEND_MUSCAT36
	    database = new DBDatabase(myparams, readonly);
#endif
	    break;
	case DBTYPE_INMEMORY:
#ifdef MUS_BUILD_BACKEND_INMEMORY
	    database = new InMemoryDatabase(myparams, readonly);
#endif
	    break;
	case DBTYPE_QUARTZ:
#ifdef MUS_BUILD_BACKEND_QUARTZ
	    if (readonly) {
		database = new QuartzDatabase(myparams);
	    } else {
		database = new QuartzWritableDatabase(myparams);
	    }
#endif
	    break;
	case DBTYPE_REMOTE:
#ifdef MUS_BUILD_BACKEND_REMOTE
	    database = new NetworkDatabase(myparams, readonly);
#endif
	    break;
	default:
	    throw OmInvalidArgumentError("Unknown database type");
    }

    // Check that we have a database
    if (database.get() == NULL) {
	throw OmFeatureUnavailableError("Couldn't create database: support "
					"for specified database type not "
					"available.");
    }

    return database;
}
