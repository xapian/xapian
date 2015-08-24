/** @file brass_dbstats.h
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

#ifndef XAPIAN_INCLUDED_BRASS_DBSTATS_H
#define XAPIAN_INCLUDED_BRASS_DBSTATS_H

#include "brass_types.h"
#include "xapian/types.h"

#include "internaltypes.h"

class BrassPostListTable;

/// Brass class for database statistics.
class BrassDatabaseStats {
    /// Don't allow assignment.
    void operator=(const BrassDatabaseStats &);

    /// Don't allow copying.
    BrassDatabaseStats(const BrassDatabaseStats &);

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

    /// Oldest changeset removed when max_changesets is set
    brass_revision_number_t oldest_changeset;

  public:
    BrassDatabaseStats()
	: total_doclen(0), last_docid(0), doclen_lbound(0), doclen_ubound(0),
	  wdf_ubound(0), oldest_changeset(0) { }

    totlen_t get_total_doclen() const { return total_doclen; }

    Xapian::docid get_last_docid() const { return last_docid; }

    Xapian::termcount get_doclength_lower_bound() const {
	return doclen_lbound;
    }

    Xapian::termcount get_doclength_upper_bound() const {
	return doclen_ubound;
    }

    Xapian::termcount get_wdf_upper_bound() const { return wdf_ubound; }

    brass_revision_number_t get_oldest_changeset() const { return oldest_changeset; }

    void zero() {
	total_doclen = 0;
	last_docid = 0;
	doclen_lbound = 0;
	doclen_ubound = 0;
	wdf_ubound = 0;
	oldest_changeset = 0;
    }

    void read(BrassPostListTable & postlist_table);

    void set_last_docid(Xapian::docid did) { last_docid = did; }

    void set_oldest_changeset(brass_revision_number_t changeset) { oldest_changeset = changeset; }

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

    void write(BrassPostListTable & postlist_table) const;
};

#endif // XAPIAN_INCLUDED_BRASS_DBSTATS_H
