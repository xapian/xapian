/************************************************************
 *
 *  cvs_db_file.C implementation.
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
 *  $Id$
 *
 ************************************************************/

#include "cvs_db_file.h"

cvs_db_file::~cvs_db_file()
{
    _comment_db.close();
    _comment_id_db.close();
    _comment_id2_db.close();
    _filename_db.close();
    _file_id_db.close();
    _line_db.close();
    _revision_db.close();
    _file_revision_db.close();
    _diff_db.close();
}

int
cvs_db_file::put_comment (unsigned int & commit_id, const string & comment) 
{
    int val = 0;
    if ((val = _comment_db.open(_database_name, false)) == 0)
    {
        if ((val = _comment_db.put(commit_id, comment)) == 0)
        {
        }
    }
    return val;
}

int
cvs_db_file::get_comment (unsigned int commit_id, string & comment)
{
    int val = 0;
    if ((val = _comment_db.open(_database_name, true)) == 0)
    {
        if ((val = _comment_db.get(commit_id, comment)) == 0)
        {
        }
    }
    return val;
}

int
cvs_db_file::get_comment(unsigned int file_id, const string & revision, string & comment) 
{
    unsigned int comment_id_result = 0;
    int val = 0;
    if ((val = _comment_db.open(_database_name, true)) == 0 &&
        (val = _comment_id_db.open(_database_name, true)) == 0)
    {
        if ((val = _comment_id_db.get(file_id, revision, comment_id_result)) == 0 &&
            (val = _comment_db.get(comment_id_result, comment)) == 0)
        {
        }
    }
    return val;
}

int
cvs_db_file::put_mapping(unsigned int file_id, const string & revision, unsigned int line)
{
    int val = 0;
    if ((val = _line_db.open(_database_name, false)) == 0 &&
        (val = _revision_db.open(_database_name, false)) == 0)
    {
        if ((val = _line_db.put(file_id, revision, line)) == 0 &&
            (val = _revision_db.put(file_id, line,revision)) == 0)
        {
        }
    }
    return val;
}

int
cvs_db_file::get_revision(unsigned int file_id, unsigned int line, set<string, cvs_revision_less> & revisions)
{
    revisions.clear();
    int val = 0;
    if ((val = _revision_db.open(_database_name, true)) == 0)
    {
        if ((val = _revision_db.get(file_id, line, revisions)) == 0)
        {
        }
    }
    return val;
}

int
cvs_db_file::get_line    (unsigned int file_id, const string & revision, set<unsigned int> & lines)
{
    lines.clear();
    int val = 0;
    if ((val = _line_db.open(_database_name, true)) == 0)
    {
        if ((val = _line_db.get(file_id, revision, lines)) == 0)
        {
        }
    }
    return val;
}

int
cvs_db_file::put_revision(unsigned int file_id, const string & revision)
{
    int val = 0;
    if ((val = _file_revision_db.open(_database_name, false)) == 0)
    {
        if ((val = _file_revision_db.put(file_id, revision)) == 0)
        {
        }
    }
    return val;
}

int
cvs_db_file::clear()
{
    int val = 0;
    if ((val = _comment_db.remove(_database_name)) == 0 &&
        (val = _comment_id_db.remove(_database_name)) == 0 &&
        (val = _comment_id2_db.remove(_database_name)) == 0 &&
        (val = _filename_db.remove(_database_name)) == 0 &&
        (val = _file_id_db.remove(_database_name)) == 0 &&
        (val = _line_db.remove(_database_name)) == 0 &&
        (val = _revision_db.remove(_database_name)) == 0 &&
        (val = _file_revision_db.remove(_database_name)) == 0 &&
        (val = _diff_db.remove(_database_name)) == 0)
    {
    }
    return val;
}

int
cvs_db_file::get_revision_comment(unsigned int file_id, set<string, cvs_revision_less> & revisions, vector<string> & comments)
{
    revisions.clear();
    comments.clear();
    
    string comment;
    unsigned int comment_id;

    int val = 0;

    if ((val = _file_revision_db.open(_database_name, true)) == 0 &&
        (val = _comment_id_db.open(_database_name, true)) == 0 &&
        (val = _comment_db.open(_database_name, true)) == 0)
    {
        if (_file_revision_db.get(file_id, revisions) == 0)
        {
            set<string, cvs_revision_less>::iterator itr;
            for (itr = revisions.begin();
                 itr!= revisions.end();
                 ++itr)
            {
                if ((val = _comment_id_db.get(file_id, *itr, comment_id)) == 0 &&
                    (val = _comment_db.get(comment_id, comment)) == 0)
                {
                    comments.push_back(comment);
                } else {
                    comments.push_back("");
                }
            }
        }
    }
    return val;
}

int
cvs_db_file::put_filename         (unsigned int & file_id, const string & filename)
{
    int val = 0;
    if ((val = _filename_db.open(_database_name, false)) == 0 &&
        (val = _file_id_db.open(_database_name, false)) == 0)
    {
        if ((val = _filename_db.put(file_id, filename)) == 0 &&
            (val = _file_id_db.put(file_id, filename)) == 0)
        {
        }
    }
    return val;
}

int
cvs_db_file::get_fileid           (unsigned int & file_id, const string & filename)
{
    int val = 0;
    if ((val = _file_id_db.open(_database_name, true)) == 0)
    {
        if ((val = _file_id_db.get(file_id, filename)) == 0)
        {
        }
    }
    return val;
}

int
cvs_db_file::get_filename         (unsigned int file_id, string & filename)
{
    int val = 0;
    if ((val = _filename_db.open(_database_name, true)) == 0)
    {
        if ((val = _filename_db.get(file_id, filename)) == 0)
        {
        }
    }
    return val;
}

int
cvs_db_file::sync()
{
    if (!_read_only) {
        _comment_db.sync();
        _comment_id_db.sync();
        _filename_db.sync();
        _line_db.sync();
        _revision_db.sync();
        _file_revision_db.sync();
        _diff_db.sync();
        _comment_id2_db.sync();
    }
    return 0;
}

int
cvs_db_file::get_diff(unsigned int fileId, const string & revision, 
                      vector<unsigned int> & s1, vector<unsigned int> & s2, 
                      vector<unsigned int> & d1, vector<unsigned int> & d2, vector<char> & type)
{
    int val = 0;
    if ((val = _diff_db.open(_database_name, true)) == 0)
    {
        if ((val = _diff_db.get(fileId, revision, s1, s2, d1, d2, type)) == 0)
        {
        }
    }
    return val;
}

int
cvs_db_file::put_diff(unsigned int fileId, const string & revision, 
                      const vector<unsigned int> & s1, const vector<unsigned int> & s2, 
                      const vector<unsigned int> & d1, const vector<unsigned int> & d2, const vector<char> & type)
{
    int val = 0;
    if ((val = _diff_db.open(_database_name, false)) == 0)
    {
        if ((val = _diff_db.put(fileId, revision, s1, s2, d1, d2, type)) == 0)
        {
        }
    }
    return val;
}

int
cvs_db_file::put_commit(unsigned int file_id, const string & revision, unsigned int commit_id) 
{
    int val = 0;
    if ((val = _comment_id_db.open(_database_name, false)) == 0 &&
        (val = _comment_id2_db.open(_database_name, false)) == 0)
    {
        if ((val = _comment_id_db.put(file_id, revision, commit_id)) == 0 &&
            (val = _comment_id2_db.put(commit_id, file_id, revision)) == 0)
        {
        }
    }
    return val;
}

int
cvs_db_file::get_commit(unsigned int file_id, const string & revision, unsigned int & commit_id)
{
    int val = 0;
    if ((val = _comment_id_db.open(_database_name, true)) == 0)
    {
        if ((val = _comment_id_db.get(file_id, revision, commit_id)) == 0)
        {
        }
    }
    return val;
}

int
cvs_db_file::get_commit(unsigned int commit_id, vector<unsigned int> & fileids, vector<string> & revisions)
{
    int val = 0;
    if ((val = _comment_id2_db.open(_database_name, true)) == 0)
    {
        if ((val = _comment_id2_db.get(commit_id, fileids, revisions)) == 0)
        {
        }
    }
    return val;
}

int
cvs_db_file::get_commit_count     (unsigned int & count){
    int val = 0;
    if ((val = _comment_db.open(_database_name, true)) == 0)
    {
        if ((val = _comment_db.count(count)) == 0)
        {
        }
    }
    return val;
}

int
cvs_db_file::get_filename_count   (unsigned int & count){
    int val = 0;
    if ((val = _filename_db.open(_database_name, true)) == 0)
    {
        if ((val = _filename_db.count(count)) == 0)
        {
        }
    }
    return val;
}
