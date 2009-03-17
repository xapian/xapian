/* utils.cc: Various useful utilities
 *
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002 Ananova Ltd
 * Copyright 2003,2004,2005,2007,2008 Olly Betts
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

#include "xapian/error.h"
#include "safedirent.h"
#include "safeerrno.h"

#include <cstdio> // For sprintf()/snprintf().
#include <sys/types.h>
#include <cfloat>
#include <cmath>

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

#ifdef __WIN32__
string
om_tostring(__int64 val)
{
    // Avoid a format string warning from GCC - mingw uses the MS C runtime DLL
    // which does understand "%I64d", but GCC doesn't know that.
    static const char fmt[] = { '%', 'I', '6', '4', 'd', 0 };
    CONVERT_TO_STRING(fmt)
}
#endif

string
om_tostring(unsigned long long int val)
{
    CONVERT_TO_STRING("%llu")
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
    return val ? "1" : "0";
}

bool
file_exists(const string &fname)
{
    struct stat sbuf;
    // exists && is a regular file
    return stat(fname, &sbuf) == 0 && S_ISREG(sbuf.st_mode);
}

bool
dir_exists(const string &fname)
{
    struct stat sbuf;
    // exists && is a directory
    return stat(fname, &sbuf) == 0 && S_ISDIR(sbuf.st_mode);
}

class dircloser {
    DIR * dir;
  public:
    dircloser(DIR * dir_) : dir(dir_) {}
    ~dircloser() {
	if (dir != NULL) {
	    closedir(dir);
	    dir = NULL;
	}
    }
};

void
removedir(const string &dirname)
{
    DIR * dir;

    dir = opendir(dirname.c_str());
    if (dir == NULL) {
	if (errno == ENOENT) return;
	throw Xapian::DatabaseError("Cannot open directory '" + dirname + "'", errno);
    }

    {
	dircloser dc(dir);
	while (true) {
	    errno = 0;
	    struct dirent * entry = readdir(dir);
	    if (entry == NULL) {
		if (errno == 0)
		    break;
		throw Xapian::DatabaseError("Cannot read entry from directory at '" + dirname + "'", errno);
	    }
	    string name(entry->d_name);
	    if (name == "." || name == "..")
		continue;
	    if (unlink(dirname + "/" + name)) {
		throw Xapian::DatabaseError("Cannot remove file '" + string(entry->d_name) + "'", errno);
	    }
	}
    }
    if (rmdir(dirname)) {
	throw Xapian::DatabaseError("Cannot remove directory '" + dirname + "'", errno);
    }
}

namespace Xapian {
namespace Internal {

bool within_DBL_EPSILON(double a, double b) {
    return fabs(a - b) >= DBL_EPSILON;
}

}
}
