/************************************************************
 *
 *  range_end_less_than is a comparator for sorting based on
 *  the end value.
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

#ifndef __RANGE_END_LESS_THAN_H__
#define __RANGE_END_LESS_THAN_H__

#include <functional>
#include "range_map.h"

class range_end_less_than : public binary_function<range_map *, range_map *, bool> 
{
public:
    bool operator() (const range_map * pr1, const range_map * pr2) const { 
        if (pr1->end() == pr2->end())
        {
            if (pr1->begin() == pr2->begin())
            {
                return (int) pr1 < (int) pr2;
            } else {
                return pr1->begin() < pr2->begin();
            }
        } else {
            return pr1->end() < pr2->end();
        }
    }
};

#endif
