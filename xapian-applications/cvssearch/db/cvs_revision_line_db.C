/************************************************************
 *
 *  cvs_revision_line_db.C implementation.
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

#include "cvs_revision_line_db.h"
#include <strstream>

cvs_revision_line_db::cvs_revision_line_db(DbEnv *dbenv, u_int32_t flags)
    :cvs_db("file_revision_line-line", dbenv, flags)
{
}

/**
 * Opens the database storing fileId,revision,line_new->line_old
 *
 * @param filename the name of the physical file where the database
 *                 is located/to be created.
 * @return error code of the open operation.
 **/
int
cvs_revision_line_db::do_open(const string & filename)
{
    int val = 0;
    try {
        val = _db.open(filename.c_str(), _db_name.c_str(), DB_HASH, DB_CREATE, 0);
    } catch (DbException & e) {
        cerr << "SleepyCat Exception: " << e.what() << endl;
    }
    return val;
}

/**
 * Get the line_index given a fileId, a revision and the line index in the newest file
 *
 * @param fileId the recno of the filename to get.
 * @param revision the revision where the result lines are affected.
 * @param line_new the line in the newest file
 * @param line_old the corresponding line in the revision, 0 if doesn't exist.
 * @return error code of the get operation.
 **/
int
cvs_revision_line_db::get(unsigned int fileId, const string & revision, unsigned int line_new, unsigned int & line_old)
{
    ostrstream ost;
    string skey = ost.str();

    int val = 0;
    try {
        Dbt key((void *) skey.c_str(), skey.length()+1);
        Dbt data((void *) line_old, sizeof(unsigned int));

        val = _db.get(0, &key, &data, 0);
        if (data.get_data())
        {
            line_old = * (unsigned int *) data.get_data();
        }
    }  catch (DbException& e ) {
        cerr << "SleepyCat Exception: " << e.what() << endl;
    }
    return val;
}

/**
 * Puts a (fileId,revision,line_new)->line_old into the database.
 * @param fileId the recno of the filename to get.
 * @param revision the revision where the result lines are affected.
 * @param line_new the line in the newest file
 * @param line_old the corresponding line in the revision, 0 if doesn't exist.
 *
 * @return error code of the put operation.
 **/
int
cvs_revision_line_db::put(unsigned int fileId, const string & revision, unsigned int line_new, unsigned int line_old)
{
    int val = 0;
    ostrstream ost;
    ost << fileId << ':' << revision << ':' << line_new << ends;
    string skey = ost.str();
    cout << "key " << skey << ":" << line_old << endl;
    try {
        Dbt key ((void *) skey.c_str(), skey.length()+1);
        Dbt data((void *) &line_old, sizeof(unsigned int));
        val = _db.put(0, &key, &data, 0);
    }  catch (DbException& e ) {
        cerr << "SleepyCat Exception: " << e.what() << endl;
    }
    return val;
}
