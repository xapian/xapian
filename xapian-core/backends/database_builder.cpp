/* database_builder.cc: Builder for creating databases */

#include "omassert.h"
#include "database_builder.h"

// Include headers for all the database types
#include "da/da_database.h"
#include "textfile/textfile_database.h"
#include "sleepy/sleepy_database.h"
#include "multi/multi_database.h"
#include "database.h"

IRSingleDatabase *
DatabaseBuilder::make(om_database_type type)
{
    IRSingleDatabase * database = NULL;

    switch(type) {
	case OM_DBTYPE_DA:
	    database = new DADatabase;
	    break;
	case OM_DBTYPE_TEXTFILE:
	    database = new TextfileDatabase;
	    break;
	case OM_DBTYPE_SLEEPY:
	    database = new SleepyDatabase;
	    break;
    }

    if(database == NULL) {
	throw OmError("Couldn't create database");
    }

    return database;
}

IRGroupDatabase *
DatabaseBuilder::makegroup(om_databasegroup_type type)
{
    IRGroupDatabase * database = NULL;

    switch(type) {
	case OM_DBGRPTYPE_MULTI:
	    database = new MultiDatabase;
	    break;
    }

    if(database == NULL) {
	throw OmError("Couldn't create database group");
    }

    return database;
}
