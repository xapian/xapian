/************************************************************
 *
 * virtual_iostream is an interface for IO manipulation of 
 * derived objects from/to streams.
 *
 * any derived class must override the virtual functions
 * read(istream &) and show(ostream &).
 * 
 * e.g. 
 * the following is possible if class someobject derives
 * virtual_iostream:
 *
 *   some_object a;
 *   cin >> a;
 *   ...
 *   cout << a;
 *
 * $Id$
 *
 ************************************************************/

#ifndef __VIRTUAL_IOSTREAM_H__
#define __VIRTUAL_IOSTREAM_H__

#include "virtual_istream.h"
#include "virtual_ostream.h"

class virtual_iostream : public virtual_istream, public virtual_ostream
{
};

#endif
