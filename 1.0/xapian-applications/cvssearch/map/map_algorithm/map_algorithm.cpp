/************************************************************
 *
 *  map_algorithm.cpp implementation.
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

#include "map_algorithm.h"

istream &
map_algorithm::read(istream & is)
{
    cvs_log log;
    is >> log;
    if (log.read_status())
    {
        parse_log(log);
    }
    return is;
}

void
map_algorithm::parse_diff(const cvs_log_entry & log_entry1, const cvs_log_entry & log_entry2, const diff & diff)
{
    for (unsigned int j = 0; j < diff.size(); ++j)
    {
        parse_diff_entry(log_entry1, diff[j]);
    }
}


map_algorithm::map_algorithm()
    :_updates(0),
     _deletes(0),
     _searches(0)
{
}
