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

extern string scvs_update;
extern string scvs_diff;

static unsigned int get_length(const cvs_log & log, unsigned int j)
{
    ostrstream ost;
    ost << scvs_update << "-r" << log[j].revision() 
        << " " << log.file_name() << " 2>/dev/null|wc -l" << ends;
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

        if (j == log.size()-1)
        {
            unsigned int length = get_length(log, j);
            if (length > 0)
            {
                ostrstream ost2;
                ost2 << "1," << length << "d0" << endl << ends;
                istrstream ist(ost2.str());
                diff diff_output;
                ist >> diff_output;

                if (diff_output.read_status())
                {
                    parse_diff(log[j], log[j], diff_output);
                }
            }
        }
        else
        {
            ost << scvs_diff
                << "-r" << log[j].revision()   << " " 
                << "-r" << log[j+1].revision() << " "
                << log.file_name() << ends;
            
            process p(ost.str());
            istream *pis = p.process_output();
            if (*pis)
            {    
                aligned_diff diff_output;
                *pis >> diff_output;
                parse_diff(log[j], log[j+1], diff_output);

            }
        }
    }
}

