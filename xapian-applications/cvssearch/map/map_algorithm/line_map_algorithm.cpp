/************************************************************
 *
 *  line_map_algorithm.cpp implementation.
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

#include "line_map_algorithm.h"
using namespace std;

ostream &
line_map_algorithm::show(ostream & os) const
{
    for (unsigned int i = 1; i < _line_maps.size(); ++i)
    {
        os << _index << _line_maps[i] << "\003" << "\002" << endl;
    }
    return os;
}

unsigned int
line_map_algorithm::mappings() const
{
    unsigned int sum = 0;
    for (unsigned int i = 1; i < _line_maps.size(); ++i)
    {
        sum += _line_maps[i].size();
    }
    return sum;
}
