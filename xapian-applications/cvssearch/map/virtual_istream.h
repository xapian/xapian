/************************************************************
 *
 * virtual_istream is an interface for IO manipulation of 
 * derived objects from streams.
 *
 * any derived class must override the virtual functions
 * read(istream &)
 * 
 * e.g. 
 * the following is possible if class someobject derives
 * virtual_iostream:
 *
 *   some_object a;
 *   cin >> a;
 *   ...
 *
 * $Id$
 *
 ************************************************************/

#ifndef __VIRTUAL_ISTREAM_H__
#define __VIRTUAL_ISTREAM_H__

#include <iostream>
using std::istream;
using std::ostream;

class virtual_istream
{
private:
    bool _read;
protected:
    virtual istream & read(istream &) = 0;
public:
    virtual_istream() : _read (false) {}
    virtual ~virtual_istream() {}
    friend  istream & operator >> (istream & is , virtual_istream & r) { return r.read(is); }
    
    bool read_status() const { return _read; }
    bool read_status(bool b) { return _read = b;}
};

#endif
