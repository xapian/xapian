/* multialltermslist.cc
 *
 * ----START-LICENCE----
 * Copyright 1999,2000 BrightStation PLC
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 * USA
 * -----END-LICENCE-----
 */

#include "multialltermslist.h"

MultiAllTermsList::MultiAllTermsList(const std::vector<RefCntPtr<AllTermsList> > &lists_)
	: lists(lists_), is_at_end(false)
{
    update_current();
}

MultiAllTermsList::~MultiAllTermsList()
{
}

void
MultiAllTermsList::update_current()
{
    bool found_term = false;

    std::vector<RefCntPtr<AllTermsList> >::const_iterator i;
    for (i = lists.begin(); i != lists.end(); ++i) {
	if ((*i)->at_end()) {
	    continue;
	} else if (!found_term) {
	    current = (*i)->get_termname();
	    found_term = true;
	} else {
	    std::string newterm = (*i)->get_termname();
	    if (newterm < current) {
		current = newterm;
	    }
	}
    }
    if (!found_term) {
	is_at_end = true;
    }
}

const om_termname
MultiAllTermsList::get_termname() const
{
    return current;
}

om_doccount
MultiAllTermsList::get_termfreq() const
{
    om_doccount termfreq = 0;

    std::vector<RefCntPtr<AllTermsList> >::const_iterator i;
    for (i = lists.begin(); i!=lists.end(); ++i) {
	if (!(*i)->at_end() &&
	    (*i)->get_termname() == current) {
	    termfreq += (*i)->get_termfreq();
	}
    }
    return termfreq;
}

om_termcount
MultiAllTermsList::get_collection_freq() const
{
    om_termcount collection_freq = 0;

    std::vector<RefCntPtr<AllTermsList> >::const_iterator i;
    for (i = lists.begin(); i!=lists.end(); ++i) {
	if (!(*i)->at_end() &&
	    (*i)->get_termname() == current) {
	    collection_freq += (*i)->get_collection_freq();
	}
    }
    return collection_freq;
}

bool
MultiAllTermsList::skip_to(const om_termname &tname)
{
    bool result = false;

    std::vector<RefCntPtr<AllTermsList> >::const_iterator i;
    for (i = lists.begin(); i!=lists.end(); ++i) {
	result = result || (*i)->skip_to(tname);
    }
    update_current();

    return result;
}

bool
MultiAllTermsList::next()
{
    bool result = false;

    std::vector<RefCntPtr<AllTermsList> >::const_iterator i;
    for (i = lists.begin(); i!=lists.end(); ++i) {
	if (!(*i)->at_end() &&
	    (*i)->get_termname() == current) {
	    result = result || (*i)->next();
	}
    }
    update_current();

    return result;
}

bool
MultiAllTermsList::at_end() const
{
    return is_at_end;
}
