/* msvc_posix_wrapper.h: Provides wrappers with POSIX semantics under MSVC.
 *
 * (misnamed, this isn't MSVC specific, but __WIN32__-specific)
 *
 * Copyright 2007 Lemur Consulting Ltd
 * Copyright 2007 Olly Betts
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

#ifndef XAPIAN_INCLUDED_MSVC_POSIX_WRAPPER_H
#define XAPIAN_INCLUDED_MSVC_POSIX_WRAPPER_H

/** Version of unlink() with POSIX-like semantics (open files can be unlinked).
 *
 *  NB The file must have been opened with msvc_posix_open() for this to work.
 */
int msvc_posix_unlink(const char * filename);

/** Version of open() which allows the file to be unlinked while open. */
int msvc_posix_open(const char *filename, int flags);

/** Version of rename() which overwrites an existing destination file. */
int msvc_posix_rename(const char *from, const char *to);

#endif /* XAPIAN_INCLUDED_MSVC_POSIX_WRAPPER_H */
