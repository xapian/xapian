/** @file honey_version.h
 * @brief HoneyVersion class
 */
/* Copyright (C) 2006,2007,2008,2009,2010,2013,2014,2015,2016,2018 Olly Betts
 * Copyright (C) 2011 Dan Colish
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

#ifndef XAPIAN_INCLUDED_HONEY_VERSION_H
#define XAPIAN_INCLUDED_HONEY_VERSION_H

#include "honey_defs.h"

#include "omassert.h"

#include <cstring>
#include <string>

#include "backends/uuids.h"
#include "internaltypes.h"
#include "xapian/types.h"

namespace Honey {

class RootInfo {
    off_t offset;
    off_t root;
    honey_tablesize_t num_entries;
    /// Should be >= 4 or 0 for no compression.
    uint4 compress_min;
    std::string fl_serialised;

  public:
    void init(uint4 compress_min_);

    void serialise(std::string &s) const;

    bool unserialise(const char ** p, const char * end);

    off_t get_offset() const { return offset; }
    off_t get_root() const { return root; }
    honey_tablesize_t get_num_entries() const { return num_entries; }
    uint4 get_compress_min() const { return compress_min; }
    const std::string & get_free_list() const { return fl_serialised; }

    void set_num_entries(honey_tablesize_t n) { num_entries = n; }
    void set_offset(off_t offset_) { offset = offset_; }
    void set_root(off_t root_) { root = root_; }
    void set_free_list(const std::string & s) { fl_serialised = s; }
};

}

/** Maximum size to allow for honey version file data in single file DB. */
#define HONEY_VERSION_MAX_SIZE 1024

/** The HoneyVersion class manages the revision files.
 *
 *  The "iamhoney" file (currently) contains a "magic" string identifying
 *  that this is a honey database, a database format version number, the UUID
 *  of the database, the revision of the database, and the root block info for
 *  each table.
 */
class HoneyVersion {
    honey_revision_number_t rev;

    Honey::RootInfo root[Honey::MAX_];
    Honey::RootInfo old_root[Honey::MAX_];

    /// The UUID of this database.
    Uuid uuid;

    /** File descriptor.
     *
     *  When committing, this hold the file descriptor of the new changes file
     *  between the call to the write() and sync() methods.
     *
     *  For a single-file database (when db_dir.empty()), this holds the fd of
     *  that file for use in read().
     */
    int fd;

    /** Offset into the file at which the version data starts.
     *
     *  Will be 0, except for an embedded multi-file database.
     */
    off_t offset;

    /// The database directory.
    std::string db_dir;

    /// The number of documents in the database.
    Xapian::doccount doccount;

    /// The total of the lengths of all documents in the database.
    Xapian::totallength total_doclen;

    /// Greatest document id ever used in this database.
    Xapian::docid last_docid;

    /// A lower bound on the smallest document length in this database.
    Xapian::termcount doclen_lbound;

    /// An upper bound on the greatest document length in this database.
    Xapian::termcount doclen_ubound;

    /// An upper bound on the greatest wdf in this database.
    Xapian::termcount wdf_ubound;

    /// An upper bound on the spelling wordfreq in this database.
    Xapian::termcount spelling_wordfreq_ubound;

    /// Oldest changeset removed when max_changesets is set
    mutable honey_revision_number_t oldest_changeset;

    /** A lower bound on the number of unique terms in a document in this
     *  database.
     */
    Xapian::termcount uniq_terms_lbound;

    /** An upper bound on the number of unique terms in a document in this
     *  database.
     */
    Xapian::termcount uniq_terms_ubound;

    /// The serialised database stats.
    std::string serialised_stats;

    // Serialise the database stats.
    void serialise_stats();

    // Unserialise the database stats.
    void unserialise_stats();

  public:
    explicit HoneyVersion(const std::string & db_dir_ = std::string())
	: rev(0), fd(-1), offset(0), db_dir(db_dir_),
	  doccount(0), total_doclen(0), last_docid(0),
	  doclen_lbound(0), doclen_ubound(0),
	  wdf_ubound(0), spelling_wordfreq_ubound(0),
	  oldest_changeset(0),
	  uniq_terms_lbound(0), uniq_terms_ubound(0) { }

    explicit HoneyVersion(int fd_);

    ~HoneyVersion();

    /** Create the version file. */
    void create();

    /** Read the version file and check it's a version we understand.
     *
     *  On failure, an exception is thrown.
     */
    void read();

    void cancel();

    const std::string write(honey_revision_number_t new_rev, int flags);

    bool sync(const std::string & tmpfile,
	      honey_revision_number_t new_rev, int flags);

    honey_revision_number_t get_revision() const { return rev; }

    const Honey::RootInfo& get_root(Honey::table_type tbl) const {
	return root[tbl];
    }

    Honey::RootInfo* root_to_set(Honey::table_type tbl) {
	return &root[tbl];
    }

    /// Return pointer to 16 byte UUID.
    const char * get_uuid() const {
	return uuid.data();
    }

    /// Return UUID in the standard 36 character string format.
    std::string get_uuid_string() const {
	return uuid.to_string();
    }

    Xapian::doccount get_doccount() const { return doccount; }

    Xapian::totallength get_total_doclen() const { return total_doclen; }

    Xapian::docid get_last_docid() const { return last_docid; }

    Xapian::termcount get_doclength_lower_bound() const {
	return doclen_lbound;
    }

    Xapian::termcount get_doclength_upper_bound() const {
	return doclen_ubound;
    }

    Xapian::termcount get_wdf_upper_bound() const { return wdf_ubound; }

    Xapian::termcount get_spelling_wordfreq_upper_bound() const {
	return spelling_wordfreq_ubound;
    }

    honey_revision_number_t get_oldest_changeset() const {
	return oldest_changeset;
    }

    Xapian::termcount get_unique_terms_lower_bound() const {
	return uniq_terms_lbound;
    }

    Xapian::termcount get_unique_terms_upper_bound() const {
	return uniq_terms_ubound;
    }

    void set_last_docid(Xapian::docid did) { last_docid = did; }

    void set_oldest_changeset(honey_revision_number_t changeset) const {
	oldest_changeset = changeset;
    }

    void set_spelling_wordfreq_upper_bound(Xapian::termcount ub) {
	spelling_wordfreq_ubound = ub;
    }

    void set_unique_terms_lower_bound(Xapian::termcount ub) {
	uniq_terms_lbound = ub;
    }

    void set_unique_terms_upper_bound(Xapian::termcount ub) {
	uniq_terms_ubound = ub;
    }

    void add_document(Xapian::termcount doclen) {
	++doccount;
	if (total_doclen == 0 || (doclen && doclen < doclen_lbound))
	    doclen_lbound = doclen;
	if (doclen > doclen_ubound)
	    doclen_ubound = doclen;
	total_doclen += doclen;
    }

    void delete_document(Xapian::termcount doclen) {
	--doccount;
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

    /** Merge the database stats.
     *
     *  Used by compaction.
     */
    void merge_stats(const HoneyVersion & o);

    void merge_stats(Xapian::doccount o_doccount,
		     Xapian::termcount o_doclen_lbound,
		     Xapian::termcount o_doclen_ubound,
		     Xapian::termcount o_wdf_ubound,
		     Xapian::totallength o_total_doclen,
		     Xapian::termcount o_spelling_wordfreq_ubound,
		     Xapian::termcount o_uniq_terms_lbound,
		     Xapian::termcount o_uniq_terms_ubound);

    bool single_file() const { return db_dir.empty(); }

    off_t get_offset() const { return offset; }
};

#endif // XAPIAN_INCLUDED_HONEY_VERSION_H
