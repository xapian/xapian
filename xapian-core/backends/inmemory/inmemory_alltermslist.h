/** @file
 * @brief Iterate all terms in an inmemory db
 */
/* Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2003,2008,2009,2011,2017,2024 Olly Betts
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

#ifndef XAPIAN_INCLUDED_INMEMORY_ALLTERMSLIST_H
#define XAPIAN_INCLUDED_INMEMORY_ALLTERMSLIST_H

#include "backends/alltermslist.h"
#include "inmemory_database.h"

/** class for alltermslists over several databases */
class InMemoryAllTermsList : public AllTermsList
{
  private:
    /// Copying is not allowed.
    InMemoryAllTermsList(const InMemoryAllTermsList &);

    /// Assignment is not allowed.
    void operator=(const InMemoryAllTermsList &);

    const std::map<std::string,
		   InMemoryTerm,
		   std::less<>>* tmap;

    std::map<std::string, InMemoryTerm, std::less<>>::const_iterator it;

    Xapian::Internal::intrusive_ptr<const InMemoryDatabase> database;

    std::string prefix;

  public:
    /// Constructor.
    InMemoryAllTermsList(const std::map<std::string,
					InMemoryTerm,
					std::less<>>* tmap_,
			 Xapian::Internal::intrusive_ptr<const InMemoryDatabase> database_,
			 std::string_view prefix_)
	: tmap(tmap_), it(tmap->begin()), database(database_),
	  prefix(prefix_)
    {
    }

    Xapian::termcount get_approx_size() const;

    // Get num of docs indexed by term
    Xapian::doccount get_termfreq() const;

    TermList* skip_to(std::string_view tname);

    /** next() causes the AllTermsList to move to the next term in the list.
     */
    TermList * next();
};

#ifdef DISABLE_GPL_LIBXAPIAN
# error GPL source we cannot relicense included in libxapian
#endif

#endif /* XAPIAN_INCLUDED_INMEMORY_ALLTERMSLIST_H */
