/** @file emptysubmatch.h
 *  @brief SubMatch class for a dead remote database.
 */
/* Copyright (C) 2006 Olly Betts
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

#ifndef XAPIAN_INCLUDED_EMPTYSUBMATCH_H
#define XAPIAN_INCLUDED_EMPTYSUBMATCH_H

#include "submatch.h"

class EmptySubMatch : public SubMatch {
    /// Don't allow assignment.
    void operator=(const EmptySubMatch &);

    /// Don't allow copying.
    EmptySubMatch(const EmptySubMatch &);

  public:
    /// Constructor.
    EmptySubMatch() {}

    /// Fetch and collate statistics.
    bool prepare_match(bool nowait);

    /// Start the match.
    void start_match(Xapian::doccount maxitems);

    /// Get PostList and term info.
    PostList * get_postlist_and_term_info(MultiMatch *matcher,
	map<string, Xapian::MSet::Internal::TermFreqAndWeight> *termfreqandwts);
};

#endif /* XAPIAN_INCLUDED_EMPTYSUBMATCH_H */
