#include "cvs_filename_db.h"

cvs_filename_db::cvs_filename_db(DbEnv *dbenv, u_int32_t flags)
    :cvs_db(dbenv, flags, "rec-file")
{
}

int
cvs_filename_db::open(const string & filename)
{
    return _db.open(filename.c_str(), _db_name.c_str(), DB_RECNO, DB_CREATE, 0);
}

int
cvs_filename_db::get(unsigned int fileId, string & filename)
{
    try {
        db_recno_t rec = (db_recno_t) fileId;
        Dbt key(((void *) &rec), sizeof(db_recno_t));
        Dbt data(0, 0);    
        int val = _db.get(0, &key, &data, 0);
        filename = (char *) data.get_data();
        return val;
    }  catch (DbException& e ) {
        cerr << "SleepyCat Exception: " << e.what() << endl;
    }
}

int
cvs_filename_db::put(unsigned int & fileId, const string & filename)
{
    try {
        db_recno_t rec = 0;
        const char * cfilename = filename.c_str();
        Dbt key ((void *) &rec, sizeof(db_recno_t));
        Dbt data((void *) cfilename, filename.length()+1);
        int val = _db.put(0, &key, &data, DB_APPEND);

        fileId = (unsigned int) *((db_recno_t *) key.get_data());
        cout << "FileId " << fileId << endl;
        return val;
    }  catch (DbException& e ) {
        cerr << "SleepyCat Exception: " << e.what() << endl;
    }
}
