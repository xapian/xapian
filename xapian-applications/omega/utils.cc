/** @file
 * @brief string conversion utility functions for omega
 */
/* Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2003,2004,2006,2010,2011,2019 Olly Betts
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

#include "utils.h"

#include <cassert>
#include <stdio.h> // for sprintf/snprintf
#include <cstdlib>

#include <string>

using namespace std;

// This ought to be enough for any of the conversions below.
#define BUFSIZE 100

#ifdef SNPRINTF
#define CONVERT_TO_STRING(FMT) \
    char buf[BUFSIZE];\
    int len = SNPRINTF(buf, BUFSIZE, (FMT), val);\
    if (len == -1 || len > BUFSIZE) return string(buf, BUFSIZE);\
    return string(buf, len);
#else
#define CONVERT_TO_STRING(FMT) \
    char buf[BUFSIZE];\
    buf[BUFSIZE - 1] = '\0';\
    sprintf(buf, (FMT), val);\
    if (buf[BUFSIZE - 1]) abort(); /* Uh-oh, buffer overrun */ \
    return string(buf);
#endif

int
string_to_int(const string &s)
{
    return atoi(s.c_str());
}

string
double_to_string(double val)
{
    CONVERT_TO_STRING("%f")
}

void
trim(string & s)
{
    string::size_type first_nonspace;
    first_nonspace = s.find_first_not_of(" \t\r\n\v");
    if (first_nonspace == string::npos) {
	// String is all whitespace.
	s.resize(0);
    } else {
	// Remove any trailing whitespace.
	string::size_type len = s.find_last_not_of(" \t\r\n\v");
	assert(len != string::npos);
	if (len < s.size() - 1)
	    s.resize(len + 1);
	// Remove any leading whitespace.
	if (first_nonspace > 0)
	    s.erase(0, first_nonspace);
    }
}
