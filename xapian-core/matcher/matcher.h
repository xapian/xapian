/** @file
 * @brief Matcher class
 */
/* Copyright (C) 2017,2018,2019 Olly Betts
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

#ifndef XAPIAN_INCLUDED_MATCHER_H
#define XAPIAN_INCLUDED_MATCHER_H

#ifndef PACKAGE
# error config.h must be included first in each C++ source file
#endif

#include "api/enquireinternal.h"
#include "localsubmatch.h"
#include "remotesubmatch.h"
#include "weight/weightinternal.h"

#include "xapian/database.h"
#include "xapian/query.h"

#include <memory>
#include <vector>

namespace Xapian {
    class KeyMaker;
    class MatchDecider;
    class MSet;
    class Weight;
}

class Matcher {
    typedef Xapian::Internal::opt_intrusive_ptr<Xapian::MatchSpy> opt_ptr_spy;

    Xapian::Database db;

    Xapian::Query query;

    /** LocalSubMatch objects for local databases.
     *
     *  The entries are at the same index as the corresponding shard in the
     *  Database object, with NULL entries for any remote shards.
     */
    std::vector<std::unique_ptr<LocalSubMatch>> locals;

#ifdef XAPIAN_HAS_REMOTE_BACKEND
    /** RemoteSubMatch objects for remote databases.
     *
     *  Unlike @a locals, this *only* contains entries for remote shards, and
     *  each RemoteSubMatch object knows its shard index.
     *
     *  If poll() isn't available so that select() has to be used to
     *  wait for fds to become ready to read, objects with an fd < FD_SETSIZE
     *  come first, and those with an fd >= FD_SETSIZE are put at the end with
     *  @a first_oversize recording the partition point.
     */
    std::vector<std::unique_ptr<RemoteSubMatch>> remotes;

# ifndef HAVE_POLL
    /** Partition point in @a remotes for fds < FD_SETSIZE.
     *
     *  remotes[i]->get_read_fd() < FD_SETSIZE for i < first_oversize,
     *  remotes[i]->get_read_fd() >= FD_SETSIZE for i >= first_oversize.
     */
    std::size_t first_oversize;
# endif
#endif

    Matcher(const Matcher&) = delete;

    Matcher& operator=(const Matcher&) = delete;

    Xapian::MSet get_local_mset(Xapian::doccount first,
				Xapian::doccount maxitems,
				Xapian::doccount check_at_least,
				const Xapian::Weight& wtscheme,
				const Xapian::MatchDecider* mdecider,
				const Xapian::KeyMaker* sorter,
				Xapian::valueno collapse_key,
				Xapian::doccount collapse_max,
				int percent_threshold,
				double percent_threshold_factor,
				double weight_threshold,
				Xapian::Enquire::docid_order order,
				Xapian::valueno sort_key,
				Xapian::Enquire::Internal::sort_setting sort_by,
				bool sort_val_reverse,
				double time_limit,
				const std::vector<opt_ptr_spy>& matchspies);

    /// Perform action on remotes as they become ready using poll() or select().
    template<typename Action> void for_all_remotes(Action action);

  public:
    /** Constructor.
     *
     *  @param db_		Database to search
     *  @param query		Query object
     *  @param query_length	Query length
     *  @param rset		Relevance set (NULL for none)
     *  @param stats		Object to collate stats into
     *  @param wtscheme		Weight object to use as factory
     *  @param have_mdecider	MatchDecider specified?
     *  @param collapse_key	value slot to collapse on (Xapian::BAD_VALUENO
     *				which means no collapsing)
     *  @param collapse_max	Maximum number of documents with the same key
     *				to allow
     *  @param percent_threshold Lower bound on percentage score
     *  @param weight_threshold	Lower bound on weight
     *  @param order		Xapian::docid sort order
     *  @param sort_key		Value slot to sort on
     *  @param sort_by		What to sort results on
     *  @param sort_val_reverse	Reverse direction keys sort in?
     *  @param time_limit	time in seconds after which to disable
     *				check_at_least (0.0 means don't).
     *  @param matchspies	MatchSpy objects to use
     */
    Matcher(const Xapian::Database& db_,
	    const Xapian::Query& query,
	    Xapian::termcount query_length,
	    const Xapian::RSet* rset,
	    Xapian::Weight::Internal& stats,
	    const Xapian::Weight& wtscheme,
	    bool have_mdecider,
	    Xapian::valueno collapse_key,
	    Xapian::doccount collapse_max,
	    int percent_threshold,
	    double weight_threshold,
	    Xapian::Enquire::docid_order order,
	    Xapian::valueno sort_key,
	    Xapian::Enquire::Internal::sort_setting sort_by,
	    bool sort_val_reverse,
	    double time_limit,
	    const std::vector<opt_ptr_spy>& matchspies);

    /** Run the match and produce an MSet object.
     *
     *  @param first		Zero-based index of the first result to return
     *				(which supports retrieving pages of results).
     *  @param maxitems		The maximum number of documents to return.
     *  @param checkatleast	Check at least this many documents.  By default
     *				Xapian will avoiding considering documents
     *				which it can prove can't match, which is faster
     *				but can result in a loose bounds on and a poor
     *				estimate of the total number of matches -
     *				setting checkatleast higher allows trading off
     *				speed for tighter bounds and a more accurate
     *				estimate.
     *  @param mset		MSet object to full in
     *  @param stats		Collated stats
     *  @param wtscheme		Weight object to use as factory
     *  @param mdecider		MatchDecider to use (NULL for none)
     *  @param sorter		KeyMaker for sort keys (NULL for none)
     *  @param collapse_key	value slot to collapse on (Xapian::BAD_VALUENO
     *				which means no collapsing)
     *  @param collapse_max	Maximum number of documents with the same key
     *				to allow
     *  @param collapse_key	value slot to collapse on (Xapian::BAD_VALUENO
     *				which means no collapsing)
     *  @param collapse_max	Maximum number of documents with the same key
     *				to allow
     *  @param percent_threshold Lower bound on percentage score
     *  @param weight_threshold	Lower bound on weight
     *  @param order		Xapian::docid sort order
     *  @param sort_key		Value slot to sort on
     *  @param sort_by		What to sort results on
     *  @param sort_val_reverse	Reverse direction keys sort in?
     *  @param time_limit	time in seconds after which to disable
     *				check_at_least (0.0 means don't).
     *  @param matchspies	MatchSpy objects to use
     */
    Xapian::MSet get_mset(Xapian::doccount first,
			  Xapian::doccount maxitems,
			  Xapian::doccount check_at_least,
			  Xapian::Weight::Internal& stats,
			  const Xapian::Weight& wtscheme,
			  const Xapian::MatchDecider* mdecider,
			  const Xapian::KeyMaker* sorter,
			  Xapian::valueno collapse_key,
			  Xapian::doccount collapse_max,
			  int percent_threshold,
			  double weight_threshold,
			  Xapian::Enquire::docid_order order,
			  Xapian::valueno sort_key,
			  Xapian::Enquire::Internal::sort_setting sort_by,
			  bool sort_val_reverse,
			  double time_limit,
			  const std::vector<opt_ptr_spy>& matchspies);
};

#endif // XAPIAN_INCLUDED_MATCHER_H
