/* database_factory.h
 */

#ifndef _database_factory_h_
#define _database_factory_h_

#include "omtypes.h"
#include "database.h"

class DatabaseFactory {
    public:
	IRSingleDatabase * make(om_database_type);
	IRGroupDatabase * makegroup(om_databasegroup_type);
};

#endif /* _database_factory_h_ */
