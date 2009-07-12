/* omtime.h: Class representing a time to subsecond accuracy
 *
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002 Ananova Ltd
 * Copyright 2003,2005,2006 Olly Betts
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

#ifndef OM_HGUARD_OMTIME_H
#define OM_HGUARD_OMTIME_H

#ifdef HAVE_GETTIMEOFDAY
#include <sys/time.h>
#include <unistd.h>
#endif
#ifdef HAVE_FTIME
#include <sys/timeb.h>   /* time */
#endif
#include <ctime>

#include <xapian/types.h>

/// A class representing a time
class OmTime {
    public:
	/// Initialise to 0
	OmTime() : sec(0), usec(0) {}

	/// Return the current time.
	static OmTime now();

	/// Initialised
	OmTime(long int msec) : sec(msec / 1000), usec((msec % 1000) * 1000) {}
	OmTime(long int sec_, long int usec_) : sec(sec_), usec(usec_) {}

	void operator+= (Xapian::timeout msecs) {
	    usec += msecs * 1000;
	    sec += usec / 1000000;
	    usec %= 1000000;
	}

	void operator+= (const OmTime &add) {
	    usec += add.usec;
	    sec += add.sec + usec / 1000000;
	    usec %= 1000000;
	}

	OmTime operator+ (Xapian::timeout msecs) const {
	    OmTime result;
	    result.usec = usec + msecs * 1000;
	    result.sec = sec + result.usec / 1000000;
	    result.usec %= 1000000;
	    return result;
	}

	OmTime operator+ (const OmTime &add) const {
	    OmTime result;
	    result.usec = usec + add.usec;
	    result.sec = sec + add.sec + result.usec / 1000000;
	    result.usec %= 1000000;
	    return result;
	}

	OmTime operator- (const OmTime &sub) const {
	    OmTime result;
	    result.usec = usec - sub.usec;
	    result.sec = sec - sub.sec;
	    if (result.usec < 0) {
		result.usec += 1000000;
		result.sec -= 1;
	    }
	    return result;
	}

	bool operator> (const OmTime &rhs) const {
	    if (sec > rhs.sec) return true;
	    if (sec < rhs.sec) return false;
	    return (usec > rhs.usec);
	}

	bool is_set() const { return sec != 0 || usec != 0; }

	double as_double() const {
	    return double(sec) + (double(usec) / 1000000.0);
	}

	long int sec;
	long int usec;
};

inline OmTime
OmTime::now()
{
    OmTime result;
#ifdef HAVE_GETTIMEOFDAY
    struct timeval tv;
    if (gettimeofday(&tv, 0) == 0) {
	result.sec = tv.tv_sec;
	result.usec = tv.tv_usec;
	return result;
    }
#endif
#ifdef FTIME_RETURNS_VOID
    struct timeb tp;
    ftime(&tp);
    result.sec = tp.time;
    result.usec = tp.millitm * 1000;
    return result;
#else
# ifdef HAVE_FTIME
    struct timeb tp;
    if (ftime(&tp) == 0) {
	result.sec = tp.time;
	result.usec = tp.millitm * 1000;
	return result;
    }
# endif
    result.sec = time(NULL);
    result.usec = 0;
    return result;
#endif
}

#endif /* OM_HGUARD_OMTIME_H */
