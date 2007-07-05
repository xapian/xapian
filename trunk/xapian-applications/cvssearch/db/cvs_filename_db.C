/************************************************************
 *
 *  cvs_filename_db.C implementation.
 *
 *  (c) 2001 Andrew Yao (andrewy@users.sourceforge.net)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *  Usage:
 *
 *  See cvs_db_file.h
 *
 *  $Id$
 *
 ************************************************************/

#include "cvs_filename_db.h"
using namespace std;

cvs_filename_db::cvs_filename_db(DbEnv *dbenv, u_int32_t flags)
    :cvs_db("recno-filename", "4", dbenv, flags)
{
}

cvs_filename_db::cvs_filename_db(const string & name, const string & index, DbEnv *dbenv, u_int32_t flags)
    :cvs_db(name, index, dbenv, flags)
{
}

int
cvs_filename_db::count(unsigned int & count)
{
    int val = 0;
    DB_BTREE_STAT *sp;
    try {
        val = _db.stat(&sp, /*malloc,*/ DB_RECORDCOUNT);
        count = sp->bt_nkeys;
        free(sp);
    } catch (DbException& e ) {
        cerr << "SleepyCat Exception: " << e.what() << endl;
    }
    return val;
}

/**
 * Opens the database storing recno->filename.
 *
 * @param filename the name of the physical file where the database
 *                 is located/to be created.
 * @param read_only set database to be read_only if true.
 * @return error code of the open operation.
 **/
int
cvs_filename_db::do_open(const string & filename, bool read_only)
{
    int val = 0;
    try {
        unsigned int flag = read_only ? DB_RDONLY : DB_CREATE;
#if DB_VERSION_MAJOR > 4 || (DB_VERSION_MAJOR == 4 && DB_VERSION_MINOR >= 1)
        val = _db.open(NULL, filename.c_str(), _db_name.c_str(), DB_RECNO, flag, 0);
#else
        val = _db.open(filename.c_str(), _db_name.c_str(), DB_RECNO, flag, 0);
#endif
    }  catch (DbException& e ) {
        cerr << "SleepyCat Exception: " << e.what() << endl;
    }
    return val;
}

/**
 * Gets a filename given a recno specified as fileId.
 *
 * @param fileId the recno of the filename to get.
 * @param filename location where the returned filename is stored.
 *
 * @return error code of the get operation.
 **/
int
cvs_filename_db::get(unsigned int fileId, string & filename)
{
    int val = 0;
    try {
        db_recno_t rec = (db_recno_t) fileId;
        Dbt key(((void *) &rec), sizeof(db_recno_t));
        Dbt data;
        val = _db.get(0, &key, &data, 0);
        if (data.get_data())
        {
            filename = (char *) data.get_data();
        }
    }  catch (DbException& e ) {
        cerr << "SleepyCat Exception: " << e.what() << endl;
    }
    return val;
}

/**
 * Puts a filename into the database and obtain its recno.
 *
 * @param fileId the returned recno of the filename.
 * @param filename the filename to be stored.
 *
 * @return error code of the put operation.
 **/
int
cvs_filename_db::put(unsigned int & fileId, const string & filename)
{
    int val = 0;
    try {
        db_recno_t rec = 0;
        Dbt key ((void *) &rec, sizeof(db_recno_t));
        Dbt data((void *) filename.c_str(), filename.length()+1);
        val = _db.put(0, &key, &data, DB_APPEND);

        if (key.get_data())
        {
            fileId = (unsigned int) *((db_recno_t *) key.get_data());
        }
    }  catch (DbException& e ) {
        cerr << "SleepyCat Exception: " << e.what() << endl;
    }
    return val;
}
