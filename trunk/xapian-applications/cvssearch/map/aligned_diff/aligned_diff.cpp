/************************************************************
 *
 *  aligned_diff.cpp aligned_diff implementation.
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

#include "aligned_diff.h"
#include "alignment.h"
#include "line_sequence.h"

istream &
aligned_diff::read(istream & is)
{
    // ----------------------------------------
    // read each diff entry
    // ----------------------------------------
    while (is)
    {
        diff_entry entry;
        is >> entry;
        if (entry.read_status())
        {
            if (entry.type() == e_change)
            {
                line_sequence l1(entry.source_line());
                line_sequence l2(entry.dest_line());

                alignment<line_sequence> alignment_o( l1, l2, entry.source().begin()-1, entry.dest().begin()-1);
                alignment_o.find_optimal_alignment();

                const list<diff_entry> & aligned_entries = alignment_o.entries();
                list<diff_entry>::const_iterator itr;
                for (itr = aligned_entries.begin();
                     itr!= aligned_entries.end();
                     ++itr)
                {
                    _entries.push_back(*itr);
                    read_status(true);
                }
            }
            else
            {
                _entries.push_back(entry);
                read_status(true);
            }
        }
        else
        {
            break;
        }
    }

     return is;
}
