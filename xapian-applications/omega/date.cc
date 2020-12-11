/** @file
 * @brief date range parsing routines for omega
 */
/* Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2001 James Aylett
 * Copyright 2001,2002 Ananova Ltd
 * Copyright 2002 Intercede 1749 Ltd
 * Copyright 2002,2003,2006,2014,2016,2017,2018 Olly Betts
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

#include <config.h>

#include "date.h"

#include <vector>

#include <cstdlib>
#include <ctime>
#include "parseint.h"

using namespace std;

static int
last_day(int y, int m)
{
    static const int l[13] = {
	0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
    };
    if (m != 2) return l[m];
    return 28 + (y % 4 == 0 && (y % 100 != 0 || y % 400 == 0));
}

// Write exactly w chars to buffer p representing integer v.
//
// The result is left padded with zeros if v < pow(10, w - 1).
//
// If v >= pow(10, w), then the output will show v % pow(10, w) (i.e. the
// most significant digits are lost).
static void
format_int_fixed_width(char * p, int v, int w)
{
    while (--w >= 0) {
	p[w] = '0' + (v % 10);
	v /= 10;
    }
}

static Xapian::Query
date_range_filter(int y1, int m1, int d1, int y2, int m2, int d2)
{
    if (y1 > y2 || (y1 == y2 && (m1 > m2 || (m1 == m2 && d1 > d2)))) {
	// Start is after end.
	return Xapian::Query::MatchNothing;
    }
    char buf[10];
    format_int_fixed_width(buf + 1, y1, 4);
    format_int_fixed_width(buf + 5, m1, 2);
    vector<Xapian::Query> v;

    int d_last = last_day(y1, m1);
    int d_end = d_last;
    if (d1 == 1 && m1 == 1 && y1 != y2) {
	--y1;
	goto whole_year_at_start;
    }
    if (y1 == y2 && m1 == m2 && d2 < d_last) {
	d_end = d2;
    }
    // Deal with any initial partial month
    if (d1 > 1 || d_end < d_last) {
	buf[0] = 'D';
	for ( ; d1 <= d_end; ++d1) {
	    format_int_fixed_width(buf + 7, d1, 2);
	    v.push_back(Xapian::Query(string(buf, 9)));
	}
    } else {
	buf[0] = 'M';
	v.push_back(Xapian::Query(string(buf, 7)));
    }

    if (y1 == y2 && m1 == m2) {
	return Xapian::Query(Xapian::Query::OP_OR, v.begin(), v.end());
    }

    {
	int m_last = (y1 < y2) ? 12 : m2 - 1;
	while (++m1 <= m_last) {
	    format_int_fixed_width(buf + 5, m1, 2);
	    buf[0] = 'M';
	    v.push_back(Xapian::Query(string(buf, 7)));
	}
    }

    if (y1 < y2) {
whole_year_at_start:
	while (++y1 < y2) {
	    format_int_fixed_width(buf + 1, y1, 4);
	    buf[0] = 'Y';
	    v.push_back(Xapian::Query(string(buf, 5)));
	}
	format_int_fixed_width(buf + 1, y2, 4);
	if (m2 == 12 && d2 >= 31) {
	    v.push_back(Xapian::Query(string(buf, 5)));
	    goto whole_year_at_end;
	}
	buf[0] = 'M';
	for (m1 = 1; m1 < m2; ++m1) {
	    format_int_fixed_width(buf + 5, m1, 2);
	    v.push_back(Xapian::Query(string(buf, 7)));
	}
    }

    format_int_fixed_width(buf + 5, m2, 2);

    // Deal with any final partial month
    if (d2 < last_day(y2, m2)) {
	buf[0] = 'D';
	for (d1 = 1; d1 <= d2; ++d1) {
	    format_int_fixed_width(buf + 7, d1, 2);
	    v.push_back(Xapian::Query(string(buf, 9)));
	}
    } else {
	buf[0] = 'M';
	v.push_back(Xapian::Query(string(buf, 7)));
    }

whole_year_at_end:
    return Xapian::Query(Xapian::Query::OP_OR, v.begin(), v.end());
}

static int DIGIT(char ch) { return ch - '0'; }

static int DIGIT2(const char *p) {
    return DIGIT(p[0]) * 10 + DIGIT(p[1]);
}

static int DIGIT4(const char *p) {
    return DIGIT2(p) * 100 + DIGIT2(p + 2);
}

static void
parse_date(const string & date, int *y, int *m, int *d, bool start)
{
    // Support YYYYMMDD, YYYYMM and YYYY.
    if (date.size() < 4) {
	// We default to the start of 1970 when START isn't specified, so it
	// seems logical to here do that here too.
	*y = 1970;
    } else {
	*y = DIGIT4(date.c_str());
    }
    if (date.size() < 6) {
	if (start) {
	    *m = 1;
	    *d = 1;
	} else {
	    *m = 12;
	    *d = 31;
	}
	return;
    }
    *m = DIGIT2(date.c_str() + 4);
    if (date.size() < 8) {
	if (start) {
	    *d = 1;
	} else {
	    *d = last_day(*y, *m);
	}
	return;
    }
    *d = DIGIT2(date.c_str() + 6);
}

Xapian::Query
date_range_filter(const string & date_start, const string & date_end,
		  const string & date_span)
{
    int y1, m1, d1, y2, m2, d2;
    if (!date_span.empty()) {
	unsigned int days;
	if (!parse_unsigned(date_span.c_str(), days)) {
	    throw "Datespan value must be >= 0";
	}
	time_t secs = days * (24 * 60 * 60);
	if (!date_end.empty()) {
	    parse_date(date_end, &y2, &m2, &d2, false);
	    struct tm t;
	    t.tm_year = y2 - 1900;
	    t.tm_mon = m2 - 1;
	    t.tm_mday = d2;
	    t.tm_hour = 12;
	    t.tm_min = t.tm_sec = 0;
	    t.tm_isdst = -1;
	    time_t then = mktime(&t) - secs;
	    struct tm *t2 = localtime(&then);
	    y1 = t2->tm_year + 1900;
	    m1 = t2->tm_mon + 1;
	    d1 = t2->tm_mday;
	} else if (!date_start.empty()) {
	    parse_date(date_start, &y1, &m1, &d1, true);
	    struct tm t;
	    t.tm_year = y1 - 1900;
	    t.tm_mon = m1 - 1;
	    t.tm_mday = d1;
	    t.tm_hour = 12;
	    t.tm_min = t.tm_sec = 0;
	    t.tm_isdst = -1;
	    time_t end = mktime(&t) + secs;
	    struct tm *t2 = localtime(&end);
	    y2 = t2->tm_year + 1900;
	    m2 = t2->tm_mon + 1;
	    d2 = t2->tm_mday;
	} else {
	    time_t end = time(NULL);
	    struct tm *t = localtime(&end);
	    y2 = t->tm_year + 1900;
	    m2 = t->tm_mon + 1;
	    d2 = t->tm_mday;
	    time_t then = end - secs;
	    struct tm *t2 = localtime(&then);
	    y1 = t2->tm_year + 1900;
	    m1 = t2->tm_mon + 1;
	    d1 = t2->tm_mday;
	}
    } else {
	if (date_start.empty()) {
	    y1 = 1970;
	    m1 = 1;
	    d1 = 1;
	} else {
	    parse_date(date_start, &y1, &m1, &d1, true);
	}
	if (date_end.empty()) {
	    time_t now = time(NULL);
	    struct tm *t = localtime(&now);
	    y2 = t->tm_year + 1900;
	    m2 = t->tm_mon + 1;
	    d2 = t->tm_mday;
	} else {
	    parse_date(date_end, &y2, &m2, &d2, false);
	}
    }
    return date_range_filter(y1, m1, d1, y2, m2, d2);
}
