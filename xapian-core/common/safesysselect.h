/* safesysselect.h: #include <sys/select.h> with portability workarounds.
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
 */

#ifndef XAPIAN_INCLUDED_SAFESYSSELECT_H
#define XAPIAN_INCLUDED_SAFESYSSELECT_H

#ifndef PACKAGE
# error You must #include <config.h> before #include "safesysselect.h"
#endif

#ifdef HAVE_SYS_SELECT_H
// According to POSIX 1003.1-2001.
# include <sys/select.h>
#else
// According to earlier standards.
# include <sys/time.h>
# include <sys/types.h>
# include <unistd.h>
#endif

// On Solaris FDSET uses memset but fails to prototype it.
#include <cstring>

#endif // XAPIAN_INCLUDED_SAFESYSSELECT_H
