/************************************************************
 *
 *  backward_map_algorithm.h does line profiling 
 *  and do it backwards from the most recent to the earliest.
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

#ifndef __BACKWARD_MAP_ALGORITHM_H__
#define __BACKWARD_MAP_ALGORITHM_H__

#include "cvs_log.h"
#include "map_algorithm.h"
#include "cvs_db_file.h"

class backward_map_algorithm : public map_algorithm
{
protected:
    cvs_db_file * _db_file;
    unsigned int _file_id;
    unsigned int _lines;
    virtual void parse_log(const cvs_log & log);
    virtual void init(const cvs_log_entry &, unsigned int) = 0;
    virtual void last(const cvs_log_entry &, unsigned int) = 0;
    diff * get_diff (const cvs_log & log, unsigned int j);
    diff * save_diff(const cvs_log & log, unsigned int j);
    diff * calc_diff(const cvs_log & log, unsigned int j);
    diff * read_diff(const cvs_log & log, unsigned int j);

public:
    unsigned int lines() const { return _lines;}
    backward_map_algorithm(unsigned int file_id, cvs_db_file * db_file) : _db_file(db_file), _file_id(file_id) {}
};

#endif
