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

#include "utils.h"

#include <sys/stat.h>

// Convert a number to a string
#include <strstream.h>
string
om_inttostring(int a)
{   
    // Use ostrstream (because ostringstream often doesn't exist)
    char buf[100];  // Very big (though we're also bounds checked)
    ostrstream ost(buf, 100);
    ost << a << ends;
    return string(buf);
}

string
doubletostring(double a)
{   
    // Use ostrstream (because ostringstream often doesn't exist)
    char buf[100];  // Very big (though we're also bounds checked)
    ostrstream ost(buf, 100);
    ost << a << ends;
    return string(buf);
}

int
map_string_to_value(const StringAndValue * haystack,
		    const string needle)
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
file_exists(const string &fname)
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
files_exist(const vector<string> &fnames)
{
    for (vector<string>::const_iterator i = fnames.begin();
	 i != fnames.end();
	 i++) {
	if (!file_exists(*i)) return false;
    }
    return true;
}
