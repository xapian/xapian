/** @file honey_alldocspostlist.h
 * @brief A PostList which iterates over all documents in a HoneyDatabase.
 */
/* Copyright (C) 2006,2007,2008,2009,2017 Olly Betts
 * Copyright (C) 2008 Lemur Consulting Ltd
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
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

#ifndef XAPIAN_INCLUDED_HONEY_ALLDOCSPOSTLIST_H
#define XAPIAN_INCLUDED_HONEY_ALLDOCSPOSTLIST_H

#include <string>

#include "api/leafpostlist.h"

class HoneyDatabase;

class HoneyAllDocsPostList : public LeafPostList {
    /// Don't allow assignment.
    HoneyAllDocsPostList& operator=(const HoneyAllDocsPostList&) = delete;

    /// Don't allow copying.
    HoneyAllDocsPostList(const HoneyAllDocsPostList&) = delete;

    /// The number of documents in the database.
    Xapian::doccount doccount;

  public:
    HoneyAllDocsPostList(Xapian::Internal::intrusive_ptr<const HoneyDatabase> db_,
			 Xapian::doccount doccount_);

    Xapian::doccount get_termfreq() const;

    Xapian::termcount get_doclength() const;

    Xapian::docid get_docid() const;

    Xapian::termcount get_wdf() const;

    bool at_end() const;

    PostList* next(double w_min);

    PostList* skip_to(Xapian::docid did, double w_min);

    PostList* check(Xapian::docid did, double w_min, bool& valid);

    std::string get_description() const;
};

#endif // XAPIAN_INCLUDED_HONEY_ALLDOCSPOSTLIST_H
