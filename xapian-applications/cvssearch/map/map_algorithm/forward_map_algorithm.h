/************************************************************
 *
 *  forward_map_algorithm.h does line profiling 
 *  and do it forwards from the earliest to the most recent.
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

#ifndef __FORWARD_MAP_ALGORITHM_H__
#define __FORWARD_MAP_ALGORITHM_H__

#include "cvs_log.h"
#include "map_algorithm.h"

class forward_map_algorithm : public map_algorithm
{
protected:
    virtual void parse_log(const cvs_log & log);
    
public:
    virtual ~forward_map_algorithm() {}
};

#endif
