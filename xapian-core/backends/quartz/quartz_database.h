/* quartz_database.h: C++ class definition for quartz database
 *
 * ----START-LICENCE----
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002 Ananova Ltd
 * Copyright 2002,2003 Olly Betts
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 * USA
 * -----END-LICENCE-----
 */

#ifndef OM_HGUARD_QUARTZ_DATABASE_H
#define OM_HGUARD_QUARTZ_DATABASE_H

#include "database.h"

class QuartzTableManager;
class QuartzBufferedTableManager;
class QuartzModifications;
class QuartzTermList;

#include "autoptr.h"

#include "quartz_types.h"

#include <map>

/** A backend designed for efficient indexing and retrieval, using
 *  compressed posting lists and a btree storage scheme.
 */
class QuartzDatabase : public Xapian::Database::Internal {
    friend class QuartzWritableDatabase;
    friend class QuartzTermList;
    private:
	/** Pointer to table manager.
	 */
	AutoPtr<QuartzTableManager> tables;

	//@{
	/** Implementation of virtual methods: see Database for details.
	 */
	virtual void do_begin_session();
	virtual void do_end_session();
	virtual void do_flush();

	virtual void do_begin_transaction();
	virtual void do_commit_transaction();
	virtual void do_cancel_transaction();

	virtual Xapian::docid do_add_document(const Xapian::Document & document);
	virtual void do_delete_document(Xapian::docid did);
	virtual void do_replace_document(Xapian::docid did,
					 const Xapian::Document & document);

	virtual void do_reopen();
	//@}

	/// Implementation of open_post_list()
	LeafPostList * open_post_list_internal(const string & tname,
		Xapian::Internal::RefCntPtr<const Xapian::Database::Internal> ptrtothis) const;

	/// Implementation of open_term_list()
	LeafTermList * open_term_list_internal(Xapian::docid did,
		Xapian::Internal::RefCntPtr<const Xapian::Database::Internal> ptrtothis) const;

    public:
	/** Create and open a quartz database.
	 *
	 *  @exception Xapian::DatabaseOpeningError thrown if database can't be opened.
	 *
	 *  @param dbdir directory holding quartz tables
	 */
	QuartzDatabase(const string &dbdir);

	/** Constructor used by QuartzWritableDatabase.
	 *
	 *  @param tables_ A pointer to the tables to use.
	 */
	QuartzDatabase(AutoPtr<QuartzTableManager> tables_);

	~QuartzDatabase();

	/** Virtual methods of Database.
	 */
	//@{
	Xapian::doccount  get_doccount() const;
	Xapian::doclength get_avlength() const;
	Xapian::doclength get_doclength(Xapian::docid did) const;
	Xapian::doccount get_termfreq(const string & tname) const;
	Xapian::termcount get_collection_freq(const string & tname) const;
	bool term_exists(const string & tname) const;

	LeafPostList * do_open_post_list(const string & tname) const;
	LeafTermList * open_term_list(Xapian::docid did) const;
	Xapian::Document::Internal * open_document(Xapian::docid did, bool lazy = false) const;
	PositionList * open_position_list(Xapian::docid did,
					  const string & tname) const;
	TermList * open_allterms() const;
	//@}
};

/** A writable quartz database.
 */
class QuartzWritableDatabase : public Xapian::Database::Internal {
    private:
	/** Pointer to buffered table manager.
	 *
	 *  This points to the same object as tables, but is used for
	 *  modification access.
	 */
	QuartzBufferedTableManager * buffered_tables;

	/** A count of the number of changes since the last flush:
	 *  FIXME: this should be replaced by keeping track of the memory used
	 *  up, and flushing when it reaches a threshold value.
	 */
	mutable int changecount;

	mutable quartz_totlen_t totlen_added;
	mutable quartz_totlen_t totlen_removed;
	mutable map<string, pair<Xapian::termcount_diff, Xapian::termcount_diff> >
		freq_deltas;
	mutable map<Xapian::docid, Xapian::termcount> doclens;
	/// Modifications to posting lists.
	mutable map<string, map<Xapian::docid,
				pair<char, Xapian::termcount> > > mod_plists;

	/** The readonly database encapsulated in the writable database.
	 */
	QuartzDatabase database_ro;

	//@{
	/** Implementation of virtual methods: see Database for details.
	 */
	virtual void do_begin_session();
	virtual void do_end_session();
	virtual void do_flush();

	void do_flush_const() const;

	virtual void do_begin_transaction();
	virtual void do_commit_transaction();
	virtual void do_cancel_transaction();

	virtual Xapian::docid do_add_document(const Xapian::Document & document);
	virtual void do_delete_document(Xapian::docid did);
	virtual void do_replace_document(Xapian::docid did,
					 const Xapian::Document & document);

	virtual void do_reopen();
	//@}

    public:
	/** Create and open a writable quartz database.
	 *
	 *  @exception Xapian::DatabaseOpeningError thrown if database can't be opened.
	 *
	 *  @param dir directory holding quartz tables
	 */
	QuartzWritableDatabase(const string &dir, int action, int block_size);

	~QuartzWritableDatabase();

	/** Virtual methods of Database.
	 */
	//@{
	Xapian::doccount  get_doccount() const;
	Xapian::doclength get_avlength() const;
	Xapian::doclength get_doclength(Xapian::docid did) const;
	Xapian::doccount get_termfreq(const string & tname) const;
	Xapian::termcount get_collection_freq(const string & tname) const;
	bool term_exists(const string & tname) const;

	LeafPostList * do_open_post_list(const string & tname) const;
	LeafTermList * open_term_list(Xapian::docid did) const;
	Xapian::Document::Internal * open_document(Xapian::docid did, bool lazy = false) const;
	PositionList * open_position_list(Xapian::docid did,
					  const string & tname) const;
	TermList * open_allterms() const;
	//@}
};

#endif /* OM_HGUARD_QUARTZ_DATABASE_H */
