/** @file
 * @brief PostList which applies a MatchDecider
 */
/* Copyright (C) 2017 Olly Betts
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

#ifndef XAPIAN_INCLUDED_DECIDERPOSTLIST_H
#define XAPIAN_INCLUDED_DECIDERPOSTLIST_H

#include "selectpostlist.h"
#include "valuestreamdocument.h"

#include <xapian/document.h>
#include <xapian/matchdecider.h>

namespace Xapian {
class MatchDecider;
}

/// PostList which applies a MatchDecider
class DeciderPostList : public SelectPostList {
    /// The MatchDecider to apply.
    const Xapian::MatchDecider* decider;

    /// The document to test.
    Xapian::Document doc;

    /// Test the current with the MatchDecider.
    bool test_doc();

  public:
    DeciderPostList(PostList* pl_,
		    const Xapian::MatchDecider* decider_,
		    ValueStreamDocument* vsdoc,
		    PostListTree* pltree_)
	: SelectPostList(pl_, pltree_), decider(decider_), doc(vsdoc)
    {
	// These get zeroed once per shard in use, but that all happens before
	// the match starts so isn't a problem.
	decider->docs_allowed_ = decider->docs_denied_ = 0;
    }

    std::string get_description() const;
};

#endif // XAPIAN_INCLUDED_DECIDERPOSTLIST_H
