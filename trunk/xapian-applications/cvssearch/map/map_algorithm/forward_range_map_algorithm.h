/************************************************************
 *
 *  forward_range_map_algorithm.h does line profiling using the
 *  range method and do it forwards from the earliest to the most recent.
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

#ifndef __FORWARD_RANGE_MAP_ALGORITHM_H__
#define __FORWARD_RANGE_MAP_ALGORITHM_H__

#include "range_map_algorithm.h"
#include "forward_map_algorithm.h"
#include "cvs_db_file.h"

class forward_range_map_algorithm : public forward_map_algorithm, virtual public range_map_algorithm
{
protected:
    cvs_db_file * _db_file;
    const cvs_log & _log;

    virtual void parse_diff_entry(const cvs_log_entry & log_entry, const diff_entry & diff_entry);
    ostream & show(ostream & os) const { return range_map_algorithm::show(os);}

public:
    forward_range_map_algorithm(const cvs_log &, unsigned int index, cvs_db_file * db_file = 0);
    virtual ~forward_range_map_algorithm() {}
    unsigned int size()     const { return range_map_algorithm::size(); }
    unsigned int lines()    const { return range_map_algorithm::lines();}
    unsigned int mappings() const { return range_map_algorithm::mappings();}

    void get_line_mappings(cvs_db_file & db_file) const;
};

#endif
