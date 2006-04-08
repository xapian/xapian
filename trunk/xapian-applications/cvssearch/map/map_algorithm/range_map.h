/************************************************************
 *
 *  range_map.h maps a range to a log entry.
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

#ifndef __RANGE_MAP_H__
#define __RANGE_MAP_H__

#include "range.h"
#include "virtual_iostream.h"
#include "cvs_log_entry.h"

class range_map : public virtual_ostream
{
private:
    const cvs_log_entry & _log_entry;
    range _range;
protected:
    ostream & show(ostream &) const;
public:
    range_map(const cvs_log_entry &, const range &);
    const range & source() const { return _range;}
          range & source()       { return _range;}
    const cvs_log_entry & log_entry() const { return _log_entry; }
    unsigned int begin() const { return _range.begin(); }
    unsigned int end()   const { return _range.end(); }
};
#endif
