/************************************************************
 *
 *  backward_map_algorithm.cpp implementation.
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

#include "backward_map_algorithm.h"
#include <strstream>
#include "process.h"
#include "aligned_diff.h"

extern bool output_html;
extern bool read_mode;

extern string scvs_update;
extern string scvs_diff;

static unsigned int get_length(const cvs_log & log, unsigned int j)
{
    ostrstream ost;
    ost << scvs_update << "-r" << log[j].revision() 
        << " " << log.file_name() << " 2>/dev/null |wc -l" << ends;
    process p(ost.str());
    istream *pis = p.process_output();
    unsigned int length = 0;
    if (*pis)
    {
        *pis >> length;
    }
    return length;
}

void
backward_map_algorithm::parse_log(const cvs_log & log)
{
    for (unsigned int j = 0; j < log.size(); ++j)
    {
        ostrstream ost;
        if (j == 0)
        {
            unsigned int length = get_length(log, j);
            init(log[0], length);
        }

        diff * pdiff = 0;
        pdiff = get_diff(log, j);

        if (pdiff && pdiff->read_status()) 
        {
            if (j == log.size()-1)
            {
                parse_diff(log[j], log[j], *pdiff);
            }
            else
            {
                parse_diff(log[j], log[j+1], *pdiff);
            }
        }
        delete pdiff;
    }
}

diff *
backward_map_algorithm::save_diff(const cvs_log & log, unsigned int j)
{
    diff * pdiff = calc_diff(log, j);

    if (pdiff && _db_file && !read_mode) 
    {
        // ----------------------------------------
        // save diff to database
        // ----------------------------------------
        vector<unsigned int> s1;
        vector<unsigned int> s2;
        vector<unsigned int> d1;
        vector<unsigned int> d2;
        vector<char> type;
        
        for (unsigned int i = 0; i < pdiff->size(); ++i) 
        {
            s1.push_back((*pdiff)[i].source().begin());
            s2.push_back((*pdiff)[i].source().end());
            d1.push_back((*pdiff)[i].dest().begin());
            d2.push_back((*pdiff)[i].dest().end());
            switch ((*pdiff)[i].type())
            {
            case e_add:
                type.push_back('a');
                break;
            case e_delete:
                type.push_back('d');
                break;
            case e_change:
                type.push_back('c');
                break;
            case e_none:
                type.push_back(' ');
                break;
            }
        }
        _db_file->put_diff(_file_id, log[j].revision(), s1, s2, d1, d2, type);
    }
    return pdiff;
}

diff *
backward_map_algorithm::read_diff(const cvs_log & log, unsigned int j)
{
    diff * pdiff = 0;
    
    if (_db_file) 
    {
        vector<unsigned int> s1;
        vector<unsigned int> s2;
        vector<unsigned int> d1;
        vector<unsigned int> d2;
        vector<char> type;

        if (_db_file->get_diff(_file_id, log[j].revision(), s1, s2, d1, d2, type) == 0)
        {
            pdiff = new diff();
            if (pdiff) 
            {
                for (unsigned int i = 0; i < s1.size(); ++i) 
                {
                    pdiff->add(diff_entry(s1[i],s2[i],d1[i],d2[i],type[i]));
                }
                pdiff->read_status(true);
            }
        }
    }

    return pdiff;
}

diff *
backward_map_algorithm::get_diff(const cvs_log & log, unsigned int j)
{
    diff * pdiff = 0;
    if ((pdiff = read_diff(log, j)) != 0) {
        return pdiff;
    } else {
        return save_diff(log, j);
    }
}

diff *
backward_map_algorithm::calc_diff(const cvs_log & log, unsigned int j) 
{
    diff * pdiff = 0;
    if (j == log.size()-1)
    {
        unsigned int length = get_length(log, j);
        if (length > 0)
        {
            ostrstream ost2;
            ost2 << "1," << length << "d0" << endl << ends;
            istrstream ist(ost2.str());
            pdiff = new diff();
            if (pdiff) {
                ist >> *pdiff;
            }
        }
    }
    else {
        ostrstream ost;
        ost << scvs_diff
            << "-r" << log[j].revision()   << " " 
            << "-r" << log[j+1].revision() << " "
            << log.file_name() << ends;
        
        process p(ost.str());
        istream *pis = p.process_output();
        if (*pis)
        {    
            pdiff = new aligned_diff();
            if (pdiff) {
                *pis >> *pdiff;
            }
        }
    }
    return pdiff;
}
