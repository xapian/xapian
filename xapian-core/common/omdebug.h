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
#include "omstringstream.h"

#ifdef MUS_DEBUG_VERBOSE
#include <iostream>
#include <fstream>
#include <memory>
#include <vector>
#include <strstream.h>

#include "omlocks.h"

/** The types of debug output.  These are specified within a DEBUGMSG in
 *  the code by the final portion of the name: ie, UNKNOWN, LOCK, etc...
 */
enum om_debug_types {
    /** A debug message of unknown type: probably just not been classified
     *  yet.
     */
    OM_DEBUG_UNKNOWN,

    /** A debug message involved with locking or unlocking something.
     */
    OM_DEBUG_LOCK,

    /** A debug message to do with a database backend.
     */
    OM_DEBUG_DB,

    /** A debug message to do with the matcher.
     */
    OM_DEBUG_MATCH,

    /** A debug message to do with the expander.
     */
    OM_DEBUG_EXPAND,

    /** A debug message to do with calculating weights.
     */
    OM_DEBUG_WTCALC,

    /** A debug message to do with some part of the API.
     */
    OM_DEBUG_API,

    /** A debug message to report the calling of an API method.
     *  (The aim is that all API methods will produce such messages.)
     */
    OM_DEBUG_APICALL,

    /** A debug message to report exceptions being called.
     */
    OM_DEBUG_EXCEPTION
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

	/// Whether the output stream has been opened.
	bool output_initialised;

	/// Open the output stream (if it hasn't already been opened)
	void open_output();

	/// Whether the list of types wanted has been initialised.
	bool types_initialised;

	/// Initialise the list of types wanted.
	void select_types();

	/// Whether the mutex has been initialised.
	bool mutex_initialised;

	/** A mutex to protect the displaying of messages.
	 *  Note that this is a pointer, not a member, because of Solaris
	 *  not initialising global statics (or, at least, clearing the
	 *  data of any global statics when main() is called).  We also
	 *  don't initialise the mutex in the constructor, for this reason.
	 */
	OmLock * mutex;

	/// Initialised the mutex
	void initialise_mutex();

	/** File to send this output to.  If this is null, it'll go to
	 *  stderr.
	 */
	auto_ptr<std::ofstream> to;
    public:
	/// Operator for outputting something
	ostream & operator << (enum om_debug_types type);

	/// Check whether a given type is wanted for output
	bool want_type(enum om_debug_types type);

	/// Get the mutex to use to protect access to the debug object.
	OmLock * get_mutex();

	/// Standard constructor
	OmDebug();

	/// Standard destructor
	~OmDebug();
};

extern OmDebug om_debug;

/** Display a debugging message, if it is of a desired type. */
// Don't bracket b, because it may have <<'s in it
#define DEBUGMSG(a,b) { \
    if(om_debug.want_type(OM_DEBUG_##a)) { \
	om_ostringstream os; \
	os << b; \
	OmLockSentry sentry(*(om_debug.get_mutex())); \
	(om_debug << OM_DEBUG_##a << os.str()).flush(); \
    } \
}

#ifdef HAVE_LIBPTHREAD
#define THREAD_INFO " (Thread " << pthread_self() << ")"
#else // HAVE_LIBPTHREAD
#define THREAD_INFO
#endif // HAVE_LIBPTHREAD

#define DEBUGLINE(a,b) DEBUGMSG(a, "Om" THREAD_INFO ": " << b << endl)

/** Class to manage printing a message at the start and end of a method call.
 */
class OmDebugApiCall {
    private:
	/** The name of the method being called. */
        string methodname;

	/** The return value. */
        string returnval;
    public:
	/** Constructor: called at the beginning of the method. */
        OmDebugApiCall(string methodname_, string params)
		: methodname(methodname_) {
	    DEBUGLINE(APICALL, "Calling " << methodname << "(" <<
		      params << ")");
	};

	/** Optionally called to specify a return value. */
        void setreturnval(string returnval_) { returnval = returnval_; }

	/** Destructor: displays message indicating that method has returned */
        ~OmDebugApiCall() {
            DEBUGLINE(APICALL, methodname << "() returning " << returnval);
        }
};

/** Display a message indicating that a method has been called, and another
 *  message when the method ends.
 */
#define DEBUGAPICALL(a,b) \
    string omdebugapicall_str; \
    {\
	om_ostringstream os; \
	os << b; \
	omdebugapicall_str = os.str(); \
    } \
    OmDebugApiCall omdebugapicall(a, omdebugapicall_str);

/** Use in conjunction with DEBUGAPICALL - specify the value that the method
 *  is going to return.
 */
#define DEBUGAPIRETURN(a) { \
    om_ostringstream os; \
    os << a; \
    omdebugapicall.setreturnval(os.str()); \
}

#else /* MUS_DEBUG_VERBOSE */
#define DEBUGMSG(a,b)
#define DEBUGAPICALL(a,b)
#define DEBUGAPIRETURN(a)
#define DEBUGLINE(a,b)
#endif /* MUS_DEBUG_VERBOSE */

#define DebugMsg(a) DEBUGMSG(UNKNOWN, a)

#endif /* OM_HGUARD_OMDEBUG_H */
