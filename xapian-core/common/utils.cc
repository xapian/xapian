/* utils.cc: Various useful utilities
 *
 * ----START-LICENCE----
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002 Ananova Ltd
 * Copyright 2003 Olly Betts
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

#ifdef HAVE_SNPRINTF
/* This so we can use snprintf */
# ifndef _ISOC99_SOURCE
#  define _ISOC99_SOURCE
# endif
#endif

#include "utils.h"

#include <stdio.h>

using namespace std;

#define BUFSIZE 100

#ifdef HAVE_SNPRINTF
#define CONVERT_TO_STRING(FMT) \
    char buf[BUFSIZE];\
    int len = snprintf(buf, BUFSIZE, (FMT), val);\
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

// Convert a number to a string
string
om_tostring(int val)
{
    CONVERT_TO_STRING("%d")
}

string
om_tostring(unsigned int val)
{
    CONVERT_TO_STRING("%u")
}

string
om_tostring(long int val)
{
    CONVERT_TO_STRING("%ld")
}

string
om_tostring(unsigned long int val)
{
    CONVERT_TO_STRING("%lu")
}

string
om_tostring(double val)
{
    CONVERT_TO_STRING("%.20g")
}

string
om_tostring(const void * val)
{
    CONVERT_TO_STRING("%p")
}

string
om_tostring(bool val)
{
    return val ? "true" : "false";
}

void
split_words(string text, vector<string> &words, char ws)
{
    if (text.length() > 0 && text[0] == ws) {
	text.erase(0, text.find_first_not_of(ws));
    }
    while (text.length() > 0) {
	words.push_back(text.substr(0, text.find_first_of(ws)));
	text.erase(0, text.find_first_of(ws));
	text.erase(0, text.find_first_not_of(ws));
    }
}

int
map_string_to_value(const StringAndValue * haystack, const string & needle)
{
    while (haystack->name[0] != '\0') {
	if (haystack->name == needle) break;
	haystack++;
    }
    return haystack->value;
}

/** Return true if the file fname exists
 */
bool
file_exists(const string &fname)
{
    struct stat sbuf;
    // exists && is a regular file
    return stat(fname, &sbuf) == 0 && S_ISREG(sbuf.st_mode);
}
