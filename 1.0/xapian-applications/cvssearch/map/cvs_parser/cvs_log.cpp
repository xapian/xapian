/************************************************************
 *
 *  cvs_log.cpp implementation.
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

#include "cvs_log.h"
#include "cvs_output.h"
#include <iterator>
using namespace std;

extern string scvs_root;

istream &
cvs_log::read(istream & is)
{
    unsigned int offset = 1;
    // ----------------------------------------
    // strip off the ending '/' if any
    // ----------------------------------------
    if (scvs_root[scvs_root.length()-1] == '/')
    {
        offset = 0;
    }

    string line;
    while (getline(is, line))
    {
//         if (line.find(cvs_output::cvs_log_rcs_file_tag) == 0)
//         {
//             unsigned int n = line.length() - 
//                 cvs_output::cvs_log_rcs_file_tag.length()-2-scvs_root.length()-offset;
//             unsigned int pos = cvs_output::cvs_log_rcs_file_tag.length()+
//                 scvs_root.length() + offset;
            
//         }
        if (line.find(cvs_output::cvs_log_filename_tag) == 0)
        {
            // _pathname = line.substr(pos,n);
            _filename = line.substr(cvs_output::cvs_log_filename_tag.length());
            _pathname = _filename;
        }
        if (line == cvs_output::cvs_log_separator)
        {
            break;
        }
    }

    // ----------------------------------------
    // at this point, either the istream is 
    // exhausted because no match is found,
    // or a match is found. lets read the entries.
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

