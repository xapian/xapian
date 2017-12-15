/** @file honey_postlist.h
 * @brief PostList in a honey database.
 */
/* Copyright (C) 2007,2009,2011,2013,2015,2016 Olly Betts
 * Copyright (C) 2009 Lemur Consulting Ltd
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

#ifndef XAPIAN_INCLUDED_HONEY_POSTLIST_H
#define XAPIAN_INCLUDED_HONEY_POSTLIST_H

#include "api/leafpostlist.h"

#include <string>

class HoneyDatabase;

/** PostList in a honey database. */
class HoneyPostList : public LeafPostList {
    /// Don't allow assignment.
    HoneyPostList& operator=(const HoneyPostList &) = delete;

    /// Don't allow copying.
    HoneyPostList(const HoneyPostList &) = delete;

  public:
    HoneyPostList(Xapian::Internal::intrusive_ptr<const HoneyDatabase> db_,
		  const std::string& term_,
		  bool)
	: LeafPostList(term_) { (void)db_; }

    Xapian::doccount get_termfreq() const;

    LeafPostList* open_nearby_postlist(const std::string& term_) const;

    Xapian::docid get_docid() const;

    Xapian::termcount get_wdf() const;

    bool at_end() const;

    PositionList* read_position_list();

    PositionList* open_position_list() const;

    PostList* next(double w_min);

    PostList* skip_to(Xapian::docid did, double w_min);

    PostList* check(Xapian::docid did, double w_min, bool& valid);

    std::string get_description() const;
};

#endif // XAPIAN_INCLUDED_HONEY_POSTLIST_H
