/************************************************************
 *
 *  map_algorithm.h is the base class for cvs mapping algorithms.
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

#ifndef __MAP_ALGORITHM_H__
#define __MAP_ALGORITHM_H__

#include "cvs_log.h"
#include "diff.h"
#include "virtual_iostream.h"

#include "cvs_db_file.h"

class map_algorithm : public virtual_iostream
{
protected:
    istream & read(istream &);
    virtual void parse_diff(const cvs_log_entry & log_entry1, const cvs_log_entry & log_entry2, const diff &);
    virtual void parse_log(const cvs_log &) = 0;
    virtual void parse_diff_entry(const cvs_log_entry &, const diff_entry &) = 0;
    
    unsigned int _updates;
    unsigned int _deletes;
    unsigned int _searches;

public:
    virtual ~map_algorithm () {}
    map_algorithm();
    virtual unsigned int lines() const = 0;
    virtual unsigned int size()  const = 0;
    virtual unsigned int mappings()  const = 0;
    
    unsigned int updates()  const { return _updates;}
    unsigned int deletes()  const { return _deletes;}
    unsigned int searches() const { return _searches;}
};

#endif
