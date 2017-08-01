/** @file glass_database.h
 * @brief C++ class definition for glass database
 */
/* Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002 Ananova Ltd
 * Copyright 2002,2003,2004,2005,2006,2007,2008,2009,2010,2011,2012,2013,2014,2015,2016 Olly Betts
 * Copyright 2008 Lemur Consulting Ltd
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

#ifndef OM_HGUARD_GLASS_DATABASE_H
#define OM_HGUARD_GLASS_DATABASE_H

#include "backends/backends.h"
#include "backends/database.h"
#include "glass_changes.h"
#include "glass_docdata.h"
#include "glass_inverter.h"
#include "glass_positionlist.h"
#include "glass_postlist.h"
#include "glass_spelling.h"
#include "glass_synonym.h"
#include "glass_termlisttable.h"
#include "glass_values.h"
#include "glass_version.h"
#include "../flint_lock.h"
#include "glass_defs.h"
#include "backends/valuestats.h"

#include "noreturn.h"

#include "xapian/compactor.h"
#include "xapian/constants.h"

#include <map>

class GlassTermList;
class GlassAllDocsPostList;
class RemoteConnection;

/** A backend designed for efficient indexing and retrieval, using
 *  compressed posting lists and a btree storage scheme.
 */
class GlassDatabase : public Xapian::Database::Internal {
    friend class GlassWritableDatabase;
    friend class GlassTermList;
    friend class GlassPostList;
    friend class GlassAllTermsList;
    friend class GlassAllDocsPostList;
    private:
	/** Directory to store databases in.
	 */
	std::string db_dir;

	/** Whether the database is readonly.
	 */
	bool readonly;

	/** The file describing the Glass database.
	 *  This file has information about the format of the database
	 *  which can't easily be stored in any of the individual tables.
	 */
	GlassVersion version_file;

	/** Table storing posting lists.
	 *
	 *  Whenever an update is performed, this table is the first to be
	 *  updated: therefore, its most recent revision number is the most
	 *  recent anywhere in the database.
	 */
	mutable GlassPostListTable postlist_table;

	/** Table storing position lists.
	 */
	mutable GlassPositionListTable position_table;

	/** Table storing term lists.
	 */
	GlassTermListTable termlist_table;

	/** Value manager. */
	mutable GlassValueManager value_manager;

	/** Table storing synonym data.
	 */
	mutable GlassSynonymTable synonym_table;

	/** Table storing spelling correction data.
	 */
	mutable GlassSpellingTable spelling_table;

	/** Table storing document data.
	 */
	GlassDocDataTable docdata_table;

	/// Lock object.
	FlintLock lock;

	/// Replication changesets.
	GlassChanges changes;

	/** Return true if a database exists at the path specified for this
	 *  database.
	 */
	bool database_exists();

	/** Create new tables, and open them.
	 *  Any existing tables will be removed first.
	 */
	void create_and_open_tables(int flags, unsigned int blocksize);

	/** Open all tables at most recent revision.
	 *
	 *  @exception Xapian::DatabaseCorruptError is thrown if a problem is
	 *  found with the database's format.
	 *
	 *  @return false if the tables were already open at the most recent
	 *  revision.
	 */
	bool open_tables(int flags);

	/** Get a write lock on the database, or throw an
	 *  Xapian::DatabaseLockError if failure.
	 *
	 *  @param flags Bit-wise or of zero or more Xapian::DB_* constants
	 *
	 *  @param creating true if the database is in the process of being
	 *  created - if false, will throw a DatabaseOpening error if the lock
	 *  can't be acquired and the database doesn't exist.
	 */
	void get_database_write_lock(int flags, bool creating);

	/** Get an object holding the next revision number which should be
	 *  used in the tables.
	 *
	 *  @return the next revision number.
	 */
	glass_revision_number_t get_next_revision_number() const;

	/** Set the revision number in the tables.
	 *
	 *  This updates the disk tables so that the currently open revision
	 *  becomes the specified revision number.
	 *
	 *  @param new_revision The new revision number to store.  This must
	 *          be greater than the current revision number.  FIXME: If
	 *          we support rewinding to a previous revision, maybe this
	 *          needs to be greater than any previously used revision.
	 */
	void set_revision_number(int flags, glass_revision_number_t new_revision);

	/** Re-open tables to recover from an overwritten condition,
	 *  or just get most up-to-date version.
	 */
	bool reopen();

	/** Close all the tables permanently.
	 */
	void close();

	/** Called if a modifications fail.
	 *
	 *  @param msg is a string description of the exception that was
	 *  raised when the modifications failed.
	 */
	void modifications_failed(glass_revision_number_t new_revision,
				  const std::string & msg);

	/** Apply any outstanding changes to the tables.
	 *
	 *  If an error occurs during this operation, this will be signalled
	 *  by an exception being thrown.  In this case the contents of the
	 *  tables on disk will be left in an unmodified state (though possibly
	 *  with increased revision numbers), and the outstanding changes will
	 *  be lost.
	 */
	void apply();

	/** Cancel any outstanding changes to the tables.
	 */
	void cancel();

	/** Send a set of messages which transfer the whole database.
	 */
	void send_whole_database(RemoteConnection & conn, double end_time);

	/** Get the revision stored in a changeset.
	 */
	void get_changeset_revisions(const string & path,
				     glass_revision_number_t * startrev,
				     glass_revision_number_t * endrev) const;

    public:
	/** Create and open a glass database.
	 *
	 *  @exception Xapian::DatabaseCorruptError is thrown if a problem is
	 *	       found with the database's format.
	 *
	 *  @exception Xapian::DatabaseOpeningError thrown if database can't
	 *             be opened.
	 *
	 *  @exception Xapian::DatabaseVersionError thrown if database is in an
	 *             unsupported format.  This implies that the database was
	 *             created by an older or newer version of Xapian.
	 *
	 *  @param dbdir directory holding glass tables
	 *
	 *  @param block_size Block size, in bytes, to use when creating
	 *                    tables.  This is only important, and has the
	 *                    correct value, when the database is being
	 *                    created.
	 */
	explicit GlassDatabase(const string &db_dir_, int flags = Xapian::DB_READONLY_,
		      unsigned int block_size = 0u);

	explicit GlassDatabase(int fd);

	~GlassDatabase();

	/// Get a postlist table cursor (used by GlassValueList).
	GlassCursor * get_postlist_cursor() const {
	    return postlist_table.cursor_get();
	}

	/** Get an object holding the revision number which the tables are
	 *  opened at.
	 *
	 *  @return the current revision number.
	 */
	glass_revision_number_t get_revision_number() const;

	/** Virtual methods of Database::Internal. */
	//@{
	Xapian::doccount get_doccount() const;
	Xapian::docid get_lastdocid() const;
	Xapian::totallength get_total_length() const;
	Xapian::termcount get_doclength(Xapian::docid did) const;
	Xapian::termcount get_unique_terms(Xapian::docid did) const;
	void get_freqs(const string & term,
		       Xapian::doccount * termfreq_ptr,
		       Xapian::termcount * collfreq_ptr) const;
	Xapian::doccount get_value_freq(Xapian::valueno slot) const;
	std::string get_value_lower_bound(Xapian::valueno slot) const;
	std::string get_value_upper_bound(Xapian::valueno slot) const;
	Xapian::termcount get_doclength_lower_bound() const;
	Xapian::termcount get_doclength_upper_bound() const;
	Xapian::termcount get_wdf_upper_bound(const string & term) const;
	bool term_exists(const string & tname) const;
	bool has_positions() const;

	LeafPostList * open_post_list(const string & tname) const;
	ValueList * open_value_list(Xapian::valueno slot) const;
	Xapian::Document::Internal * open_document(Xapian::docid did, bool lazy) const;

	PositionList * open_position_list(Xapian::docid did, const string & term) const;
	TermList * open_term_list(Xapian::docid did) const;
	TermList * open_allterms(const string & prefix) const;

	TermList * open_spelling_termlist(const string & word) const;
	TermList * open_spelling_wordlist() const;
	Xapian::doccount get_spelling_frequency(const string & word) const;

	TermList * open_synonym_termlist(const string & term) const;
	TermList * open_synonym_keylist(const string & prefix) const;

	string get_metadata(const string & key) const;
	TermList * open_metadata_keylist(const std::string &prefix) const;
	void write_changesets_to_fd(int fd,
				    const string & start_revision,
				    bool need_whole_db,
				    Xapian::ReplicationInfo * info);
	string get_revision_info() const;
	string get_uuid() const;

	void request_document(Xapian::docid /*did*/) const;
	void readahead_for_query(const Xapian::Query &query);
	//@}

	XAPIAN_NORETURN(void throw_termlist_table_close_exception() const);

	int get_backend_info(string * path) const {
	    if (path) *path = db_dir;
	    return BACKEND_GLASS;
	}

	bool single_file() const { return version_file.single_file(); }

	void get_used_docid_range(Xapian::docid & first,
				  Xapian::docid & last) const;

	/** Return true if there are uncommitted changes. */
	virtual bool has_uncommitted_changes() const;

	bool locked() const;

	static void compact(Xapian::Compactor * compactor,
			    const char * destdir,
			    int fd,
			    const std::vector<Xapian::Database::Internal *> & sources,
			    const std::vector<Xapian::docid> & offset,
			    size_t block_size,
			    Xapian::Compactor::compaction_level compaction,
			    unsigned flags,
			    Xapian::docid last_docid);
};

/** A writable glass database.
 */
class GlassWritableDatabase : public GlassDatabase {
	mutable Inverter inverter;

	mutable map<Xapian::valueno, ValueStats> value_stats;

	/** The number of documents added, deleted, or replaced since the last
	 *  flush.
	 */
	mutable Xapian::doccount change_count;

	/// If change_count reaches this threshold we automatically flush.
	Xapian::doccount flush_threshold;

	/** A pointer to the last document which was returned by
	 *  open_document(), or NULL if there is no such valid document.  This
	 *  is used purely for comparing with a supplied document to help with
	 *  optimising replace_document.  When the document internals are
	 *  deleted, this pointer gets set to NULL.
	 */
	mutable Xapian::Document::Internal * modify_shortcut_document;

	/** The document ID for the last document returned by open_document().
	 */
	mutable Xapian::docid modify_shortcut_docid;

	/** Check if we should autoflush.
	 *
	 *  Called at the end of each document changing operation.
	 */
	void check_flush_threshold();

	/// Flush any unflushed postlist changes, but don't commit them.
	void flush_postlist_changes() const;

	/// Close all the tables permanently.
	void close();

	/// Apply changes.
	void apply();

	//@{
	/** Implementation of virtual methods: see Database::Internal for
	 *  details.
	 */
	void commit();

	/** Cancel pending modifications to the database. */
	void cancel();

	Xapian::docid add_document(const Xapian::Document & document);
	Xapian::docid add_document_(Xapian::docid did, const Xapian::Document & document);
	// Stop the default implementation of delete_document(term) and
	// replace_document(term) from being hidden.  This isn't really
	// a problem as we only try to call them through the base class
	// (where they aren't hidden) but some compilers generate a warning
	// about the hiding.
#ifndef _MSC_VER
	using Xapian::Database::Internal::delete_document;
	using Xapian::Database::Internal::replace_document;
#endif
	void delete_document(Xapian::docid did);
	void replace_document(Xapian::docid did, const Xapian::Document & document);

	Xapian::Document::Internal * open_document(Xapian::docid did,
						   bool lazy) const;

	//@}

    public:
	/** Create and open a writable glass database.
	 *
	 *  @exception Xapian::DatabaseOpeningError thrown if database can't
	 *             be opened.
	 *
	 *  @exception Xapian::DatabaseVersionError thrown if database is in an
	 *             unsupported format.  This implies that the database was
	 *             created by an older or newer version of Xapian.
	 *
	 *  @param dir directory holding glass tables
	 */
	GlassWritableDatabase(const string &dir, int flags, int block_size);

	~GlassWritableDatabase();

	/** Virtual methods of Database::Internal. */
	//@{
	Xapian::termcount get_doclength(Xapian::docid did) const;
	Xapian::termcount get_unique_terms(Xapian::docid did) const;
	void get_freqs(const string & term,
		       Xapian::doccount * termfreq_ptr,
		       Xapian::termcount * collfreq_ptr) const;
	Xapian::doccount get_value_freq(Xapian::valueno slot) const;
	std::string get_value_lower_bound(Xapian::valueno slot) const;
	std::string get_value_upper_bound(Xapian::valueno slot) const;
	bool term_exists(const string & tname) const;
	bool has_positions() const;

	LeafPostList * open_post_list(const string & tname) const;
	ValueList * open_value_list(Xapian::valueno slot) const;
	PositionList * open_position_list(Xapian::docid did, const string & term) const;
	TermList * open_term_list(Xapian::docid did) const;
	TermList * open_allterms(const string & prefix) const;

	void add_spelling(const string & word, Xapian::termcount freqinc) const;
	void remove_spelling(const string & word, Xapian::termcount freqdec) const;
	TermList * open_spelling_wordlist() const;

	TermList * open_synonym_keylist(const string & prefix) const;
	void add_synonym(const string & word, const string & synonym) const;
	void remove_synonym(const string & word, const string & synonym) const;
	void clear_synonyms(const string & word) const;

	void set_metadata(const string & key, const string & value);
	void invalidate_doc_object(Xapian::Document::Internal * obj) const;
	//@}

	/** Return true if there are uncommitted changes. */
	bool has_uncommitted_changes() const;
};

#endif /* OM_HGUARD_GLASS_DATABASE_H */
