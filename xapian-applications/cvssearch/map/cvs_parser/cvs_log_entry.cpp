/************************************************************
 *
 * cvs_log_entry implementation.
 * 
 * This file is a part of the cvssearch library.
 * 
 * $Id$
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
    if ((new_pos = line.find(cvs_output::cvs_log_date_tag, old_pos)) != -1) 
    {
        new_pos = cvs_output::cvs_log_date_tag.length();
        old_pos = new_pos;
    }

    if ((new_pos = line.find(cvs_output::cvs_log_author_tag, old_pos)) != -1) 
    {
        _date = line.substr(old_pos, new_pos-old_pos);
        cout << "DATE|" << _date << "|" << endl;
        new_pos += cvs_output::cvs_log_author_tag.length();
        old_pos = new_pos;
    }

    if ((new_pos = line.find(cvs_output::cvs_log_state_tag, old_pos)) != -1) 
    {
        _author = line.substr(old_pos, new_pos-old_pos);
        cout << "AUTH|" << _author << "|" << endl;
        new_pos += cvs_output::cvs_log_state_tag.length();
        old_pos = new_pos;
    }

    if ((new_pos = line.find(cvs_output::cvs_log_lines_tag, old_pos)) != -1) 
    {
        _state = line.substr(old_pos, new_pos-old_pos);
        cout << "STAT|" << _state << "|" << endl;
        new_pos += cvs_output::cvs_log_lines_tag.length();
        old_pos = new_pos;
    }
    
    _lines = line.substr(old_pos);
    cout << "LINE|" << _lines << "|" << endl;
    old_pos = new_pos;
    
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
           << separator << "comments " << _comments
           << separator;
    }
    return os;
}

