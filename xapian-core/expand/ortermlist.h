/** @file ortermlist.h
 * @brief Merge two TermList objects using an OR operation.
 */
/* Copyright (C) 2007 Olly Betts
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
 */

#ifndef XAPIAN_INCLUDED_ORTERMLIST_H
#define XAPIAN_INCLUDED_ORTERMLIST_H

#include "termlist.h"

class Xapian::Internal::ExpandStats;

class OrTermList : public TermList {
    /// The two TermList objects we're merging.
    TermList *left, *right;

    /** The current term for left and right respectively.
     *
     *  Until next() is first called, these will be empty strings.  Once next()
     *  has been called, they won't be empty (since the empty string isn't a
     *  valid term).
     */
    std::string left_current, right_current;

  public:
    OrTermList(TermList * left_, TermList * right_)
	: left(left_), right(right_) { }

    ~OrTermList();

    Xapian::termcount get_approx_size() const;

    void accumulate_stats(Xapian::Internal::ExpandStats & stats) const;

    std::string get_termname() const;

    Xapian::termcount get_wdf() const;

    Xapian::doccount get_termfreq() const;

    Xapian::termcount get_collection_freq() const;

    TermList *next();

    bool at_end() const;

    Xapian::termcount positionlist_count() const;

    Xapian::PositionIterator positionlist_begin() const;
};

#endif // XAPIAN_INCLUDED_ORTERMLIST_H
