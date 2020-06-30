/** @file valuestreamdocument.cc
 * @brief A document which gets its values from a ValueStreamManager.
 */
/* Copyright (C) 2009,2011,2014,2017 Olly Betts
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

#include "valuestreamdocument.h"

#include "backends/multi/multi_database.h"
#include "omassert.h"

using namespace std;

static void
clear_valuelists(map<Xapian::valueno, ValueList *> & valuelists)
{
    map<Xapian::valueno, ValueList *>::const_iterator i;
    for (i = valuelists.begin(); i != valuelists.end(); ++i) {
	delete i->second;
    }
    valuelists.clear();
}

ValueStreamDocument::~ValueStreamDocument()
{
    delete doc;
    clear_valuelists(valuelists);
}

void
ValueStreamDocument::new_shard(Xapian::doccount n)
{
    AssertRel(n,>,0);
    AssertRel(n,<,n_shards);
    current = unsigned(n);
    // This method should only be called for a MultiDatabase.
    auto multidb = static_cast<MultiDatabase*>(db.internal.get());
    database = multidb->shards[n];
    // Ensure set_document()'s "same docid" check doesn't misfire.
    did = 0;
    clear_valuelists(valuelists);
}

string
ValueStreamDocument::fetch_value(Xapian::valueno slot) const
{
    pair<map<Xapian::valueno, ValueList *>::iterator, bool> ret;
    ret = valuelists.insert(make_pair(slot, static_cast<ValueList*>(NULL)));
    ValueList * vl;
    if (ret.second) {
	// Entry didn't already exist, so open a value list for slot.
	vl = database->open_value_list(slot);
	ret.first->second = vl;
    } else {
	vl = ret.first->second;
	if (!vl) {
	    return string();
	}
    }

    if (vl->check(did)) {
	if (vl->at_end()) {
	    delete vl;
	    ret.first->second = NULL;
	} else if (vl->get_docid() == did) {
	    return vl->get_value();
	}
    }

    return string();
}

void
ValueStreamDocument::fetch_all_values(map<Xapian::valueno, string> & v) const
{
    if (!doc) {
	doc = database->open_document(did, true);
    }
    doc->fetch_all_values(v);
}

string
ValueStreamDocument::fetch_data() const
{
    if (!doc) {
	doc = database->open_document(did, true);
    }
    return doc->fetch_data();
}
