/** @file chert_modifiedpostlist.cc
 * @brief A ChertPostList plus pending modifications
 */
/* Copyright (C) 2006,2007,2008,2009,2010,2011,2014,2015 Olly Betts
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
#include "chert_modifiedpostlist.h"

#include "chert_database.h"
#include "debuglog.h"

ChertModifiedPostList::~ChertModifiedPostList()
{
    delete poslist;
}

void
ChertModifiedPostList::skip_deletes(double w_min)
{
    while (!ChertPostList::at_end()) {
	while (it != mods.end() && it->second.first == 'D' &&
	       it->first < ChertPostList::get_docid())
	    ++it;
	if (it == mods.end()) return;
	if (it->first != ChertPostList::get_docid()) return;
	if (it->second.first != 'D') return;
	++it;
	ChertPostList::next(w_min);
    }
    while (it != mods.end() && it->second.first == 'D') ++it;
}

Xapian::doccount
ChertModifiedPostList::get_termfreq() const
{
    Xapian::doccount tf;
    this_db->get_freqs(term, &tf, NULL);
    return tf;
}

Xapian::docid
ChertModifiedPostList::get_docid() const
{
    if (it == mods.end()) return ChertPostList::get_docid();
    if (ChertPostList::at_end()) return it->first;
    Assert(it->second.first != 'D');
    return min(it->first, ChertPostList::get_docid());
}

Xapian::termcount
ChertModifiedPostList::get_doclength() const
{
    LOGCALL(DB, Xapian::termcount, "ChertModifiedPostList::get_doclength", NO_ARGS);
    if (it != mods.end() && (ChertPostList::at_end() || it->first <= ChertPostList::get_docid()))
	RETURN(this_db->get_doclength(it->first));
    RETURN(ChertPostList::get_doclength());
}

Xapian::termcount
ChertModifiedPostList::get_unique_terms() const
{
    LOGCALL(DB, Xapian::termcount, "ChertModifiedPostList::get_unique_terms", NO_ARGS);
    if (it != mods.end() && (ChertPostList::at_end() || it->first <= ChertPostList::get_docid()))
	RETURN(this_db->get_unique_terms(it->first));
    RETURN(ChertPostList::get_unique_terms());
}

Xapian::termcount
ChertModifiedPostList::get_wdf() const
{
    if (ChertPostList::at_end()) return it->second.second;
    Xapian::docid unmod_did = ChertPostList::get_docid();
    if (it != mods.end() && it->first <= unmod_did) {
	if (it->first < unmod_did) return it->second.second;
	return it->second.second;
    }
    return ChertPostList::get_wdf();
}

PositionList *
ChertModifiedPostList::read_position_list()
{
    if (it != mods.end() && (ChertPostList::at_end() || it->first <= ChertPostList::get_docid())) {
	if (poslist) {
	    delete poslist;
	    poslist = NULL;
	}
	poslist = this_db->open_position_list(it->first, term);
	return poslist;
    }
    return ChertPostList::read_position_list();
}

PositionList *
ChertModifiedPostList::open_position_list() const
{
    if (it != mods.end() && (ChertPostList::at_end() || it->first <= ChertPostList::get_docid())) {
	return this_db->open_position_list(it->first, term);
    }
    return ChertPostList::open_position_list();
}

PostList *
ChertModifiedPostList::next(double w_min)
{
    if (have_started) {
	if (ChertPostList::at_end()) {
	    ++it;
	    skip_deletes(w_min);
	    return NULL;
	}
	Xapian::docid unmod_did = ChertPostList::get_docid();
	if (it != mods.end() && it->first <= unmod_did) {
	    if (it->first < unmod_did && it->second.first != 'D') {
		++it;
		skip_deletes(w_min);
		return NULL;
	    }
	    ++it;
	}
    }
    ChertPostList::next(w_min);
    skip_deletes(w_min);
    return NULL;
}

PostList *
ChertModifiedPostList::skip_to(Xapian::docid desired_did, double w_min)
{
    if (!ChertPostList::at_end()) ChertPostList::skip_to(desired_did, w_min);
    /* FIXME: should we use lower_bound() on the map? */
    while (it != mods.end() && it->first < desired_did) ++it;
    skip_deletes(w_min);
    return NULL;
}

bool
ChertModifiedPostList::at_end() const {
    return it == mods.end() && ChertPostList::at_end();
}

std::string
ChertModifiedPostList::get_description() const
{
    std::string desc = "ChertModifiedPostList(";
    desc += ChertPostList::get_description();
    desc += ')';
    return desc;
}
