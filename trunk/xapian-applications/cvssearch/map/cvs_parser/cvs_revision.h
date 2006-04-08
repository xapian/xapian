/************************************************************
 *
 *  cvs_revision.h holds the name of a revision (e.g. 1.1)
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

#ifndef __CVS_REVISION_H__
#define __CVS_REVISION_H__

#include "virtual_iostream.h"
#include <string>
using std::string;

class cvs_revision : public virtual_iostream
{
private:
    string _revision;
protected:
    istream & read(istream &);
    ostream & show(ostream &) const;
public:
    operator string() const { return _revision;}
};

#endif
