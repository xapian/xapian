/************************************************************
 *
 *  cvs_comment_id_db.C implementation.
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

#include "cvs_comment_id2_db.h"
#include <strstream>
using namespace std;

cvs_comment_id2_db::cvs_comment_id2_db(DbEnv *dbenv, u_int32_t flags)
    :cvs_db("id-file_rev", "8", dbenv, flags)
{
}

/**
 * Opens the database storing comment id->file_id:revision.
 *
 * @param filename the name of the physical file where the database
 *                 is located/to be created.
 * @param read_only set database to be read_only if true.
 * @return error code of the open operation.
 **/
int
cvs_comment_id2_db::do_open(const string & filename, bool read_only)
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
 * Gets a comment_id given a file id and a revision.
 *
 * @param fileId the recno of the filename to get.
 * @param revision the revision should we obtain the comment for.
 * @param comment_id the location where the result is stored.
 *
 * @return error code of the get operation.
 **/
int
cvs_comment_id2_db::get(unsigned int comment_id, vector<unsigned int> & fileIds, vector<string> & revisions)
{
    int val = 0;
    ostrstream ost;
    ost << comment_id << ends;
    string skey = ost.str();
    ost.freeze(0);

    try {
        Dbt key ((void *) skey.c_str(), skey.length()+1);
        Dbt data;
        Dbc * pcursor = 0;
        _db.cursor(0, &pcursor, 0);

        if (pcursor) 
        {
            if ((val = pcursor->get(&key, &data, DB_SET)) != DB_NOTFOUND) 
            {
                if (data.get_data()) 
                {
                    unsigned int file_id;
                    string revision;
                    char temp;
                    istrstream ist((const char *) data.get_data());
                    ist >> file_id;  // get file_id
                    ist >> temp;     // get ':'
                    ist >> revision; // get revision
                    fileIds.push_back(file_id);
                    revisions.push_back(revision);
                }
                while ((val = pcursor->get(&key, &data, DB_NEXT)) != DB_NOTFOUND) 
                {
                    if (strcmp(skey.c_str(), (char *) key.get_data()))
                    {
                        break;
                    }
                    if (data.get_data()) 
                    {
                        unsigned int file_id;
                        string revision;
                        char temp;
                        istrstream ist((const char *) data.get_data());
                        ist >> file_id;  // get file_id
                        ist >> temp;     // get ':'
                        ist >> revision; // get revision
                        fileIds.push_back(file_id);
                        revisions.push_back(revision);
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
 * Puts a comment_id into the database,
 * the comment_id is the recno of the comment associated
 * with the filename with recno fileId and the revision.
 *
 * @param fileId the recno of the filename
 * @param revision the revision of the comment
 * @param the recno of the comment
 *
 * @return error code of the put operation.
 **/
int
cvs_comment_id2_db::put(unsigned int comment_id, unsigned int fileId, const string & revision)
{
    int val = 0;
    try {
        ostrstream ost;
        ost << fileId << ':' << revision << ends;
        string sdata = ost.str();
        ost.freeze(0);

        ostrstream ost1;
        ost1 << comment_id << ends;
        string skey = ost1.str();
        ost1.freeze(0);
        
        Dbt key ((void *) skey.c_str(), skey.length() + 1);
        Dbt data((void *) sdata.c_str(), sdata.length() + 1);

        val = _db.put(0, &key, &data, 0);
    }  catch (DbException& e ) {
        cerr << "SleepyCat Exception: " << e.what() << endl;
    }
    return val;
}
