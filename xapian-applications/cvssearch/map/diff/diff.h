/************************************************************
 *
 *  diff.h holds a set of differences between two files.
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

#ifndef __DIFF_H__
#define __DIFF_H__

#include "collection.h"
#include "virtual_iostream.h"
#include "diff_entry.h"

class diff : public collection<diff_entry>, public virtual_iostream
{
protected:
    bool _read_content;
    bool _aligned;
    virtual istream & read(istream &);
    virtual ostream & show(ostream &) const;

public:
    virtual ~diff();
    diff (bool read_content = true) : _read_content(read_content), _aligned(false) {}
    bool operator==(const diff & r) const;
    void align_top();
    void unalign_top();
};
#endif
