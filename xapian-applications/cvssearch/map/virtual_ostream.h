/************************************************************
 *
 * virtual_ostream is an interface for IO manipulation of 
 * derived objects to streams.
 *
 * any derived class must override the virtual functions
 * ostream & show(ostream &) const.
 * 
 * e.g. 
 * the following is possible if class someobject derives
 * virtual_iostream:
 *
 *   some_object a;
 *   ...
 *   cout << a;
 *
 * $Id$
 *
 ************************************************************/

#ifndef __VIRTUAL_OSTREAM_H__
#define __VIRTUAL_OSTREAM_H__

#include <iostream>
using std::istream;
using std::ostream;

class virtual_ostream
{
protected:
    virtual ostream & show(ostream &) const = 0;
public:
    virtual ~virtual_ostream() {}
    friend  ostream & operator << (ostream & os , const virtual_ostream & r) { return r.show(os); }
};

#endif
