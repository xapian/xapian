#include "cvs_comment_db.h"

cvs_comment_db::cvs_comment_db(DbEnv *dbenv, u_int32_t flags)
    :cvs_db(dbenv, flags, "id-comment")
{
}

int
cvs_comment_db::open(const string & filename)
{
    return _db.open(filename.c_str(), _db_name.c_str(), DB_RECNO, DB_CREATE, 0);
}

int
cvs_comment_db::get(unsigned int comment_id, string & comment)
{
    try {
        db_recno_t rec = comment_id;
        Dbt key(((void *) &rec), sizeof(rec));
        Dbt data;    
        int val = _db.get(0, &key, &data, 0);
        comment = (char *) data.get_data();
        return val;
    }  catch (DbException& e ) {
        cerr << "SleepyCat Exception: " << e.what() << endl;
    }
}

int
cvs_comment_db::put(unsigned int & comment_id, const string & comment)
{
    try {
        db_recno_t rec = 0;
        const char * ccomment = comment.c_str();
        Dbt key ((void *) &rec, sizeof(rec));
        Dbt data((void *) ccomment, comment.length()+1);
        int val = _db.put(0, &key, &data, DB_APPEND);
        comment_id = (unsigned int) *((db_recno_t *) key.get_data());
        return val;
    }  catch (DbException& e ) {
        cerr << "SleepyCat Exception: " << e.what() << endl;
    }
}
