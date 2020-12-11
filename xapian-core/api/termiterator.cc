/** @file
 *  @brief Class for iterating over a list of terms.
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

#include "xapian/termiterator.h"

#include "debuglog.h"
#include "omassert.h"
#include "termlist.h"

using namespace std;

namespace Xapian {

void
TermIterator::decref()
{
    Assert(internal);
    if (--internal->_refs == 0)
	delete internal;
}

void
TermIterator::post_advance(Internal * res)
{
    if (res) {
	// This can happen with iterating allterms from multiple databases.
	++res->_refs;
	decref();
	internal = res;
    }
    if (internal->at_end()) {
	decref();
	internal = NULL;
    }
}

TermIterator::TermIterator(Internal *internal_) : internal(internal_)
{
    LOGCALL_CTOR(API, "TermIterator", internal_);
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

TermIterator::TermIterator(const TermIterator & o)
    : internal(o.internal)
{
    LOGCALL_CTOR(API, "TermIterator", o);
    if (internal)
	++internal->_refs;
}

TermIterator &
TermIterator::operator=(const TermIterator & o)
{
    LOGCALL(API, TermIterator &, "TermIterator::operator=", o);
    if (o.internal)
	++o.internal->_refs;
    if (internal)
	decref();
    internal = o.internal;
    RETURN(*this);
}

string
TermIterator::operator*() const
{
    LOGCALL(API, string, "TermIterator::operator*", NO_ARGS);
    Assert(internal);
    RETURN(internal->get_termname());
}

TermIterator &
TermIterator::operator++()
{
    LOGCALL(API, TermIterator &, "TermIterator::operator++", NO_ARGS);
    Assert(internal);
    post_advance(internal->next());
    RETURN(*this);
}

Xapian::termcount
TermIterator::get_wdf() const
{
    LOGCALL(API, Xapian::termcount, "TermIterator::get_wdf", NO_ARGS);
    Assert(internal);
    RETURN(internal->get_wdf());
}

Xapian::doccount
TermIterator::get_termfreq() const
{
    LOGCALL(API, Xapian::doccount, "TermIterator::get_termfreq", NO_ARGS);
    Assert(internal);
    RETURN(internal->get_termfreq());
}

Xapian::termcount
TermIterator::positionlist_count() const
{
    LOGCALL(API, Xapian::termcount, "TermIterator::positionlist_count", NO_ARGS);
    Assert(internal);
    RETURN(internal->positionlist_count());
}

PositionIterator
TermIterator::positionlist_begin() const
{
    LOGCALL(API, PositionIterator, "TermIterator::positionlist_begin", NO_ARGS);
    Assert(internal);
    RETURN(internal->positionlist_begin());
}

void
TermIterator::skip_to(const string & term)
{
    LOGCALL_VOID(API, "TermIterator::skip_to", term);
    if (internal)
	post_advance(internal->skip_to(term));
}

std::string
TermIterator::get_description() const
{
#if 0 // FIXME: Add TermIterator::Internal::get_description() method.
    string desc = "TermIterator(";
    if (internal)
	desc += internal->get_description();
    desc += ')';
    return desc;
#else
    return "TermIterator()";
#endif
}

}
