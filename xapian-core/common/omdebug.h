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
#include <memory>
#include <vector>
#include <stdio.h>

#ifdef MUS_USE_PTHREAD
#include <pthread.h>
#endif /* MUS_USE_PTHREAD */

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
    OM_DEBUG_EXCEPTION,

    /** A debug message involved with locking or unlocking something in a database.
     */
    OM_DEBUG_DBLOCK,

    /** A value equal to the number of known types.
     */
    OM_DEBUG_NUMTYPES
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
	std::vector<bool> unwanted_types;

	/// Open the output stream
	void open_output();

	/// Initialise the list of types wanted
	void select_types();

	/// Initialised the mutex
	void initialise_mutex();

	/// Whether the object has been initialised
	bool initialised;

	/** Stream to send output to.  If this is null, it'll go to stderr.
	 */
	FILE * outfile;

#ifdef MUS_USE_PTHREAD
	/// Mutex
	pthread_mutex_t * mutex;
#endif /* MUS_USE_PTHREAD */
    public:
	/// Method for outputting something
	void display_message(enum om_debug_types type, std::string msg);

	/// Check whether a given type is wanted for output
	bool want_type(enum om_debug_types type);

	/// Explicitly cause the omdebug object to be initialsed
	void initialise();

	/// Standard constructor
	OmDebug();

	/// Standard destructor
	~OmDebug();
};

extern OmDebug om_debug;

/** Display a debugging message, if it is of a desired type. */
// Don't bracket b, because it may have <<'s in it
#define DEBUGMSG2(a,b) { \
    if(om_debug.want_type(a)) { \
	om_ostringstream os; \
	os << b; \
	om_debug.display_message(a, os.str()); \
    } \
}

#ifdef HAVE_LIBPTHREAD
#define THREAD_INFO " (Thread " << pthread_self() << ")"
#else // HAVE_LIBPTHREAD
#define THREAD_INFO
#endif // HAVE_LIBPTHREAD

#define DEBUGLINE2(a,b) DEBUGMSG2(a, "Om" THREAD_INFO ": " << b << '\n')

/** Class to manage printing a message at the start and end of a method call.
 */
class OmDebugCall {
    private:
	/** The name of the method being called. */
        std::string methodname;

	/** The return value. */
        std::string returnval;

	/** The type of message to emit. */
	enum om_debug_types type;
    public:
	/** Constructor: called at the beginning of the method. */
        OmDebugCall(enum om_debug_types type_,
		    std::string methodname_,
		    std::string params)
		: methodname(methodname_),
		  type(type_)
	{
	    DEBUGLINE2(type, methodname << "(" << params << ") called");
	};

	/** Optionally called to specify a return value. */
        void setreturnval(std::string returnval_) { returnval = returnval_; }

	/** Destructor: displays message indicating that method has returned */
        ~OmDebugCall() {
            DEBUGLINE2(type, methodname << "() returning " << returnval);
        }
};

/** Display a message indicating that a method has been called, and another
 *  message when the method ends.
 */
#define DEBUGCALL(t,a,b) \
    std::string omdebugapicall_str; \
    std::string omdebugapicall_method; \
    {\
	om_ostringstream os1; \
	os1 << "[" << ((void *)this) << "] " << a; \
	omdebugapicall_method = os1.str(); \
	om_ostringstream os2; \
	os2 << b; \
	omdebugapicall_str = os2.str(); \
    } \
    OmDebugCall omdebugapicall(OM_DEBUG_##t, omdebugapicall_method, omdebugapicall_str);

/** Equivalent of DEBUGCALL for static methods.
 */
#define DEBUGCALL_STATIC(t,a,b) \
    std::string omdebugapicall_str; \
    std::string omdebugapicall_method; \
    {\
	om_ostringstream os1; \
	os1 << "[static   ] " << a; \
	omdebugapicall_method = os1.str(); \
	om_ostringstream os2; \
	os2 << b; \
	omdebugapicall_str = os2.str(); \
    } \
    OmDebugCall omdebugapicall(OM_DEBUG_##t, omdebugapicall_method, omdebugapicall_str);


/** Use in conjunction with DEBUGCALL - specify the value that the method
 *  is going to return.
 */
#define DEBUGRETURN(a) { \
    om_ostringstream os; \
    os << a; \
    omdebugapicall.setreturnval(os.str()); \
}

#define DEBUGMSG(a,b) DEBUGMSG2(OM_DEBUG_##a, b)
#define DEBUGLINE(a,b) DEBUGLINE2(OM_DEBUG_##a, b)

using std::endl;

#else /* MUS_DEBUG_VERBOSE */
#define DEBUGMSG(a,b)
#define DEBUGLINE(a,b)
#define DEBUGCALL(t,a,b)
#define DEBUGCALL_STATIC(t,a,b)
#define DEBUGRETURN(a)
#endif /* MUS_DEBUG_VERBOSE */

#define DEBUGAPICALL(a,b) DEBUGCALL(APICALL, a, b)
#define DEBUGAPICALL_STATIC(a,b) DEBUGCALL_STATIC(APICALL, a, b)
#define DEBUGAPIRETURN(a) DEBUGRETURN(a)
#define DebugMsg(a) DEBUGMSG(UNKNOWN, a)

#endif /* OM_HGUARD_OMDEBUG_H */
