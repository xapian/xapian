/* quartz_database.h: C++ class definition for quartz database
 *
 * ----START-LICENCE----
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002 Ananova Ltd
 * Copyright 2002 Olly Betts
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

/** A backend designed for efficient indexing and retrieval, using
 *  compressed posting lists and a btree storage scheme.
 */
class QuartzDatabase : public Database {
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

	virtual om_docid do_add_document(const OmDocument & document);
	virtual void do_delete_document(om_docid did);
	virtual void do_replace_document(om_docid did,
					 const OmDocument & document);

	virtual void do_reopen();
	//@}

	/// Implementation of open_post_list()
	LeafPostList * open_post_list_internal(const string & tname,
				RefCntPtr<const Database> ptrtothis) const;

	/// Implementation of open_term_list()
	LeafTermList * open_term_list_internal(om_docid did,
				RefCntPtr<const Database> ptrtothis) const;

    public:
	/** Create and open a quartz database.
	 *
	 *  @exception OmOpeningError thrown if database can't be opened.
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
	om_doccount  get_doccount() const;
	om_doclength get_avlength() const;
	om_doclength get_doclength(om_docid did) const;
	om_doccount get_termfreq(const string & tname) const;
	om_termcount get_collection_freq(const string & tname) const;
	bool term_exists(const string & tname) const;

	LeafPostList * do_open_post_list(const string & tname) const;
	LeafTermList * open_term_list(om_docid did) const;
	Document * open_document(om_docid did, bool lazy = false) const;
	AutoPtr<PositionList> open_position_list(om_docid did,
					  const string & tname) const;
	TermList * open_allterms() const;
	//@}
};

/** A writable quartz database.
 */
class QuartzWritableDatabase : public Database {
    private:
	/** Pointer to buffered table manager.
	 *
	 *  This points to the same object as tables, but is used for
	 *  modification access.
	 */
	QuartzBufferedTableManager * buffered_tables;

	/** A count of the number of changes since the last flush:
	 *  FIXME: this should be replaced by keeping track of the memory used
	 *  up, and flushing when it reaches a critical value.
	 */
	int changecount;

	/** The readonly database encapsulated in the writable database.
	 */
	QuartzDatabase database_ro;

	//@{
	/** Implementation of virtual methods: see Database for details.
	 */
	virtual void do_begin_session();
	virtual void do_end_session();
	virtual void do_flush();

	virtual void do_begin_transaction();
	virtual void do_commit_transaction();
	virtual void do_cancel_transaction();

	virtual om_docid do_add_document(const OmDocument & document);
	virtual void do_delete_document(om_docid did);
	virtual void do_replace_document(om_docid did,
					 const OmDocument & document);

	virtual void do_reopen();
	//@}

    public:
	/** Create and open a writable quartz database.
	 *
	 *  @exception OmOpeningError thrown if database can't be opened.
	 *
	 *  @param dir directory holding quartz tables
	 */
	QuartzWritableDatabase(const string &dir, int action, int block_size);

	~QuartzWritableDatabase();

	/** Virtual methods of Database.
	 */
	//@{
	om_doccount  get_doccount() const;
	om_doclength get_avlength() const;
	om_doclength get_doclength(om_docid did) const;
	om_doccount get_termfreq(const string & tname) const;
	om_termcount get_collection_freq(const string & tname) const;
	bool term_exists(const string & tname) const;

	LeafPostList * do_open_post_list(const string & tname) const;
	LeafTermList * open_term_list(om_docid did) const;
	Document * open_document(om_docid did, bool lazy = false) const;
	AutoPtr<PositionList> open_position_list(om_docid did,
					  const string & tname) const;
	TermList * open_allterms() const;
	//@}
};

#endif /* OM_HGUARD_QUARTZ_DATABASE_H */
