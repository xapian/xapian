/* brass_database.h: C++ class definition for brass database
 *
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002 Ananova Ltd
 * Copyright 2002,2003,2004,2005,2006,2007,2008,2009,2010,2012,2013 Olly Betts
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

#ifndef OM_HGUARD_BRASS_DATABASE_H
#define OM_HGUARD_BRASS_DATABASE_H

#include "database.h"
#include "brass_dbstats.h"
#include "brass_inverter.h"
#include "brass_positionlist.h"
#include "brass_postlist.h"
#include "brass_record.h"
#include "brass_spelling.h"
#include "brass_synonym.h"
#include "brass_termlisttable.h"
#include "brass_values.h"
#include "brass_version.h"
#include "../flint_lock.h"
#include "brass_types.h"
#include "valuestats.h"

#include "noreturn.h"

#include <map>

class BrassTermList;
class BrassAllDocsPostList;
class RemoteConnection;

/** A backend designed for efficient indexing and retrieval, using
 *  compressed posting lists and a btree storage scheme.
 */
class BrassDatabase : public Xapian::Database::Internal {
    friend class BrassWritableDatabase;
    friend class BrassTermList;
    friend class BrassPostList;
    friend class BrassAllTermsList;
    friend class BrassAllDocsPostList;
    private:
	/** Directory to store databases in.
	 */
	std::string db_dir;

	/** Whether the database is readonly.
	 */
	bool readonly;

	/** The file describing the Brass database.
	 *  This file has information about the format of the database
	 *  which can't easily be stored in any of the individual tables.
	 */
	BrassVersion version_file;

	/* Make postlist_table public so that bin/xapian-check.cc can get the
	 * revision number for the open database without us having to add to
	 * change the ABI.  On trunk the checking code is in the library, so
	 * this is a 1.2-only hack.
	 */
    public:
	/** Table storing posting lists.
	 *
	 *  Whenever an update is performed, this table is the first to be
	 *  updated: therefore, its most recent revision number is the most
	 *  recent anywhere in the database.
	 */
	mutable BrassPostListTable postlist_table;

    private:
	/** Table storing position lists.
	 */
	BrassPositionListTable position_table;

	/** Table storing term lists.
	 */
	BrassTermListTable termlist_table;

	/** Value manager. */
	mutable BrassValueManager value_manager;

	/** Table storing synonym data.
	 */
	mutable BrassSynonymTable synonym_table;

	/** Table storing spelling correction data.
	 */
	mutable BrassSpellingTable spelling_table;

	/** Table storing records.
	 *
	 *  Whenever an update is performed, this table is the last to be
	 *  updated: therefore, its most recent revision number is the most
	 *  recent consistent revision available.  If this table's most
	 *  recent revision number is not available for all tables, there
	 *  is no consistent revision available, and the database is corrupt.
	 */
	BrassRecordTable record_table;

	/// Lock object.
	FlintLock lock;

	/** The maximum number of changesets which should be kept in the
	 *  database. */
	unsigned int max_changesets;

	/// Database statistics.
	BrassDatabaseStats stats;

	/** Return true if a database exists at the path specified for this
	 *  database.
	 */
	bool database_exists();

	/** Create new tables, and open them.
	 *  Any existing tables will be removed first.
	 */
	void create_and_open_tables(unsigned int blocksize);

	/** Open all tables at most recent consistent revision.
	 *
	 *  @exception Xapian::DatabaseCorruptError is thrown if there is no
	 *  consistent revision available.
	 */
	void open_tables_consistent();

	/** Get a write lock on the database, or throw an
	 *  Xapian::DatabaseLockError if failure.
	 *
	 *  @param creating true if the database is in the process of being
	 *  created - if false, will throw a DatabaseOpening error if the lock
	 *  can't be acquired and the database doesn't exist.
	 */
	void get_database_write_lock(bool creating);

	/** Open tables at specified revision number.
	 *
	 *  @exception Xapian::InvalidArgumentError is thrown if the specified
	 *  revision is not available.
	 */
	void open_tables(brass_revision_number_t revision);

	/** Get an object holding the next revision number which should be
	 *  used in the tables.
	 *
	 *  @return the next revision number.
	 */
	brass_revision_number_t get_next_revision_number() const;

	/** Set the revision number in the tables.
	 *
	 *  This updates the disk tables so that the currently open revision
	 *  becomes the specified revision number.
	 *
	 *  @param new_revision The new revision number to store.  This must
	 *          be greater than the latest revision number (see
	 *          get_latest_revision_number()), or undefined behaviour will
	 *          result.
	 */
	void set_revision_number(brass_revision_number_t new_revision);

	/** Re-open tables to recover from an overwritten condition,
	 *  or just get most up-to-date version.
	 */
	void reopen();

	/** Close all the tables permanently.
	 */
	void close();

	/** Called if a modifications fail.
	 *
	 *  @param msg is a string description of the exception that was
	 *  raised when the modifications failed.
	 */
	void modifications_failed(brass_revision_number_t old_revision,
				  brass_revision_number_t new_revision,
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
				     brass_revision_number_t * startrev,
				     brass_revision_number_t * endrev) const;

    public:
	/** Create and open a brass database.
	 *
	 *  @exception Xapian::DatabaseCorruptError is thrown if there is no
	 *             consistent revision available.
	 *
	 *  @exception Xapian::DatabaseOpeningError thrown if database can't
	 *             be opened.
	 *
	 *  @exception Xapian::DatabaseVersionError thrown if database is in an
	 *             unsupported format.  This implies that the database was
	 *             created by an older or newer version of Xapian.
	 *
	 *  @param dbdir directory holding brass tables
	 *
	 *  @param block_size Block size, in bytes, to use when creating
	 *                    tables.  This is only important, and has the
	 *                    correct value, when the database is being
	 *                    created.
	 */
	BrassDatabase(const string &db_dir_, int action = XAPIAN_DB_READONLY,
		       unsigned int block_size = 0u);

	~BrassDatabase();

	/// Get a postlist table cursor (used by BrassValueList).
	BrassCursor * get_postlist_cursor() const {
	    return postlist_table.cursor_get();
	}

	/** Get an object holding the revision number which the tables are
	 *  opened at.
	 *
	 *  @return the current revision number.
	 */
	brass_revision_number_t get_revision_number() const;

	/** Virtual methods of Database::Internal. */
	//@{
	Xapian::doccount  get_doccount() const;
	Xapian::docid get_lastdocid() const;
	totlen_t get_total_length() const;
	Xapian::doclength get_avlength() const;
	Xapian::termcount get_doclength(Xapian::docid did) const;
	Xapian::doccount get_termfreq(const string & tname) const;
	Xapian::termcount get_collection_freq(const string & tname) const;
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
	//@}

	XAPIAN_NORETURN(void throw_termlist_table_close_exception() const);
};

/** A writable brass database.
 */
class BrassWritableDatabase : public BrassDatabase {
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
	/** Create and open a writable brass database.
	 *
	 *  @exception Xapian::DatabaseOpeningError thrown if database can't
	 *             be opened.
	 *
	 *  @exception Xapian::DatabaseVersionError thrown if database is in an
	 *             unsupported format.  This implies that the database was
	 *             created by an older or newer version of Xapian.
	 *
	 *  @param dir directory holding brass tables
	 */
	BrassWritableDatabase(const string &dir, int action, int block_size);

	~BrassWritableDatabase();

	/** Virtual methods of Database::Internal. */
	//@{
	Xapian::termcount get_doclength(Xapian::docid did) const;
	Xapian::doccount get_termfreq(const string & tname) const;
	Xapian::termcount get_collection_freq(const string & tname) const;
	Xapian::doccount get_value_freq(Xapian::valueno slot) const;
	std::string get_value_lower_bound(Xapian::valueno slot) const;
	std::string get_value_upper_bound(Xapian::valueno slot) const;
	bool term_exists(const string & tname) const;

	LeafPostList * open_post_list(const string & tname) const;
	ValueList * open_value_list(Xapian::valueno slot) const;
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
};

#endif /* OM_HGUARD_BRASS_DATABASE_H */
