/************************************************************
 *
 *  diff.cpp implementation.
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


#include "diff.h"
#include <algorithm>

istream & 
diff::read(istream & is)
{
    // ----------------------------------------
    // read each diff entry
    // ----------------------------------------
    while (is)
    {
        diff_entry entry;
        is >> entry;
        if (entry.read_status())
        {
            _entries.push_back(entry);
            read_status(true);
        }
        else
        {
            break;
        }
    }
    return is;
}

ostream & 
diff::show(ostream & os) const
{
    copy (_entries.begin(), _entries.end(), 
          ostream_iterator<diff_entry>(os, "\n"));
    return os;
}

diff::~diff() 
{
}
