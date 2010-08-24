/** @file cputimer.cc
 * @brief Measure CPU time.
 */
/* Copyright (C) 2009 Olly Betts
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
 */

#include <config.h>

#include "cputimer.h"

#include "testsuite.h"

#include "safeerrno.h"

#ifdef HAVE_GETRUSAGE
# include <sys/time.h>
# include <sys/resource.h>
#elif defined HAVE_TIMES
# include <sys/times.h>
# ifdef HAVE_SYSCONF
#  include "safeunistd.h"
# endif
#elif defined HAVE_FTIME
# include <sys/timeb.h>
#else
# include <ctime>
#endif

#include <cstring>
#include <string>

using namespace std;

double
CPUTimer::get_current_cputime() const
{
    double t = 0;
#ifdef HAVE_GETRUSAGE
    struct rusage r;
    if (getrusage(RUSAGE_SELF, &r) == -1) {
	FAIL_TEST(string("Couldn't measure CPU for self: ") + strerror(errno));
    }

    t = r.ru_utime.tv_sec + r.ru_stime.tv_sec;
    t += (r.ru_utime.tv_usec + r.ru_stime.tv_usec) * 0.000001;
#elif defined HAVE_TIMES
    struct tms b;
    if (times(&b) == (clock_t)-1) {
	FAIL_TEST(string("Couldn't measure CPU: ") + strerror(errno));
    }
    t = (double)(b.tms_utime + b.tms_stime);
# ifdef HAVE_SYSCONF
    t /= sysconf(_SC_CLK_TCK);
# else
    t /= CLK_TCK;
# endif
#else
    // FIXME: Fallback to just using wallclock time, which is probably only
    // going to be used on Microsoft Windows, where nobody has implemented
    // the code required to get the CPU time used by a process.
# ifdef HAVE_FTIME 
    struct timeb tb;
#  ifdef FTIME_RETURNS_VOID
    ftime(&tb);
    t = tb.time + (tb.millitm * 0.001);
#  else
    if (ftime(&tb) == -1) {
	t = time(NULL);
    } else {
	t = tb.time + (tb.millitm * 0.001);
    }
#  endif
# else
    t = time(NULL);
# endif
#endif

    return t;
}
