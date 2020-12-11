/** @file
 *  @brief Functions for handling a time or time interval in a double.
 */
/* Copyright (C) 2010,2011,2013,2014,2015,2020 Olly Betts
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
 */

#ifndef XAPIAN_INCLUDED_REALTIME_H
#define XAPIAN_INCLUDED_REALTIME_H

#ifndef PACKAGE
# error config.h must be included first in each C++ source file
#endif

#include <cerrno>
#include <cmath>
#include <ctime>

#ifndef __WIN32__
# ifdef HAVE_FTIME
#  include <sys/timeb.h>
# endif
# ifdef HAVE_GETTIMEOFDAY
#  include <sys/time.h>
# endif
# include "safesysselect.h"
#else
# include <sys/types.h>
# include <sys/timeb.h>
extern void xapian_sleep_milliseconds(unsigned int millisecs);
#endif

namespace RealTime {

/// Return the current time.
inline double now() {
#if defined HAVE_CLOCK_GETTIME
    // We prefer functions which can return the time with a higher precision.
    // On POSIX platforms that means we prefer clock_gettime() over
    // gettimeofday() over ftime().
    //
    // (Also, POSIX.1-2008 stopped specifying ftime(), and marked gettimeofday()
    // as obsolete, recommending clock_gettime() instead.)
    //
    // Modern mingw provides clock_gettime().  For older mingw and MSVC we fall
    // back to _ftime64().
    struct timespec ts;
    if (usual(clock_gettime(CLOCK_REALTIME, &ts) == 0))
	return ts.tv_sec + (ts.tv_nsec * 1e-9);
    return double(std::time(NULL));
#elif !defined __WIN32__
# if defined HAVE_GETTIMEOFDAY
    struct timeval tv;
    if (usual(gettimeofday(&tv, NULL) == 0))
	return tv.tv_sec + (tv.tv_usec * 1e-6);
    return double(std::time(NULL));
# elif defined HAVE_FTIME
    struct timeb tp;
#  ifdef FTIME_RETURNS_VOID
    ftime(&tp);
#  else
    if (rare(ftime(&tp) != 0))
	return double(std::time(NULL));
#  endif
    return tp.time + (tp.millitm * 1e-3);
# else
    return double(std::time(NULL));
# endif
#else
    // For __WIN32__.
    struct __timeb64 tp;
    _ftime64(&tp);
    return tp.time + tp.millitm * 1e-3;
#endif
}

/** Return the end time for a timeout in @a timeout seconds.
 *
 *  If @a timeout is 0, that means "no timeout", so 0 is returned.  Otherwise
 *  the current time plus @a timeout seconds is returned.
 */
inline double end_time(double timeout) {
    return (timeout == 0.0 ? timeout : timeout + now());
}

#if defined HAVE_NANOSLEEP || defined HAVE_TIMER_CREATE
/// Fill in struct timespec from number of seconds in a double.
inline void to_timespec(double t, struct timespec *ts) {
    double secs;
    ts->tv_nsec = long(std::modf(t, &secs) * 1e9);
    ts->tv_sec = long(secs);
}
#endif

/// Fill in struct timeval from number of seconds in a double.
#ifndef __WIN32__
inline void to_timeval(double t, struct timeval *tv) {
    double secs;
    tv->tv_usec = long(std::modf(t, &secs) * 1e6);
    tv->tv_sec = long(secs);
}
#else
// Use a macro to avoid having to pull in winsock2.h just for struct timeval.
# define to_timeval(T, TV) to_timeval_((T), (TV)->tv_sec, (TV)->tv_usec)

inline void to_timeval_(double t, long & tv_sec, long & tv_usec) {
    double secs;
    tv_usec = long(std::modf(t, &secs) * 1e6);
    tv_sec = long(secs);
}
#endif

/// Sleep until the time represented by this object.
inline void sleep(double t) {
#ifdef HAVE_NANOSLEEP
    // Available on modern POSIX systems and under modern mingw.
    double delta = t - RealTime::now();
    if (delta <= 0.0)
	return;
    struct timespec ts;
    to_timespec(delta, &ts);
    while (nanosleep(&ts, &ts) < 0 && errno == EINTR) { }
#elif !defined __WIN32__
    double delta;
    struct timeval tv;
    do {
	delta = t - RealTime::now();
	if (delta <= 0.0)
	    return;
	to_timeval(delta, &tv);
    } while (select(0, NULL, NULL, NULL, &tv) < 0 &&
	     (errno == EINTR || errno == EAGAIN));
#else
    double delta = t - RealTime::now();
    if (delta <= 0.0)
	return;
    while (rare(delta > 4294967.0)) {
	xapian_sleep_milliseconds(4294967000u);
	delta -= 4294967.0;
    }
    xapian_sleep_milliseconds(unsigned(delta * 1000.0));
#endif
}

}

#endif // XAPIAN_INCLUDED_REALTIME_H
