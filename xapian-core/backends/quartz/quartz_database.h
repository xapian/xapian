/* quartz_database.h: C++ class definition for quartz database
 *
 * ----START-LICENCE----
 * Copyright 1999,2000 BrightStation PLC
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

class QuartzDBManager;
class QuartzModifications;
#include <memory>
#include "omlocks.h"

/** A backend designed for efficient indexing and retrieval, using
 *  compressed posting lists and the Berkeley database library (version
 *  3.1).
 *
 *  BerkeleyDB is available from http://www.sleepycat.com/
 */
class QuartzDatabase : public IRDatabase {
    friend class DatabaseBuilder;
    private:
	/** Mutex to protect this object against concurrent access.
	 */
	OmLock quartz_mutex;

	/** Pointer to database manager.
	 */
	auto_ptr<QuartzDBManager> db_manager;

	/** Pointer to database modifications.
	 */
	auto_ptr<QuartzModifications> modifications;

	/** Logfile recording database modifications.
	 */
	string modification_logfile;

	/** Flag saying whether we're using transactions or not.
	 */
	bool use_transactions;

	/** Flag saying whether we're readonly or not.
	 */
	bool readonly;

	/** Create and open a quartz database.
	 *
	 *  @exception OmOpeningError thrown if database can't be opened.
	 *
	 *  @param params Parameters supplied by the user to specify the
	 *                location of the database to open.
	 */
	QuartzDatabase(const OmSettings & params, bool readonly);

	//@{
	/** Implementation of virtual methods: see IRDatabase for details.
	 */
	void do_begin_session(om_timeout timeout);
	void do_end_session();
	void do_flush();

	void do_begin_transaction();
	void do_commit_transaction();
	void do_cancel_transaction();

	om_docid do_add_document(const OmDocumentContents & document);
	void do_delete_document(om_docid did);
	void do_replace_document(om_docid did,
				 const OmDocumentContents & document);
	OmDocumentContents do_get_document(om_docid did);
	//@}

    public:
	~QuartzDatabase();

	// Virtual methods of IRDatabase
	om_doccount  get_doccount() const;
	om_doclength get_avlength() const;
	om_doclength get_doclength(om_docid did) const;
	om_doccount get_termfreq(const om_termname & tname) const;
	bool term_exists(const om_termname & tname) const;

	LeafPostList * open_post_list(const om_termname& tname) const;
	LeafTermList * open_term_list(om_docid did) const;
	LeafDocument * open_document(om_docid did) const;
};

#endif /* OM_HGUARD_QUARTZ_DATABASE_H */
