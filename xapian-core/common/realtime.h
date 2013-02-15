/** @file realtime.h
 *  @brief Functions for handling a time or time interval in a double.
 */
/* Copyright (C) 2010,2013 Olly Betts
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

#include <cmath>
#include <ctime>
#include "safeerrno.h"
#include "safeunistd.h"

#ifndef __WIN32__
# ifdef HAVE_FTIME
#  include <sys/timeb.h>
# endif
# ifdef HAVE_GETTIMEOFDAY
#  include <sys/time.h>
# endif
#else
# include <sys/types.h>
# include <sys/timeb.h>
extern void xapian_sleep_milliseconds(unsigned int millisecs);
#endif

namespace RealTime {

/// Return the current time.
inline double now() {
#ifndef __WIN32__
    // POSIX.1-2008 stopped specifying ftime(), so prefer gettimeofday().
# ifdef HAVE_GETTIMEOFDAY
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

/// Sleep until the time represented by this object.
inline void sleep(double t) {
#ifndef __WIN32__
    double delta;
    struct timeval tv;
    do {
	delta = t - RealTime::now();
	if (delta <= 0.0)
	    return;
	tv.tv_sec = long(delta);
	tv.tv_usec = long(std::fmod(delta, 1.0) * 1e6);
    } while (select(0, NULL, NULL, NULL, &tv) < 0 && errno == EINTR);
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
