/* acconfig.h : config.h template
 *
 * ----START-LICENCE----
 * Copyright 1999,2000 BrightStation PLC
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

#ifndef OM_HGUARD_OM_CONFIG_H
#define OM_HGUARD_OM_CONFIG_H
@TOP@

/* package name */
#undef PACKAGE

/* version */
#undef VERSION

/* Define if you want to build the (old) Muscat 3.6 backend */
#undef MUS_BUILD_BACKEND_MUSCAT36

/* Define if you want to build sleepycat backend */
#undef MUS_BUILD_BACKEND_SLEEPY

/* Define if you want to build quartz backend */
#undef MUS_BUILD_BACKEND_QUARTZ

/* Define if you want to build inmemory backend */
#undef MUS_BUILD_BACKEND_INMEMORY

/* Define if you want to build net database backend */
#undef MUS_BUILD_BACKEND_NET

/* Define if you want debugging to be enabled (will cause some slow down) */
#undef MUS_DEBUG

/* Define if you want paranoid debugging to be enabled (will cause
 *   significant slow-down) */
#undef MUS_DEBUG_PARANOID

/* Define if you want lots of debugging messages */
#undef MUS_DEBUG_VERBOSE

/* Define if you want error-checking mutexes */
#undef MUS_MUTEX_ERRCHECK

/* Define if you want pthreads support built in */
#undef MUS_USE_PTHREAD

/* Define if getopt.h is available */
#undef HAVE_GETOPT_H

/* Define if <sstream> is available */
#undef HAVE_SSTREAM

/* Define if <streambuf> is available */
#undef HAVE_STREAMBUF

/* Define if socklen_t is available */
#undef HAVE_SOCKLEN_T

@BOTTOM@
#ifdef MUS_MUTEX_ERRCHECK
/* We need _GNU_SOURCE to get the right definitions
 * of pthread_mutexattr_settype() from pthread.h.
 * Ick.
 */
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif /* _GNU_SOURCE */
#endif /* MUS_MUTEX_ERRCHECK */

#ifndef HAVE_GETOPT_H
/* needed for Solaris for getopt to be defined in stdlib.h */
#define __EXTENSIONS__
#endif /* HAVE_GETOPT_H */

#endif /* OM_HGUARD_OM_CONFIG_H */
