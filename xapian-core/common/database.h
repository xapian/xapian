/* database.h: database class declarations
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

#ifndef OM_HGUARD_DATABASE_H
#define OM_HGUARD_DATABASE_H

#include "om/omtypes.h"

#include "database_builder.h"
#include "om/omindexdoc.h"
#include "refcnt.h"
#include "omlocks.h"
#include "emptypostlist.h"

class LeafDocument;
class LeafPostList;
class LeafTermList;

/** Base class for databases.
 *
 *  All classes derived from Database must have DatabaseBuilder as
 *  a friend, so that they can be constructed in a unified way.
 */
class Database : public RefCntBase {
    private:
	/// Copies are not allowed.
	Database(const Database &);

	/// Assignment is not allowed.
	void operator=(const Database &);

	/// Flag recording whether an explicit session is in progress
	bool session_in_progress;
	
	/// Flag recording whether a transaction is in progress
	bool transaction_in_progress;
	
	/// Mutex protecting access to "*_in_progress" flags.
	OmLock mutex;

	/// Virtual method providing implementation of begin_session();
	virtual void do_begin_session(om_timeout timeout) = 0;

	/// Virtual method providing implementation of end_session();
	virtual void do_end_session() = 0;

	/// Virtual method providing implementation of flush();
	virtual void do_flush() = 0;

	/// Virtual method providing implementation of begin_transaction();
	virtual void do_begin_transaction() = 0;

	/// Virtual method providing implementation of commit_transaction();
	virtual void do_commit_transaction() = 0;

	/// Virtual method providing implementation of cancel_transaction();
	virtual void do_cancel_transaction() = 0;

	/// Virtual method providing implementation of add_document();
	virtual om_docid do_add_document(
		const OmDocumentContents & document) = 0;

	/// Virtual method providing implementation of delete_document();
	virtual void do_delete_document(om_docid did) = 0;

	/// Virtual method providing implementation of replace_document();
	virtual void do_replace_document(om_docid did,
		const OmDocumentContents & document) = 0;

	/// Virtual method providing implementation of get_document();
	virtual OmDocumentContents do_get_document(om_docid did) = 0;

    protected:
    	/** Create a database - called only by derived classes.
	 */
	Database();

	/** Internal method providing implementation of end_session().
	 *
	 *  Derived class' destructors should call this method before
	 *  destroying the database to ensure that no sessions or
	 *  transactions are in progress at destruction time.
	 *
	 *  Note that it is not safe to throw exceptions from destructors,
	 *  so when called from destructors this must be enclosed in a
	 *  try - catch clause; exceptions caught by this can be ignored,
	 *  since the destructor shouldn't be called without having called
	 *  end_session() first, unless an error is already happening.
	 */
	void internal_end_session();

	/** Method definied by subclass to actually open a posting list.
	 *
	 *  This is a list of all the documents which contain a given term.
	 *
	 *  @param tname  The term whose posting list is being requested.
	 *
	 *  @return       A pointer to the newly created posting list.
	 *                This object must be deleted by the caller after
	 *                use.
	 */
	virtual LeafPostList * do_open_post_list(const om_termname & tname) const = 0;

    public:
	/** Destroy the database.
	 *  
	 *  This method should not be called until all objects using the
	 *  database have been cleaned up.
	 *
	 *  If any sessions or transactions are in progress, they should
	 *  be finished by calling end_session(), cancel_transaction() or
	 *  commit_transaction() - if this is not done, the destructor
	 *  will attempt to clean things up by cancelling the transaction
	 *  and ending the session, but errors produced by these operations
	 *  will not be reported.
	 */
        virtual ~Database();

	//////////////////////////////////////////////////////////////////
	// Database statistics:
	// ====================

	/** Return the number of docs in this (sub) database.
	 */
	virtual om_doccount  get_doccount() const = 0;

	/** Return the average length of a document in this (sub) database.
	 *
	 *  See Database::get_doclength() for the meaning of document
	 *  length within Muscat.
	 */
	virtual om_doclength get_avlength() const = 0;

	/** Get the length of a given document.
	 *
	 *  Document length, for the purposes of Muscat, is defined to be
	 *  the number of instances of terms within a document.  Expressed
	 *  differently, the sum of the within document frequencies over
	 *  all the terms in the document.
	 *
	 *  @param did  The document id of the document whose length is
	 *              being requested.
	 */
	virtual om_doclength get_doclength(om_docid did) const = 0;

	/** Return the number of documents indexed by a given term.  This
	 *  may be an approximation, but must be an upper bound (ie,
	 *  greater or equal to the true value), and should be as accurate
	 *  as possible.
	 *
	 *  @param tname  The term whose term frequency is being requested.
	 */
	virtual om_doccount get_termfreq(const om_termname & tname) const = 0;

	/** Check whether a given term is in the database.
	 *
	 *  This method should normally be functionally equivalent to
	 *  (get_termfreq() != 0), but this equivalence should not be
	 *  relied upon.  For example, in a database which allowed
	 *  deleting, get_termfreq() might return the term frequency before
	 *  the term was deleted, whereas term_exists() might be more
	 *  up-to-date.
	 *
	 *  This method will also often be considerably more efficient than
	 *  get_termfreq().
	 *
	 *  @param tname  The term whose presence is being checked.
	 */
	virtual bool term_exists(const om_termname & tname) const = 0;

	//////////////////////////////////////////////////////////////////
	// Data item access methods:
	// =========================

	/** Open a posting list.
	 *
	 *  This is a list of all the documents which contain a given term.
	 *
	 *  @param tname  The term whose posting list is being requested.
	 *
	 *  @return       A pointer to the newly created posting list.
	 *                This object must be deleted by the caller after
	 *                use.
	 */
	LeafPostList * open_post_list(const om_termname & tname) const {
	    if (!term_exists(tname)) {
		DEBUGLINE(MATCH, tname + " is not in database.");
		// Term doesn't exist in this database.  However, we create
		// a (empty) postlist for it to help make distributed searching
		// cleaner (term might exist in other databases).
		// This is similar to using the muscat3.6 zerofreqs option.
		return new EmptyPostList();
	    }
	    return do_open_post_list(tname);
	}

	/** Open a term list.
	 *
	 *  This is a list of all the terms contained by a given document.
	 *
	 *  @param did    The document id whose term list is being requested.
	 *
	 *  @return       A pointer to the newly created posting list.
	 *                This object must be deleted by the caller after
	 *                use.
	 */
	virtual LeafTermList * open_term_list(om_docid did) const = 0;

	/** Open a document.
	 *
	 *  This is used to access the keys and data associated with a
	 *  document.  See class LeafDocument for details of accessing
	 *  the keys and data.
	 *
	 *  @param did    The document id which is being requested.
	 *
	 *  @return       A pointer to the newly created document object.
	 *                This object must be deleted by the caller after
	 *                use.
	 */
	virtual LeafDocument * open_document(om_docid did) const = 0;


	//////////////////////////////////////////////////////////////////
	// Modifying the database:
	// =======================

	/** Start a modification session on the database.
	 *
	 *  See OmWritableDatabase::begin_session() for more information.
	 */
	void begin_session(om_timeout timeout);

	/** End a modification session on the database.
	 *
	 *  See OmWritableDatabase::end_session() for more information.
	 */
	void end_session();

	/** Flush modifications to the database.
	 *
	 *  See OmWritableDatabase::flush() for more information.
	 */
	void flush();

	/** Begin a transaction.
	 *
	 *  See OmWritableDatabase::begin_transaction() for more information.
	 */
	void begin_transaction();

	/** Commit a transaction.
	 *
	 *  See OmWritableDatabase::commit_transaction() for more information.
	 */
	void commit_transaction();

	/** Cancel a transaction.
	 *
	 *  See OmWritableDatabase::cancel_transaction() for more information.
	 */
	void cancel_transaction();

	/** Add a new document to the database.
	 *
	 *  See OmWritableDatabase::add_document() for more information.
	 */
	om_docid add_document(const OmDocumentContents & document,
			      om_timeout timeout = 0);

	/** Delete a document in the database.
	 *
	 *  See OmWritableDatabase::delete_document() for more information.
	 */
	void delete_document(om_docid did, om_timeout timeout = 0);

	/** Replace a given document in the database.
	 *
	 *  See OmWritableDatabase::replace_document() for more information.
	 */
	void replace_document(om_docid did,
			      const OmDocumentContents & document,
			      om_timeout timeout = 0);

	/** Get a document from the database.
	 *
	 *  See OmWritableDatabase::get_document() for more information.
	 */
	OmDocumentContents get_document(om_docid did);


	//////////////////////////////////////////////////////////////////
	// Introspection methods:
	// ======================

	/** Determine whether the database is a network database.  This is
	 *  used by MultiMatch to decide whether to use a LocalMatch or a
	 *  NetworkMatch to perform a search over the database.
	 *
	 *  The default implementation returns "false".
	 */
	virtual bool is_network() const;
};

inline bool Database::is_network() const
{
    return false;
}

#endif /* OM_HGUARD_DATABASE_H */
