/** @file deciderpostlist.cc
 * @brief PostList which applies a MatchDecider
 */
/* Copyright 2017 Olly Betts
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

#include "deciderpostlist.h"

#include "omassert.h"
#include <xapian/matchdecider.h>

bool
DeciderPostList::test_doc()
{
    // We know that doc holds a ValueStreamDocument.
    Xapian::Document::Internal* doc_int = doc.internal.get();
    ValueStreamDocument* vsdoc = static_cast<ValueStreamDocument*>(doc_int);

    vsdoc->set_shard_document(pl->get_docid());

    bool decision = (*decider)(doc);
    if (decision) {
	++decider->docs_allowed_;
    } else {
	++decider->docs_denied_;
    }
    return decision;
}

string
DeciderPostList::get_description() const
{
    string desc = "DeciderPostList(";
    desc += pl->get_description();
    desc += ')';
    return desc;
}
