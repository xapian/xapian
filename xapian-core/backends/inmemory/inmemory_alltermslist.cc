/** @file
 * @brief Iterate all terms in an inmemory db
 */
/* Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2003,2004,2007,2008,2009,2017 Olly Betts
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301
 * USA
 */

#include <config.h>
#include "inmemory_alltermslist.h"

#include "stringutils.h"

using namespace std;

Xapian::termcount
InMemoryAllTermsList::get_approx_size() const
{
    // This may be an over-estimate due to deleted entries, and we may be
    // restricted to a prefix, but we only use this value to build a balanced
    // or-tree, and it'll do a decent job for that.
    return tmap->size();
}

Xapian::doccount
InMemoryAllTermsList::get_termfreq() const
{
    if (database->is_closed()) InMemoryDatabase::throw_database_closed();
    Assert(it != tmap->end());
    Assert(!it->first.empty());
    /* FIXME: this isn't quite right. */
    return it->second.docs.size();
}

TermList *
InMemoryAllTermsList::skip_to(const string &tname_)
{
    if (database->is_closed()) InMemoryDatabase::throw_database_closed();
    string tname(tname_);
    Assert(it != tmap->end());
    if (!it->first.empty()) {
	// Don't skip backwards.
	if (tname <= it->first) return NULL;
    } else {
	// Don't skip to before where we're supposed to start.
	if (tname < prefix) {
	    tname = prefix;
	} else if (tname.empty()) {
	    ++it;
	    while (it != tmap->end() && it->second.term_freq == 0) ++it;
	    if (it == tmap->end())
		return this;
	    current_term = it->first;
	    return NULL;
	}
    }
    it = tmap->lower_bound(tname);
    while (it != tmap->end() && it->second.term_freq == 0) ++it;
    if (it == tmap->end() || !startswith(it->first, prefix)) {
	return this;
    }
    current_term = it->first;
    return NULL;
}

TermList *
InMemoryAllTermsList::next()
{
    if (database->is_closed()) InMemoryDatabase::throw_database_closed();
    Assert(it != tmap->end());
    if (it->first.empty() && !prefix.empty()) {
	it = tmap->lower_bound(prefix);
    } else {
	++it;
    }
    while (it != tmap->end() && it->second.term_freq == 0) ++it;
    if (it == tmap->end() || !startswith(it->first, prefix)) {
	return this;
    }
    current_term = it->first;
    return NULL;
}

#ifdef DISABLE_GPL_LIBXAPIAN
# error GPL source we cannot relicense included in libxapian
#endif
