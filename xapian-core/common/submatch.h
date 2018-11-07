/** @file submatch.h
 *  @brief base class for sub-matchers
 */
/* Copyright (C) 2006,2007,2009,2011,2014 Olly Betts
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

#ifndef XAPIAN_INCLUDED_SUBMATCH_H
#define XAPIAN_INCLUDED_SUBMATCH_H

#include "xapian/intrusive_ptr.h"
#include <xapian/types.h>

#include "api/omenquireinternal.h"
#include "api/postlist.h"
#include "xapian/weight.h"

class MultiMatch;

class SubMatch : public Xapian::Internal::intrusive_base {
  public:
    /** Virtual destructor.
     *
     *  Required because we have virtual methods and delete derived objects
     *  via a pointer to this base class.
     */
    virtual ~SubMatch() { }

    /** Fetch and collate statistics.
     *
     *  Before we can calculate term weights we need to fetch statistics from
     *  each database involved and collate them.
     *
     *  @param nowait	A RemoteSubMatch may not be able to report statistics
     *			when first asked.  If nowait is true, it will return
     *			false in this situation allowing the matcher to ask
     *			other database.  If nowait is false, then this method
     *			will block until statistics are available.
     *
     *  @param total_stats A stats object to which the statistics should be
     *			added.
     *
     *  @return		If nowait is true and results aren't available yet
     *			then false will be returned and this method must be
     *			called again before the match can proceed.  If results
     *			are available or nowait is false, then this method
     *			returns true.
     */
    virtual bool prepare_match(bool nowait,
			       Xapian::Weight::Internal & total_stats) = 0;

    /** Start the match.
     *
     *  @param first          The first item in the result set to return.
     *  @param maxitems       The maximum number of items to return.
     *  @param check_at_least The minimum number of items to check.
     *  @param total_stats    The total statistics for the collection.
     */
    virtual void start_match(Xapian::doccount first,
			     Xapian::doccount maxitems,
			     Xapian::doccount check_at_least,
			     Xapian::Weight::Internal & total_stats) = 0;

    /// Get PostList.
    virtual PostList * get_postlist(MultiMatch *matcher,
				    Xapian::termcount * total_subqs_ptr) = 0;
};

#endif /* XAPIAN_INCLUDED_SUBMATCH_H */
