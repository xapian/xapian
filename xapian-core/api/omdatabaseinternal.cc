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

/////////////////////////////////////
// Methods of OmDatabase::Internal //
/////////////////////////////////////

OmDatabase::Internal::Internal(const OmSettings &params, bool readonly)
{
    add_database(params, readonly);
}

void
OmDatabase::Internal::add_database(const OmSettings & params, bool readonly)
{
    OmLockSentry locksentry(mutex);

    // Forget existing multidatabase
    multi_database = 0;

    // Open database (readonly) and add it to the list
    OmRefCntPtr<IRDatabase> newdb(DatabaseBuilder::create(params, readonly));
    databases.push_back(newdb);
}

void
OmDatabase::Internal::add_database(const OmSettings & params)
{
    add_database(params, true);
}

void
OmDatabase::Internal::add_database(OmRefCntPtr<IRDatabase> newdb)
{
    OmLockSentry locksentry(mutex);

    // Forget existing multidatabase
    multi_database = 0;

    // Add database to the list
    databases.push_back(newdb);
}

OmRefCntPtr<MultiDatabase>
OmDatabase::Internal::get_multi_database()
{
    OmLockSentry locksentry(mutex);

    if (databases.size() == 0) {
	throw OmInvalidArgumentError("No databases specified to search.");
    }

//    if (databases.size() == 1) return databases[0];

    if (multi_database.get() == 0) {
	multi_database = new MultiDatabase(databases);
    }

    return multi_database;
}

//////////////////////////////////////////////
// Methods of OmDatabase::InternalInterface //
//////////////////////////////////////////////

OmRefCntPtr<MultiDatabase>
OmDatabase::InternalInterface::get_multi_database(const OmDatabase &db)
{
    return db.internal->get_multi_database();
}
