/* database_builder.h
 *
 * ----START-LICENCE----
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
