/************************************************************
 *
 * exception stores an error messages and can be
 * displayed.
 * 
 * $Id$
 *
 ************************************************************/

#ifndef __RANGE_EXCEPTION_H__
#define __RANGE_EXCEPTION_H__

#include <exception>
#include "virtual_iostream.h"
#include <string>
using std::string;

class range;
class range_exception : public exception, public virtual_iostream
{
private:
    string _error;
protected:
    virtual istream & read(istream &);
    virtual ostream & show(ostream &) const;
public:
    range_exception(const range &);
    range_exception(unsigned int begin, unsigned int end);
};

#endif
