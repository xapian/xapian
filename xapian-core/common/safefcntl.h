/** @file
 * @brief #include <fcntl.h>, but working around broken platforms.
 */
/* Copyright (C) 2006,2007,2012,2018 Olly Betts
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

#ifndef XAPIAN_INCLUDED_SAFEFCNTL_H
#define XAPIAN_INCLUDED_SAFEFCNTL_H

#include <fcntl.h>

#ifdef _AIX

#include <stdarg.h>

// On AIX, O_CLOEXEC may be a 64-bit constant which won't fit in "int flags".
// The solution is to call open64x() instead of open() when the flags don't fit
// in an int, which this overload achieves.

inline int open(const char *filename, int64_t flags, ...) {
    va_list ap;
    va_start(ap, flags);
    mode_t mode = 0;
    if (flags & O_CREAT) {
	mode = va_arg(ap, mode_t);
    }
    va_end(ap);
    // open64x() takes a non-const path but is not documented as modifying it.
    char* f = const_cast<char*>(filename);
    return open64x(f, flags, mode, 0);
}

#elif defined __cplusplus && defined open

// On some versions of Solaris, fcntl.h pollutes the namespace by #define-ing
// "open" to "open64" when largefile support is enabled.  This causes problems
// if you have a method called "open" (other symbols are also #define-d
// e.g. "creat" to "creat64", but only "open" is a problem for Xapian so
// that's the only one we currently fix).

inline int fcntl_open_(const char *filename, int flags, mode_t mode) {
    return open(filename, flags, mode);
}

inline int fcntl_open_(const char *filename, int flags) {
    return open(filename, flags);
}

#undef open

inline int open(const char *filename, int flags, mode_t mode) {
    return fcntl_open_(filename, flags, mode);
}

inline int open(const char *filename, int flags) {
    return fcntl_open_(filename, flags);
}

#endif

// O_BINARY is only useful for platforms like Windows which distinguish between
// text and binary files, but it's cleaner to define it to 0 here for other
// platforms so we can avoid #ifdef where we need to use it in the code.
#ifndef __WIN32__
# ifndef O_BINARY
#  define O_BINARY 0
# endif
#endif

#ifndef O_CLOEXEC
# ifdef O_NOINHERIT
#  define O_CLOEXEC O_NOINHERIT
# else
// If O_CLOEXEC isn't supported, we probably can't mark fds as close-on-exec.
#  define O_CLOEXEC 0
# endif
#endif

#endif /* XAPIAN_INCLUDED_SAFEFCNTL_H */
