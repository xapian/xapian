/* omdebug.h : Provide debugging message facilities
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

// Note: we use macros to define our assertions, since with a template
// approach the location strings typically don't get thrown away by the
// compiler.

#ifndef OM_HGUARD_OMDEBUG_H
#define OM_HGUARD_OMDEBUG_H

#include "config.h"
#include "omassert.h"

#ifdef MUS_DEBUG_VERBOSE
#include <iostream>
#include <fstream>
#include <memory>
#include <vector>

enum om_debug_types {
    OM_DEBUG_UNKNOWN,
    OM_DEBUG_LOCK,
    OM_DEBUG_DB,
    OM_DEBUG_WTCALC,
    OM_DEBUG_API
};

/** Class to manage verbose debugging output
 */
class OmDebug {
    private:
	/// Copying not allowed
	OmDebug(OmDebug &);

	/// Assignment not allowed
	void operator=(OmDebug &);

	/** List of types unwanted.
	 *
	 *  If an item is present and has the value of true, messages of
	 *  the corresponding type will be suppressed.  If an item is not
	 *  present, or is false, messages of the corresponding type will
	 *  be displayed.
	 */
	vector<bool> unwanted_types;

	/** File to send this output to.  If this is null, it'll go to
	 *  stderr.
	 */
	auto_ptr<std::ofstream> to;
    public:
	/// Stream
	ostream out;

	/// Check whether a given type is wanted for output
	bool want_type(enum om_debug_types type);

	/// Standard constructor
	OmDebug();

	/// Standard destructor
	~OmDebug();
};

extern OmDebug om_debug;


// Don't bracket a, because it may have <<'s in it
// Send to cout, not cerr, so that output appears on page in CGI scripts
// (otherwise it clogs up error logs)
// FIXME - should send it to a file.
#define DEBUGMSG(a,b) if(om_debug.want_type(OM_DEBUG_##a)) { om_debug.out << b ; cerr.flush(); }
#else
#define DEBUGMSG(a,b)
#endif

#define DebugMsg(a) DEBUGMSG(UNKNOWN, a)

#endif /* OM_HGUARD_OMDEBUG_H */
