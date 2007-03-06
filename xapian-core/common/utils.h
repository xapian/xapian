/* utils.h: Various useful utilities
 *
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002 Ananova Ltd
 * Copyright 2003,2004,2005,2006,2007 Olly Betts
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

#include <string>
using std::string;

#include <stdlib.h>
#include <sys/types.h>
#include "safesysstat.h"
#include "safefcntl.h"
#include "safeunistd.h"

/// Convert a string to a string!
inline string om_tostring(const string &s) { return s; }

/// Convert an integer to a string
string om_tostring(int a);

/// Convert an unsigned integer to a string
string om_tostring(unsigned int a);

/// Convert a long integer to a string
string om_tostring(long int a);

/// Convert an unsigned long integer to a string
string om_tostring(unsigned long int a);

#ifdef __WIN32__
/// Convert a 64 bit integer to a string
string om_tostring(__int64 a);
#endif

/// Convert a double to a string
string om_tostring(double a);

/// Convert a bool to a string
string om_tostring(bool a);

/// Convert a pointer to a string
string om_tostring(const void * a);

/** Return true if the file fname exists.
 */
bool file_exists(const string &fname);

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
#ifdef __WIN32__
inline int mkdir(const string &filename, int /*mode*/) {
    return _mkdir(filename.c_str());
}
#else
inline int mkdir(const string &filename, mode_t mode) {
    return mkdir(filename.c_str(), mode);
}
#endif

/// Allow stat to work directly on C++ strings.
inline int stat(const string &filename, struct stat *buf) {
    return stat(filename.c_str(), buf);
}

/// Touch a file.
inline void touch(const string &filename) {
   int fd = open(filename.c_str(), O_CREAT|O_WRONLY, 0644);
   if (fd >= 0) close(fd);
}

#ifdef __WIN32__
inline unsigned int sleep(unsigned int secs) {
    _sleep(secs * 1000);
    return 0;
}
#endif

// Like C's isXXXXX() but:
//  (a) always work in the C locale
//  (b) handle signed char as well as unsigned char
//  (c) have a suitable signature for use as predicates with find_if()
//  (d) add negated versions isnotXXXXX() which are useful as predicates
//  (e) add some extra categories we find useful

namespace Xapian {
    namespace Internal {
	const unsigned char IS_DIGIT = 0x01;
	const unsigned char IS_LOWER = 0x02;
	const unsigned char IS_UPPER = 0x04;
	const unsigned char IS_HEX   = 0x08;
	const unsigned char IS_SIGN  = 0x10;
	const unsigned char IS_SPACE = 0x20;
	extern const unsigned char is_tab[];
	extern const unsigned char lo_tab[];
	extern const unsigned char up_tab[];
    }
}

// Add explicit conversion to bool to prevent compiler warning from "aCC +w":
// Warning (suggestion) 818: [...] # Type `int' is larger than type `bool',
// truncation in value may result.

inline bool C_isdigit(char ch) {
    using namespace Xapian::Internal;
    return bool(is_tab[static_cast<unsigned char>(ch)] & IS_DIGIT);
}

inline bool C_isxdigit(char ch) {
    using namespace Xapian::Internal;
    return bool(is_tab[static_cast<unsigned char>(ch)] & IS_HEX);
}

inline bool C_isupper(char ch) {
    using namespace Xapian::Internal;
    return bool(is_tab[static_cast<unsigned char>(ch)] & IS_UPPER);
}

inline bool C_islower(char ch) {
    using namespace Xapian::Internal;
    return bool(is_tab[static_cast<unsigned char>(ch)] & IS_LOWER);
}

inline bool C_isalpha(char ch) {
    using namespace Xapian::Internal;
    return bool(is_tab[static_cast<unsigned char>(ch)] & (IS_UPPER|IS_LOWER));
}

inline bool C_isalnum(char ch) {
    using namespace Xapian::Internal;
    return bool(is_tab[static_cast<unsigned char>(ch)] & (IS_UPPER|IS_LOWER|IS_DIGIT));
}

inline bool C_isspace(char ch) {
    using namespace Xapian::Internal;
    return bool(is_tab[static_cast<unsigned char>(ch)] & IS_SPACE);
}

inline bool C_issign(char ch) {
    using namespace Xapian::Internal;
    return bool(is_tab[static_cast<unsigned char>(ch)] & IS_SIGN);
}

inline bool C_isupdig(char ch) {
    using namespace Xapian::Internal;
    return bool(is_tab[static_cast<unsigned char>(ch)] & (IS_UPPER|IS_DIGIT));
}

inline bool C_isnotdigit(char ch) { return !C_isdigit(ch); }
inline bool C_isnotxdigit(char ch) { return !C_isxdigit(ch); }
inline bool C_isnotupper(char ch) { return !C_isupper(ch); }
inline bool C_isnotlower(char ch) { return !C_islower(ch); }
inline bool C_isnotalpha(char ch) { return !C_isalpha(ch); }
inline bool C_isnotalnum(char ch) { return !C_isalnum(ch); }
inline bool C_isnotspace(char ch) { return !C_isspace(ch); }
inline bool C_isnotsign(char ch) { return !C_issign(ch); }

inline char C_tolower(char ch) {
    using namespace Xapian::Internal;
    return lo_tab[static_cast<unsigned char>(ch)];
}

inline char C_toupper(char ch) {
    using namespace Xapian::Internal;
    return up_tab[static_cast<unsigned char>(ch)];
}

#endif /* OM_HGUARD_UTILS_H */
