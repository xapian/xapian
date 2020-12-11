/** @file
 * @brief Xapian::MSet internals
 */
/* Copyright 2016,2017 Olly Betts
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

#ifndef XAPIAN_INCLUDED_MSETINTERNAL_H
#define XAPIAN_INCLUDED_MSETINTERNAL_H

#include "enquireinternal.h"
#include "net/serialise.h"
#include "result.h"
#include "weight/weightinternal.h"

#include "xapian/intrusive_ptr.h"
#include "xapian/mset.h"
#include "xapian/types.h"

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

class Matcher;

namespace Xapian {

/// Xapian::MSet internals.
class MSet::Internal : public Xapian::Internal::intrusive_base {
    friend class MSet;
    friend class MSetIterator;
    friend class ::Matcher;

    /// Don't allow assignment.
    void operator=(const Internal &) = delete;

    /// Don't allow copying.
    Internal(const Internal &) = delete;

    /// Relevance weights for non-query terms for generating snippets.
    mutable std::unordered_map<std::string, double> snippet_bg_relevance;

    /// The items in the MSet.
    std::vector<Result> items;

    /// For looking up query term frequencies and weights.
    std::unique_ptr<Xapian::Weight::Internal> stats;

    Xapian::Internal::intrusive_ptr<const Enquire::Internal> enquire;

    Xapian::doccount matches_lower_bound = 0;

    Xapian::doccount matches_estimated = 0;

    Xapian::doccount matches_upper_bound = 0;

    Xapian::doccount uncollapsed_lower_bound = 0;

    Xapian::doccount uncollapsed_estimated = 0;

    Xapian::doccount uncollapsed_upper_bound = 0;

    Xapian::doccount first = 0;

    double max_possible = 0;

    double max_attained = 0;

    /// Scale factor to convert weights to percentages.
    double percent_scale_factor = 0;

  public:
    Internal() {}

    Internal(Xapian::doccount first_,
	     Xapian::doccount matches_upper_bound_,
	     Xapian::doccount matches_lower_bound_,
	     Xapian::doccount matches_estimated_,
	     Xapian::doccount uncollapsed_upper_bound_,
	     Xapian::doccount uncollapsed_lower_bound_,
	     Xapian::doccount uncollapsed_estimated_,
	     double max_possible_,
	     double max_attained_,
	     std::vector<Result>&& items_,
	     double percent_scale_factor_)
	: items(std::move(items_)),
	  matches_lower_bound(matches_lower_bound_),
	  matches_estimated(matches_estimated_),
	  matches_upper_bound(matches_upper_bound_),
	  uncollapsed_lower_bound(uncollapsed_lower_bound_),
	  uncollapsed_estimated(uncollapsed_estimated_),
	  uncollapsed_upper_bound(uncollapsed_upper_bound_),
	  first(first_),
	  max_possible(max_possible_),
	  max_attained(max_attained_),
	  percent_scale_factor(percent_scale_factor_) {}

    void set_first(Xapian::doccount first_) { first = first_; }

    void set_enquire(const Xapian::Enquire::Internal* enquire_) {
	enquire = enquire_;
    }

    Xapian::Weight::Internal* get_stats() const { return stats.get(); }

    void set_stats(Xapian::Weight::Internal* stats_) { stats.reset(stats_); }

    double get_percent_scale_factor() const { return percent_scale_factor; }

    Xapian::Document get_document(Xapian::doccount index) const;

    void fetch(Xapian::doccount first, Xapian::doccount last) const;

    void set_item_weight(Xapian::doccount i, double weight);

    int convert_to_percent(double weight) const;

    void unshard_docids(Xapian::doccount shard, Xapian::doccount n_shards);

    void merge_stats(const Internal* o, bool collapsing);

    std::string snippet(const std::string & text, size_t length,
			const Xapian::Stem & stemmer,
			unsigned flags,
			const std::string & hi_start,
			const std::string & hi_end,
			const std::string & omit) const;

    /** Serialise this object.
     *
     *  @return		The serialisation of this object.
     */
    std::string serialise() const;

    /** Unserialise a serialised Xapian::MSet::Internal object.
     *
     *  @param p	Pointer to the start of the string to unserialise.
     *  @param p_end	Pointer to the end of the string to unserialise.
     *
     *  This object is updated with the unserialised data.
     */
    void unserialise(const char * p, const char * p_end);

    /// Return a string describing this object.
    std::string get_description() const;
};

}

#endif // XAPIAN_INCLUDED_MSETINTERNAL_H
