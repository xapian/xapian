/** @file
 * @brief Portable implementation of timegm().
 */
/* Copyright (c) 2013,2024,2025 Olly Betts
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#include <config.h>
#include "timegm.h"

#include "setenv.h"
#include <time.h>

time_t
safe_mktime(struct tm* tm)
{
    time_t r = mktime(tm);
    if (r == time_t(-1) && tm->tm_year < 70 &&
	(sizeof(time_t) > 4 || tm->tm_year >= 1)) {
	/* Microsoft's mktime() treats years before 1970 as an error, unlike
	 * most other implementations.
	 *
	 * We workaround this to support older years by calling mktime() for a
	 * date offset such that it's after 1970 (but before 3000 which is the
	 * highest year Microsoft's mktime() handles, and also before 2038 for
	 * 32-bit time_t), and such that the leap year pattern matches.  For
	 * 32-bit time_t we just need to add a multiple of 4, but for 64-bit
	 * time_t we need to add a multiple of 400.
	 *
	 * We require the year to be >= 1901 for 32-bit time_t since the
	 * oldest representable date in signed 32-bit time_t is in 1901
	 * so there's no point retrying anything older:
	 *
	 *   Fri 13 Dec 1901 20:45:52 UTC
	 *
	 * For larger time_t we support any tm_year value which fits in an int.
	 * This is somewhat dubious before the adoption of the Gregorian
	 * calendar but it matches what most mktime() implementations seem to
	 * do.
	 */
	int y = tm->tm_year;
	int y_offset = sizeof(time_t) > 4 ?
	    ((-1 - y) / 400 + 1) * 400 :
	    (76 - y) & ~3;
	tm->tm_year = y + y_offset;
	r = mktime(tm);
	tm->tm_year = y;
	if (r != time_t(-1)) {
	    // The two magic numbers are the average number of seconds in a
	    // year for a 400 year cycle and for a 4 year cycle (one which
	    // includes a leap year).
	    r -= y_offset * time_t(sizeof(time_t) > 4 ? 31556952 : 31557600);
	}
    }
    return r;
}

#ifndef HAVE_TIMEGM

time_t
fallback_timegm(struct tm* tm)
{
    static bool set_tz = false;
    if (!set_tz) {
	setenv("TZ", "", 1);
	tzset();
	set_tz = true;
    }
    return safe_mktime(tm);
}

#endif
