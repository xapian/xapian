/* daalltermslist.cc
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

#include "da_alltermslist.h"

DAAllTermsList::DAAllTermsList(RefCntPtr<const Database> database_,
			       const DA_term_info &term_,
			       DA_file *DA_t_)
	: database(database_),
	  term(term_),
	  DA_t(DA_t_),
	  is_at_end(false)
{
    update_cache();
}

DAAllTermsList::~DAAllTermsList()
{
}

void
DAAllTermsList::update_cache()
{
    current_term = std::string(reinterpret_cast<char *>(term.term + 1),
			       static_cast<std::string::size_type>(term.term[0]-1));
    termfreq = term.freq;
}

om_termname
DAAllTermsList::get_termname() const
{
    return current_term;
}

om_doccount
DAAllTermsList::get_termfreq() const
{
    return termfreq;
}

om_termcount
DAAllTermsList::get_collection_freq() const
{
    throw OmUnimplementedError("Collection frequency is not available in DA databases");
}

bool
DAAllTermsList::skip_to(const om_termname &tname)
{
    std::string kstring;
    kstring += static_cast<char>(tname.length() + 1);
    kstring += tname;

    bool result = DA_term(reinterpret_cast<const byte *>(kstring.data()),
			  &term, DA_t);
    update_cache();
    return result;
}

bool
DAAllTermsList::next()
{
    if (!DA_next_term(&term, DA_t)) {
	is_at_end = true;
	return false;
    }
    update_cache();
    return true;
}

bool
DAAllTermsList::at_end() const
{
    return is_at_end;
}
