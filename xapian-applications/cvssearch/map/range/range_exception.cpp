/************************************************************
 *
 *  range_exception.cpp implementation.
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

#include "range_exception.h"
#include "range.h"
#include <strstream>
using namespace std;

range_exception::range_exception(const range & r)
{
    range_exception(r.begin(), r.end());
}

range_exception::range_exception(unsigned int begin, unsigned int end)
{
    ostrstream s;
    s << "[" << begin << "," << end << ")" << ends;
    _error = s.str();
    s.freeze(0);
}

ostream & 
range_exception::show(ostream & os) const
{
    return os << "Exception: " << _error << " occurred" << endl;
}
