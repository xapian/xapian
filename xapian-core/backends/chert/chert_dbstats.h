/** @file chert_dbstats.h
 * @brief Chert class for database statistics.
 */
/* Copyright (C) 2009 Olly Betts
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

#ifndef XAPIAN_INCLUDED_CHERT_DBSTATS_H
#define XAPIAN_INCLUDED_CHERT_DBSTATS_H

#include "chert_types.h"
#include "xapian/types.h"

#include "internaltypes.h"

class ChertPostListTable;

/// Chert class for database statistics.
class ChertDatabaseStats {
    /// Don't allow assignment.
    void operator=(const ChertDatabaseStats &);

    /// Don't allow copying.
    ChertDatabaseStats(const ChertDatabaseStats &);

    /// The total of the lengths of all documents in the database.
    totlen_t total_doclen;

    /// Greatest document id ever used in this database.
    Xapian::docid last_docid;

    /// A lower bound on the smallest document length in this database.
    Xapian::termcount doclen_lbound;

    /// An upper bound on the greatest document length in this database.
    Xapian::termcount doclen_ubound;

    /// An upper bound on the greatest wdf in this database.
    Xapian::termcount wdf_ubound;

  public:
    ChertDatabaseStats()
	: total_doclen(0), last_docid(0), doclen_lbound(0), doclen_ubound(0),
	  wdf_ubound(0) { }

    totlen_t get_total_doclen() const { return total_doclen; }

    Xapian::docid get_last_docid() const { return last_docid; }

    Xapian::termcount get_doclength_lower_bound() const {
	return doclen_lbound;
    }

    Xapian::termcount get_doclength_upper_bound() const {
	return doclen_ubound;
    }

    Xapian::termcount get_wdf_upper_bound() const { return wdf_ubound; }

    void zero() {
	total_doclen = 0;
	last_docid = 0;
	doclen_lbound = 0;
	doclen_ubound = 0;
	wdf_ubound = 0;
    }

    void read(ChertPostListTable & postlist_table);

    void set_last_docid(Xapian::docid did) { last_docid = did; }

    void add_document(Xapian::termcount doclen) {
	if (total_doclen == 0 || (doclen && doclen < doclen_lbound))
	    doclen_lbound = doclen;
	if (doclen > doclen_ubound)
	    doclen_ubound = doclen;
	total_doclen += doclen;
    }

    void delete_document(Xapian::termcount doclen) {
	total_doclen -= doclen;
	// If the database no longer contains any postings, we can reset
	// doclen_lbound, doclen_ubound and wdf_ubound.
	if (total_doclen == 0) {
	    doclen_lbound = 0;
	    doclen_ubound = 0;
	    wdf_ubound = 0;
	}
    }

    void check_wdf(Xapian::termcount wdf) {
	if (wdf > wdf_ubound) wdf_ubound = wdf;
    }

    Xapian::docid get_next_docid() { return ++last_docid; }

    void write(ChertPostListTable & postlist_table) const;
};

#endif // XAPIAN_INCLUDED_CHERT_DBSTATS_H
