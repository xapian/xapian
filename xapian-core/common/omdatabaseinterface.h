/* omdatabaseinterface.h: Extra interface to OmDatabaseGroup
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

#ifndef OM_HGUARD_OMDATABASEINTERFACE_H
#define OM_HGUARD_OMDATABASEINTERFACE_H

#include <vector>

#include <om/omenquire.h>
#include "database_builder.h"
#include "multi_database.h"

/** This class is used basically to add an interface to OmDatabaseGroup
 *  which isn't exported to the API.  Internal OM functions can get at
 *  this interface by going through this friend class.
 */
class OmDatabaseGroup::InternalInterface {
    public:
	/** Create a MultiDatabase from an OmDatabaseGroup.
	 *
	 *  @param dbg		The source OmDatabaseGroup object.
	 */
	static OmRefCntPtr<MultiDatabase>
		make_multidatabase(const OmDatabaseGroup &dbg);
};

#endif // OM_HGUARD_OMDATABASEINTERFACE_H
