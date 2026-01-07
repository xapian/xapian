/** @file
 * @brief Return document ids from an external source.
 */
/* Copyright 2008-2026 Olly Betts
 * Copyright 2009 Lemur Consulting Ltd
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

#include <config.h>

#include "externalpostlist.h"

#include <xapian/postingsource.h>

#include "debuglog.h"
#include "estimateop.h"
#include "omassert.h"

using namespace std;

ExternalPostList::ExternalPostList(const Xapian::Database& db,
				   Xapian::PostingSource* source_,
				   EstimateOp* estimate_op,
				   double factor_,
				   bool* max_weight_cached_flag_ptr,
				   Xapian::doccount shard_index)
    : factor(factor_)
{
    Assert(source_);
    Xapian::PostingSource* newsource = source_->clone();
    if (newsource != NULL) {
	source = newsource->release();
    } else if (shard_index == 0) {
	// Allow use of a non-clone-able PostingSource with a non-sharded
	// Database.
	source = source_;
    } else {
	throw Xapian::InvalidOperationError("PostingSource subclass must "
					    "implement clone() to support use "
					    "with a sharded database");
    }
    source->set_max_weight_cached_flag_ptr_(max_weight_cached_flag_ptr);
    source->reset(db, shard_index);
    termfreq = source->get_termfreq_est();
    if (estimate_op) {
	estimate_op->report_termfreqs(source->get_termfreq_min(),
				      termfreq,
				      source->get_termfreq_max());
    }
}

Xapian::docid
ExternalPostList::get_docid() const
{
    LOGCALL(MATCH, Xapian::docid, "ExternalPostList::get_docid", NO_ARGS);
    Assert(current);
    RETURN(current);
}

double
ExternalPostList::get_weight(Xapian::termcount,
			     Xapian::termcount,
			     Xapian::termcount) const
{
    LOGCALL(MATCH, double, "ExternalPostList::get_weight", NO_ARGS);
    Assert(source);
    if (factor == 0.0) RETURN(factor);
    RETURN(factor * source->get_weight());
}

double
ExternalPostList::recalc_maxweight()
{
    LOGCALL(MATCH, double, "ExternalPostList::recalc_maxweight", NO_ARGS);
    // source will be NULL here if we've reached the end.
    if (!source) RETURN(0.0);
    if (factor == 0.0) RETURN(0.0);
    RETURN(factor * source->get_maxweight());
}

PositionList *
ExternalPostList::read_position_list()
{
    return NULL;
}

PostList *
ExternalPostList::update_after_advance() {
    LOGCALL(MATCH, PostList *, "ExternalPostList::update_after_advance", NO_ARGS);
    Assert(source);
    if (source->at_end()) {
	LOGLINE(MATCH, "ExternalPostList now at end");
	source = NULL;
    } else {
	current = source->get_docid();
    }
    RETURN(NULL);
}

PostList *
ExternalPostList::next(double w_min)
{
    LOGCALL(MATCH, PostList *, "ExternalPostList::next", w_min);
    Assert(source);
    source->next(w_min);
    RETURN(update_after_advance());
}

PostList *
ExternalPostList::skip_to(Xapian::docid did, double w_min)
{
    LOGCALL(MATCH, PostList *, "ExternalPostList::skip_to", did | w_min);
    Assert(source);
    if (did <= current) RETURN(NULL);
    source->skip_to(did, w_min);
    RETURN(update_after_advance());
}

PostList *
ExternalPostList::check(Xapian::docid did, double w_min, bool &valid)
{
    LOGCALL(MATCH, PostList *, "ExternalPostList::check", did | w_min | valid);
    Assert(source);
    if (did <= current) {
	valid = true;
	RETURN(NULL);
    }
    valid = source->check(did, w_min);
    if (source->at_end()) {
	LOGLINE(MATCH, "ExternalPostList now at end");
	source = NULL;
    } else {
	current = valid ? source->get_docid() : current;
    }
    RETURN(NULL);
}

bool
ExternalPostList::at_end() const
{
    LOGCALL(MATCH, bool, "ExternalPostList::at_end", NO_ARGS);
    RETURN(!source);
}

Xapian::termcount
ExternalPostList::count_matching_subqs() const
{
    return 1;
}

string
ExternalPostList::get_description() const
{
    string desc = "ExternalPostList(";
    if (source) desc += source->get_description();
    desc += ")";
    return desc;
}
