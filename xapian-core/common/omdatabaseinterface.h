/* omdatabaseinterface.h: Extra interface to OmDatabase
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

#ifndef OM_HGUARD_OMDATABASEINTERFACE_H
#define OM_HGUARD_OMDATABASEINTERFACE_H

#include <om/omdatabase.h>

/** This class is used basically to add an interface to OmDatabase
 *  which isn't exported to the API.  Internal OM classes can get at
 *  this interface by going through this friend class, without us having
 *  to expose the names of such classes in friend class declarations in
 *  the externally visible omdatabase.h header.
 */
class OmDatabase::InternalInterface {
    public:
	/** Return a pointer to an OmDatabase's Internals.
	 *
	 *  @param db		The source OmDatabase object.
	 *
	 *  @return  A pointer to the internals.
	 */
	static OmDatabase::Internal *get(const OmDatabase &db) {
	    return db.internal;
	}
};

#endif // OM_HGUARD_OMDATABASEINTERFACE_H
