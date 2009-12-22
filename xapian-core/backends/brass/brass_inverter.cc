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
    map<string, PostingChanges>::iterator j;
    j = postlist_changes.find(term);
    if (j == postlist_changes.end()) return;

    // Flush buffered changes for just this term's postlist.
    table.merge_changes(term, j->second);
    postlist_changes.erase(j);
}

void
Inverter::flush(BrassPostListTable & table)
{
    flush_doclengths(table);
    map<string, PostingChanges>::const_iterator j;
    for (j = postlist_changes.begin(); j != postlist_changes.end(); ++j) {
	table.merge_changes(j->first, j->second);
    }
    postlist_changes.clear();
}
