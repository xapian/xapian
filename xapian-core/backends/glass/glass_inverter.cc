/** @file
 * @brief Inverter class which "inverts the file".
 */
/* Copyright (C) 2009,2013 Olly Betts
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

#include "glass_inverter.h"

#include "glass_postlist.h"
#include "glass_positionlist.h"

#include "api/termlist.h"

#include <map>
#include <string>

using namespace std;

void
Inverter::store_positions(const GlassPositionListTable & position_table,
			  Xapian::docid did,
			  const string & tname,
			  const vector<Xapian::termpos> & posvec,
			  bool modifying)
{
    string s;
    position_table.pack(s, posvec);
    if (modifying && has_positions_cache != 0) {
	// If we add positions we must then have positions, but if we remove
	// positions we don't know if we then have them or not.
	has_positions_cache = s.empty() ? -1 : 1;

	auto i = pos_changes.find(tname);
	if (i != pos_changes.end()) {
	    map<Xapian::docid, string> & m = i->second;
	    auto j = m.find(did);
	    if (j != m.end()) {
		// Update existing entry.
		swap(j->second, s);
		return;
	    }
	}
	const string & key = position_table.make_key(did, tname);
	string old_tag;
	if (position_table.get_exact_entry(key, old_tag) && s == old_tag) {
	    // Identical to existing entry on disk.
	    return;
	}
    } else {
	// If we add positions, we must then have positions.
	if (!s.empty()) has_positions_cache = 1;
    }
    set_positionlist(did, tname, s);
}

void
Inverter::set_positionlist(const GlassPositionListTable & position_table,
			   Xapian::docid did,
			   const string & tname,
			   const Xapian::TermIterator & term,
			   bool modifying)
{
    const std::vector<Xapian::termpos> * ptr;
    ptr = term.internal->get_vector_termpos();
    if (ptr) {
	if (!ptr->empty()) {
	    store_positions(position_table, did, tname, *ptr, modifying);
	    return;
	}
    } else {
	Xapian::PositionIterator pos = term.positionlist_begin();
	if (pos != term.positionlist_end()) {
	    vector<Xapian::termpos> posvec(pos, Xapian::PositionIterator());
	    store_positions(position_table, did, tname, posvec, modifying);
	    return;
	}
    }
    // If we get here, the new position list was empty.
    if (modifying)
	delete_positionlist(did, tname);
}

void
Inverter::set_positionlist(Xapian::docid did,
			   const string & term,
			   const string & s)
{
    has_positions_cache = s.empty() ? -1 : 1;
    pos_changes.insert(make_pair(term, map<Xapian::docid, string>()))
	.first->second[did] = s;
}

void
Inverter::delete_positionlist(Xapian::docid did,
			      const string & term)
{
    set_positionlist(did, term, string());
}

bool
Inverter::get_positionlist(Xapian::docid did,
			   const string & term,
			   string & s) const
{
    auto i = pos_changes.find(term);
    if (i == pos_changes.end())
	return false;
    const map<Xapian::docid, string> & m = i->second;
    auto j = m.find(did);
    if (j == m.end())
	return false;
    s = j->second;
    return true;
}

bool
Inverter::has_positions(const GlassPositionListTable & position_table) const
{
    if (has_positions_cache < 0) {
	// FIXME: Can we cheaply keep track of some things to make this more
	// efficient?  E.g. how many sets and deletes we had in total perhaps.
	glass_tablesize_t changes = 0;
	for (const auto& i : pos_changes) {
	    const map<Xapian::docid, string>& m = i.second;
	    for (const auto& j : m) {
		const string & s = j.second;
		if (!s.empty())
		    return true;
		++changes;
	    }
	}

	// We have positions unless all the existing entries are removed.
	has_positions_cache = (changes != position_table.get_entry_count());
    }
    return has_positions_cache;
}

void
Inverter::flush_doclengths(GlassPostListTable & table)
{
    table.merge_doclen_changes(doclen_changes);
    doclen_changes.clear();
}

void
Inverter::flush_post_list(GlassPostListTable & table, const string & term)
{
    map<string, PostingChanges>::iterator i;
    i = postlist_changes.find(term);
    if (i == postlist_changes.end()) return;

    // Flush buffered changes for just this term's postlist.
    table.merge_changes(term, i->second);
    postlist_changes.erase(i);
}

void
Inverter::flush_all_post_lists(GlassPostListTable & table)
{
    map<string, PostingChanges>::const_iterator i;
    for (i = postlist_changes.begin(); i != postlist_changes.end(); ++i) {
	table.merge_changes(i->first, i->second);
    }
    postlist_changes.clear();
}

void
Inverter::flush_post_lists(GlassPostListTable & table, const string & pfx)
{
    if (pfx.empty())
	return flush_all_post_lists(table);

    map<string, PostingChanges>::iterator i, begin, end;
    begin = postlist_changes.lower_bound(pfx);
    string pfxinc = pfx;
    while (true) {
	if (pfxinc.back() != '\xff') {
	    ++pfxinc.back();
	    end = postlist_changes.lower_bound(pfxinc);
	    break;
	}
	pfxinc.resize(pfxinc.size() - 1);
	if (pfxinc.empty()) {
	    end = postlist_changes.end();
	    break;
	}
    }

    for (i = begin; i != end; ++i) {
	table.merge_changes(i->first, i->second);
    }

    // Erase all the entries in one go, as that's:
    //  O(log(postlist_changes.size()) + O(number of elements removed)
    postlist_changes.erase(begin, end);
}

void
Inverter::flush(GlassPostListTable & table)
{
    flush_doclengths(table);
    flush_all_post_lists(table);
}

void
Inverter::flush_pos_lists(GlassPositionListTable & table)
{
    for (auto i : pos_changes) {
	const string & term = i.first;
	const map<Xapian::docid, string> & m = i.second;
	for (auto j : m) {
	    Xapian::docid did = j.first;
	    const string & s = j.second;
	    if (!s.empty())
		table.set_positionlist(did, term, s);
	    else
		table.delete_positionlist(did, term);
	}
    }
    pos_changes.clear();
    has_positions_cache = -1;
}
