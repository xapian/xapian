/** @file
 * @brief Merge two TermList objects using an OR operation.
 */
/* Copyright (C) 2007-2023 Olly Betts
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

#include "ortermlist.h"

#include "debuglog.h"
#include "omassert.h"

#include <xapian/positioniterator.h>

using namespace std;

void
OrTermList::check_started() const
{
    Assert(!left->get_termname().empty());
    Assert(!right->get_termname().empty());
}

OrTermList::~OrTermList()
{
    delete left;
    delete right;
}

Xapian::termcount
OrTermList::get_approx_size() const
{
    LOGCALL(EXPAND, Xapian::termcount, "OrTermList::get_approx_size", NO_ARGS);
    // This is actually the upper bound, but we only use this to order the
    // binary tree of OrTermList objects so it's probably not worth trying
    // to be more sophisticated.
    RETURN(left->get_approx_size() + right->get_approx_size());
}

void
OrTermList::accumulate_stats(Xapian::Internal::ExpandStats & stats) const
{
    LOGCALL_VOID(EXPAND, "OrTermList::accumulate_stats", stats);
    check_started();
    if (cmp <= 0)
	left->accumulate_stats(stats);
    if (cmp >= 0)
	right->accumulate_stats(stats);
}

Xapian::termcount
OrTermList::get_wdf() const
{
    LOGCALL(EXPAND, Xapian::termcount, "OrTermList::get_wdf", NO_ARGS);
    check_started();
    if (cmp < 0) RETURN(left->get_wdf());
    if (cmp > 0) RETURN(right->get_wdf());
    RETURN(left->get_wdf() + right->get_wdf());
}

Xapian::doccount
OrTermList::get_termfreq() const
{
    LOGCALL(EXPAND, Xapian::doccount, "OrTermList::get_termfreq", NO_ARGS);
    check_started();
    if (cmp < 0)
	RETURN(left->get_termfreq());
    Assert(cmp > 0 || left->get_termfreq() == right->get_termfreq());
    RETURN(right->get_termfreq());
}

TermList *
OrTermList::next()
{
    LOGCALL(EXPAND, TermList *, "OrTermList::next", NO_ARGS);
    // If we've not started yet, cmp will be zero so we'll take the third case
    // below which is what we want to do to get started.
    if (cmp < 0) {
	TermList* lret = left->next();
	if (lret == left) {
	    TermList *ret = right;
	    right = NULL;
	    // Prune.
	    RETURN(ret);
	}
	if (lret) {
	    delete left;
	    left = lret;
	}
    } else if (cmp > 0) {
	TermList* rret = right->next();
	if (rret == right) {
	    TermList *ret = left;
	    left = NULL;
	    // Prune.
	    RETURN(ret);
	}
	if (rret) {
	    delete right;
	    right = rret;
	}
    } else {
	TermList* lret = left->next();
	if (lret && lret != left) {
	    delete left;
	    left = lret;
	    lret = NULL;
	}
	TermList* rret = right->next();
	if (rret && rret != right) {
	    delete right;
	    right = rret;
	    rret = NULL;
	}
	if (lret) {
	    if (rret)
		return this;
	    TermList *ret = right;
	    right = NULL;
	    // Prune.
	    RETURN(ret);
	}
	if (rret) {
	    TermList *ret = left;
	    left = NULL;
	    // Prune.
	    RETURN(ret);
	}
    }
    cmp = left->get_termname().compare(right->get_termname());
    current_term = cmp < 0 ? left->get_termname() : right->get_termname();
    RETURN(NULL);
}

TermList *
OrTermList::skip_to(const string & term)
{
    LOGCALL(EXPAND, TermList *, "OrTermList::skip_to", term);
    TermList* lret = left->skip_to(term);
    if (lret && lret != left) {
	delete left;
	left = lret;
	lret = NULL;
    }
    TermList* rret = right->skip_to(term);
    if (rret && rret != right) {
	delete right;
	right = rret;
	rret = NULL;
    }
    if (lret) {
	// Left at end.
	if (rret) {
	    // Both at end.
	    RETURN(this);
	}
	TermList *ret = right;
	right = NULL;
	// Prune.
	RETURN(ret);
    }
    if (rret) {
	// Right at end.
	TermList *ret = left;
	left = NULL;
	// Prune.
	RETURN(ret);
    }
    cmp = left->get_termname().compare(right->get_termname());
    current_term = cmp < 0 ? left->get_termname() : right->get_termname();
    RETURN(NULL);
}

Xapian::termcount
OrTermList::positionlist_count() const
{
    Assert(false);
    return 0;
}

PositionList*
OrTermList::positionlist_begin() const
{
    Assert(false);
    return NULL;
}


Xapian::doccount
FreqAdderOrTermList::get_termfreq() const
{
    LOGCALL(EXPAND, Xapian::doccount, "FreqAdderOrTermList::get_termfreq", NO_ARGS);
    check_started();
    if (cmp < 0) RETURN(left->get_termfreq());
    if (cmp > 0) RETURN(right->get_termfreq());
    RETURN(left->get_termfreq() + right->get_termfreq());
}
