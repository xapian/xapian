#include "cvs_line_db.h"
#include <strstream>

cvs_line_db::cvs_line_db(DbEnv *dbenv, u_int32_t flags)
    :cvs_db(dbenv, flags, "file_rev-line")
{
}

int
cvs_line_db::open(const string & filename)
{
    return _db.open(filename.c_str(), _db_name.c_str(), DB_HASH, DB_CREATE|DB_DUP, 0);
}

int
cvs_line_db::get(unsigned int fileId, const string & revision, vector<unsigned int> & result)
{
    ostrstream ost;
    ost << fileId << ':' << revision << ends;
    string skey = ost.str();
    const char *ckey = skey.c_str();
    try {
        Dbt key(((void *) ckey), skey.length()+1);
        Dbt data;
        Dbc * pcursor = 0;
        _db.cursor(0, &pcursor, 0);
        
        if (pcursor) {
            if (pcursor->get(&key, &data, DB_SET) != DB_NOTFOUND) {
                if (data.get_data()) {
                    result.push_back(*(unsigned int *) data.get_data());
                }
                while (pcursor->get(&key, &data, DB_NEXT) != DB_NOTFOUND) {
                    if (data.get_data()) {
                        result.push_back(*(unsigned int *) data.get_data());
                    }
                }
            }
            pcursor->close();
        }
        return 0;
    }  catch (DbException& e ) {
        cerr << "SleepyCat Exception: " << e.what() << endl;
    }
}

int
cvs_line_db::put(unsigned int fileId, const string & revision, unsigned int line)
{
    ostrstream ost;
    ost << fileId << ':' << revision << ends;
    string skey = ost.str();
    const char *ckey = skey.c_str();
    try {
        Dbt key ((void *) ckey, skey.length()+1);
        Dbt data((void *) &line, sizeof(unsigned int));
        return _db.put(0, &key, &data, 0);
    }  catch (DbException& e ) {
        cerr << "SleepyCat Exception: " << e.what() << endl;
    }
}
