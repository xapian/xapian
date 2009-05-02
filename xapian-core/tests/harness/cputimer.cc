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
#else
# include <time.h>
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

    t += r.ru_utime.tv_sec + r.ru_stime.tv_sec;
    t += (r.ru_utime.tv_usec + r.ru_stime.tv_usec) * 0.000001;

    if (getrusage(RUSAGE_CHILDREN, &r) == -1) {
	FAIL_TEST(string("Couldn't measure CPU for kids: ") + strerror(errno));
    }

    t += r.ru_utime.tv_sec + r.ru_stime.tv_sec;
    t += (r.ru_utime.tv_usec + r.ru_stime.tv_usec) * 0.000001;
#elif defined HAVE_TIMES
    struct tms b;
    if (times(&b) == (clock_t)-1) {
	FAIL_TEST(string("Couldn't measure CPU: ") + strerror(errno));
    }
    t = (double)(b.tms_utime + b.tms_stime + b.tms_cutime + b.tms_cstime);
# ifdef HAVE_SYSCONF
    t /= sysconf(_SC_CLK_TCK);
# else
    t /= CLK_TCK;
# endif
#else 
    t = time(NULL);
#endif

    return t;
}
