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
    _filename_db.close();
    _line_db.close();
    _revision_db.close();
    _file_revision_db.close();
    _revision_line_db.close();
}

int
cvs_db_file::put_comment(unsigned int file_id, const string & revision, const string & comment)
{
    int val = 0;
    if ((val = _comment_db.open(_database_name)) == 0 &&
        (val = _comment_id_db.open(_database_name)) == 0)
    {
        unsigned int comment_id;
        if ((val = _comment_db.put(comment_id, comment)) == 0 &&
            (val = _comment_id_db.put(file_id, revision, comment_id)) == 0)
        {
        }
    }
    return val;
}

int
cvs_db_file::get_comment(unsigned int file_id, const string & revision, string & comment) 
{
    unsigned int comment_id_result;
    int val = 0;
    if ((val = _comment_db.open(_database_name)) == 0 &&
        (val = _comment_id_db.open(_database_name)) == 0)
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
    if ((val = _line_db.open(_database_name)) == 0 &&
        (val = _revision_db.open(_database_name)) == 0)
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
    if ((val = _revision_db.open(_database_name)) == 0)
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
    if ((val = _line_db.open(_database_name)) == 0)
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
    if ((val = _file_revision_db.open(_database_name)) == 0)
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
        (val = _filename_db.remove(_database_name)) == 0 &&
        (val = _line_db.remove(_database_name)) == 0 &&
        (val = _revision_db.remove(_database_name)) == 0 &&
        (val = _file_revision_db.remove(_database_name)) == 0)
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

    if ((val = _file_revision_db.open(_database_name)) == 0 &&
        (val = _comment_id_db.open(_database_name)) == 0 &&
        (val = _comment_db.open(_database_name)) == 0)
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
    if ((val = _filename_db.open(_database_name)) == 0)
    {
        if ((val = _filename_db.put(file_id, filename)) == 0)
        {
        }
    }
    return val;
}

int
cvs_db_file::get_filename         (unsigned int file_id, string & filename)
{
    int val = 0;
    if ((val = _filename_db.open(_database_name)) == 0)
    {
        if ((val = _filename_db.get(file_id, filename)) == 0)
        {
        }
    }
    return val;
}

int
cvs_db_file::put_line_mapping (unsigned int file_id, const string & revision, unsigned int line_new, unsigned int line_old)
{
    int val = 0;
    if ((val = _revision_line_db.open(_database_name)) == 0)
    {
        if ((val = _revision_line_db.put(file_id, revision, line_new, line_old)) == 0)
        {
        }
    }
    return val;
}

int
cvs_db_file::get_line_mapping (unsigned int file_id, const string & revision, unsigned int line_new, unsigned int & line_old)
{
    int val = 0;
    if ((val = _revision_line_db.open(_database_name)) == 0)
    {
        if ((val = _revision_line_db.get(file_id, revision, line_new, line_old)) == 0)
        {
        }
    }
    return val;
}

int
cvs_db_file::sync()
{
    _comment_db.sync();
    _comment_id_db.sync();
    _filename_db.sync();
    _line_db.sync();
    _revision_db.sync();
    _file_revision_db.sync();
    _revision_line_db.sync();
    return 0;
}
