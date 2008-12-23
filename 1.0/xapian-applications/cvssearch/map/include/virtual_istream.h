/************************************************************
 *
 *  virtual_istream.h allow derived classes to be read from
 *  input streams.
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
 *
 *  Usage:
 *
 *  any derived class must override the virtual functions
 *  read(istream &)
 * 
 *  the following is possible if class someobject derives
 *  virtual_iostream:
 *
 *  some_object a;
 *  cin >> a;
 *  ...
 *
 *  $Id$
 *
 ************************************************************/

#ifndef __VIRTUAL_ISTREAM_H__
#define __VIRTUAL_ISTREAM_H__

#include <iostream>
using std::istream;
using std::ostream;

/**
 * allows derived class to read from input streams.
 **/
class virtual_istream
{
private:
    bool _read;
protected:
    /**
     * overwrite this virtual function to allow derived class to read from an input stream.
     * @param is the input stream.
     **/
    virtual istream & read(istream & is) = 0;
public:
    virtual_istream() : _read (false) {}
    virtual ~virtual_istream() {}
    friend  istream & operator >> (istream & is , virtual_istream & r) { return r.read(is); }
    
    /**
     * gets the reading status, the value is set by read_status(bool);
     **/
    bool read_status() const { return _read; }

    /**
     * read(is) should call this function to signal reading is successful.
     **/
    bool read_status(bool b) { return _read = b;}
};

#endif
