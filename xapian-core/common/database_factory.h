/* database_factory.h
 */

#ifndef _database_factory_h_
#define _database_factory_h_

enum _om_database_type {
    OM_DBTYPE_DA,
    OM_DBTYPE_TEXTFILE,
    OM_DBTYPE_SLEEPY
};

enum _om_databasegroup_type {
    OM_DBGRPTYPE_MULTI
};

typedef enum _om_databasegroup_type om_databasegroup_type;
typedef enum _om_database_type om_database_type;

class IRDatabase;
class IRDatabaseGroup;

class DatabaseFactory {
    public:
	IRDatabase * make(om_database_type);
	IRDatabaseGroup * makegroup(om_databasegroup_type);
};

#endif /* _database_factory_h_ */
