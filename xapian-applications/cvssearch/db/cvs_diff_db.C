/************************************************************
 *
 *  cvs_diff_db.C implementation.
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

#include <util.h>
#include "cvs_diff_db.h"
#include <assert.h>

cvs_diff_db::cvs_diff_db(DbEnv *dbenv, u_int32_t flags)
    :cvs_db("file_revision-diff", "7", dbenv, flags)
{
}

/**
 * Opens the database storing file_id:revision->comment id.
 *
 * @param filename the name of the physical file where the database
 *                 is located/to be created.
 * @param read_only set database to be read_only if true.
 * @return error code of the open operation.
 **/
int
cvs_diff_db::do_open(const string & filename, bool read_only)
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
 * Gets a diff given a file id and a revision.
 *
 * @param fileId the recno of the filename to get.
 * @param revision the revision should we obtain the comment for.
 * @param diff the location where the result is stored.
 *
 * @return error code of the get operation.
 **/
int
cvs_diff_db::get(unsigned int fileId, const string & revision, 
                 vector<unsigned int> & s1,
                 vector<unsigned int> & s2,
                 vector<unsigned int> & d1,
                 vector<unsigned int> & d2,
                 vector<char> & type)
{
    s1.clear();
    s2.clear();
    d1.clear();
    d2.clear();
    type.clear();
    unsigned int is1, is2, id1, id2;
    char ctype;
    char tmp;

    int val = 0;
    try {
        string skey = uint_to_string(fileId) + ':' + revision;

        Dbt key ((void *) skey.c_str(), skey.length() + 1);
        Dbt data;
        val = _db.get(0, &key, &data, 0);
        if (data.get_data())
        {
            const char *p = (const char *) data.get_data();
	    int count;
            while (sscanf(p, "%u,%u%c%u,%u.%n", &is1, &is2, &ctype, &id1, &id2,
			  &count) >= 5)
            {
		p += count;
                switch(ctype) {
                case 'a':
                    is1-=1;
                    is2-=1;
                    id2-=1;
                    break;
                case 'd':
                    is2-=1;
                    id1-=1;
                    id2-=1;
                    break;
                case 'c':
                    is2-=1;
                    id2-=1;
                    break;
                default:
		    assert(false);
                }

                s1.push_back(is1);
                s2.push_back(is2);
                d1.push_back(id1);
                d2.push_back(id2);
                type.push_back(ctype);
            }
        }
    } catch (DbException& e) {
        cerr << "SleepyCat Exception: " << e.what() << endl;
    }
    return val;
}

/**
 * Puts a diff into the database,
 * the diff is the recno of the comment associated
 * with the filename with recno fileId and the revision.
 *
 * @param fileId the recno of the filename
 * @param revision the revision of the comment
 * @param s1, s2, d1, d2, type denotes all the diffs.
 *
 * @return error code of the put operation.
 **/
int
cvs_diff_db::put(unsigned int fileId, const string & revision,
                 const vector<unsigned int> & s1,
                 const vector<unsigned int> & s2,
                 const vector<unsigned int> & d1,
                 const vector<unsigned int> & d2,
                 const vector<char> & type)
{
    try {
        string skey = uint_to_string(fileId) + ':' + revision;

        assert(s1.size() == s2.size() &&
               s1.size() == d1.size() &&
               s1.size() == d2.size() &&
               s1.size() == type.size());

        string sdata;
        for (unsigned int i = 0; i < s1.size(); ++i) 
        {
            sdata += uint_to_string(s1[i]);
	    sdata += ',';
            sdata += uint_to_string(s2[i]);
	    sdata += type[i];
            sdata += uint_to_string(d1[i]);
	    sdata += ',';
            sdata += uint_to_string(d2[i]);
	    sdata += '.';
        }

        Dbt key ((void *)  skey.c_str(), skey.length() + 1);
        Dbt data((void *) sdata.c_str(), sdata.length() + 1);

        return _db.put(0, &key, &data, 0);
    } catch (DbException& e) {
        cerr << "SleepyCat Exception: " << e.what() << endl;
    }
    return 0;
}
