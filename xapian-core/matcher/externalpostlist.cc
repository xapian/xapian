/** @file externalpostlist.cc
 * @brief Return document ids from an external source.
 */
/* Copyright 2008,2009,2010,2011 Olly Betts
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
#include "omassert.h"

using namespace std;

ExternalPostList::ExternalPostList(const Xapian::Database & db,
				   Xapian::PostingSource *source_,
				   double factor_,
				   MultiMatch * matcher)
    : source(source_), source_is_owned(false), current(0), factor(factor_)
{
    Assert(source);
    Xapian::PostingSource * newsource = source->clone();
    if (newsource != NULL) {
	source = newsource;
	source_is_owned = true;
    }
    source->register_matcher_(static_cast<void*>(matcher));
    source->init(db);
}

ExternalPostList::~ExternalPostList()
{
    if (source_is_owned) {
	delete source;
    }
}

Xapian::doccount
ExternalPostList::get_termfreq_min() const
{
    Assert(source);
    return source->get_termfreq_min();
}

Xapian::doccount
ExternalPostList::get_termfreq_est() const
{
    Assert(source);
    return source->get_termfreq_est();
}

Xapian::doccount
ExternalPostList::get_termfreq_max() const
{
    Assert(source);
    return source->get_termfreq_max();
}

double
ExternalPostList::get_maxweight() const
{
    LOGCALL(MATCH, double, "ExternalPostList::get_maxweight", NO_ARGS);
    // source will be NULL here if we've reached the end.
    if (source == NULL) RETURN(0.0);
    if (factor == 0.0) RETURN(0.0);
    RETURN(factor * source->get_maxweight());
}

Xapian::docid
ExternalPostList::get_docid() const
{
    LOGCALL(MATCH, Xapian::docid, "ExternalPostList::get_docid", NO_ARGS);
    Assert(current);
    RETURN(current);
}

double
ExternalPostList::get_weight() const
{
    LOGCALL(MATCH, double, "ExternalPostList::get_weight", NO_ARGS);
    Assert(source);
    if (factor == 0.0) RETURN(factor);
    RETURN(factor * source->get_weight());
}

Xapian::termcount
ExternalPostList::get_doclength() const
{
    Assert(false);
    return 0;
}

Xapian::termcount
ExternalPostList::get_unique_terms() const
{
    Assert(false);
    return 0;
}

double
ExternalPostList::recalc_maxweight()
{
    return ExternalPostList::get_maxweight();
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
	if (source_is_owned) delete source;
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
	if (source_is_owned) delete source;
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
    RETURN(source == NULL);
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
