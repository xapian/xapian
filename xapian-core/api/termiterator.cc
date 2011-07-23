/** @file termiterator.cc
 *  @brief Class for iterating over a list of terms.
 */
/* Copyright (C) 2008,2009,2010 Olly Betts
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

TermIterator::TermIterator() : internal(NULL) { }

TermIterator::~TermIterator() { }

TermIterator::TermIterator(Internal *internal_) : internal(internal_)
{
    if (!internal_) return;

    Internal * res = internal->next();
    if (res)
	internal = res;
    if (internal->at_end()) internal = NULL;
}

TermIterator::TermIterator(const TermIterator & o)
    : internal(o.internal) { }

TermIterator &
TermIterator::operator=(const TermIterator & o)
{
    internal = o.internal;
    return *this;
}

string
TermIterator::operator*() const
{
    LOGCALL(API, string, "TermIterator::operator*", NO_ARGS);
    Assert(internal.get());
    RETURN(internal->get_termname());
}

TermIterator &
TermIterator::operator++()
{
    LOGCALL(API, TermIterator &, "TermIterator::operator++", NO_ARGS);
    Assert(internal.get());
    Internal * res = internal->next();
    if (res)
	internal = res;
    if (internal->at_end()) internal = NULL;
    RETURN(*this);
}

Xapian::termcount
TermIterator::get_wdf() const
{
    LOGCALL(API, Xapian::termcount, "TermIterator::get_wdf", NO_ARGS);
    Assert(internal.get());
    RETURN(internal->get_wdf());
}

Xapian::doccount
TermIterator::get_termfreq() const
{
    LOGCALL(API, Xapian::doccount, "TermIterator::get_termfreq", NO_ARGS);
    Assert(internal.get());
    RETURN(internal->get_termfreq());
}

Xapian::termcount
TermIterator::positionlist_count() const
{
    LOGCALL(API, Xapian::termcount, "TermIterator::positionlist_count", NO_ARGS);
    Assert(internal.get());
    RETURN(internal->positionlist_count());
}

PositionIterator
TermIterator::positionlist_begin() const
{
    LOGCALL(API, PositionIterator, "TermIterator::positionlist_begin", NO_ARGS);
    Assert(internal.get());
    RETURN(internal->positionlist_begin());
}

void
TermIterator::skip_to(const string & term)
{
    LOGCALL_VOID(API, "TermIterator::skip_to", term);
    Assert(internal.get());
    Internal * res = internal->skip_to(term);
    if (res)
	internal = res;
    if (internal->at_end()) internal = NULL;
}

std::string
TermIterator::get_description() const
{
#if 0 // FIXME: Add TermIterator::Internal::get_description() method.
    string desc = "TermIterator(";
    if (internal.get()) desc += internal->get_description();
    desc += ')';
    return desc;
#else
    return "TermIterator()";
#endif
}

}
