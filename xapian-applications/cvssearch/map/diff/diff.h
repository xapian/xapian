/************************************************************
 *
 *  diff.h holds a set of differences between two files.
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

#ifndef __DIFF_H__
#define __DIFF_H__

#include "collection.h"
#include "virtual_iostream.h"
#include "diff_entry.h"

/**
 * diff stores a list of differences read from an input stream.
 **/
class diff : public collection<diff_entry>, public virtual_iostream
{
protected:
    bool _read_content;
    bool _aligned;

    /**
     * reads diff from the input stream.
     **/
    virtual istream & read(istream &);

    /**
     * prints the diff to output stream os.
     *
     **/
    virtual ostream & show(ostream & os) const;

public:
    virtual ~diff();
    /**
     * constructor.
     *
     * @param read_content is a flag to specify whether
     * the input we want to parse contain actual diff content or not
     * ie.. lines begin with '>' '<', '-', if false,
     * the input is assumed to contain only lines of
     * "s1,s2td1,d2". 
     * this flag is extremely important and mustn't set to be wrong.
     **/
    diff (bool read_content = true) : _read_content(read_content), _aligned(false) {}

    /**
     * compares two diff output, they are equal if all entries are equal.
     **/
    bool operator==(const diff & r) const;

    /**
     * align each diff entry so the begin values
     * for the source and destination are equal.
     *
     * because the diff entry are read in
     * increasing order, each entry affects
     * all subsequent entries, but the source 
     * range produced by cvs diff 
     * refers to the old position
     *
     * e.g.
     * 2a3,4   <- this causes a shift of +2
     *            add to the source of subsequent
     *            entries.
     * 
     * 5,6c7,8 <- this causes no shift
     **/
    void align_top();

    /**
     * undo the effect of align_top.
     **/
    void unalign_top();
};
#endif
