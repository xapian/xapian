/* utils.cc: string utils for omega
 *
 * ----START-LICENCE----
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2003,2004 Olly Betts
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 * USA
 * -----END-LICENCE-----
 */

#include <config.h>

#include <string>
#include <vector>
#include <stdio.h>

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
int_to_string(int val)
{
    CONVERT_TO_STRING("%d")
}

vector<string>
split(const string &s, char at)
{
    size_t p = 0, q;
    vector<string> v;
    while (true) {	    
	q = s.find(at, p);
	v.push_back(s.substr(p, q - p));
	if (q == string::npos) break;
	p = q + 1;
    }
    return v;
}

vector<string>
split(const string &s, const string &at)
{
    size_t p = 0, q;
    vector<string> v;
    while (true) {	    
	q = s.find_first_of(at, p);
	v.push_back(s.substr(p, q - p));
	if (q == string::npos) break;
	p = q + 1;
    }
    return v;
}
