/** @file submatch.h
 *  @brief base class for sub-matchers
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

#ifndef XAPIAN_INCLUDED_SUBMATCH_H
#define XAPIAN_INCLUDED_SUBMATCH_H

#include <xapian/base.h>
#include <xapian/types.h>

#include "omenquireinternal.h"
#include "postlist.h"

class SubMatch : public Xapian::Internal::RefCntBase {
  public:
    /// Virtual destructor required because we have virtual methods.
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
     *  @return		If nowait is true and results aren't available yet
     *			then false will be returned and this method must be
     *			called again before the match can proceed.  If results
     *			are available or nowait is false, then this method
     *			returns true.
     */
    virtual bool prepare_match(bool nowait) = 0;

    /// Start the match.
    virtual void start_match(Xapian::doccount maxitems) = 0;

    /// Get PostList and term info.
    virtual PostList * get_postlist_and_term_info(MultiMatch *matcher,
	map<string, Xapian::MSet::Internal::TermFreqAndWeight> *termfreqandwts)
	= 0;
};

#endif /* XAPIAN_INCLUDED_SUBMATCH_H */
