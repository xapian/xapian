/************************************************************
 *
 *  backward_line_map_algorithm.h does line profiling using the
 *  line method and do it backwards from the most recent to the earliest.
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

#ifndef __BACKWARD_LINE_MAP_ALGORITHM_H_
#define __BACKWARD_LINE_MAP_ALGORITHM_H_

#include "backward_map_algorithm.h"
#include "line_map_algorithm.h"
#include <vector>
using std::vector;
#include <set>
using std::set;

class backward_line_map_algorithm : public backward_map_algorithm, public line_map_algorithm
{
protected:
    const cvs_log & _log;
    const string & _filename;
    const string & _pathname;

    vector <unsigned int> _contents;
    unsigned int _current_index;
    virtual void parse_diff_entry(const cvs_log_entry & log_entry, const diff_entry & diff_entry);
    virtual void parse_diff(const cvs_log_entry & log_entry1, const cvs_log_entry & log_entry2, const diff & diff);
    virtual void init(const cvs_log_entry & log_entry, unsigned int);
    virtual void last(const cvs_log_entry & log_entry, unsigned int);
    ostream & show(ostream & os) const { return line_map_algorithm::show(os);}
public:
    backward_line_map_algorithm(const cvs_log & log, unsigned int index, cvs_db_file * db_file = 0);
    unsigned int size() const     { return line_map_algorithm::size();}
    unsigned int lines() const    { return line_map_algorithm::lines();}
    unsigned int mappings() const { return line_map_algorithm::mappings();}

    void get_line_mappings(cvs_db_file & db_file) const;
};

#endif
