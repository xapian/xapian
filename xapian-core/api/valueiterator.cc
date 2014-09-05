/** @file valueiterator.cc
 *  @brief Class for iterating over document values.
 */
/* Copyright (C) 2008,2009,2013 Olly Betts
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

#include "xapian/valueiterator.h"

#include "debuglog.h"
#include "omassert.h"
#include "valuelist.h"

using namespace std;

namespace Xapian {

ValueIterator::ValueIterator() : internal(NULL) { }

ValueIterator::~ValueIterator() { }

ValueIterator::ValueIterator(Internal *internal_) : internal(internal_)
{
    internal->next();
    if (internal->at_end()) internal = NULL;
}

ValueIterator::ValueIterator(const ValueIterator & o)
    : internal(o.internal) { }

ValueIterator::ValueIterator(const ValueIteratorEnd_ &)
    : internal(NULL) { }

ValueIterator &
ValueIterator::operator=(const ValueIterator & o)
{
    internal = o.internal;
    return *this;
}

ValueIterator &
ValueIterator::operator=(const ValueIteratorEnd_ &)
{
    internal = NULL;
    return *this;
}

string
ValueIterator::operator*() const
{
    LOGCALL(API, string, "ValueIterator::operator*", NO_ARGS);
    Assert(internal.get());
    RETURN(internal->get_value());
}

ValueIterator &
ValueIterator::operator++()
{
    LOGCALL(API, ValueIterator &, "ValueIterator::operator++", NO_ARGS);
    Assert(internal.get());
    internal->next();
    if (internal->at_end()) internal = NULL;
    RETURN(*this);
}

Xapian::docid
ValueIterator::get_docid() const
{
    LOGCALL(API, Xapian::docid, "ValueIterator::get_docid", NO_ARGS);
    Assert(internal.get());
    RETURN(internal->get_docid());
}

Xapian::valueno
ValueIterator::get_valueno() const
{
    LOGCALL(API, Xapian::valueno, "ValueIterator::get_valueno", NO_ARGS);
    Assert(internal.get());
    RETURN(internal->get_valueno());
}

void
ValueIterator::skip_to(Xapian::docid docid_or_slot)
{
    LOGCALL_VOID(API, "ValueIterator::skip_to", docid_or_slot);
    if (!internal.get()) return;
    internal->skip_to(docid_or_slot);
    if (internal->at_end()) internal = NULL;
}

bool
ValueIterator::check(Xapian::docid did)
{
    LOGCALL(API, bool, "ValueIterator::check", did);
    if (!internal.get()) RETURN(true);
    if (!internal->check(did)) RETURN(false);
    if (internal->at_end()) internal = NULL;
    RETURN(true);
}

std::string
ValueIterator::get_description() const
{
    string desc = "ValueIterator(";
    if (internal.get()) desc += internal->get_description();
    desc += ')';
    return desc;
}

}
