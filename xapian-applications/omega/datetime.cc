/** @file
 * @brief Parse and format date/time strings
 */
/* Copyright (c) 2013,2014,2015,2016,2019 Olly Betts
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

#include "datetime.h"

#include "stdclamp.h"
#include "timegm.h"

#include <cstdlib>

using namespace std;

time_t
parse_datetime(const string & s)
{
    struct tm t;
    const char * p = s.c_str();
    char * q;
    if (s.find('T') != string::npos || s.find('-') != string::npos) {
	// E.g. "2013-01-17T09:10:55Z"
	t.tm_year = strtoul(p, &q, 10) - 1900;
	p = q;
	if (*p == '-') {
	    t.tm_mon = strtoul(p + 1, &q, 10) - 1;
	    p = q;
	} else {
	    t.tm_mon = 0;
	}
	if (*p == '-') {
	    t.tm_mday = strtoul(p + 1, &q, 10);
	    p = q;
	} else {
	    t.tm_mday = 1;
	}
	if (*p == 'T') {
	    t.tm_hour = strtoul(p + 1, &q, 10);
	    p = q;
	    if (*p == ':') {
		t.tm_min = strtoul(p + 1, &q, 10);
		p = q;
	    } else {
		t.tm_min = 0;
	    }
	    if (*p == ':') {
		t.tm_sec = strtoul(p + 1, &q, 10);
		p = q;
	    } else {
		t.tm_sec = 0;
	    }
	} else {
	    t.tm_hour = t.tm_min = t.tm_sec = 0;
	}
	if (*p == 'Z') {
	    // FIXME: always assume UTC for now...
	}
    } else {
	// As produced by LibreOffice HTML export.
	// E.g.
	// "20130117;09105500" == 2013-01-17T09:10:55
	// "20070903;200000000000" == 2007-09-03T00:02:00
	// "20070831;5100000000000" == 2007-08-31T00:51:00
	unsigned long v = strtoul(p, &q, 10);
	if (v == 0) {
	    // LibreOffice sometimes exports "0;0".  A date of "0" is
	    // clearly invalid.
	    return time_t(-1);
	}
	p = q;
	t.tm_mday = v % 100;
	v /= 100;
	t.tm_mon = v % 100 - 1;
	t.tm_year = v / 100 - 1900;
	if (*p == ';') {
	    ++p;
	    v = strtoul(p, &q, 10);
	    v /= (q - p > 10) ? 1000000000 : 100;
	    t.tm_sec = v % 100;
	    v /= 100;
	    t.tm_min = v % 100;
	    t.tm_hour = v / 100;
	} else {
	    t.tm_hour = t.tm_min = t.tm_sec = 0;
	}
    }
    t.tm_isdst = -1;

    return timegm(&t);
}

// Write exactly w chars to buffer p representing integer v.
//
// The result is left padded with zeros if v < pow(10, w - 1).
//
// If v >= pow(10, w), then the output will show v % pow(10, w) (i.e. the
// most significant digits are lost).
static void
format_int_fixed_width(char* p, int v, int w)
{
    while (--w >= 0) {
	p[w] = '0' + (v % 10);
	v /= 10;
    }
}

string
date_to_string(int year, int month, int day)
{
    year = STD_CLAMP(year, 0, 9999);
    month = STD_CLAMP(month, 1, 12);
    day = STD_CLAMP(day, 1, 31);
    char buf[8];
    format_int_fixed_width(buf, year, 4);
    format_int_fixed_width(buf + 4, month, 2);
    format_int_fixed_width(buf + 6, day, 2);
    return string(buf, 8);
}
