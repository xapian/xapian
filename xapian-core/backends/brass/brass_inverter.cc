/** @file brass_inverter.cc
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

#include "brass_inverter.h"

#include "brass_postlist.h"
#include "brass_positionlist.h"

#include <map>
#include <string>

using namespace std;

void
Inverter::set_positionlist(Xapian::docid did,
			   const string & term,
			   const string & s)
{
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
    map<string, map<Xapian::docid, string> >::const_iterator i;
    i = pos_changes.find(term);
    if (i == pos_changes.end())
	return false;
    const map<Xapian::docid, string> & m = i->second;
    map<Xapian::docid, string>::const_iterator j;
    j = m.find(did);
    if (j == m.end())
	return false;
    s = j->second;
    return true;
}

bool
Inverter::has_positions(const BrassPositionListTable & position_table) const
{
    if (pos_changes.empty())
	return !position_table.empty();

    // FIXME: Can we cheaply keep track of some things to make this more
    // efficient?  E.g. how many sets and deletes we had in total perhaps.
    brass_tablesize_t changes = 0;
    map<string, map<Xapian::docid, string> >::const_iterator i;
    for (i = pos_changes.begin(); i != pos_changes.end(); ++i) {
	const map<Xapian::docid, string> & m = i->second;
	map<Xapian::docid, string>::const_iterator j;
	for (j = m.begin(); j != m.end(); ++j) {
	    const string & s = j->second;
	    if (!s.empty())
		return true;
	    ++changes;
	}
    }

    // We have positions unless all the existing entries are removed.
    return changes != position_table.get_entry_count();
}

void
Inverter::flush_doclengths(BrassPostListTable & table)
{
    table.merge_doclen_changes(doclen_changes);
    doclen_changes.clear();
}

void
Inverter::flush_post_list(BrassPostListTable & table, const string & term)
{
    map<string, PostingChanges>::iterator i;
    i = postlist_changes.find(term);
    if (i == postlist_changes.end()) return;

    // Flush buffered changes for just this term's postlist.
    table.merge_changes(term, i->second);
    postlist_changes.erase(i);
}

void
Inverter::flush_all_post_lists(BrassPostListTable & table)
{
    map<string, PostingChanges>::const_iterator i;
    for (i = postlist_changes.begin(); i != postlist_changes.end(); ++i) {
	table.merge_changes(i->first, i->second);
    }
    postlist_changes.clear();
}

void
Inverter::flush_post_lists(BrassPostListTable & table, const string & pfx)
{
    if (pfx.empty())
	return flush_all_post_lists(table);

    map<string, PostingChanges>::iterator i, begin, end;
    begin = postlist_changes.lower_bound(pfx);
    end = postlist_changes.upper_bound(pfx);

    for (i = begin; i != end; ++i) {
	table.merge_changes(i->first, i->second);
    }

    // Erase all the entries in one go, as that's:
    //  O(log(postlist_changes.size()) + O(number of elements removed)
    postlist_changes.erase(begin, end);
}

void
Inverter::flush(BrassPostListTable & table)
{
    flush_doclengths(table);
    flush_all_post_lists(table);
}

void
Inverter::flush_pos_lists(BrassPositionListTable & table)
{
    map<string, map<Xapian::docid, string> >::const_iterator i;
    for (i = pos_changes.begin(); i != pos_changes.end(); ++i) {
	const string & term = i->first;
	const map<Xapian::docid, string> & m = i->second;
	map<Xapian::docid, string>::const_iterator j;
	for (j = m.begin(); j != m.end(); ++j) {
	    Xapian::docid did = j->first;
	    const string & s = j->second;
	    if (!s.empty())
		table.set_positionlist(did, term, s);
	    else
		table.delete_positionlist(did, term);
	}
    }
    pos_changes.clear();
}
