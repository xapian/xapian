/************************************************************
 *
 *  range_map_algorithm.h specify those methods relates to the range method.
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

#ifndef __RANGE_MAP_ALGORITHM_H__
#define __RANGE_MAP_ALGORITHM_H__

#include "virtual_ostream.h"
#include "cvs_log.h"
#include "cvs_log_entry.h"
#include "diff_entry.h"
#include "diff.h"
#include "diff_entry.h"
#include "range_begin_less_than.h"
#include "range_end_less_than.h"
#include "map_algorithm.h"

#include <vector>
#include <set>
#include <iterator>
#include <list>
#include <string>
using std::set;
using std::vector;
using std::list;
using std::string;

typedef multiset<range_map *, range_begin_less_than> range_map_begin_set;
typedef multiset<range_map *, range_end_less_than> range_map_end_set;

class range_map_algorithm : public virtual_ostream
{
protected:
    unsigned int _index;

    multiset<range_map *, range_begin_less_than> _begin_entries;
    multiset<range_map *, range_end_less_than> _end_entries;

    ostream & show(ostream &) const;

    static range_map * split(range_map *, unsigned int pos);
    static void clear_and_print(ostream & os, list<range_map *> & temp, unsigned int file_index, unsigned int line_index);

public:
    range_map_algorithm(unsigned int index) : _index(index) {}
    virtual ~range_map_algorithm();
    unsigned int lines()    const;
    unsigned int size()     const  { return _begin_entries.size(); }
    unsigned int mappings() const;
};

#endif
