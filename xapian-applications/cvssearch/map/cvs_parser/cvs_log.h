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
 *  Usage:
 *
 *  suppose cin contain output from a cvs log.
 *  cvs_log log;
 *  cin >> log;
 *
 *  // l should be ready to use.
 *
 *  cout << log;
 *
 *  $Id$
 *
 ************************************************************/

#ifndef __CVS_LOG_H__
#define __CVS_LOG_H__

#include "collection.h"
#include "virtual_iostream.h"
#include "cvs_log_entry.h"

/**
 * cvs_log is a class that holds a cvs log result.
 **/
class cvs_log : public collection<cvs_log_entry>, public virtual_iostream
{
private:
    string _filename;
    string _pathname;
protected:
    /**
     * reads the log from an input stream.
     * 
     * @param is the input stream.
     * @return the input stream.
     **/
    istream & read(istream & is);

    /**
     * prints the log.
     *
     * @param os the output stream.
     * @return theoutput stream.
     **/
    ostream & show(ostream & os) const;
public:
    /**
     * gets the Working File entry found in the log.
     * @return the Working File entry.
     **/
    string file_name() const {return _filename;}

    /**
     * gets the RCS File entry found in the log.
     * @return the RCS File entry relative to the $CVSROOT directory.
     **/
    string path_name() const {return _pathname;}
};

#endif
