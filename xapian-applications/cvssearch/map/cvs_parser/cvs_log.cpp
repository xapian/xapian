/************************************************************
 *
 * cvs_log implementation.
 * 
 * This file is a part of the cvssearch library.
 * 
 * $Id$
 *
 ************************************************************/

#include "cvs_log.h"
#include "cvs_output.h"

extern string scvs_root;

istream &
cvs_log::read(istream & is)
{
    unsigned int offset = 1;
    if (scvs_root[scvs_root.length()-1] == '/')
    {
        offset = 0;
    }

    string line;
    while (getline(is, line))
    {
        if (line.find(cvs_output::cvs_log_rcs_file_tag) == 0)
        {
            unsigned int n = line.length() - 
                cvs_output::cvs_log_rcs_file_tag.length()-2-scvs_root.length()-offset;
            unsigned int pos = cvs_output::cvs_log_rcs_file_tag.length()+
                scvs_root.length() + offset;
            _pathname = line.substr(pos,n);
        }
        if (line.find(cvs_output::cvs_log_filename_tag) == 0)
        {
            _filename = line.substr(cvs_output::cvs_log_filename_tag.length());
        }
        if (line == cvs_output::cvs_log_separator)
        {
            break;
        }
    }

    // ----------------------------------------
    // at this point, either the istream is 
    // exhausted because no match is found,
    // or a match is found.
    // ----------------------------------------

    while (is) 
    {
        cvs_log_entry a;
        is >> a;
        if (a.read_status())
        {
            _entries.push_back(a);
            if (a.is_first_entry())
            {
                break;
            }
        } else
        {
            break;
        }
    }
    return is;
}

ostream &
cvs_log:: show(ostream & os) const
{
    os << "Filename: " << _filename << endl
       << "Pathname: " << _pathname << endl;
    copy (_entries.begin(), _entries.end(), ostream_iterator<cvs_log_entry>(os, "\n"));
    return os;
}

