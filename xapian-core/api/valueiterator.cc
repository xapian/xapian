/** @file valueiterator.cc
 *  @brief Class for iterating over document values.
 */
/* Copyright (C) 2008,2009,2011 Olly Betts
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

void
ValueIterator::deref()
{
    Assert(internal);
    if (--internal->_refs == 0)
	delete internal;
}

ValueIterator::ValueIterator(Internal *internal_) : internal(internal_)
{
    Assert(internal);
    ++internal->_refs;
    internal->next();
    if (internal->at_end()) {
	deref();
	internal = NULL;
    }
}

ValueIterator::ValueIterator(const ValueIterator & o)
    : internal(o.internal)
{
    if (internal)
	++internal->_refs;
}

ValueIterator &
ValueIterator::operator=(const ValueIterator & o)
{
    if (o.internal)
	++o.internal->_refs;
    if (internal)
	deref();
    internal = o.internal;
    return *this;
}

string
ValueIterator::operator*() const
{
    LOGCALL(API, string, "ValueIterator::operator*", NO_ARGS);
    Assert(internal);
    RETURN(internal->get_value());
}

ValueIterator &
ValueIterator::operator++()
{
    LOGCALL(API, ValueIterator &, "ValueIterator::operator++", NO_ARGS);
    Assert(internal);
    internal->next();
    if (internal->at_end()) {
	deref();
	internal = NULL;
    }
    RETURN(*this);
}

Xapian::docid
ValueIterator::get_docid() const
{
    LOGCALL(API, Xapian::docid, "ValueIterator::get_docid", NO_ARGS);
    Assert(internal);
    RETURN(internal->get_docid());
}

Xapian::valueno
ValueIterator::get_valueno() const
{
    LOGCALL(API, Xapian::valueno, "ValueIterator::get_valueno", NO_ARGS);
    Assert(internal);
    RETURN(internal->get_valueno());
}

void
ValueIterator::skip_to(Xapian::docid docid_or_slot)
{
    LOGCALL_VOID(API, "ValueIterator::skip_to", docid_or_slot);
    Assert(internal);
    internal->skip_to(docid_or_slot);
    if (internal->at_end()) {
	deref();
	internal = NULL;
    }
}

bool
ValueIterator::check(Xapian::docid docid)
{
    LOGCALL(API, bool, "ValueIterator::check", docid);
    Assert(internal);
    if (!internal->check(docid)) return false;
    if (internal->at_end()) {
	deref();
	internal = NULL;
    }
    return true;
}

std::string
ValueIterator::get_description() const
{
    string desc = "ValueIterator(";
    if (internal)
	desc += internal->get_description();
    desc += ')';
    return desc;
}

}
