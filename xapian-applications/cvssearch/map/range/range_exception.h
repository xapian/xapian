/************************************************************
 *
 *  range_exception.h is a class stores an error messages and can be
 *  displayed.
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

#ifndef __RANGE_EXCEPTION_H__
#define __RANGE_EXCEPTION_H__

#include <exception>
#include "virtual_ostream.h"
#include <string>
using std::string;
using std::exception;

class range;

/**
 * use to show exceptions related to range.
 **/
class range_exception : public exception, public virtual_ostream
{
private:
    string _error;
protected:
    virtual ostream & show(ostream &) const;
public:
    /**
     * constructor.
     * creates an exception based on a range.
     **/
    range_exception(const range &);

    /**
     * constructor.
     * creates an exception based on the begin and end values.
     **/
    range_exception(unsigned int begin, unsigned int end);

    ~range_exception() throw() {}
};

#endif
