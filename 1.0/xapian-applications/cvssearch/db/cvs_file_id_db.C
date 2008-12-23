/************************************************************
 *
 *  cvs_file_id_db.C implementation.
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

#include <iostream>
using namespace std;

#include "cvs_file_id_db.h"

cvs_file_id_db::cvs_file_id_db(DbEnv *dbenv, u_int32_t flags)
    :cvs_db("filename-file_id", "9", dbenv, flags)
{
}

/**
 * Opens the database storing filename->file_id.
 *
 * @param filename the name of the physical file where the database
 *                 is located/to be created.
 * @param read_only set database to be read_only if true.
 * @return error code of the open operation.
 **/
int
cvs_file_id_db::do_open(const string & filename, bool read_only)
{
    int val = 0;
    try {
        unsigned int flag = read_only ? DB_RDONLY : DB_CREATE;
#if DB_VERSION_MAJOR > 4 || (DB_VERSION_MAJOR == 4 && DB_VERSION_MINOR >= 1)
        val = _db.open(NULL, filename.c_str(), _db_name.c_str(), DB_HASH, flag, 0);
#else
        val = _db.open(filename.c_str(), _db_name.c_str(), DB_HASH, flag, 0);
#endif
    } catch (DbException & e) {
        cerr << "SleepyCat Exception: " << e.what() << endl;
    }
    return val;
}

/**
 * Gets a file_id given a filename.
 *
 * @param file_id the recno of the filename to get.
 * @param filename the filename to retrieve.
 *
 * @return error code of the get operation.
 **/
int
cvs_file_id_db::get(unsigned int & file_id, const string & filename)
{
    int val = 0;
    try {
        Dbt key ((void *) filename.c_str(), filename.length() + 1);
        Dbt data((void *) file_id, sizeof(unsigned int));
        val = _db.get(0, &key, &data, 0);
        if (data.get_data())
        {
            file_id = * (unsigned int *) data.get_data();
        }
    }  catch (DbException& e ) {
        cerr << "SleepyCat Exception: " << e.what() << endl;
    }
    return val;
}

/**
 * Puts a filename->file_id into the database,
 * the file_id is the recno of the filename.
 *
 * @param file_id the recno of the filename
 * @param filename the filename.
 *
 * @return error code of the put operation.
 **/
int
cvs_file_id_db::put(unsigned int file_id, const string & filename)
{
    int val = 0;
    try {
        Dbt key ((void *) filename.c_str(), filename.length() + 1);
        Dbt data((void *) &file_id, sizeof(unsigned int));

        val = _db.put(0, &key, &data, 0);
    }  catch (DbException& e ) {
        cerr << "SleepyCat Exception: " << e.what() << endl;
    }
    return val;
}
