#include "cvs_revision_db.h"
#include <strstream>

cvs_revision_db::cvs_revision_db(DbEnv *dbenv, u_int32_t flags)
    :cvs_db(dbenv, flags, "file_line-rev")
{
}

int
cvs_revision_db::open(const string & filename)
{
    return _db.open(filename.c_str(), _db_name.c_str(), DB_HASH, DB_CREATE|DB_DUP, 0);
}

int
cvs_revision_db::get(unsigned int fileId, unsigned int line, vector<string> & result)
{
    ostrstream ost;
    ost << fileId << ':' << line << ends;
    string skey = ost.str();
    const char *ckey = skey.c_str();
    try {
        Dbt key ((void *) ckey, skey.length()+1);
        Dbt data(0,0);
        Dbc * pcursor = 0;
        _db.cursor(0, &pcursor, 0);

        if (pcursor) {
            if (pcursor->get(&key, &data, DB_SET) != DB_NOTFOUND) {
                if (data.get_data()) {
                    result.push_back(string((const char *) data.get_data()));
                }
                while (pcursor->get(&key, &data, DB_NEXT) != DB_NOTFOUND) {
                    if (data.get_data()) {
                        result.push_back((const char *) data.get_data());
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
cvs_revision_db::put(unsigned int fileId, unsigned int line, const string & revision)
{
    ostrstream ost;
    ost << fileId << ':' << line << ends;
    string skey = ost.str();
    try {
        Dbt key((void *) skey.c_str(), skey.length()+1);
        Dbt data((void *) revision.c_str(), revision.length()+1);
        return _db.put(0, &key, &data, 0);
    }  catch (DbException& e ) {
        cerr << "SleepyCat Exception: " << e.what() << endl;
    }
}
