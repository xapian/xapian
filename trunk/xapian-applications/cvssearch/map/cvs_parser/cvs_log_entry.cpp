/************************************************************
 *
 *  cvs_log_entry.cpp implmentation.
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

#include "cvs_log_entry.h"
#include "cvs_output.h"

#include <string>
using std::string;

cvs_log_entry::cvs_log_entry()
    :_init(false)
{
}

istream & 
cvs_log_entry::read(istream & is)
{
    read_status(false);
    _init = false;
    _comments = "";

    string line;
    // ----------------------------------------
    // read until a line begins "revision: "
    // ----------------------------------------
    is >> _revision;

    // ----------------------------------------
    // get the line starting date:
    // ----------------------------------------
    getline(is,line);
    unsigned int old_pos = 0;
    unsigned int new_pos = 0;
    if ((new_pos = line.find(cvs_output::cvs_log_date_tag, old_pos)) != line.npos) 
    {
        new_pos = cvs_output::cvs_log_date_tag.length();
        old_pos = new_pos;
    }

    if ((new_pos = line.find(cvs_output::cvs_log_author_tag, old_pos)) != line.npos) 
    {
        _date = line.substr(old_pos, new_pos-old_pos);
        new_pos += cvs_output::cvs_log_author_tag.length();
        old_pos = new_pos;
    }

    if ((new_pos = line.find(cvs_output::cvs_log_state_tag, old_pos)) != line.npos) 
    {
        _author = line.substr(old_pos, new_pos-old_pos);
        new_pos += cvs_output::cvs_log_state_tag.length();
        old_pos = new_pos;
    }

    if ((new_pos = line.find(cvs_output::cvs_log_lines_tag, old_pos)) != line.npos) 
    {
        _state = line.substr(old_pos, new_pos-old_pos);
        new_pos += cvs_output::cvs_log_lines_tag.length();
        old_pos = new_pos;
    }
    
    _lines = line.substr(old_pos);
    
    while (getline(is, line))
    {
        // ----------------------------------------
        // ignore the branch info if one is found
        // ----------------------------------------
        if (line.find(cvs_output::cvs_log_branches_tag) == 0)
        {
            continue;
        }
        if (line == cvs_output::cvs_log_separator)
        {
            read_status(true);
            break;
        }

        if (line == cvs_output::cvs_log_end_marker)
        {
            read_status(true);
            _init = true;
            break;
        }
        if (line == cvs_output::cvs_log_empty_comment)
        {
            // ----------------------------------------
            // we want to read until either the 
            // log_separator marker or the log_end_marker
            // is reached
            // ----------------------------------------
            continue;
        }
        _comments += line + "\n";
    }
    return is;
}

ostream & 
cvs_log_entry::show(ostream & os) const
{
    static char separator = '\003';
    if (read_status())
    {
        return os << separator << "revision " << _revision
                  << separator << "date " << _date
                  << separator << "author " << _author
                  << separator << "state " << _state
                  << separator << "lines " << _lines
                  << separator << "comments " << _comments;
    }
    return os;
}

