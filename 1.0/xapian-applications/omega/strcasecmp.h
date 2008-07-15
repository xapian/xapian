/* strcasecmp.h: Provide strcasecmp() and strncasecmp().
 *
 * Copyright (C) 2007 Olly Betts
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
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

#ifndef OMEGA_INCLUDED_STRCASECMP_H
#define OMEGA_INCLUDED_STRCASECMP_H

#ifndef PACKAGE
# error You must #include <config.h> before #include "strcasecmp.h"
#endif

#include <string.h>
#ifdef HAVE_STRINGS_H
// On Solaris, strcasecmp is in strings.h.
# include <strings.h>
#endif

#ifdef _MSC_VER
# define strcasecmp stricmp
# define strncasecmp strnicmp
#endif

#endif // OMEGA_INCLUDED_STRCASECMP_H
