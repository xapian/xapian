/************************************************************
 *
 * range_exception implementation.
 * 
 * $Id$
 *
 ************************************************************/

#include "range_exception.h"
#include "range.h"
#include <strstream>

range_exception::range_exception(const range & r)
{
    range_exception(r.begin(), r.end());
}

range_exception::range_exception(unsigned int begin, unsigned int end)
{
    ostrstream s;
    s << "[" << begin << "," << end << ")" << ends;
    _error = s.str();
}

/*
 * virtual_iostream derived funtions
 */
istream & 
range_exception::read(istream & is)
{
    return is >> _error;
}

ostream & 
range_exception::show(ostream & os) const
{
    return os << "Exception: " << _error << " occurred" << endl;
}
