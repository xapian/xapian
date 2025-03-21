/** @file
 * @brief HoneyInverter class which "inverts the file".
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

#include "honey_inverter.h"

#include "honey_positionlist.h"
#include "honey_postlist.h"
#include "honey_postlisttable.h"

#include "api/termlist.h"

#include <map>
#include <string>

using namespace std;

void
HoneyInverter::store_positions(const HoneyPositionTable& position_table,
			       Xapian::docid did,
			       const string& term,
			       const Xapian::VecCOW<Xapian::termpos>& posvec,
			       bool modifying)
{
    string s;
    position_table.pack(s, posvec);
    if (modifying) {
	auto i = pos_changes.find(term);
	if (i != pos_changes.end()) {
	    map<Xapian::docid, string>& m = i->second;
	    auto j = m.find(did);
	    if (j != m.end()) {
		// Update existing entry.
		swap(j->second, s);
		return;
	    }
	}
	const string& key = position_table.make_key(did, term);
	string old_tag;
	if (position_table.get_exact_entry(key, old_tag) && s == old_tag) {
	    // Identical to existing entry on disk.
	    return;
	}
    }
    set_positionlist(did, term, s);
}

void
HoneyInverter::set_positionlist(const HoneyPositionTable& position_table,
				Xapian::docid did,
				const string& term,
				const Xapian::TermIterator& term_it,
				bool modifying)
{
    auto ptr = term_it.internal->get_vec_termpos();
    if (ptr) {
	if (!ptr->empty()) {
	    store_positions(position_table, did, term, *ptr, modifying);
	    return;
	}
    } else {
	Xapian::PositionIterator pos = term_it.positionlist_begin();
	if (pos != term_it.positionlist_end()) {
	    Xapian::VecCOW<Xapian::termpos> posvec;
	    posvec.reserve(term_it.positionlist_count());
	    while (pos != term_it.positionlist_end()) {
		posvec.push_back(*pos);
		++pos;
	    }
	    store_positions(position_table, did, term, posvec, modifying);
	    return;
	}
    }
    // If we get here, the new position list was empty.
    if (modifying)
	delete_positionlist(did, term);
}

void
HoneyInverter::set_positionlist(Xapian::docid did,
				const string& term,
				const string& s)
{
    pos_changes.insert(make_pair(term, map<Xapian::docid, string>()))
	.first->second[did] = s;
}

void
HoneyInverter::delete_positionlist(Xapian::docid did,
				   const string& term)
{
    set_positionlist(did, term, string());
}

bool
HoneyInverter::get_positionlist(Xapian::docid did,
				const string& term,
				string& s) const
{
    auto i = pos_changes.find(term);
    if (i == pos_changes.end())
	return false;
    const map<Xapian::docid, string>& m = i->second;
    auto j = m.find(did);
    if (j == m.end())
	return false;
    s = j->second;
    return true;
}

bool
HoneyInverter::has_positions(const HoneyPositionTable& position_table) const
{
    if (pos_changes.empty())
	return !position_table.empty();

    // FIXME: Can we cheaply keep track of some things to make this more
    // efficient?  E.g. how many sets and deletes we had in total perhaps.
    honey_tablesize_t changes = 0;
    for (auto i : pos_changes) {
	const map<Xapian::docid, string>& m = i.second;
	for (auto j : m) {
	    const string& s = j.second;
	    if (!s.empty())
		return true;
	    ++changes;
	}
    }

    // We have positions unless all the existing entries are removed.
    return changes != position_table.get_entry_count();
}

void
HoneyInverter::flush_doclengths(HoneyPostListTable& table)
{
    table.merge_doclen_changes(doclen_changes);
    doclen_changes.clear();
}

void
HoneyInverter::flush_post_list(HoneyPostListTable& table, const string& term)
{
    auto i = postlist_changes.find(term);
    if (i == postlist_changes.end()) return;

    // Flush buffered changes for just this term's postlist.
    table.merge_changes(term, i->second);
    postlist_changes.erase(i);
}

void
HoneyInverter::flush_all_post_lists(HoneyPostListTable& table)
{
    for (auto&& i : postlist_changes) {
	table.merge_changes(i.first, i.second);
    }
    postlist_changes.clear();
}

void
HoneyInverter::flush_post_lists(HoneyPostListTable& table, const string& pfx)
{
    if (pfx.empty())
	return flush_all_post_lists(table);

    auto begin = postlist_changes.lower_bound(pfx);
    decltype(begin) end;
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

    for (auto i = begin; i != end; ++i) {
	table.merge_changes(i->first, i->second);
    }

    // Erase all the entries in one go, as that's:
    //  O(log(postlist_changes.size()) + O(number of elements removed)
    postlist_changes.erase(begin, end);
}

void
HoneyInverter::flush(HoneyPostListTable& table)
{
    flush_doclengths(table);
    flush_all_post_lists(table);
}

void
HoneyInverter::flush_pos_lists(HoneyPositionTable& table)
{
    for (auto i : pos_changes) {
	const string& term = i.first;
	const map<Xapian::docid, string>& m = i.second;
	for (auto j : m) {
	    Xapian::docid did = j.first;
	    const string& s = j.second;
	    if (!s.empty())
		table.set_positionlist(did, term, s);
	    else
		table.delete_positionlist(did, term);
	}
    }
    pos_changes.clear();
}
