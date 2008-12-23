/************************************************************
 *
 *  diff.h holds one difference between two files.
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

#ifndef __DIFF_ENTRY_H__
#define __DIFF_ENTRY_H__

#include "virtual_iostream.h"
#include "range.h"
#include <vector>
using std::vector;

/**
 * all types of difference.
 * e_none is an invalid type.
 **/
enum diff_type {e_add = 'a', e_change = 'c', e_delete = 'd', e_none = ' ' };

/**
 * diff_entry stores one diff result "s1,s2td1,d2".
 **/
class diff_entry : public virtual_iostream
{
private:
    range _src;
    range _dst;
    diff_type _type;
    bool _read_content;

    vector<string> _src_lines;
    vector<string> _dst_lines;

protected:
    virtual istream & read(istream &);
    virtual ostream & show(ostream &) const;
    virtual void init(unsigned int s1, unsigned int s2, unsigned int d1, unsigned d2, diff_type type);

public:
    /**
     * constructor.
     * allows a diff entry to be read later on.
     * 
     * @param read_content is a flag to specify whether
     * the input we want to parse contain actual diff content or not
     * ie.. lines begin with '>' '<', '-', if false,
     * the input is assumed to contain only lines of
     * "s1,s2td1,d2". 
     * this flag is extremely important and mustn't set to be wrong.
     **/
    diff_entry::diff_entry(bool read_content = true);

    /**
     * constructor.
     * creates an entry based on source/destination values.
     * these values correspond to what we see in a diff output.
     **/
    diff_entry::diff_entry(unsigned int s1, unsigned int s2, unsigned int d1, unsigned d2, diff_type type, bool read_content = true);

    /**
     * constructor.
     * creates an entry based on source/destination values.
     * these values correspond to what we see in a diff output.
     **/
    diff_entry::diff_entry(unsigned int s1, unsigned int s2, unsigned int d1, unsigned d2, char type, bool read_content = true);

    virtual ~diff_entry();

    /**
     * gets the source range.
     *
     * @return the reference to the source range.
     *
     * 3a4,5's source is [4,4);
     * 4,5d3's source is [4,6);
     * 3,4c5,6 source is [3,4);
     **/
    range &     source() { return _src;}

    /**
     * gets the dest destination.
     *
     * @return the reference to the dest range.
     *
     * 3a4,5's dest is [4,6);
     * 4,5d3's dest is [4,4);
     * 3,4c5,6 dest is [5,6);
     **/
    range &     dest()   { return _dst;}

    /**
     * gets the diff type.
     * @return either e_add, e_change or e_delete, may be modified.
     **/
    diff_type & type()   { return _type;}

    /**
     * gets the source range.
     *
     * @return a const reference to the source range.
     **/
    const range &     source() const { return _src;}

    /**
     * gets the dest range.
     *
     * @return a const reference to the dest range.
     **/
    const range &     dest()   const { return _dst;}

    /**
     * gets the diff type.
     * @return either e_add, e_change or e_delete, may not be modified.
     **/
    const diff_type & type()   const { return _type;}

    /**
     * gets the size difference between source and destination.
     * @return dest().size() - source().size();
     **/
    int size() const;

    /**
     * gets the source lines.
     * @returns the source only if the type is e_change.
     **/
    const vector<string> & source_line() const { return _src_lines; }

    /**
     * gets the destination lines.
     * @returns the dest only if the type is e_change.
     **/
    const vector<string> & dest_line() const   { return _dst_lines; }

    /**
     * compares two diff entries.
     * @return true if the two sources are equal and the two dest are equal.
     **/
    bool operator==(const diff_entry & r) const;

    bool operator!=(const diff_entry & r) const {
	return !this->operator == (r);
    }
};
#endif
