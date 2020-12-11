/** @file
 * @brief Set of documents judged as relevant
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
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
 */

#ifndef XAPIAN_INCLUDED_RSETINTERNAL_H
#define XAPIAN_INCLUDED_RSETINTERNAL_H

#include <xapian/rset.h>

#include "backends/multi.h"

#include <set>
#include <vector>

namespace Xapian {

class RSet::Internal : public Xapian::Internal::intrusive_base {
  public:
    // We want to be able to iterate the contents in docid order, and we don't
    // expect people to mark vast numbers of documents as relevant, so use
    // std::set rather than std::unordered_set.
    std::set<Xapian::docid> docs;

    void shard(Xapian::doccount n_shards, std::vector<Xapian::RSet>& rsets) {
	if (n_shards == 1 || docs.empty()) {
	    // Either there's a single database (in which case we just need
	    // to return ourself as the sharded RSet), or there are no relevance
	    // judgements (in which case we can use ourself as the internals for
	    // all the empty sharded RSets).
	    rsets.resize(n_shards, RSet(this));
	    return;
	}

	// Using resize() here would result in all the entries having the same
	// internals.
	rsets.reserve(n_shards);
	for (Xapian::doccount i = 0; i < n_shards; ++i) {
	    rsets.emplace_back(RSet());
	}

	for (auto&& did : docs) {
	    Xapian::docid shard_did = shard_docid(did, n_shards);
	    rsets[shard_number(did, n_shards)].add_document(shard_did);
	}
    }
};

}

#endif // XAPIAN_INCLUDED_RSETINTERNAL_H
