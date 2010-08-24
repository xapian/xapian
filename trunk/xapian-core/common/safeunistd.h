/* safeunistd.h: <unistd.h>, but with compat. and large file support for MSVC.
 *
 * Copyright (C) 2007 Olly Betts
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

#ifndef XAPIAN_INCLUDED_SAFEUNISTD_H
#define XAPIAN_INCLUDED_SAFEUNISTD_H

#ifndef _MSC_VER
# include <unistd.h>
#else

// sys/types.h has a typedef for off_t so make sure we've seen that before
// we hide it behind a #define.
# include <sys/types.h>

// MSVC doesn't even HAVE unistd.h - io.h seems the nearest equivalent.
// We also need to do some renaming of functions to get versions which
// work on large files.
# include <io.h>

# ifdef lseek
#  undef lseek
# endif

# ifdef off_t
#  undef off_t
# endif

# define lseek(FD, OFF, WHENCE) _lseeki64(FD, OFF, WHENCE)
# define off_t __int64

// process.h is needed for getpid().
# include <process.h>

#endif

#ifdef __WIN32__

inline unsigned int
sleep(unsigned int seconds)
{
    // Use our own little helper function to avoid pulling in <windows.h>.
    extern void xapian_sleep_milliseconds(unsigned int millisecs);

    // Sleep takes a time interval in milliseconds, whereas POSIX sleep takes
    // a time interval in seconds, so we need to multiply 'seconds' by 1000.
    //
    // But make sure the multiplication won't overflow!  4294967 seconds is
    // nearly 50 days, so just sleep for that long and return the number of
    // seconds left to sleep for.  The common case of sleep(CONSTANT) should
    // optimise to just xapian_sleep_milliseconds(CONSTANT).
    if (seconds > 4294967u) {
	xapian_sleep_milliseconds(4294967000u);
	return seconds - 4294967u;
    }
    xapian_sleep_milliseconds(seconds * 1000u);
    return 0;
}

#endif

#endif /* XAPIAN_INCLUDED_SAFEUNISTD_H */
