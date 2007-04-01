/* omdebug.cc: Debugging class
 *
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002 Ananova Ltd
 * Copyright 2003,2004,2005,2006 Olly Betts
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301
 * USA
 */

#include <config.h>

#ifdef XAPIAN_DEBUG_VERBOSE

#include "omdebug.h"
#include "utils.h"

OmDebug om_debug;

#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "safefcntl.h"
#include "safeerrno.h"

#include <string>

using namespace std;

#define ENV_DEBUG_LOG   "XAPIAN_DEBUG_LOG"
#define ENV_DEBUG_FLAGS "XAPIAN_DEBUG_FLAGS"

OmDebug::OmDebug() : initialised(false), wanted_types(0), fd(2)
{
    // Can't do much in this constructor, because on Solaris the contents get
    // wiped just before the start of main().
}

OmDebug::~OmDebug()
{
    display_message(OM_DEBUG_UNKNOWN, "Xapian debug version, closing down\n");
    initialised = false;
}

void
OmDebug::initialise()
{
    if (!initialised) {
	initialised = true;
	// We get this as soon as we can - possible race condition exists here
	// if the initialise() method is not explicitly called.
	const char * typestring = getenv(ENV_DEBUG_FLAGS);
	if (typestring) wanted_types = atoi(typestring);

	const char * filename = getenv(ENV_DEBUG_LOG);
	if (filename) {
	    string s(filename);
	    string::size_type token = s.find("%%");
	    if (token != string::npos) {
		s.replace(token, 2, om_tostring(getpid()));
	    }

	    // mingw doesn't support O_SYNC, and it's not vital - it just
	    // ensures that debug output is written to disk, so that none
	    // is lost if we crash...
#ifdef O_SYNC
	    fd = open(s.c_str(), O_CREAT | O_WRONLY | O_SYNC | O_APPEND, 0644);
#else
	    fd = open(s.c_str(), O_CREAT | O_WRONLY | O_APPEND, 0644);
#endif

	    if (fd == -1) {
		fd = 2;
		display_message(OM_DEBUG_UNKNOWN, "Can't open requested debug log `" + s + "' - using stderr.\n");
	    }
	}

	display_message(OM_DEBUG_UNKNOWN, "Xapian debug build initialised\n");
    }
}

bool
OmDebug::want_type(enum om_debug_types type)
{
    initialise();
    return (wanted_types >> type) & 1;
}

void
OmDebug::display_message(enum om_debug_types type, string msg)
{
    if (!want_type(type)) return;
    string line(om_tostring(int(type)));
    line += ']';
    line += msg;
    size_t written = 0;
    while (written < line.size()) {
	ssize_t last_write = write(fd, line.data(), line.size());
	if (last_write < 0) {
	    if (errno != EINTR)
		return; // Ignore errors
	    else
		last_write = 0;
	}
	written += last_write;
    }
}

#endif /* XAPIAN_DEBUG_VERBOSE */

#ifdef XAPIAN_DEBUG_PROFILE

#include "omdebug.h"

#include <sys/time.h>

struct timeval Xapian::Internal::Timer::paused;

struct timeval * Xapian::Internal::Timer::pstart = NULL;

list<Xapian::Internal::Timer *> Xapian::Internal::Timer::stack;

int Xapian::Internal::Timer::depth = 0;

#endif
