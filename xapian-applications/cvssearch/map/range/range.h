/************************************************************
 *
 * A range can be either
 * a single line:      [line, line+1)
 * a range of lines:   [begin, end)
 * an insertion point: [line, line)
 *
 * class invariant: _begin <= _end
 * 
 * $Id$
 *
 ************************************************************/

#ifndef __RANGE_H__
#define __RANGE_H__

#include "range_exception.h"

class range
{
private:
    unsigned int _begin;
    unsigned int _end;
    bool _half_changed;
    
public:
    range(unsigned int first_line, unsigned int last_line) throw (range_exception);
    range(unsigned int single_line = 0);

    virtual ~range();

    unsigned int begin() const { return _begin;}
    unsigned int end()   const { return _end;}
    unsigned int begin(unsigned int, bool valid = true) throw (range_exception);
    unsigned int end  (unsigned int, bool valid = true) throw (range_exception);

    unsigned int begin_shift(int offset)                throw (range_exception);
    unsigned int end_shift  (int offset)                throw (range_exception);

    range & operator+=  (int offset) throw (range_exception);
    unsigned int size() const;

    friend ostream & operator<< (ostream &, const range &);

    bool intersect(const range &) const;

    range intersect_range(const range &) const;
    range union_range(const range &) const;

    range operator-(const range &);
    bool operator==(const range &) const;
};
#endif
