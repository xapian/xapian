/************************************************************
 *
 *  code_to_html.cpp implementation.
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

#include "code_to_html.h"

ostream &
code_to_html::show(ostream & os) const
{
    unsigned int offset = 0;
    for (unsigned int i = 0; i < _line.length() && i + offset < _width; ++i)
    {
        if (0) {
        } else if (_line[i] == '<') {
            os << "&lt;";
        } else if (_line[i] == '\t') {
            os << "    ";
            if (offset + 3 < _width) {
                offset += 3;
            }
        } else {
            os << _line[i];
        }
    }

    if (_line.length() + offset > _width)
    {
        os << " ...";
    }
    return os;
}
