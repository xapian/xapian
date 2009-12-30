/** @file brass_inverter.cc
 * @brief Inverter class which "inverts the file".
 */
/* Copyright (C) 2009 Olly Betts
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

#include <map>
#include <string>

using namespace std;

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
