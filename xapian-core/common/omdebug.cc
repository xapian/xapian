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

#include "omdebug.h"

#ifdef MUS_USE_PTHREAD
#include <pthread.h>
#endif /* MUS_USE_PTHREAD */

OmDebug om_debug;

#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <fstream>

#define OM_ENV_DEBUG_FILE  "OM_DEBUG_FILE"
#define OM_ENV_DEBUG_TYPES "OM_DEBUG_TYPES"

OmDebug::OmDebug()
	: initialised(false),
	  outfile(0)
{
    // Can't do much in this constructor, because on Solaris the contents get wiped
    // just before the start of main().
}

OmDebug::~OmDebug()
{
    display_message(OM_DEBUG_UNKNOWN,
		    std::string("Om debugging version, closing down\n"));
    if(initialised) {
#ifdef MUS_USE_PTHREAD
	delete mutex;
	mutex = 0;
#endif /* MUS_USE_PTHREAD */
	initialised = false;
    }
}

void
OmDebug::open_output()
{
    char * filename = getenv(OM_ENV_DEBUG_FILE);
    if (filename != 0) {
	outfile = fopen(filename, "w");
	if (outfile == 0) {
	    fprintf(stderr, "Can't open requested debug file `%s' using stderr.\n",
		    filename);
	    fflush(stderr);
	}
    }
}

void
OmDebug::select_types()
{
    char * typestring = getenv(OM_ENV_DEBUG_TYPES);
    if (typestring != 0) {
	unsigned int types = atoi(typestring);
	for (int i=0; i<OM_DEBUG_NUMTYPES; ++i) {
	    if(types & 1) {
		unwanted_types.push_back(false);
	    } else {
		unwanted_types.push_back(true);
	    }
	    types = types >> 1;
	}
    }
}

void
OmDebug::initialise_mutex()
{
#ifdef MUS_USE_PTHREAD
    mutex = new pthread_mutex_t;
    pthread_mutex_init(mutex, 0);
#endif /* MUS_USE_PTHREAD */
}

void
OmDebug::initialise()
{
    if(!initialised) {
	initialise_mutex();
	// We get this as soon as we can - possible race condition exists here if the
	// initialise() method is not explicitly called.
#ifdef MUS_USE_PTHREAD
	pthread_mutex_lock(mutex);
#endif
	select_types();
	open_output();
	initialised = true;
#ifdef MUS_USE_PTHREAD
	pthread_mutex_unlock(mutex);
#endif

	display_message(OM_DEBUG_UNKNOWN,
			std::string("Om debugging version, initialised\n"));
    }
}

bool
OmDebug::want_type(enum om_debug_types type)
{
    initialise();

    if (unwanted_types.size() == 0) {
	return true;
    }
    if (unwanted_types.size() <= static_cast<unsigned int>(type) ||
	unwanted_types[type] == true) {
	return false;
    }
    return true;
}

void
OmDebug::display_message(enum om_debug_types type, std::string msg)
{
    initialise();
    if(!want_type(type)) return;

#ifdef MUS_USE_PTHREAD
    pthread_mutex_lock(mutex);
#endif
    if (outfile) {
	fprintf(outfile, "{%d}%s", type, msg.c_str());
	fflush(outfile);
    } else {
	fprintf(stderr, "{%d}%s", type, msg.c_str());
	fflush(stderr);
    }
#ifdef MUS_USE_PTHREAD
	pthread_mutex_unlock(mutex);
#endif
}

#endif /* MUS_DEBUG_VERBOSE */

#ifdef MUS_DEBUG_PROFILE

#include "omdebug.h"

#include <sys/time.h>

struct timeval OmTimer::paused;

struct timeval * OmTimer::pstart = NULL;

list<OmTimer *> OmTimer::stack;

int OmTimer::depth = 0;

#endif
