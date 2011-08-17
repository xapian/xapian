/** @file brass_dbstats.cc
 * @brief Brass class for database statistics.
 */
/* Copyright (C) 2009 Olly Betts
 * Copyright (C) 2011 Dan Colish
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

#include "brass_dbstats.h"

#include "brass_postlist.h"

using namespace std;

/// The key in the postlist table which we use to store our encoded statistics.
static const string DATABASE_STATS_KEY(1, '\0');

void
BrassDatabaseStats::read(BrassPostListTable & postlist_table)
{
    string data;
    if (!postlist_table.get_exact_entry(DATABASE_STATS_KEY, data)) {
	// If there's no entry yet, then all the values are zero.
	zero();
	return;
    }

    const char * p = data.data();
    const char * end = p + data.size();

    if (unpack_uint(&p, end, &last_docid) &&
	unpack_uint(&p, end, &doclen_lbound) &&
	unpack_uint(&p, end, &wdf_ubound) &&
	unpack_uint(&p, end, &doclen_ubound) &&
	unpack_uint(&p, end, &oldest_changeset) &&
	unpack_uint_last(&p, end, &total_doclen)) {
	// doclen_ubound should always be >= wdf_ubound, so we store the
	// difference as it may encode smaller.  wdf_ubound is likely to
	// be larger than doclen_lbound.
	doclen_ubound += wdf_ubound;
	return;
    }

    if (p)
	throw Xapian::DatabaseCorruptError("Bad encoded DB stats (overflowed)");

    throw Xapian::DatabaseCorruptError("Bad encoded DB stats (out of data)");
}

void
BrassDatabaseStats::write(BrassPostListTable & postlist_table) const
{
    string data;
    pack_uint(data, last_docid);
    pack_uint(data, doclen_lbound);
    pack_uint(data, wdf_ubound);
    // doclen_ubound should always be >= wdf_ubound, so we store the
    // difference as it may encode smaller.  wdf_ubound is likely to
    // be larger than doclen_lbound.
    pack_uint(data, doclen_ubound - wdf_ubound);
    pack_uint(data, oldest_changeset);
    // Micro-optimisation: total_doclen is likely to be the largest value, so
    // store it last as pack_uint_last() uses a slightly more compact encoding
    // - this could save us a few bytes!
    pack_uint_last(data, total_doclen);
    postlist_table.add(DATABASE_STATS_KEY, data);
}
