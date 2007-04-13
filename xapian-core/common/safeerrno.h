/* safeerrno.h: #include <errno.h>, but working around broken platforms.
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

#ifndef XAPIAN_INCLUDED_SAFEERRNO_H
#define XAPIAN_INCLUDED_SAFEERRNO_H

#ifndef PACKAGE
# error You must #include <config.h> before #include "safeerrno.h"
#endif

// Compaq's C++ compiler requires sys/errno.h to be included, followed by
// errno.h, otherwise you don't get EINTR or most of the other EXXX codes
// defined.
#if defined __DECCXX && defined HAVE_SYS_ERRNO_H
# include <sys/errno.h>
#endif
#include <errno.h>

#endif // XAPIAN_INCLUDED_SAFEERRNO_H
