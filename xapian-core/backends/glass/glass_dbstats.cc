/** @file glass_dbstats.cc
 * @brief Glass class for database statistics.
 */
/* Copyright (C) 2009,2010,2014,2015 Olly Betts
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

#include "glass_dbstats.h"

#include "pack.h"
#include "xapian/error.h"

using namespace std;

void
GlassDatabaseStats::unserialise(const string & serialised_stats)
{
    if (serialised_stats.empty()) {
	// If there's no entry yet, then all the values are zero.
	zero();
	return;
    }

    const char * p = serialised_stats.data();
    const char * end = p + serialised_stats.size();

    if (unpack_uint(&p, end, &doccount) &&
	unpack_uint(&p, end, &last_docid) &&
	unpack_uint(&p, end, &doclen_lbound) &&
	unpack_uint(&p, end, &wdf_ubound) &&
	unpack_uint(&p, end, &doclen_ubound) &&
	unpack_uint(&p, end, &oldest_changeset) &&
	unpack_uint_last(&p, end, &total_doclen)) {
	// last_docid must always be >= doccount.
	last_docid += doccount;
	// doclen_ubound should always be >= wdf_ubound, so we store the
	// difference as it may encode smaller.  wdf_ubound is likely to
	// be larger than doclen_lbound.
	doclen_ubound += wdf_ubound;
	return;
    }

    const char * m = p ?
	"Bad serialised DB stats (overflowed)" :
	"Bad serialised DB stats (out of data)";
    throw Xapian::DatabaseCorruptError(m);
}

string
GlassDatabaseStats::serialise() const
{
    string data;
    pack_uint(data, doccount);
    // last_docid must always be >= doccount.
    pack_uint(data, last_docid - doccount);
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
    return data;
}
