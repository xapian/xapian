/* acconfig.h: config.h template
 *
 * ----START-LICENCE----
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002 Ananova Ltd
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 * USA
 * -----END-LICENCE-----
 */

#ifndef OM_HGUARD_CONFIG_H
#define OM_HGUARD_CONFIG_H
@TOP@

/* package name */
#undef PACKAGE

/* version */
#undef VERSION

/* Define if you want to build the (old) Muscat 3.6 backend */
#undef MUS_BUILD_BACKEND_MUSCAT36

/* Define if you want to build quartz backend */
#undef MUS_BUILD_BACKEND_QUARTZ

/* Define if you want to build inmemory backend */
#undef MUS_BUILD_BACKEND_INMEMORY

/* Define if you want to build net database backend */
#undef MUS_BUILD_BACKEND_REMOTE

/* Define if you want debugging to be enabled (will cause some slow down) */
#undef MUS_DEBUG

/* Define if you want paranoid debugging to be enabled (will cause
 *   significant slow-down) */
#undef MUS_DEBUG_PARANOID

/* Define if you want lots of debugging messages */
#undef MUS_DEBUG_VERBOSE

/* Define if <sstream> is available */
#undef HAVE_SSTREAM

/* Define if <streambuf> is available */
#undef HAVE_STREAMBUF

/* Define if socklen_t is available */
#undef HAVE_SOCKLEN_T

/* Location of platform independent support files */
#undef DATADIR

@BOTTOM@

#endif /* OM_HGUARD_CONFIG_H */
