/* database_builder.cc: Builder for creating databases */

#include "omassert.h"
#include "database_builder.h"

// Include headers for all the database types
#ifdef MUS_BUILD_BACKEND_MUSCAT36
#include "muscat36/da_database.h"
//#include "muscat36/db_database.h"
#endif
#ifdef MUS_BUILD_BACKEND_INMEMORY
#include "inmemory/inmemory_database.h"
#endif
#ifdef MUS_BUILD_BACKEND_SLEEPY
#include "sleepy/sleepy_database.h"
#endif
#ifdef MUS_BUILD_BACKEND_MULTI
#include "multi/multi_database.h"
#endif
#include "database.h"

IRDatabase *
DatabaseBuilder::create(const DatabaseBuilderParams & params)
{
    IRDatabase * database = NULL;

    // Create database of correct type, and 
    switch(params.type) {
	case OM_DBTYPE_NULL:
	    throw OmInvalidArgumentError("Unspecified database type");
	    break;
	case OM_DBTYPE_MUSCAT36_DA_F:
#ifdef MUS_BUILD_BACKEND_MUSCAT36
	    database = new DADatabase(0);
#endif
	    break;
	case OM_DBTYPE_MUSCAT36_DA_H:
#ifdef MUS_BUILD_BACKEND_MUSCAT36
	    database = new DADatabase(1);
#endif
	    break;
	case OM_DBTYPE_MUSCAT36_DB_F:
#ifdef MUS_BUILD_BACKEND_MUSCAT36
	    //database = new DBDatabase(0);
#endif
	    break;
	case OM_DBTYPE_MUSCAT36_DB_H:
#ifdef MUS_BUILD_BACKEND_MUSCAT36
	    //database = new DBDatabase(1);
#endif
	    break;
	case OM_DBTYPE_INMEMORY:
#ifdef MUS_BUILD_BACKEND_INMEMORY
	    database = new InMemoryDatabase;
#endif
	    break;
	case OM_DBTYPE_SLEEPY:
#ifdef MUS_BUILD_BACKEND_SLEEPY
	    database = new SleepyDatabase;
#endif
	    break;
	case OM_DBTYPE_MULTI:
#ifdef MUS_BUILD_BACKEND_MULTI
	    database = new MultiDatabase;
#endif
	    break;
	default:
	    throw OmInvalidArgumentError("Unknown database type");
    }

    // Check that we have a database
    if(database == NULL) {
	throw OmOpeningError("Couldn't create database");
    }

    // Open the database with the specified parameters
    try {
	database->open(params);
    } catch (...) {
	delete database;
	throw;
    }

    // Set the root of the database, if specified, otherwise it will default
    // to itself.
    // Doing this after opening ensures that for a group database, all the
    // sub-databases also have their root set.
    if(params.root != NULL) database->set_root(params.root);
    
    return database;
}
