/** @file orpositionlist.cc
 * @brief Merge two PositionList objects using an OR operation.
 */
/* Copyright (C) 2007,2010,2016,2017 Olly Betts
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

#include "orpositionlist.h"

#include "debuglog.h"

using namespace std;

Xapian::termcount
OrPositionList::get_size() const
{
    LOGCALL(EXPAND, Xapian::termcount, "OrPositionList::get_size", NO_ARGS);
    // This is actually the upper bound, but generally there's only one term
    // at each position, so it'll usually be correct too.
    Xapian::termcount size = 0;
    for (auto pl : pls) size += pl->get_size();
    RETURN(size);
}

Xapian::termpos
OrPositionList::get_position() const
{
    LOGCALL(EXPAND, Xapian::termpos, "OrPositionList::get_position", NO_ARGS);
    RETURN(current_pos);
}

// PositionList::next() is actually rarely used - ExactPhrasePostList will
// never call it, while PhrasePostList will only call it once for the first
// subquery and NearPostList will call it to start subqueries if we're near
// the start of the document, and also if a candidate match has two subqueries
// at the same position.
void
OrPositionList::next()
{
    LOGCALL_VOID(EXPAND, "OrPositionList::next", NO_ARGS);
    bool first = current.empty();
    if (first) current.resize(pls.size());
    Xapian::termpos old_pos = current_pos;
    current_pos = Xapian::termpos(-1);
    size_t j = 0;
    for (size_t i = 0; i != pls.size(); ++i) {
	PositionList* pl = pls[i];
	Xapian::termpos pos;
	if (first || current[i] <= old_pos) {
	    pl->next();
	    if (pl->at_end()) continue;
	    pos = pl->get_position();
	} else {
	    pos = current[i];
	}
	current_pos = min(current_pos, pos);
	current[j] = pos;
	if (i != j) pls[j] = pls[i];
	++j;
    }
    pls.resize(j);
}

// A min-heap seems like an obvious optimisation here, but is only useful when
// handling clumps of terms - in particular when skip_to() advances all the
// sublists, the heap doesn't help (but we have the cost of rebuilding it, or N
// pop+push calls which has a worse complexity than rebuilding).
void
OrPositionList::skip_to(Xapian::termpos termpos)
{
    LOGCALL_VOID(EXPAND, "OrPositionList::skip_to", termpos);
    bool first = current.empty();
    if (termpos <= current_pos && !first) return;
    if (first) current.resize(pls.size());
    current_pos = Xapian::termpos(-1);
    size_t j = 0;
    for (size_t i = 0; i != pls.size(); ++i) {
	PositionList* pl = pls[i];
	Xapian::termpos pos;
	if (first || termpos > current[i]) {
	    pl->skip_to(termpos);
	    if (pl->at_end()) continue;
	    pos = pl->get_position();
	} else {
	    pos = current[i];
	}
	current_pos = min(current_pos, pos);
	current[j] = pos;
	if (i != j) pls[j] = pls[i];
	++j;
    }
    pls.resize(j);
}

bool
OrPositionList::at_end() const
{
    LOGCALL(EXPAND, bool, "OrPositionList::at_end", NO_ARGS);
    RETURN(pls.empty());
}
