/* omdebug.cc: Debugging class
 *
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002 Ananova Ltd
 * Copyright 2003,2004,2005,2006,2008 Olly Betts
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

#include <config.h>

#ifdef XAPIAN_DEBUG_PROFILE

#include "omdebug.h"

struct timeval Xapian::Internal::Timer::paused;

struct timeval * Xapian::Internal::Timer::pstart = NULL;

std::list<Xapian::Internal::Timer *> Xapian::Internal::Timer::stack;

int Xapian::Internal::Timer::depth = 0;

#endif
