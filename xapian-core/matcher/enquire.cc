/* enquire.cc: External interface for running queries
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

#include "omerror.h"
#include "omassert.h"
#include "omenquire.h"

#include "database.h"
#include "database_builder.h"
#include "irdocument.h"

#include <vector>
#include <map>

//////////////////////////////////////////////////////////////
// Mapping of database names, as strings, to database types //
//////////////////////////////////////////////////////////////

template<class X> struct stringToType {
    string name;
    X type;
};

template<class X> class stringToTypeMap {
    public:
	static stringToType<X> types[];
	static X get_type(string needle) {
	    stringToType<X>* haystack = types;
	    while(haystack->name.size() != 0) {
		if(haystack->name == needle) break;
		haystack++;
	    }
	    return haystack->type;
	}
};

stringToType<om_database_type> stringToTypeMap<om_database_type>::types[] = {
    { "da_flimsy",		OM_DBTYPE_DA		},
    { "inmemory",		OM_DBTYPE_INMEMORY	},
    { "multidb",		OM_DBTYPE_MULTI		},
    { "sleepycat",		OM_DBTYPE_SLEEPY	},
    { "",			OM_DBTYPE_NULL		}  // End
};

//////////////////////////////////////
// Internal state of enquire object //
//////////////////////////////////////

class EnquireState {
    public:
	IRDatabase * database;

	EnquireState() : database(NULL) {}
	~EnquireState() {
	    delete database;
	}
};

//////////////////////////////
// Initialise and shut down //
//////////////////////////////

Enquire::Enquire()
{
    state = new EnquireState();
}

Enquire::~Enquire()
{
    delete state;
    state = NULL;
}

//////////////////
// Set database //
//////////////////

void
Enquire::set_database(string spec, bool readonly)
{
    DebugMsg("Database specifier: `" << spec << "'" << endl);

    // Extract the type
    string::size_type colonpos = spec.find_first_of(":");
    if(colonpos == spec.npos) throw OmError("Invalid database specifier");
    string type = spec.substr(0, colonpos);
    spec.erase(0, colonpos + 1);

    // Convert type into an om_database_type
    om_database_type dbtype = stringToTypeMap<om_database_type>::get_type(type);

    // Extract the entries into a list
    vector<string> entries;
    while((colonpos = spec.find_first_of(":")) != spec.npos) {
	entries.push_back(spec.substr(0, colonpos));
	DebugMsg("entry `" << spec.substr(0, colonpos) << "' ");
	spec.erase(0, colonpos + 1);
    }
    if(spec.size() != 0) {
	entries.push_back(spec);
	DebugMsg("entry `" << spec << "' ");
    }
    DebugMsg(entries.size() << " entries" << endl);

    // Prepare params to build database with
    DatabaseBuilderParams params(dbtype, readonly);
    params.paths = entries;

    // Use params to create database
    state->database = DatabaseBuilder::create(params);
}


