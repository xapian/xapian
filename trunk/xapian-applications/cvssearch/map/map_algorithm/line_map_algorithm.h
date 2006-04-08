/************************************************************
 *
 *  line_map_algorithm.h specify those methods relates to the line method.
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

#ifndef __LINE_MAP_ALGORITHM_H__
#define __LINE_MAP_ALGORITHM_H__

#include "virtual_ostream.h"
#include "line_map.h"
#include <vector>
using std::vector;

class line_map_algorithm : public virtual_ostream
{
protected:
    unsigned int _index;
    vector <line_map> _line_maps;
    ostream & show(ostream &) const;
public:
    line_map_algorithm(unsigned int index) : _index(index) {}
    virtual ~line_map_algorithm() {}
    unsigned int lines() const { return _line_maps.size()-1; }
    unsigned int size()  const { return _line_maps.size()-1; }
    unsigned int mappings() const;
};

#endif
