/** @file backendmanager_local.h
 * @brief BackendManager subclass for local databases.
 */
/* Copyright (C) 2007,2008,2009 Olly Betts
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

#ifndef XAPIAN_INCLUDED_BACKENDMANAGER_LOCAL_H
#define XAPIAN_INCLUDED_BACKENDMANAGER_LOCAL_H

#include <xapian.h>

#if defined XAPIAN_HAS_CHERT_BACKEND
# include "backendmanager_chert.h"
# define BackendManagerLocal BackendManagerChert
#elif defined XAPIAN_HAS_BRASS_BACKEND
# include "backendmanager_brass.h"
# define BackendManagerLocal BackendManagerBrass
#elif defined XAPIAN_HAS_FLINT_BACKEND
# include "backendmanager_flint.h"
# define BackendManagerLocal BackendManagerFlint
#else
# include "backendmanager.h"
class BackendManagerLocal : public BackendManager {
    BackendManagerLocal() {
	SKIP_TEST("No local database backend enabled");
    }
};
#endif

#endif // XAPIAN_INCLUDED_BACKENDMANAGER_LOCAL_H
