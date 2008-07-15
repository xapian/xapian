/* multi_alltermslist.cc
 *
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2003,2007 Olly Betts
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

#include "multialltermslist.h"

#include "omassert.h"

using namespace std;

MultiAllTermsList::MultiAllTermsList(const vector<TermList *> &lists_)
	: lists(lists_), is_at_end(false), started(false)
{
}

MultiAllTermsList::~MultiAllTermsList()
{
    vector<TermList *>::const_iterator i;
    for (i = lists.begin(); i != lists.end(); ++i) {
	delete *i;
    }
    lists.clear();
}

void
MultiAllTermsList::update_current()
{
    bool found_term = false;

    vector<TermList *>::const_iterator i;
    for (i = lists.begin(); i != lists.end(); ++i) {
	if ((*i)->at_end()) {
	    continue;
	} else if (!found_term) {
	    current = (*i)->get_termname();
	    found_term = true;
	} else {
	    string newterm = (*i)->get_termname();
	    if (newterm < current) {
		current = newterm;
	    }
	}
    }
    if (!found_term) {
	is_at_end = true;
    }
}

Xapian::termcount
MultiAllTermsList::get_approx_size() const
{
    Xapian::termcount size = 0;

    vector<TermList *>::const_iterator i;
    for (i = lists.begin(); i != lists.end(); ++i) {
	size += (*i)->get_approx_size();
    }
    return size;
}

string
MultiAllTermsList::get_termname() const
{
    Assert(started);
    return current;
}

Xapian::doccount
MultiAllTermsList::get_termfreq() const
{
    Assert(started);
    Xapian::doccount termfreq = 0;

    vector<TermList *>::const_iterator i;
    for (i = lists.begin(); i != lists.end(); ++i) {
	if (!(*i)->at_end() &&
	    (*i)->get_termname() == current) {
	    termfreq += (*i)->get_termfreq();
	}
    }
    return termfreq;
}

Xapian::termcount
MultiAllTermsList::get_collection_freq() const
{
    Xapian::termcount collection_freq = 0;

    vector<TermList *>::const_iterator i;
    for (i = lists.begin(); i != lists.end(); ++i) {
	if (!(*i)->at_end() &&
	    (*i)->get_termname() == current) {
	    collection_freq += (*i)->get_collection_freq();
	}
    }
    return collection_freq;
}

TermList *
MultiAllTermsList::skip_to(const string &tname)
{
    started = true;

    vector<TermList *>::const_iterator i;
    for (i = lists.begin(); i != lists.end(); ++i) {
	if (!(*i)->at_end())
	    (*i)->skip_to(tname);
    }
    update_current();

    return NULL;
}

TermList *
MultiAllTermsList::next()
{
    if (!started) {
	started = true;

	vector<TermList *>::const_iterator i;
	for (i = lists.begin(); i != lists.end(); ++i) {
	    (*i)->next();
	}
    } else {
	vector<TermList *>::const_iterator i;
	for (i = lists.begin(); i != lists.end(); ++i) {
	    if (!(*i)->at_end() && (*i)->get_termname() == current) {
		(*i)->next();
	    }
	}
    }
    update_current();
    return NULL;
}

bool
MultiAllTermsList::at_end() const
{
    Assert(started);

    return is_at_end;
}
