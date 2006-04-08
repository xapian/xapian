/************************************************************
 *
 *  cvs_revision.cpp implementation.
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

#include "cvs_revision.h"
#include "cvs_output.h"

/**
 * tries to find the line signaling a revision.
 *
 * @param is input stream
 * @return input stream at the end of the stream, or at just
 * after the line starting with "revision "
 */
istream & 
cvs_revision::read(istream & is)
{
    string line;
    while (getline(is, line))
    {
        if (line.find(cvs_output::cvs_log_revision_tag) == 0)
        {
            _revision = line.substr(cvs_output::cvs_log_revision_tag.length());
            break;
        }
    }
    return is;
}


ostream & 
cvs_revision::show(ostream & os) const
{
    os << _revision;
    return os;
}
