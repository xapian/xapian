/************************************************************
 *
 *  forward_map_algorithm.cpp implementation.
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
 
#include "forward_map_algorithm.h"
#include <strstream>
#include "process.h"
#include "aligned_diff.h"

extern string scvs_diff;
extern string scvs_update;

void
forward_map_algorithm::parse_log(const cvs_log & log)
{
    for (unsigned int j = log.size(); j >= 1; --j)
    {
        ostrstream ost;
        if (j <= log.size() -1)
        {
            ost << scvs_diff
                << "-r" << log[j].revision()   << " " 
                << "-r" << log[j-1].revision() << " "
                << log.file_name() << ends;
            
            process * p;
            if ((p = new process(ost.str())) != 0)
            {
                istream *pis = p->process_output();
                if (*pis)
                {    
                    aligned_diff diff_output;
                    *pis >> diff_output;
                    diff_output.align_top();
                    parse_diff(log[j-1], log[j], diff_output);
                }
                delete p;
            }
            ost.freeze(0);
        }
        else
        {
            ost << scvs_update << "-r" << log[j-1].revision() 
                << " " << log.file_name() << " 2>/dev/null|wc -l" << ends;
            process * p;
            if ((p = new process(ost.str())) != 0)
            {
                istream *pis = p->process_output();
                if (*pis)
                {
                    unsigned int length;
                    *pis >> length;
                    ostrstream ost2;
                    if (length > 0)
                    {
                        ost2 << "0a1," << length << endl << ends;
                        diff diff_output;
                        istrstream ist(ost2.str());
                        ost2.freeze(0);
                        ist >> diff_output;
                        parse_diff(log[j-1], log[j], diff_output);
                    }
                }
                delete p;
            }
        }
    }
}

