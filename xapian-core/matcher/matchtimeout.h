/** @file
 * @brief Time limits for the matcher
 */
/* Copyright (C) 2013,2014,2015,2016,2017 Olly Betts
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

#ifndef XAPIAN_INCLUDED_MATCHTIMEOUT_H
#define XAPIAN_INCLUDED_MATCHTIMEOUT_H

#ifndef PACKAGE
# error config.h must be included first in each C++ source file
#endif

#ifdef HAVE_TIMER_CREATE
#include "realtime.h"

#include <signal.h>
#include <time.h>
#include "safeunistd.h" // For _POSIX_* feature test macros.

extern "C" {

static void
set_timeout_flag(union sigval sv)
{
    *(reinterpret_cast<volatile bool*>(sv.sival_ptr)) = true;
}

}

// The monotonic clock is the better basis for timeouts, but not always
// available.

#ifndef _POSIX_MONOTONIC_CLOCK
const clockid_t TIMEOUT_CLOCK = CLOCK_REALTIME;
#elif defined __sun
// Solaris defines CLOCK_MONOTONIC, but "man timer_create" doesn't mention it
// and when using it timer_create() fails with "EPERM" (perhaps you need to be
// root to use it?  I can't test that).
//
// Solaris defines _POSIX_MONOTONIC_CLOCK so we need to special case.
const clockid_t TIMEOUT_CLOCK = CLOCK_REALTIME;
#elif defined __CYGWIN__
// https://cygwin.com/cygwin-api/std-notes.html currently (2016-05-13) says:
//
//     clock_nanosleep currently supports only CLOCK_REALTIME and
//     CLOCK_MONOTONIC.  clock_setres, clock_settime, and timer_create
//     currently support only CLOCK_REALTIME.
//
// So CLOCK_MONOTONIC is defined, but not supported by timer_create().
const clockid_t TIMEOUT_CLOCK = CLOCK_REALTIME;
#else
const clockid_t TIMEOUT_CLOCK = CLOCK_MONOTONIC;
#endif

class TimeOut {
    struct sigevent sev;
    timer_t timerid;
    volatile bool expired;

    TimeOut(const TimeOut&) = delete;

    TimeOut& operator=(const TimeOut&) = delete;

  public:
    explicit TimeOut(double limit) : expired(false) {
	if (limit > 0) {
	    sev.sigev_notify = SIGEV_THREAD;
	    sev.sigev_notify_function = set_timeout_flag;
	    sev.sigev_notify_attributes = NULL;
	    sev.sigev_value.sival_ptr =
		static_cast<void*>(const_cast<bool*>(&expired));
	    if (usual(timer_create(TIMEOUT_CLOCK, &sev, &timerid) == 0)) {
		struct itimerspec interval;
		interval.it_interval.tv_sec = 0;
		interval.it_interval.tv_nsec = 0;
		RealTime::to_timespec(limit, &interval.it_value);
		if (usual(timer_settime(timerid, 0, &interval, NULL) == 0)) {
		    // Timeout successfully set.
		    return;
		}
		timer_delete(timerid);
	    }
	}
	sev.sigev_notify = SIGEV_NONE;
    }

    ~TimeOut() {
	if (sev.sigev_notify != SIGEV_NONE) {
	    timer_delete(timerid);
	    sev.sigev_notify = SIGEV_NONE;
	}
    }

    bool timed_out() const { return expired; }
};
#else
class TimeOut {
  public:
    explicit TimeOut(double) { }
    bool timed_out() const { return false; }
};
#endif

#endif // XAPIAN_INCLUDED_MATCHTIMEOUT_H
