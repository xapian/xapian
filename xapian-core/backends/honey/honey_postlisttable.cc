/** @file honey_postlisttable.cc
 * @brief Subclass of HoneyTable which holds postlists.
 */
/* Copyright (C) 2007,2008,2009,2010,2013,2014,2015,2016,2017 Olly Betts
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

#include "honey_postlisttable.h"

#include "honey_database.h"
#include "honey_postlist.h"

HoneyPostList*
HoneyPostListTable::open_post_list(const HoneyDatabase* db,
				   const std::string& term) const
{
    return new HoneyPostList(db, term);
}

void
HoneyPostListTable::get_freqs(const std::string& term,
			      Xapian::doccount* termfreq_ptr,
			      Xapian::termcount* collfreq_ptr) const
{
    (void)term;
    (void)termfreq_ptr;
    (void)collfreq_ptr;
    // TODO0
}

Xapian::termcount
HoneyPostListTable::get_doclength(Xapian::docid did) const
{
    (void)did;
    // TODO0
    return 0;
}

void
HoneyPostListTable::get_used_docid_range(Xapian::docid& first,
					 Xapian::docid& last) const
{
    // TODO1
    first = 1;
    last = Xapian::docid(-1);
}
