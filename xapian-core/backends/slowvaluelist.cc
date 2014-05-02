/** @file slowvaluelist.cc
 * @brief Slow implementation for backends which don't streamed values.
 */
/* Copyright (C) 2008,2014 Olly Betts
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
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

#include "slowvaluelist.h"

#include "document.h"
#include "str.h"

#include "xapian/error.h"

#include "autoptr.h"

using namespace std;

Xapian::docid
SlowValueList::get_docid() const
{
    return current_did;
}

string
SlowValueList::get_value() const
{
    return current_value;
}

Xapian::valueno
SlowValueList::get_valueno() const
{
    return slot;
}

bool
SlowValueList::at_end() const
{
    return last_docid == 0;
}

void
SlowValueList::next()
{
    // Compare before incrementing, since last_docid could be the largest
    // representable value in which case current_did will wrap to zero before
    // exceeding it.
    while (current_did++ < last_docid) {
	try {
	    // Open document lazily so that we don't waste time checking for
	    // its existence.
	    AutoPtr<Xapian::Document::Internal>
		doc(db->open_document(current_did, true));
	    if (!doc.get()) continue;
	    string value = doc->get_value(slot);
	    if (!value.empty()) {
		swap(current_value, value);
		return;
	    }
	} catch (const Xapian::DocNotFoundError &) {
	}
    }

    // Indicate that we're at_end().
    last_docid = 0;
}

void
SlowValueList::skip_to(Xapian::docid did)
{
    if (did <= current_did) return;

    if (did > last_docid) {
	// Indicate that we're at_end().
	last_docid = 0;
	return;
    }

    current_did = did - 1;
    next();
}

bool
SlowValueList::check(Xapian::docid did)
{
    if (did <= current_did) return true;

    if (did > last_docid) {
	// Indicate that we're at_end().
	last_docid = 0;
	return true;
    }

    current_did = did;
    try {
	AutoPtr<Xapian::Document::Internal>
	doc(db->open_document(current_did, true));
	if (doc.get()) {
	    current_value = doc->get_value(slot);
	    if (!current_value.empty()) return true;
	}
    } catch (const Xapian::DocNotFoundError &) {
    }
    return false;
}

string
SlowValueList::get_description() const
{
    string desc = "SlowValueList(slot=";
    desc += str(slot);
    if (last_docid != 0) {
	desc += ", docid=";
	desc += str(current_did);
	desc += ", value=\"";
	desc += current_value;
	desc += "\")";
    } else {
	desc += ", atend)";
    }
    return desc;
}
