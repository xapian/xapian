/* omdebug.h: Provide debugging message facilities
 *
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002 Ananova Ltd
 * Copyright 2003,2004,2005,2006,2007 Olly Betts
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

// Note: we use macros to define our assertions, since with a template
// approach the location strings typically don't get thrown away by the
// compiler.

#ifndef OM_HGUARD_OMDEBUG_H
#define OM_HGUARD_OMDEBUG_H

#ifdef XAPIAN_DEBUG_VERBOSE

#include <xapian/visibility.h>

#include "omstringstream.h"

#include <vector>

#ifdef __WIN32__
# include "safewindows.h"
# define getpid() GetCurrentProcessId()
#else
# include <unistd.h>
#endif

/** The types of debug output.  These are specified within a DEBUGMSG in
 *  the code by the final portion of the name: ie, UNKNOWN, LOCK, etc...
 */
enum om_debug_types {
    /** A debug message of unknown type: probably just not been classified
     *  yet.
     */
    OM_DEBUG_UNKNOWN,

    /// @internal Was OM_DEBUG_LOCK - insert DUMMY to preserve numbers for now
    OM_DEBUG_DUMMY,

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

    /** A debug message in the indexing system
     */
    OM_DEBUG_INDEXER,

    /** Type for messages from methods performing introspection (eg,
     *  get_description()).
     */
    OM_DEBUG_INTRO,

    /** Type for messages to do with the remote backend. */
    OM_DEBUG_REMOTE,

    /** Type for messages from the lemon generated QueryParser code. */
    OM_DEBUG_QUERYPARSER,

    /** A value equal to the number of known types.
     */
    OM_DEBUG_NUMTYPES
};

/** Class to manage verbose debugging output
 */
class XAPIAN_VISIBILITY_DEFAULT OmDebug {
    private:
	/// Copying not allowed
	OmDebug(OmDebug &);

	/// Assignment not allowed
	void operator=(OmDebug &);

	/// Whether the object has been initialised
	bool initialised;

	/** Bitmap of types of log message to output.
	 *
	 *  If a bit is unset, messages of the corresponding type will be
	 *  suppressed.
	 */
	unsigned int wanted_types;

	/// file descriptor to send debug output to.
	int fd;

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

XAPIAN_VISIBILITY_DEFAULT
extern OmDebug om_debug;

/** Display a debugging message, if it is of a desired type. */
// Don't bracket b, because it may have <<'s in it
#define DEBUGMSG2(a,b) do { \
    if (om_debug.want_type(a)) { \
	om_ostringstream os; \
	os << b; \
	om_debug.display_message(a, os.str()); \
    } \
} while (0)

#define DEBUGLINE2(a,b) DEBUGMSG2(a, "Xapian " << getpid() << ": " << b << '\n')

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
	}

	/** Optionally called to specify a return value. */
        void setreturnval(std::string returnval_) { returnval = returnval_; }

	/** Destructor: displays message indicating that method has returned */
        ~OmDebugCall() {
            DEBUGLINE2(type, methodname << "() returning " << returnval);
        }
};

// 2nd level of stringize definition not needed for the use we put this
// to in this file (since we always use it within a macro here) but
// is required in general  (#N doesn't work outside a macro definition)
#ifndef STRINGIZE
# define STRINGIZE(N) _STRINGIZE(N)
# define _STRINGIZE(N) #N
#endif

/** Display a message indicating that a method has been called, and another
 *  message when the method ends.
 */
#define DEBUGCALL(t,r,a,b) \
    std::string omdebugapicall_str; \
    std::string omdebugapicall_method; \
    typedef r omdebugapicallreturn_t; \
    if (om_debug.want_type(OM_DEBUG_##t)) { \
	om_ostringstream os1; \
	os1 << "[" << static_cast<const void*>(this) << "] " << STRINGIZE(r) << " " << a; \
	omdebugapicall_method = os1.str(); \
	om_ostringstream os2; \
	os2 << b; \
	omdebugapicall_str = os2.str(); \
    } \
    OmDebugCall omdebugapicall(OM_DEBUG_##t, omdebugapicall_method, omdebugapicall_str);

/** Equivalent of DEBUGCALL for static methods.
 */
#define DEBUGCALL_STATIC(t,r,a,b) \
    std::string omdebugapicall_str; \
    std::string omdebugapicall_method; \
    typedef r omdebugapicallreturn_t; \
    if (om_debug.want_type(OM_DEBUG_##t)) { \
	om_ostringstream os1; \
	os1 << "[static   ] " << STRINGIZE(r) << " " << a; \
	omdebugapicall_method = os1.str(); \
	om_ostringstream os2; \
	os2 << b; \
	omdebugapicall_str = os2.str(); \
    } \
    OmDebugCall omdebugapicall(OM_DEBUG_##t, omdebugapicall_method, omdebugapicall_str);

#define RETURN(A) do { \
    omdebugapicallreturn_t omdebugapicallreturn = (A); \
    om_ostringstream os; \
    os << omdebugapicallreturn; \
    omdebugapicall.setreturnval(os.str()); \
    return omdebugapicallreturn; \
} while (0)

#define DEBUGMSG(a,b) DEBUGMSG2(OM_DEBUG_##a, b)
#define DEBUGLINE(a,b) DEBUGLINE2(OM_DEBUG_##a, b)

using std::endl;

#elif defined(XAPIAN_DEBUG_PROFILE)

#define DEBUGMSG(a,b) (void)0
#define DEBUGLINE(a,b) (void)0
#define RETURN(A) return (A)

#include <sys/time.h>
#include <stdio.h>

class Xapian::Internal::Timer {
    private:
	std::string call;

	// time routine entered (to subtract from parent)
	struct timeval entry;

	// time routine started executing
	struct timeval start;

	// dead time (time spent paused in subroutines)
	struct timeval dead;

	// kids time (time spent running in subroutines)
	struct timeval kids;

	// time pause() called
	static struct timeval paused;

	// pointer to start time so resume can start the clock
	static struct timeval * pstart;

	static list<Timer *> stack;

	static int depth;

    public:
	Timer(const std::string &call_) : call(call_) {
	    stack.push_back(this);
	    depth++;
	    entry = paused;
	    pstart = &start;
	    timerclear(&dead);
	    timerclear(&kids);
	}

	~Timer() {
	    gettimeofday(&paused, NULL);
	    {
		if (stack.empty()) abort();
		stack.pop_back();
		depth--;

		// running time is paused - start - dead
		int runu = paused.tv_usec - start.tv_usec - dead.tv_usec;
		int runs = paused.tv_sec - start.tv_sec - dead.tv_sec;
		runs += runu / 1000000;
		runu %= 1000000;
		if (runu < 0) {
		    runu += 1000000;
		    runs--;
		}

		if (!stack.empty()) {
		    struct timeval * k = &(stack.back()->kids);
		    k->tv_sec += runs;
		    k->tv_usec += runu;
		}

		// subtract time spent in kids
		int myu = runu - kids.tv_usec;
		int mys = runs - kids.tv_sec;
		mys += myu / 1000000;
		myu %= 1000000;
		if (myu < 0) {
		    myu += 1000000;
		    mys--;
		}
		fprintf(stderr, "% 5d.%06d % 5d.%06d %-*s%s\n", runs, runu,
			mys, myu, depth, "", call.c_str());
	    }

	    // subtract entry from start (dead time 1)
	    int usec = start.tv_usec - entry.tv_usec;
	    int sec = start.tv_sec - entry.tv_sec;
	    sec += usec / 1000000;
	    usec %= 1000000;
	    if (usec < 0) {
		usec += 1000000;
		sec--;
	    }

	    // dead time for subroutines
	    usec += dead.tv_usec;
	    sec += dead.tv_sec;

	    // subtract paused (dead time 2 part a)
	    usec -= paused.tv_usec;
	    sec -= paused.tv_sec;

	    pstart = NULL;
	    struct timeval * d = NULL;
	    if (!stack.empty()) {
		d = &(stack.back()->dead);
		d->tv_sec += sec;
		d->tv_usec += usec;
	    }
	    gettimeofday(&paused, NULL);
	    if (d) {
		d->tv_sec += paused.tv_sec;
		d->tv_usec += paused.tv_usec;
	    }
	}

	static void pause() {
	    gettimeofday(&paused, NULL);
	}

	static void resume() {
	    if (pstart == NULL) abort();
	    gettimeofday(pstart, NULL);
	}
};

/** Display a message indicating that a method has been called, and another
 *  message when the method ends.
 */
#define DEBUGCALL(t,r,a,b) \
    Xapian::Internal::Timer::pause(); \
    Xapian::Internal::Timer om_time_call(a); \
    Xapian::Internal::Timer::resume();

#define DEBUGCALL_STATIC(t,r,a,b) \
    Xapian::Internal::Timer::pause(); \
    Xapian::Internal::Timer om_time_call(a); \
    Xapian::Internal::Timer::resume();

#else

#define DEBUGMSG(a,b) (void)0
#define DEBUGLINE(a,b) (void)0
#define RETURN(A) return (A)

#define DEBUGCALL(r,t,a,b) (void)0
#define DEBUGCALL_STATIC(r,t,a,b) (void)0
#endif /* XAPIAN_DEBUG_VERBOSE */

#define DEBUGAPICALL(r,a,b) DEBUGCALL(APICALL,r,a,b)
#define DEBUGAPICALL_STATIC(r,a,b) DEBUGCALL_STATIC(APICALL,r,a,b)
#define DebugMsg(a) DEBUGMSG(UNKNOWN, a)

#endif /* OM_HGUARD_OMDEBUG_H */
