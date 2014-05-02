/** @file valuestreamdocument.cc
 * @brief A document which gets its values from a ValueStreamManager.
 */
/* Copyright (C) 2009,2014 Olly Betts
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
ValueStreamDocument::new_subdb(int n)
{
    AssertRel(n,>,0);
    AssertRel(size_t(n),<,db.internal.size());
    current = unsigned(n);
    database = db.internal[n];
    clear_valuelists(valuelists);
}

string
ValueStreamDocument::do_get_value(Xapian::valueno slot) const
{
#ifdef XAPIAN_ASSERTIONS_PARANOID
    if (!doc) {
	doc = database->open_document(did, true);
    }
#endif

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
	    AssertEqParanoid(string(), doc->get_value(slot));
	    return string();
	}
    }

    if (vl->check(did)) {
	if (vl->at_end()) {
	    delete vl;
	    ret.first->second = NULL;
	} else if (vl->get_docid() == did) {
	    Assert(vl);
	    string v = vl->get_value();
	    AssertEq(v, doc->get_value(slot));
	    return v;
	}
    }
    AssertEqParanoid(string(), doc->get_value(slot));
    return string();
}

void
ValueStreamDocument::do_get_all_values(map<Xapian::valueno, string> & v) const
{
    if (!doc) {
	doc = database->open_document(did, true);
    }
    return doc->do_get_all_values(v);
}

string
ValueStreamDocument::do_get_data() const
{
    if (!doc) {
	doc = database->open_document(did, true);
    }
    return doc->do_get_data();
}
