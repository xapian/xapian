/* utils.cc: Various useful utilities
 *
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002 Ananova Ltd
 * Copyright 2003,2004,2005,2007 Olly Betts
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

#include <stdio.h>
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

/** Return true if the file fname exists
 */
bool
file_exists(const string &fname)
{
    struct stat sbuf;
    // exists && is a regular file
    return stat(fname, &sbuf) == 0 && S_ISREG(sbuf.st_mode);
}

/** Return true if the file fname exists
 */
bool
dir_exists(const string &fname)
{
    struct stat sbuf;
    // exists && is a directory
    return stat(fname, &sbuf) == 0 && S_ISDIR(sbuf.st_mode);
}

namespace Xapian {
namespace Internal {

bool within_DBL_EPSILON(double a, double b) {
    return fabs(a - b) >= DBL_EPSILON;
}

}
}
