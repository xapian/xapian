/************************************************************
 *
 *  line_map.h maps a line to a set of log entries.
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

#ifndef __LINE_MAP_H__
#define __LINE_MAP_H__

#include "virtual_ostream.h"
#include "cvs_log_entry.h"
#include <vector>
#include <assert.h>
using std::vector;

class line_map : public virtual_ostream
{
protected:
    vector<const cvs_log_entry *> _entries;
    unsigned int _index;
    ostream & show(ostream &) const;
public:
    line_map(unsigned int i) : _index(i) {}
    virtual ~line_map() {}
    unsigned int line() const { return _index;}
    unsigned int size() const { return _entries.size(); }
    void add_log_entry(const cvs_log_entry & e) { _entries.push_back(&e);}

    const cvs_log_entry & operator[](unsigned int i) const { assert (i < size()); return *(_entries[i]); }
};

#endif
