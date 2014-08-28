/** @file postingiterator.cc
 *  @brief Class for iterating over a list of document ids.
 */
/* Copyright (C) 2008,2009,2010,2011,2013 Olly Betts
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

#include "xapian/postingiterator.h"

#include "debuglog.h"
#include "omassert.h"
#include "postlist.h"

using namespace std;

namespace Xapian {

void
PostingIterator::decref()
{
    Assert(internal);
    if (--internal->_refs == 0)
	delete internal;
}

void
PostingIterator::post_advance(Internal * res)
{
    if (res) {
	// FIXME: It seems this can't happen for any PostList which we wrap
	// with PostingIterator.
	++res->_refs;
	decref();
	internal = res;
    }
    if (internal->at_end()) {
	decref();
	internal = NULL;
    }
}

PostingIterator::PostingIterator(Internal *internal_) : internal(internal_)
{
    LOGCALL_CTOR(API, "PostingIterator", internal_);
    if (!internal) return;
    try {
	++internal->_refs;
	post_advance(internal->next());
    } catch (...) {
	// The destructor only runs if the constructor completes, so we have to
	// take care of cleaning up for ourselves here.
	decref();
	throw;
    }
}

PostingIterator::PostingIterator(const PostingIterator & o)
    : internal(o.internal)
{
    LOGCALL_CTOR(API, "PostingIterator", o);
    if (internal)
	++internal->_refs;
}

PostingIterator &
PostingIterator::operator=(const PostingIterator & o)
{
    LOGCALL(API, PostingIterator &, "PostingIterator::operator=", o);
    if (o.internal)
	++o.internal->_refs;
    if (internal)
	decref();
    internal = o.internal;
    RETURN(*this);
}

Xapian::docid
PostingIterator::operator*() const
{
    LOGCALL(API, Xapian::docid, "PostingIterator::operator*", NO_ARGS);
    Assert(internal);
    RETURN(internal->get_docid());
}

PostingIterator &
PostingIterator::operator++()
{
    LOGCALL(API, PostingIterator &, "PostingIterator::operator++", NO_ARGS);
    Assert(internal);
    post_advance(internal->next());
    RETURN(*this);
}

Xapian::termcount
PostingIterator::get_wdf() const
{
    LOGCALL(API, Xapian::termcount, "PostingIterator::get_wdf", NO_ARGS);
    Assert(internal);
    RETURN(internal->get_wdf());
}

Xapian::termcount
PostingIterator::get_doclength() const
{
    LOGCALL(API, Xapian::termcount, "PostingIterator::get_doclength", NO_ARGS);
    Assert(internal);
    RETURN(internal->get_doclength());
}

Xapian::termcount
PostingIterator::get_unique_terms() const
{
    LOGCALL(API, Xapian::termcount, "PostingIterator::get_unique_terms", NO_ARGS);
    Assert(internal);
    RETURN(internal->get_unique_terms());
}

#if 0 // FIXME: TermIterator supports this, so PostingIterator really ought to.
Xapian::termcount
PostingIterator::positionlist_count() const
{
    LOGCALL(API, Xapian::termcount, "PostingIterator::positionlist_count", NO_ARGS);
    Assert(internal);
    RETURN(internal->positionlist_count());
}
#endif

PositionIterator
PostingIterator::positionlist_begin() const
{
    LOGCALL(API, PositionIterator, "PostingIterator::positionlist_begin", NO_ARGS);
    Assert(internal);
    RETURN(PositionIterator(internal->open_position_list()));
}

void
PostingIterator::skip_to(Xapian::docid did)
{
    LOGCALL_VOID(API, "PostingIterator::skip_to", did);
    if (internal)
	post_advance(internal->skip_to(did));
}

std::string
PostingIterator::get_description() const
{
    string desc = "PostingIterator(";
    if (internal)
	desc += internal->get_description();
    desc += ')';
    return desc;
}

}
