/* database_builder.h
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

#ifndef OM_HGUARD_DATABASE_BUILDER_H
#define OM_HGUARD_DATABASE_BUILDER_H

#include "om/omsettings.h"
#include "om/omtypes.h"

class Database;

/** Class used to generate databases of a given type. */
class DatabaseBuilder {
    public:
	static Database * create(const OmSettings & params, bool readonly);
};

#endif /* OM_HGUARD_DATABASE_BUILDER_H */
