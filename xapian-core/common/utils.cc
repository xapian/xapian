/* utils.cc: Various useful utilities
 *
 * ----START-LICENCE----
 * Copyright 1999,2000 BrightStation PLC
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

#include "config.h"

/** This so we can use snprintf */
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif /* _GNU_SOURCE */

#include "utils.h"

#include <sys/stat.h>
#include <stdio.h>

#define BUFSIZE 100

// Convert a number to a string
std::string
om_tostring(int val)
{
    char buf[BUFSIZE];
    int len = snprintf(buf, BUFSIZE, "%d", val);
    if(len == -1 || len > BUFSIZE) return std::string(buf, BUFSIZE);
    return std::string(buf, len);
}

std::string
om_tostring(unsigned int val)
{
    char buf[BUFSIZE];
    int len = snprintf(buf, BUFSIZE, "%u", val);
    if(len == -1 || len > BUFSIZE) return std::string(buf, BUFSIZE);
    return std::string(buf, len);
}

std::string
om_tostring(long int val)
{
    char buf[BUFSIZE];
    int len = snprintf(buf, BUFSIZE, "%ld", val);
    if(len == -1 || len > BUFSIZE) return std::string(buf, BUFSIZE);
    return std::string(buf, len);
}

std::string
om_tostring(unsigned long int val)
{
    char buf[BUFSIZE];
    int len = snprintf(buf, BUFSIZE, "%lu", val);
    if(len == -1 || len > BUFSIZE) return std::string(buf, BUFSIZE);
    return std::string(buf, len);
}

std::string
om_tostring(double val)
{
    char buf[BUFSIZE];
    int len = snprintf(buf, BUFSIZE, "%f", val);
    if(len == -1 || len > BUFSIZE) return std::string(buf, BUFSIZE);
    return std::string(buf, len);
}

std::string
om_tostring(bool val)
{
    return val ? "true" : "false";
}

int
map_string_to_value(const StringAndValue * haystack,
		    const std::string needle)
{
    while(haystack->name[0] != '\0') {
	if(haystack->name == needle) break;
	haystack++;
    }
    return haystack->value;
}

/** Return true if the files fname exists.
 */
bool
file_exists(const std::string &fname)
{
    // create a directory for sleepy indexes if not present
    struct stat sbuf;
    int result = stat(fname.c_str(), &sbuf);
    if (result < 0) {
	return false;
    } else {
	if (!S_ISREG(sbuf.st_mode)) {
	    return false;
	}
	return true;
    }
}

/** Return true if all the files fnames exist.
 */
bool
files_exist(const std::vector<std::string> &fnames)
{
    for (std::vector<std::string>::const_iterator i = fnames.begin();
	 i != fnames.end();
	 i++) {
	if (!file_exists(*i)) return false;
    }
    return true;
}
