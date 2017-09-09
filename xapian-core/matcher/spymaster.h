/** @file spymaster.h
 * @brief Class for managing MatchSpy objects during the match
 */
/* Copyright 2017 Olly Betts
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

#ifndef XAPIAN_INCLUDED_SPYMASTER_H
#define XAPIAN_INCLUDED_SPYMASTER_H

#include <xapian/intrusive_ptr.h>
#include <xapian/matchspy.h>

#include "backends/multi.h"
#include <vector>

class SpyMaster {
    /// The MatchSpy objects to apply.
    const std::vector<Xapian::Internal::opt_intrusive_ptr<Xapian::MatchSpy>>* spies;

    /// Which shards are remote.
    const std::vector<bool>* is_remote;

  public:
    SpyMaster(const std::vector<Xapian::Internal::opt_intrusive_ptr<Xapian::MatchSpy>>* spies_,
	      const std::vector<bool>* is_remote_)
	: spies(spies_->empty() ? NULL : spies_), is_remote(is_remote_)
    {}

    operator bool() const { return spies != NULL; }

    void operator()(const Xapian::Document& doc,
		    double weight,
		    Xapian::docid did) {
	if (spies != NULL) {
	    if ((*is_remote)[shard_number(did, is_remote->size())]) {
		// The MatchSpy objects get applied during the remote match.
		return;
	    }

	    for (auto spy : *spies) {
		(*spy)(doc, weight);
	    }
	}
    }
};

#endif // XAPIAN_INCLUDED_SPYMASTER_H
