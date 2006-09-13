/* utils.h: Various useful utilities
 *
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002 Ananova Ltd
 * Copyright 2003,2004,2005,2006 Olly Betts
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
#include <vector>
using std::vector;

#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#ifdef _MSC_VER
# include <direct.h>
# include <io.h>
#else
# include <unistd.h>
#endif
#include <ctype.h>

#include <fcntl.h>

#ifdef open
// On older versions of Solaris, Sun's fcntl.h pollutes the namespace by
// #define-ing open to open64 when largefile support is enabled (also creat to
// creat64, etc but that's not a problem for us).

// We can work around this, but the workaround doesn't work with Sun's C++
// compiler (at least not when open isn't #define-d) so we only enable the
// workaround if we have to...

inline int fcntl_open(const char *filename, int flags, mode_t mode) {
    return open(filename, flags, mode);
}

inline int fcntl_open(const char *filename, int flags) {
    return open(filename, flags);
}

#undef open

inline int open(const char *filename, int flags, mode_t mode) {
    return fcntl_open(filename, flags, mode);
}

inline int open(const char *filename, int flags) {
    return fcntl_open(filename, flags);
}
#endif

#ifdef _MSC_VER
#if 0
// We used to have this here "for MSVC", but it conflicts with similar
// code in io.h for MSVC7.
inline int open(const char *filename, int flags, int mode) {
    return _open(filename, flags, mode);
}

inline int open(const char *filename, int flags) {
    return _open(filename, flags);
}
#endif

#define S_ISREG(m) (((m)&_S_IFMT) == _S_IFREG)
#define S_ISDIR(m) (((m)&_S_IFMT) == _S_IFDIR)

// MSVC needs this to get SSIZE_T defined.
#include "safewindows.h"

#undef ssize_t // In case configure already defined it.
#define ssize_t SSIZE_T
#endif

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

/// Convert a double to a string
string om_tostring(double a);

/// Convert a bool to a string
string om_tostring(bool a);

/// Convert a pointer to a string
string om_tostring(const void * a);

/** Split a string into a vector of strings, using a given separator
 *  character (default space)
 */
void split_words(string text,
		 vector<string> &words,
		 char wspace = ' ');

///////////////////////////////////////////
// Mapping of types as strings to enums  //
///////////////////////////////////////////

struct StringAndValue {
    const char * name;
    int value;
};

/** Get the value associated with the given string.  If the string
 *  isn't found, the value returned is the value in the terminating
 *  object (which has a zero length string).
 *
 *  Note: this just uses a list of entries, and searches linearly
 *  through them.  Could at make this do a binary chop, but probably
 *  not worth doing so, unless list gets large.
 */
int map_string_to_value(const StringAndValue * haystack, const string & needle);

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

/// Remove a directory and contents.
void rmdir(const string &filename);

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
