/* acconfig.h : config.h template
 *
 * ----START-LICENCE----
 * Copyright 1999,2000 Dialog Corporation
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

#ifndef _om_config_h_
#define _om_config_h_
@TOP@

/* Define if you want to build DA backend */
#undef MUS_BUILD_BACKEND_DA

/* Define if you want to build sleepycat backend */
#undef MUS_BUILD_BACKEND_SLEEPY

/* Define if you want to build inmemory backend */
#undef MUS_BUILD_BACKEND_INMEMORY

/* Define if you want to build multi database backend */
#undef MUS_BUILD_BACKEND_MULTI

/* Define if you want debugging to be enabled (will cause some slow down) */
#undef MUS_DEBUG

/* Define if you want paranoid debugging to be enabled (will cause
 *   significant slow-down) */
#undef MUS_DEBUG_PARANOID

/* Define if you want lots of debugging messages */
#undef MUS_DEBUG_VERBOSE

@BOTTOM@
#endif /* _om_config_h_ */
