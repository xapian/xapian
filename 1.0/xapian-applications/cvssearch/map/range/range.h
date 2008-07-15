/************************************************************
 *
 *  range.h is a class for storing a range [b,e)
 *  A range can be either
 *  a single line:      [line, line+1)
 *  a range of lines:   [begin, end)
 *  an insertion point: [line, line)
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

#ifndef __RANGE_H__
#define __RANGE_H__

#include "range_exception.h"

/**
 * a class stores a range [begin, end) where begin <= end
 **/
class range
{
private:
    unsigned int _begin;
    unsigned int _end;
    bool _half_changed;
    
public:

    /**
     * constructor.
     *
     * e.g. 4,5 is stored as [_begin = 4, _end = 6).
     **/
    range(unsigned int first_line, unsigned int last_line) throw (range_exception);

    /**
     * constructor.
     *
     * e.g. 3 is stored as [_begin = 3, _end = 4).
     **/
    range(unsigned int single_line = 0);

    /**
     * destructor.
     **/
    virtual ~range();

    /**
     * gets the begin value.
     **/
    unsigned int begin() const { return _begin;}

    /**
     * gets the end value.
     **/
    unsigned int end()   const { return _end;}

    /**
     * sets the begin value.
     * @valid is flag to whether to always keep _begin <= _end.
     **/
    unsigned int begin(unsigned int, bool valid = true) throw (range_exception);

    /**
     * sets the end value.
     * @valid is flag to whether to always keep _begin <= _end.
     **/
    unsigned int end  (unsigned int, bool valid = true) throw (range_exception);

    /**
     * adds offset to begin.
     **/
    unsigned int begin_shift(int offset)                throw (range_exception);

    /**
     * adds offset to end.
     **/
    unsigned int end_shift  (int offset)                throw (range_exception);

    /**
     * adds offset to both begin and end.
     **/
    range & operator+=  (int offset) throw (range_exception);

    /**
     * @return end - begin. must always be unsigned.
     **/
    unsigned int size() const;

    friend ostream & operator<< (ostream &, const range &);

    /**
     * whether two ranges intersect or not.
     **/
    bool intersect(const range &) const;

    /**
     * @return the intersection of two range.
     * precondition: two ranges intersect.
     **/
    range intersect_range(const range &) const;

    /**
     * @return the union of two ranges.
     **/
    range union_range(const range &) const;

    /**
     * some complicated, not used.
     **/
    range operator-(const range &);

    /**
     * @return true if two ranges are equal.
     **/
    bool operator==(const range &) const;

    bool operator!=(const range &r) const {
	return !this->operator==(r);
    }
};
#endif
