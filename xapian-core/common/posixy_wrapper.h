/** @file posixy_wrapper.h
 * @brief Provides wrappers with POSIXy semantics.
 */
/* Copyright 2007 Lemur Consulting Ltd
 * Copyright 2007,2012,2014 Olly Betts
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

#ifndef XAPIAN_INCLUDED_POSIXY_WRAPPER_H
#define XAPIAN_INCLUDED_POSIXY_WRAPPER_H

#ifdef __WIN32__
/** Version of unlink() with POSIX-like semantics (open files can be unlinked).
 *
 *  NB The file must have been opened with posixy_open() for this to work.
 */
int posixy_unlink(const char * filename);

/** Version of open() which allows the file to be unlinked while open. */
int posixy_open(const char *filename, int flags);

inline int
posixy_open(const char *filename, int flags, int)
{
    // mode is ignored.
    return posixy_open(filename, flags);
}

/** Version of rename() which overwrites an existing destination file. */
int posixy_rename(const char *from, const char *to);
#else
# include <cstdio>
# include "safeunistd.h"
# include <sys/types.h>
# include "safesysstat.h"
# include "safefcntl.h"
# define posixy_unlink(F) unlink(F)
# define posixy_open ::open
# define posixy_rename(F, T) std::rename(F, T)
#endif

#endif /* XAPIAN_INCLUDED_POSIXY_WRAPPER_H */
