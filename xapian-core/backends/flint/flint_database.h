/* flint_database.h: C++ class definition for flint database
 *
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002 Ananova Ltd
 * Copyright 2002,2003,2004,2005,2006 Olly Betts
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

#ifndef OM_HGUARD_FLINT_DATABASE_H
#define OM_HGUARD_FLINT_DATABASE_H

#include "database.h"
#include "flint_metafile.h"
#include "flint_positionlist.h"
#include "flint_postlist.h"
#include "flint_record.h"
#include "flint_termlist.h"
#include "flint_values.h"
#include "flint_lock.h"

class FlintTermList;

#include "flint_types.h"

#include <map>

const int XAPIAN_DB_READONLY = 0;

/** A backend designed for efficient indexing and retrieval, using
 *  compressed posting lists and a btree storage scheme.
 */
class FlintDatabase : public Xapian::Database::Internal {
    friend class FlintWritableDatabase;
    friend class FlintTermList;
    private:
	/** Directory to store databases in.
	 */
	std::string db_dir;

	/** Whether the database is readonly.
	 */
	bool readonly;

	/** The file describing the Flint database.
	 *  This file has information about the format of the database
	 *  which can't easily be stored in any of the individual tables.
	 */
	FlintMetaFile metafile;

	/** Table storing posting lists.
	 *
	 *  Whenever an update is performed, this table is the first to be
	 *  updated: therefore, its most recent revision number is the most
	 *  recent anywhere in the database.
	 */
	FlintPostListTable postlist_table;

	/** Table storing position lists.
	 */
	FlintPositionListTable positionlist_table;

	/** Table storing term lists.
	 */
	FlintTermListTable termlist_table;

	/** Table storing values.
	 */
	FlintValueTable value_table;

	/** Table storing records.
	 *
	 *  Whenever an update is performed, this table is the last to be
	 *  updated: therefore, its most recent revision number is the most
	 *  recent consistent revision available.  If this tables most
	 *  recent revision number is not available for all tables, there
	 *  is no consistent revision available, and the database is corrupt.
	 */
	FlintRecordTable record_table;

	/// Lock object.
	FlintLock lock;

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
	 */
	void get_database_write_lock();

	/** Open tables at specified revision number.
	 *
	 *  @exception Xapian::InvalidArgumentError is thrown if the specified
	 *  revision is not available.
	 */
	void open_tables(flint_revision_number_t revision);

	/** Get an object holding the revision number which the tables are
	 *  opened at.
	 *
	 *  @return the current revision number.
	 */
	flint_revision_number_t get_revision_number() const;

	/** Get an object holding the next revision number which should be
	 *  used in the tables.
	 *
	 *  @return the next revision number.
	 */
	flint_revision_number_t get_next_revision_number() const;

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
	void set_revision_number(flint_revision_number_t new_revision);
	
	/** Re-open tables to recover from an overwritten condition,
	 *  or just get most up-to-date version.
	 */
	virtual void reopen();

	/** Apply any outstanding changes to the tables.
	 *
	 *  If an error occurs during the operation, this will be signalled
	 *  by a return value of false.  The tables on disk will be left in
	 *  an unmodified state (though possibly with increased revision
	 *  numbers), and the changes made will be lost.
	 *
	 *  @return true if the operation completed successfully, false
	 *          otherwise.
	 */
	void apply();

	/** Cancel any outstanding changes to the tables.
	 */
	void cancel();

    public:
	/** Create and open a flint database.
	 *
	 *  @exception Xapian::DatabaseCorruptError is thrown if there is no
	 *             consistent revision available.
	 *
	 *  @exception Xapian::DatabaseOpeningError thrown if database can't be opened.
	 *
	 *  @param dbdir directory holding flint tables
	 *
	 *  @param block_size Block size, in bytes, to use when creating
	 *                    tables.  This is only important, and has the
	 *                    correct value, when the database is being
	 *                    created.  (ie, opened writable for the first
	 *                    time).
	 */
	FlintDatabase(const string &db_dir_, int action = XAPIAN_DB_READONLY,
		       unsigned int block_size = 0u);

	~FlintDatabase();

	/** Virtual methods of Database.
	 */
	//@{
	Xapian::doccount  get_doccount() const;
	Xapian::docid get_lastdocid() const;
	Xapian::doclength get_avlength() const;
	Xapian::doclength get_doclength(Xapian::docid did) const;
	Xapian::doccount get_termfreq(const string & tname) const;
	Xapian::termcount get_collection_freq(const string & tname) const;
	bool term_exists(const string & tname) const;
	bool has_positions() const;

	LeafPostList * do_open_post_list(const string & tname) const;
	LeafTermList * open_term_list(Xapian::docid did) const;
	Xapian::Document::Internal * open_document(Xapian::docid did, bool lazy = false) const;
	PositionList * open_position_list(Xapian::docid did,
					  const string & tname) const;
	TermList * open_allterms() const;
	//@}
};

/** A writable flint database.
 */
class FlintWritableDatabase : public Xapian::Database::Internal {
    private:
	/** Unflushed changes to term frequencies and collection frequencies. */
	mutable map<string, pair<Xapian::termcount_diff, Xapian::termcount_diff> >
		freq_deltas;

	/** Document lengths of new and modified documents which haven't been flushed yet. */
	mutable map<Xapian::docid, Xapian::termcount> doclens;

	/// Modifications to posting lists.
	mutable map<string, map<Xapian::docid,
				pair<char, Xapian::termcount> > > mod_plists;

	/** The readonly database encapsulated in the writable database.
	 */
	mutable FlintDatabase database_ro;

	/** Total length of all documents including unflushed modifications.
	 */
	mutable flint_totlen_t total_length;

	/** Highest document ID ever allocated by this database.
	 */
	mutable Xapian::docid lastdocid;

	/** The number of documents added, deleted, or replaced since the last
	 *  flush.
	 */
	mutable Xapian::doccount changes_made;

	static size_t flush_threshold;

	//@{
	/** Implementation of virtual methods: see Database for details.
	 */
	void flush();

	void do_flush_const() const;

	/** Cancel pending modifications to the database. */
	void cancel();

	virtual Xapian::docid add_document(const Xapian::Document & document);
	Xapian::docid add_document_(Xapian::docid did, const Xapian::Document & document);
	virtual void delete_document(Xapian::docid did);
	virtual void replace_document(Xapian::docid did,
				      const Xapian::Document & document);
	//@}

    public:
	/** Create and open a writable flint database.
	 *
	 *  @exception Xapian::DatabaseOpeningError thrown if database can't be opened.
	 *
	 *  @param dir directory holding flint tables
	 */
	FlintWritableDatabase(const string &dir, int action, int block_size);

	~FlintWritableDatabase();

	/** Virtual methods of Database.
	 */
	//@{
	Xapian::doccount  get_doccount() const;
	Xapian::docid get_lastdocid() const;
	Xapian::doclength get_avlength() const;
	Xapian::doclength get_doclength(Xapian::docid did) const;
	Xapian::doccount get_termfreq(const string & tname) const;
	Xapian::termcount get_collection_freq(const string & tname) const;
	bool term_exists(const string & tname) const;
	bool has_positions() const;

	LeafPostList * do_open_post_list(const string & tname) const;
	LeafTermList * open_term_list(Xapian::docid did) const;
	Xapian::Document::Internal * open_document(Xapian::docid did, bool lazy = false) const;
	PositionList * open_position_list(Xapian::docid did,
					  const string & tname) const;
	TermList * open_allterms() const;
	void begin_transaction_();
	void commit_transaction_();
	void cancel_transaction_();
	//@}
};

#endif /* OM_HGUARD_FLINT_DATABASE_H */
