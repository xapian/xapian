#ifndef __CVS_DB_H__
#define __CVS_DB_H__

#include "db_cxx.h"
#include <string>
using std::string;

class cvs_db
{
protected:
    Db _db;
    string _db_name;
public:
    cvs_db(DbEnv *dbenv, u_int32_t flags, const string & name) 
        : _db(dbenv, flags), _db_name(name) {}
    virtual int open(const string & filename) = 0;
    int close(int flags = 0) {
        try {
            return _db.close(flags);
        }  catch (DbException& e ) {
            cerr << "SleepyCat Exception: " << e.what() << endl;
        }
    }
    virtual ~cvs_db() {}
};

#endif
