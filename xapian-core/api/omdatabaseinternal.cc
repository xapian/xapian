/* omdatabaseinternal.cc
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

#include "config.h"
#include "utils.h"
#include "omlocks.h"
#include "omdatabaseinternal.h"
#include "omdatabaseinterface.h"
#include "multi_database.h"

#include "omdebug.h"
#include <om/omoutput.h>
#include <vector>

OmDatabase::Internal::Internal(const OmSettings &params, bool readonly)
{
    // Open database
    mydb = DatabaseBuilder::create(params, readonly);
}

//////////////////////////////////////////
// Methods of OmDatabaseGroup::Internal //
//////////////////////////////////////////

void
OmDatabaseGroup::Internal::add_database(const OmSettings & params)
{
    OmLockSentry locksentry(mutex);

    // Forget existing multidatabase
    multi_database = 0;

    // Open database (readonly) and add it to the list
    OmRefCntPtr<IRDatabase> newdb(DatabaseBuilder::create(params, true));
    databases.push_back(newdb);
}

void
OmDatabaseGroup::Internal::add_database(OmRefCntPtr<IRDatabase> newdb)
{
    OmLockSentry locksentry(mutex);

    // Forget existing multidatabase
    multi_database = 0;

    // Add database to the list
    databases.push_back(newdb);
}

OmRefCntPtr<MultiDatabase>
OmDatabaseGroup::Internal::get_multidatabase()
{
    OmLockSentry locksentry(mutex);

    if (multi_database.get() == 0) {
	multi_database = new MultiDatabase(databases);
    }

    return multi_database;
}

///////////////////////////////////////////////////
// Methods of OmDatabaseGroup::InternalInterface //
///////////////////////////////////////////////////

OmRefCntPtr<MultiDatabase>
OmDatabaseGroup::InternalInterface::get_multidatabase(
						const OmDatabaseGroup &dbg)
{
    return dbg.internal->get_multidatabase();
}
