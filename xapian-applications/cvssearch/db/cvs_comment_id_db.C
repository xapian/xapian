#include "cvs_comment_id_db.h"
#include <strstream>

cvs_comment_id_db::cvs_comment_id_db(DbEnv *dbenv, u_int32_t flags)
    :cvs_db(dbenv, flags, "file_rev-comment_id")
{
}

int
cvs_comment_id_db::open(const string & filename)
{
    return _db.open(filename.c_str(), _db_name.c_str(), DB_HASH, DB_CREATE, 0);
}

int
cvs_comment_id_db::get(unsigned int fileId, const string & revision, unsigned int & comment_id)
{
    try {
        ostrstream ost;
        ost << fileId << ':' << revision << ends;
        string skey = ost.str();
        const char *ckey = skey.c_str();
        
        Dbt key ((void *) ckey, skey.length() + 1);
        Dbt data((void *) comment_id, sizeof(unsigned int));
        int val = _db.get(0, &key, &data, 0);
        if (data.get_data())
        {
            comment_id = * (unsigned int *) data.get_data();
        }
        return val;
    }  catch (DbException& e ) {
        cerr << "SleepyCat Exception: " << e.what() << endl;
    }
}

int
cvs_comment_id_db::put(unsigned int fileId, const string & revision, unsigned int comment_id)
{
    try {
        ostrstream ost;
        ost << fileId << ':' << revision << ends;
        string skey = ost.str();
        Dbt key ((void *) skey.c_str(), skey.length() + 1);
        Dbt data((void *) &comment_id, sizeof(unsigned int));

        return _db.put(0, &key, &data, 0);
    }  catch (DbException& e ) {
        cerr << "SleepyCat Exception: " << e.what() << endl;
    }
}
