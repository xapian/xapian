/* date.cc: date range parsing routines for omega
 *
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2001 James Aylett
 * Copyright 2001,2002 Ananova Ltd
 * Copyright 2002 Intercede 1749 Ltd
 * Copyright 2002,2003,2006,2014 Olly Betts
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

#include <stdio.h>
#include <cstdlib>
#include <ctime>

using namespace std;

static int
last_day(int y, int m)
{
    static const int l[13] = {
	0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
    };
    if (m != 2) return l[m];
    return 28 + (y % 4 == 0); // good until 2100
}

#ifndef SNPRINTF
#include <cstdarg>

static int my_snprintf(char *str, size_t size, const char *format, ...)
{
    int res;
    va_list ap;
    va_start(ap, format);
    str[size - 1] = '\0';
    res = vsprintf(str, format, ap);
    if (str[size - 1] || res < 0 || size_t(res) >= size)
	abort(); /* Overflowed! */
    va_end(ap);
    return res;
}
#else
#define my_snprintf SNPRINTF
#endif

static Xapian::Query
date_range_filter(int y1, int m1, int d1, int y2, int m2, int d2)
{
    char buf[10];
    my_snprintf(buf, 10, "D%04d%02d", y1, m1);
    vector<Xapian::Query> v;

    int d_last = last_day(y1, m1);
    int d_end = d_last;
    if (y1 == y2 && m1 == m2 && d2 < d_last) {
	d_end = d2;
    }
    // Deal with any initial partial month
    if (d1 > 1 || d_end < d_last) {
    	for ( ; d1 <= d_end ; d1++) {
	    my_snprintf(buf + 7, 3, "%02d", d1);
	    v.push_back(Xapian::Query(buf));
	}
    } else {
	buf[0] = 'M';
	v.push_back(Xapian::Query(buf));
    }
    
    if (y1 == y2 && m1 == m2) {
	return Xapian::Query(Xapian::Query::OP_OR, v.begin(), v.end());
    }

    int m_last = (y1 < y2) ? 12 : m2 - 1;
    while (++m1 <= m_last) {
	my_snprintf(buf + 5, 5, "%02d", m1);
	buf[0] = 'M';
	v.push_back(Xapian::Query(buf));
    }
	
    if (y1 < y2) {
	while (++y1 < y2) {
	    my_snprintf(buf + 1, 9, "%04d", y1);
	    buf[0] = 'Y';
	    v.push_back(Xapian::Query(buf));
	}
	my_snprintf(buf + 1, 9, "%04d", y2);
	buf[0] = 'M';
	for (m1 = 1; m1 < m2; m1++) {
	    my_snprintf(buf + 5, 5, "%02d", m1);
	    v.push_back(Xapian::Query(buf));
	}
    }
	
    my_snprintf(buf + 5, 5, "%02d", m2);

    // Deal with any final partial month
    if (d2 < last_day(y2, m2)) {
	buf[0] = 'D';
    	for (d1 = 1 ; d1 <= d2; d1++) {
	    my_snprintf(buf + 7, 3, "%02d", d1);
	    v.push_back(Xapian::Query(buf));
	}
    } else {
	buf[0] = 'M';
	v.push_back(Xapian::Query(buf));
    }

    return Xapian::Query(Xapian::Query::OP_OR, v.begin(), v.end());
}

static void
parse_date(const string & date, int *y, int *m, int *d)
{
    // FIXME: for now only support YYYYMMDD (e.g. 20011119)
    // and don't error check
    *y = atoi(date.substr(0, 4).c_str());
    *m = atoi(date.substr(4, 2).c_str());
    *d = atoi(date.substr(6, 2).c_str());
}

Xapian::Query
date_range_filter(const string & date_start, const string & date_end,
		  const string & date_span)
{
    int y1, m1, d1, y2, m2, d2;
    if (!date_span.empty()) {
	time_t secs = atoi(date_span.c_str()) * (24 * 60 * 60);
	if (!date_end.empty()) {
	    parse_date(date_end, &y2, &m2, &d2);
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
	    parse_date(date_start, &y1, &m1, &d1);	
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
	    parse_date(date_start, &y1, &m1, &d1);	
	}
	if (date_end.empty()) {
	    time_t now = time(NULL);
	    struct tm *t = localtime(&now);
	    y2 = t->tm_year + 1900;
	    m2 = t->tm_mon + 1;
	    d2 = t->tm_mday;
	} else {
	    parse_date(date_end, &y2, &m2, &d2);
	}
    }
    return date_range_filter(y1, m1, d1, y2, m2, d2);
}
