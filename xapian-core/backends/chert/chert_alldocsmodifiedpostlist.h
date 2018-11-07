/** @file chert_alldocsmodifiedpostlist.h
 * @brief A ChertAllDocsPostList plus pending modifications.
 */
/* Copyright (C) 2008 Lemur Consulting Ltd
 * Copyright (C) 2009,2011,2015 Olly Betts
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

#ifndef XAPIAN_INCLUDED_CHERT_ALLDOCSMODIFIEDPOSTLIST_H
#define XAPIAN_INCLUDED_CHERT_ALLDOCSMODIFIEDPOSTLIST_H

#include <string>

#include "chert_alldocspostlist.h"

class ChertAllDocsModifiedPostList : public ChertAllDocsPostList {
    /// Modifications to apply to the ChertAllDocsPostList.
    map<Xapian::docid, Xapian::termcount> doclens;

    /// Current position in the doclens list.
    map<Xapian::docid, Xapian::termcount>::const_iterator doclens_it;

    /// Don't allow assignment.
    void operator=(const ChertAllDocsModifiedPostList &);

    /// Don't allow copying.
    ChertAllDocsModifiedPostList(const ChertAllDocsModifiedPostList &);

    /// Skip over deleted documents after a next() or skip_to().
    void skip_deletes(double w_min);

  public:
    ChertAllDocsModifiedPostList(Xapian::Internal::intrusive_ptr<const ChertDatabase> db_,
				 Xapian::doccount doccount_,
				 const map<Xapian::docid, Xapian::termcount> & doclens_);

    Xapian::docid get_docid() const;

    Xapian::termcount get_doclength() const;

    Xapian::termcount get_unique_terms() const;

    PostList * next(double w_min);

    PostList * skip_to(Xapian::docid desired_did, double w_min);

    bool at_end() const;

    std::string get_description() const;
};

#endif // XAPIAN_INCLUDED_CHERT_ALLDOCSMODIFIEDPOSTLIST_H
