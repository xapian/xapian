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
        _comments += " " + line;
    }
    return is;
}

ostream & 
cvs_log_entry::show(ostream & os) const
{
    if (read_status())
    {
        return os << _comments;
    }
    return os;
}

