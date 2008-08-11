/* safefcntl.h: #include <fcntl.h>, but working around broken platforms.
 *
 * Copyright (C) 2006,2007 Olly Betts
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

#if defined __cplusplus && defined open

// On some versions of Solaris, fcntl.h pollutes the namespace by #define-ing
// "open" to "open64" when largefile support is enabled.  This causes problems
// if you have a method called "open" (other symbols are also #define-d
// e.g. "creat" to "creat64", but only "open" is a problem for Xapian so
// that's the only one we currently fix).

#ifdef _MSC_VER
// MSVC #define-s open but also defines a function called open, so just undef
// the macro.
# undef open
#else

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

#endif

// O_BINARY is only useful for platforms like Windows which distinguish between
// text and binary files, but it's cleaner to define it to 0 here for other
// platforms so we can avoid #ifdef where we need to use it in the code.
#ifndef __WIN32__
# ifndef O_BINARY
#  define O_BINARY 0
# endif
#endif

#endif /* XAPIAN_INCLUDED_SAFEFCNTL_H */
