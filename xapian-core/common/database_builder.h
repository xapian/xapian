/* database_builder.h
 */

#ifndef _database_builder_h_
#define _database_builder_h_

#include "omtypes.h"
#include "database.h"

class DatabaseBuilder {
    public:
	IRSingleDatabase * make(om_database_type);
	IRGroupDatabase * makegroup(om_databasegroup_type);
};

#endif /* _database_builder_h_ */
