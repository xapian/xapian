/* daalltermslist.cc
 *
 * ----START-LICENCE----
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2003 Olly Betts
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

#include <config.h>
#include "da_alltermslist.h"

DAAllTermsList::DAAllTermsList(Xapian::Internal::RefCntPtr<const Xapian::Database::Internal> database_,
			       const DA_term_info &term_,
			       DA_file *DA_t_)
	: database(database_),
	  term(term_),
	  DA_t(DA_t_),
	  started(false),
	  is_at_end(false)
{
    update_cache();
}

DAAllTermsList::~DAAllTermsList()
{
}

Xapian::termcount
DAAllTermsList::get_approx_size() const
{
    return DA_t->itemcount;
}

void
DAAllTermsList::update_cache()
{
    current_term = std::string(reinterpret_cast<char *>(term.term + 1),
			       static_cast<std::string::size_type>(term.term[0]-1));
    termfreq = term.freq;
}

string
DAAllTermsList::get_termname() const
{
    Assert(started);
    Assert(!is_at_end);
    return current_term;
}

Xapian::doccount
DAAllTermsList::get_termfreq() const
{
    Assert(started);
    Assert(!is_at_end);
    return termfreq;
}

Xapian::termcount
DAAllTermsList::get_collection_freq() const
{
    Assert(started);
    Assert(!is_at_end);
    throw Xapian::UnimplementedError("Collection frequency is not available in DA databases");
}

TermList *
DAAllTermsList::skip_to(const string &tname)
{
    started = true;
    std::string kstring;
    kstring += static_cast<char>(tname.length() + 1);
    kstring += tname;

    if (DA_term(reinterpret_cast<const byte *>(kstring.data()), &term, DA_t)) {
	update_cache();
    } else {
	is_at_end = true;
    }
    return NULL;
}

TermList *
DAAllTermsList::next()
{
    if (!started) {
	started = true;
    } else {
	if (DA_next_term(&term, DA_t)) {
	    update_cache();
	} else {
	    is_at_end = true;
	}
    }
    return NULL;
}

bool
DAAllTermsList::at_end() const
{
    return is_at_end;
}
