/************************************************************
 *
 *  cvs_log.h is a class that holds a cvs log result.
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

#ifndef __CVS_LOG_H__
#define __CVS_LOG_H__

#include "collection.h"
#include "virtual_iostream.h"
#include "cvs_log_entry.h"

class cvs_log : public collection<cvs_log_entry>, public virtual_iostream
{
private:
    string _filename;
    string _pathname;
protected:
    istream & read(istream &);
    ostream & show(ostream &) const;
public:
    string file_name() const {return _filename;}
    string path_name() const {return _pathname;}
};

#endif
