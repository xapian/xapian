/************************************************************
 *
 *  diff.h holds one difference between two files.
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

#ifndef __DIFF_ENTRY_H__
#define __DIFF_ENTRY_H__

#include "virtual_iostream.h"
#include "range.h"
#include <vector>
using std::vector;

enum diff_type {e_add = 'a', e_change = 'c', e_delete = 'd', e_none = ' ' };

class diff_entry : public virtual_iostream
{
private:
    range _src;
    range _dst;
    diff_type _type;

    vector<string> _src_lines;
    vector<string> _dst_lines;

protected:
    virtual istream & read(istream &);
    virtual ostream & show(ostream &) const;
    virtual void init(unsigned int s1, unsigned int s2, unsigned int d1, unsigned d2, diff_type type);

public:
    diff_entry::diff_entry();
    diff_entry::diff_entry(unsigned int s1, unsigned int s2, unsigned int d1, unsigned d2, diff_type type);

    virtual ~diff_entry();
    range &     source() { return _src;}
    range &     dest()   { return _dst;}
    diff_type & type()   { return _type;}

    const range &     source() const { return _src;}
    const range &     dest()   const { return _dst;}
    const diff_type & type()   const { return _type;}

    int size() const;

    const vector<string> & source_line() const { return _src_lines; }
    const vector<string> & dest_line() const   { return _dst_lines; }
};
#endif
