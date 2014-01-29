/* safesysstat.h: #include <sys/stat.h>, but enabling large file support.
 *
 * Copyright (C) 2007,2012 Olly Betts
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

#ifndef XAPIAN_INCLUDED_SAFESYSSTAT_H
#define XAPIAN_INCLUDED_SAFESYSSTAT_H

#include <sys/stat.h>

// For most platforms, AC_SYS_LARGEFILE enables support for large files at
// configure time, but MSVC doesn't use configure so we have to put the
// magic somewhere else - i.e. here!

#ifdef _MSC_VER
// MSVC needs to call _stati64() instead of stat() and the struct which holds
// the information is "struct _stati64" instead of "struct stat" so we just
// use #define to replace both in one go.  We also want to use _fstati64()
// instead of fstat() but in this case we can use a function-like macro.
//
// This hack is a problem is we ever want a method called "stat", or one called
// fstat which takes 2 parameters, but we can probably live with these
// limitations.

#ifdef stat
# undef stat
#endif

#ifdef fstat
# undef fstat
#endif

// NB: _stati64 not _stat64 (the latter just returns a 64 bit timestamp).
#define stat _stati64
#define fstat(FD, BUF) _fstati64(FD,BUF)

#endif

#ifdef __WIN32__

// MSVC lacks these POSIX macros and other compilers may too:
#ifndef S_ISDIR
# define S_ISDIR(ST_MODE) (((ST_MODE) & _S_IFMT) == _S_IFDIR)
#endif
#ifndef S_ISREG
# define S_ISREG(ST_MODE) (((ST_MODE) & _S_IFMT) == _S_IFREG)
#endif

// On UNIX, mkdir() is prototyped in <sys/stat.h> but on Windows it's in
// <direct.h>, so just include that from here to avoid build failures on
// MSVC just because of some new use of mkdir().  This also reduces the
// number of conditionalised #include statements we need in the sources.
#include <direct.h>

// Add overloaded version of mkdir which takes an (ignored) mode argument
// to allow source code to just specify a mode argument unconditionally.
//
// The () around mkdir are in case it's defined as a macro.
inline int (mkdir)(const char *pathname, mode_t /*mode*/) {
    return _mkdir(pathname);
}

#else

// These were specified by POSIX.1-1996, so most platforms should have
// these by now:
#ifndef S_ISDIR
# define S_ISDIR(ST_MODE) (((ST_MODE) & S_IFMT) == S_IFDIR)
#endif
#ifndef S_ISREG
# define S_ISREG(ST_MODE) (((ST_MODE) & S_IFMT) == S_IFREG)
#endif

#endif

#endif /* XAPIAN_INCLUDED_SAFESYSSTAT_H */
