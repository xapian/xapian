/* database_builder.h
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

#ifndef _database_builder_h_
#define _database_builder_h_

#include <string>
#include <vector>
#include "omtypes.h"

class IRDatabase;

class DatabaseBuilderParams {
    public:
	DatabaseBuilderParams(om_database_type type_new = OM_DBTYPE_NULL,
			      bool readonly_new = true,
			      IRDatabase * root_new = NULL)
		: type(type_new),
		  readonly(readonly_new),
		  root(root_new)
	{ return; }

	om_database_type type;
	bool readonly;
	IRDatabase * root;

	vector<string> paths;
	vector<DatabaseBuilderParams> subdbs;
};

class DatabaseBuilder {
    public:
	static IRDatabase * create(const DatabaseBuilderParams &);
};

#endif /* _database_builder_h_ */
