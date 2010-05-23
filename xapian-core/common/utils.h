/* utils.h: Various useful utilities
 *
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002 Ananova Ltd
 * Copyright 2003,2004,2005,2006,2007,2010 Olly Betts
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

#ifndef OM_HGUARD_UTILS_H
#define OM_HGUARD_UTILS_H

#include <xapian/visibility.h>

#include <string>
using std::string;

#include <stdlib.h>
#include <sys/types.h>
#include "safesysstat.h"
#include "safeunistd.h"

/// Convert a string to a string!
inline string om_tostring(const string &s) { return s; }

/// Convert an integer to a string
XAPIAN_VISIBILITY_DEFAULT
string om_tostring(int a);

/// Convert an unsigned integer to a string
XAPIAN_VISIBILITY_DEFAULT
string om_tostring(unsigned int a);

/// Convert a long integer to a string
XAPIAN_VISIBILITY_DEFAULT
string om_tostring(long int a);

/// Convert an unsigned long integer to a string
XAPIAN_VISIBILITY_DEFAULT
string om_tostring(unsigned long int a);

#ifdef __WIN32__
/// Convert a 64 bit integer to a string
string om_tostring(__int64 a);
#endif

/// Convert a double to a string
XAPIAN_VISIBILITY_DEFAULT
string om_tostring(double a);

/// Convert a bool to a string
XAPIAN_VISIBILITY_DEFAULT
string om_tostring(bool a);

/// Convert a pointer to a string
XAPIAN_VISIBILITY_DEFAULT
string om_tostring(const void * a);

/** Return true if the file fname exists.
 */
XAPIAN_VISIBILITY_DEFAULT
bool file_exists(const string &fname);

/** Return true if the directory dirname exists.
 */
XAPIAN_VISIBILITY_DEFAULT
bool dir_exists(const string &dirname);

/// Allow atoi to work directly on C++ strings.
inline int atoi(const string &s) { return atoi(s.c_str()); }

/// Allow unlink to work directly on C++ strings.
inline int unlink(const string &filename) { return unlink(filename.c_str()); }

/// Allow system to work directly on C++ strings.
inline int system(const string &command) { return system(command.c_str()); }

#ifdef HAVE_LINK
/// Allow link to work directly on C++ strings.
inline int link(const string &o, const string &n) {
    return link(o.c_str(), n.c_str());
}
#endif

/// Allow mkdir to work directly on C++ strings.
inline int mkdir(const string &filename, mode_t mode) {
    return mkdir(filename.c_str(), mode);
}

/// Allow stat to work directly on C++ strings.
inline int stat(const string &filename, struct stat *buf) {
    return stat(filename.c_str(), buf);
}

namespace Xapian {
    namespace Internal {
	bool within_DBL_EPSILON(double a, double b);
    }
}

/// To assist backporting patches - om_tostring has been replaced with str.
template<typename T> std::string str(T t) { return om_tostring(t); }

#endif /* OM_HGUARD_UTILS_H */
