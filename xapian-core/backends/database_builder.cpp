/* database_builder.cc: Builder for creating databases */

#include "omassert.h"
#include "database_builder.h"

// Include headers for all the database types
#include "da/da_database.h"
#include "textfile/textfile_database.h"
#include "sleepy/sleepy_database.h"
#include "multi/multi_database.h"
#include "database.h"

IRDatabase *
DatabaseBuilder::create(const DatabaseBuilderParams & params)
{
    IRDatabase * database = NULL;

    // Create database of correct type, and 
    switch(params.type) {
	case OM_DBTYPE_NULL:
	    throw OmError("Unspecified database type");
	    break;
	case OM_DBTYPE_DA:
	    database = new DADatabase;
	    break;
	case OM_DBTYPE_TEXTFILE:
	    database = new TextfileDatabase;
	    break;
	case OM_DBTYPE_SLEEPY:
	    database = new SleepyDatabase;
	    break;
	case OM_DBTYPE_MULTI:
	    database = new MultiDatabase;
	    break;
	default:
	    throw OmError("Unknown database type");
    }

    // Check that we have a database
    if(database == NULL) {
	throw OmError("Couldn't create database");
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
