/* multi_termlist.cc: C++ class definition for multiple database access
 *
 * ----START-LICENCE----
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002,2003,2004,2005 Olly Betts
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

#include "omdebug.h"
#include "multi_termlist.h"
#include "database.h"

MultiTermList::MultiTermList(LeafTermList * tl_,
			     const Xapian::Internal::RefCntPtr<const Xapian::Database::Internal> & termdb_,
			     const Xapian::Database &rootdb_)
	: tl(tl_), termdb(termdb_), rootdb(rootdb_)
{
    termfreq_factor = double(rootdb.get_doccount()) / termdb->get_doccount();
    DEBUGLINE(DB, "Approximation factor for termfreq: " << termfreq_factor);
}

MultiTermList::~MultiTermList()
{
    delete tl;
}

Xapian::termcount
MultiTermList::get_approx_size() const
{
    return tl->get_approx_size();
}

void
MultiTermList::set_weighting(const OmExpandWeight * wt_new)
{
    // Note: wt in the MultiTermList base class isn't ever set or used
    tl->set_weighting(wt_new);
}

OmExpandBits
MultiTermList::get_weighting() const {
    return tl->get_weighting();
}

string
MultiTermList::get_termname() const
{
    return tl->get_termname();
}

Xapian::termcount MultiTermList::get_wdf() const
{
    return tl->get_wdf();
}

Xapian::doccount MultiTermList::get_termfreq() const
{
    // Approximate term frequency
    return Xapian::doccount(tl->get_termfreq() * termfreq_factor);
}

TermList * MultiTermList::next()
{
    return tl->next();
}

bool MultiTermList::at_end() const
{
    return tl->at_end();
}
