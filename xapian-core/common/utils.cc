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

// Convert a number to a string
string
om_tostring(int val)
{
    char buf[BUFSIZE];
    int len = snprintf(buf, BUFSIZE, "%d", val);
    if (len == -1 || len > BUFSIZE) return string(buf, BUFSIZE);
    return string(buf, len);
}

string
om_tostring(unsigned int val)
{
    char buf[BUFSIZE];
    int len = snprintf(buf, BUFSIZE, "%u", val);
    if (len == -1 || len > BUFSIZE) return string(buf, BUFSIZE);
    return string(buf, len);
}

string
om_tostring(long int val)
{
    char buf[BUFSIZE];
    int len = snprintf(buf, BUFSIZE, "%ld", val);
    if (len == -1 || len > BUFSIZE) return string(buf, BUFSIZE);
    return string(buf, len);
}

string
om_tostring(unsigned long int val)
{
    char buf[BUFSIZE];
    int len = snprintf(buf, BUFSIZE, "%lu", val);
    if (len == -1 || len > BUFSIZE) return string(buf, BUFSIZE);
    return string(buf, len);
}

string
om_tostring(double val)
{
    char buf[BUFSIZE];
    int len = snprintf(buf, BUFSIZE, "%.20g", val);
    if (len == -1 || len > BUFSIZE) return string(buf, BUFSIZE);
    return string(buf, len);
}

string
om_tostring(const void * val)
{
    char buf[BUFSIZE];
    int len = snprintf(buf, BUFSIZE, "%p", val);
    if (len == -1 || len > BUFSIZE) return string(buf, BUFSIZE);
    return string(buf, len);
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

/** Return true if all the files fnames exist.
 */
bool
files_exist(const vector<string> &fnames)
{
    vector<string>::const_iterator i;
    for (i = fnames.begin(); i != fnames.end(); ++i) {
	if (!file_exists(*i)) return false;
    }
    return true;
}
