/* omdebug.cc: Debugging class
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

#ifdef MUS_DEBUG_VERBOSE

#include "omlocks.h"
#include "omdebug.h"

OmDebug om_debug;

#include <stdlib.h>
#include <string>
#include <fstream>

#define OM_ENV_DEBUG_FILE  "OM_DEBUG_FILE"
#define OM_ENV_DEBUG_TYPES "OM_DEBUG_TYPES"

OmDebug::OmDebug()
	: output_initialised(false),
	  types_initialised(false),
	  mutex_initialised(false)
{
}

OmDebug::~OmDebug()
{
    if(mutex_initialised) {
	delete mutex;
	mutex = 0;
	mutex_initialised = false;
    }
}

void
OmDebug::open_output()
{
    if (!output_initialised) {
	char * filename = getenv(OM_ENV_DEBUG_FILE);
	if (filename != 0) {
	    {
		// FIXME: have to do this to get around compiler brokenness
		// in gcc version 2.95.2
		auto_ptr<std::ofstream> temp(new std::ofstream(filename));
		to = temp;
	    }
	    if (to.get() == 0 || *to == 0) {
		cerr << "Can't open requested debug file `" <<
			string(filename) << "' using stderr." << endl << flush;
	    }
	}
	output_initialised = true;
    }
}

void
OmDebug::select_types()
{
    if (!types_initialised) {
	char * typestring = getenv(OM_ENV_DEBUG_TYPES);
	if (typestring != 0) {
	    unsigned int types = atoi(typestring);
	    while (types != 0) {
		if(types & 1) {
		    unwanted_types.push_back(false);
		} else {
		    unwanted_types.push_back(true);
		}
		types = types >> 1;
	    }
	}
	types_initialised = true;
    }
}

void
OmDebug::initialise_mutex()
{
    if (!mutex_initialised) {
	mutex = new OmLock();
	mutex_initialised = true;
    }
}

OmLock *
OmDebug::get_mutex()
{
    initialise_mutex();
    return mutex;
}

bool
OmDebug::want_type(enum om_debug_types type)
{
    open_output();
    select_types();

    if (unwanted_types.size() == 0) {
	return true;
    }
    if (unwanted_types.size() <= static_cast<unsigned int>(type) ||
	unwanted_types[type] == true) {
	return false;
    }
    return true;
}

ostream &
OmDebug::operator << (enum om_debug_types type)
{
    open_output();
    if (to.get() && *to) {
	return *to;
    }
    return cerr;
}

#endif /* MUS_DEBUG_VERBOSE */

