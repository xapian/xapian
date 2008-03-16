/************************************************************
 *
 *  range.cpp implementation.
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

#include <assert.h>
#include "range.h"

static unsigned int min(unsigned int x, unsigned int y)
{
    return (x < y) ? x : y;
}

static unsigned int max(unsigned int x, unsigned int y)
{
    return (x < y) ? y : x;
}

range::range(unsigned int first_line, unsigned int last_line) throw (range_exception) 
    : _begin(first_line),
      _end(last_line+1),
      _half_changed(false)

{
    assert(_begin <= _end);
    if (_begin > _end) 
    {
        throw range_exception(_begin, _end); 
    }
}

range::range(unsigned int single_line)
    : _begin(single_line), 
    _end(single_line+1),
    _half_changed(false)
{
}

range::~range()
{
}

unsigned int
range::begin(unsigned int new_begin, bool valid)  throw (range_exception)
{
    if (!valid || new_begin <= _end)
    {
        return _begin = new_begin;
    }
    throw range_exception(new_begin, _end);
}

unsigned int 
range::end(unsigned int new_end, bool valid) throw (range_exception)
{
    assert(!valid || _begin <= new_end);
    if (!valid || new_end >= _begin)
    {
        return _end = new_end;
    }
    throw range_exception(_begin, new_end);
}


unsigned int
range::begin_shift(int offset) throw (range_exception)
{
    assert(_begin + offset <= _end &&
           ((int) _begin + offset) >= 0);

    if (_begin + offset <= _end && _begin + offset >= 0)
    {
        return _begin += offset;
    }
    throw range_exception(_begin + offset, _end);
}

unsigned int
range::end_shift(int offset) throw (range_exception)
{
    assert(_begin <= _end + offset);
    if (_end + offset >= _begin)
    {
        return _end += offset;
    }
    throw range_exception(_begin, _end+offset);
}

range &
range::operator +=(int offset) throw (range_exception)
{
    assert (_begin + offset >= 0);
    if (_begin + offset >= 0)
    {
        _begin += offset;
        _end += offset;
        return *this;
    }
    throw range_exception(_begin+offset, _end);
}

unsigned int
range::size() const 
{
    return (unsigned int) (_end - _begin);
}

ostream &
operator<< (ostream & os, const range & c)
{
    if (c.begin() + 1 >= c.end())
    {
        os << c.begin();
    } else {
        os << c.begin() << "," << c.end() - 1;
    }
    return os;
}

bool
range::intersect(const range & r) const
{
    if (_end <= r.begin() ||
        r.end() <= _begin)
    {
        return false;
    }
    return true;
}

range
range::union_range(const range & r) const
{
    return range(min(r.begin(), _begin), max(r.end(), _end));
}

range
range::intersect_range(const range &r) const
{
    assert(intersect(r));
    return range(max(r.begin(), _begin), min(r.end(), _end));
}

range
range::operator-(const range & r)
{
    if (intersect(r))
    {
        range intersect = intersect_range(r);
        
        if (_begin < intersect.begin() && _end > intersect.end())
        {
            (*this) = range(_begin, intersect.begin()-1);
            return range(intersect.end(), _end-1);
        }
        else if (_begin < intersect.begin())
        {
            (*this) = range(_begin, intersect.begin()-1);
        }
        else if (_end > intersect.end())
        {
            (*this) = range(intersect.end(), _end-1);
        }
    }
    return range(0,(unsigned int)-1);
}
    
bool
range::operator==(const range & r) const
{
    return (r.begin() == _begin &&
            r.end()   == _end);
}
