/* omdebug.h: Provide debugging message facilities
 *
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002 Ananova Ltd
 * Copyright 2003,2004,2005,2006,2007,2008 Olly Betts
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

#ifndef OM_HGUARD_OMDEBUG_H
#define OM_HGUARD_OMDEBUG_H

#include "debuglog.h"

#ifdef XAPIAN_DEBUG_LOG

#include "output.h"

/** Display a message indicating that a method has been called, and another
 *  message when the method ends.
 */
#define DEBUGCALL(t,r,a,b) LOGCALL(t,r,a,b)

/** Equivalent of DEBUGCALL for static methods. */
#define DEBUGCALL_STATIC(t,r,a,b) LOGCALL_STATIC(t,r,a,b)

#elif defined(XAPIAN_DEBUG_PROFILE)

#define LOGLINE(a,b) (void)0
#define RETURN(A) return A

#include <cstdio>  // For fprintf().
#include <cstdlib> // For abort().
#include <sys/time.h>

#include <list>
#include <string>

namespace Xapian {
namespace Internal {

class Timer {
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

	static std::list<Timer *> stack;

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
		if (stack.empty()) std::abort();
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
		std::fprintf(stderr, "% 5d.%06d % 5d.%06d %-*s%s\n",
			     runs, runu, mys, myu, depth, "", call.c_str());
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
	    if (pstart == NULL) std::abort();
	    gettimeofday(pstart, NULL);
	}
};

}
}

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

#define DEBUGCALL(r,t,a,b) (void)0
#define DEBUGCALL_STATIC(r,t,a,b) (void)0

#endif /* XAPIAN_DEBUG_LOG */

#define DEBUGAPICALL(r,a,b) DEBUGCALL(APICALL,r,a,b)
#define DEBUGAPICALL_STATIC(r,a,b) DEBUGCALL_STATIC(APICALL,r,a,b)

#endif /* OM_HGUARD_OMDEBUG_H */
