/************************************************************
 *
 *  cvs_revision_db.C implementation.
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

#include "util.h"
#include "cvs_revision_db.h"

cvs_revision_db::cvs_revision_db(DbEnv *dbenv, u_int32_t flags)
    :cvs_db("file_line-revision", "6", dbenv, flags)
{
}

/**
 * Opens the database storing fileId,line->{revisions}.
 *
 * @param filename the name of the physical file where the database
 *                 is located/to be created.
 * @param read_only set database to be read_only if true.
 * @return error code of the open operation.
 **/
int
cvs_revision_db::do_open(const string & filename, bool read_only)
{
    int val = 0;
    try {
        val = _db.set_flags(DB_DUP);
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
 * Gets a set of revisions given a fileId and a line.
 *
 * @param fileId the recno of the filename to get.
 * @param line the line where we want to know which revisions changes has occurred to it.
 * @param result the container to store the result set of revisions.
 *
 * @return error code of the get operation.
 **/
int
cvs_revision_db::get(unsigned int fileId, unsigned int line, set<string, cvs_revision_less> & result)
{
    string skey = uint_to_string(fileId) + ':' + uint_to_string(line);

    int val = 0;
    try {
        Dbt key ((void *) skey.c_str(), skey.length()+1);
        Dbt data(0,0);
        Dbc * pcursor = 0;
        _db.cursor(0, &pcursor, 0);

        if (pcursor) 
        {
            if ((val = pcursor->get(&key, &data, DB_SET)) != DB_NOTFOUND) 
            {
                if (data.get_data()) 
                {
                    result.insert(string((const char *) data.get_data()));
                }
                while ((val = pcursor->get(&key, &data, DB_NEXT)) != DB_NOTFOUND) 
                {
                    if (strcmp(skey.c_str(), (char *) key.get_data()))
                    {
                        break;
                    }
                    if (data.get_data()) 
                    {
                        result.insert((const char *) data.get_data());
                    }
                }
                pcursor->close();
                return 0;
            }
            pcursor->close();
            return val;
        }
    }  catch (DbException& e ) {
        cerr << "SleepyCat Exception: " << e.what() << endl;
    }
    return val;
}

/**
 * Puts a (fileId,line)->revision into the database.
 *
 * @param fileId the recno of the filename.
 * @param line the line number (starting 1).
 * @param revision the revision where the line is changed/added.
 *
 * @return error code of the put operation.
 **/
int
cvs_revision_db::put(unsigned int fileId, unsigned int line, const string & revision)
{
    string skey = uint_to_string(fileId) + ':' + uint_to_string(line);

    int val = 0;
    try {
        Dbt key((void *) skey.c_str(), skey.length()+1);
        Dbt data((void *) revision.c_str(), revision.length()+1);
        val = _db.put(0, &key, &data, 0);
    }  catch (DbException& e ) {
        cerr << "SleepyCat Exception: " << e.what() << endl;
    }
    return val;
}
