/* database.h: database class declarations
 *
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002 Ananova Ltd
 * Copyright 2002,2003,2004,2005,2006,2007 Olly Betts
 * Copyright 2006 Richard Boulton
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

#ifndef OM_HGUARD_DATABASE_H
#define OM_HGUARD_DATABASE_H

#include <string>

#include <xapian/base.h>
#include <xapian/types.h>
#include <xapian/database.h>
#include <xapian/document.h>
#include <xapian/positioniterator.h>
#include <xapian/termiterator.h>
#include "omdebug.h"

using namespace std;

class LeafPostList;
class LeafTermList;
class RemoteDatabase;

typedef Xapian::TermIterator::Internal TermList;
typedef Xapian::PositionIterator::Internal PositionList;

namespace Xapian {

/** Base class for databases.
 */
class Database::Internal : public Xapian::Internal::RefCntBase {
    private:
	/// Copies are not allowed.
	Internal(const Internal &);

	/// Assignment is not allowed.
	void operator=(const Internal &);

    protected:
	/// Transaction state.
	enum {
	    TRANSACTION_UNIMPLEMENTED = -1, // Used by InMemory.
	    TRANSACTION_NONE = 0,
	    TRANSACTION_UNFLUSHED = 1,
	    TRANSACTION_FLUSHED = 2
	} transaction_state;

	bool transaction_active() const { return int(transaction_state) > 0; }

	/** Create a database - called only by derived classes.
	 */
	Internal();

	/** Internal method to perform cleanup when a writable database is
	 *  destroyed with unflushed changes.
	 *
	 *  A derived class' destructor should call this method before
	 *  destroying the database to ensure that no sessions or
	 *  transactions are in progress at destruction time.
	 *
	 *  Note that it is not safe to throw exceptions from destructors,
	 *  so this method will catch and discard any exceptions.
	 */
	void dtor_called();

    public:
	/** Destroy the database.
	 *
	 *  This method should not be called until all objects using the
	 *  database have been cleaned up.
	 *
	 *  If any transactions are in progress, they should
	 *  be finished by cancel_transaction() or
	 *  commit_transaction() - if this is not done, the destructor
	 *  will attempt to clean things up by cancelling the transaction,
	 *  but any errors produced by these operations will not be reported.
	 */
        virtual ~Internal();

	/** Send a keep-alive signal to a remote database, to stop
	 *  it from timing out.
	 */
	virtual void keep_alive();

	//////////////////////////////////////////////////////////////////
	// Database statistics:
	// ====================

	/** Return the number of docs in this (sub) database.
	 */
	virtual Xapian::doccount get_doccount() const = 0;

	/** Return the last used document id of this (sub) database.
	 */
	virtual Xapian::docid get_lastdocid() const = 0;

	/** Return the average length of a document in this (sub) database.
	 *
	 *  See Database::Internal::get_doclength() for the meaning of document
	 *  length within Xapian.
	 */
	virtual Xapian::doclength get_avlength() const = 0;

	/** Get the length of a given document.
	 *
	 *  Document length, for the purposes of Xapian, is defined to be
	 *  the number of instances of terms within a document.  Expressed
	 *  differently, the sum of the within document frequencies over
	 *  all the terms in the document.
	 *
	 *  @param did  The document id of the document whose length is
	 *              being requested.
	 */
	virtual Xapian::doclength get_doclength(Xapian::docid did) const = 0;

	/** Return the number of documents indexed by a given term.  This
	 *  may be an approximation, but must be an upper bound (ie,
	 *  greater or equal to the true value), and should be as accurate
	 *  as possible.
	 *
	 *  @param tname  The term whose term frequency is being requested.
	 */
	virtual Xapian::doccount get_termfreq(const string & tname) const = 0;

	/** Return the total number of occurrences of the given term.  This
	 *  is the sum of the number of ocurrences of the term in each
	 *  document: ie, the sum of the within document frequencies of the
	 *  term.
	 *
	 *  @param tname  The term whose collection frequency is being
	 *                requested.
	 */
	virtual Xapian::termcount get_collection_freq(const string & tname) const = 0;

	/** Check whether a given term is in the database.
	 *
	 *  @param tname  The term whose presence is being checked.
	 */
	virtual bool term_exists(const string & tname) const = 0;

	/** Check whether this database contains any positional information.
	 */
	virtual bool has_positions() const = 0;

	//////////////////////////////////////////////////////////////////
	// Data item access methods:
	// =========================

	/** Open a posting list.
	 *
	 *  Method defined by subclass to open a posting list.
	 *  This is a list of all the documents which contain a given term.
	 *
	 *  @param tname  The term whose posting list is being requested.
	 *
	 *  @return       A pointer to the newly created posting list.
	 *		  If the term doesn't exist, a new EmptyPostList
	 *		  object (or an object which behaves the same way) is
	 *		  returned instead, which makes it easier
	 *		  to implement a search over multiple databases.
	 *                This object must be deleted by the caller after
	 *                use.
	 */
	virtual LeafPostList * open_post_list(const string & tname) const = 0;

	/** Open a term list.
	 *
	 *  This is a list of all the terms contained by a given document.
	 *
	 *  @param did    The document id whose term list is being requested.
	 *
	 *  @return       A pointer to the newly created term list.
	 *                This object must be deleted by the caller after
	 *                use.
	 */
	virtual LeafTermList * open_term_list(Xapian::docid did) const = 0;

	/** Open an allterms list.
	 *
	 *  This is a list of all the terms in the database
	 *
	 *  @return       A pointer to the newly created allterms list.
	 *                This object must be deleted by the caller after
	 *                use.
	 */
	virtual TermList * open_allterms() const = 0;

	/** Open a position list for the given term in the given document.
	 *
	 *  @param did    The document id for which a position list is being
	 *                requested.
	 *  @param tname  The term for which a position list is being
	 *                requested.
	 *
	 *  @return       A pointer to the newly created position list.
	 *                This object must be deleted by the caller after
	 *                use.
	 */
	virtual PositionList * open_position_list(Xapian::docid did,
					const string & tname) const = 0;

	/** Open a document.
	 *
	 *  This is used to access the values and data associated with a
	 *  document.  See class Xapian::Document::Internal for further details.
	 *
	 *  @param did    The document id which is being requested.
	 *
	 *  @param lazy   Don't check the document exists immediately -
	 *                use from within the matcher where we know the
	 *                document exists, and don't want to read the
	 *                record when we just want the values.
	 *
	 *  @return       A pointer to the newly created document object.
	 *                This object must be deleted by the caller after
	 *                use.
	 */
	virtual Xapian::Document::Internal *
	open_document(Xapian::docid did, bool lazy = false) const = 0;

	/** Reopen the database to the latest available revision.
	 *
	 *  Database backends which don't support simultaneous update and
	 *  reading probably don't need to do anything here.
	 */
	virtual void reopen() { }

	//////////////////////////////////////////////////////////////////
	// Modifying the database:
	// =======================

	/** Flush pending modifications to the database.
	 *
	 *  See WritableDatabase::flush() for more information.
	 */
	virtual void flush();

	/** Cancel pending modifications to the database. */
	virtual void cancel();

	/** Begin a transaction.
	 *
	 *  See WritableDatabase::begin_transaction() for more information.
	 */
	void begin_transaction(bool flushed);

	/** Commit a transaction.
	 *
	 *  See WritableDatabase::commit_transaction() for more information.
	 */
	void commit_transaction();

	/** Cancel a transaction.
	 *
	 *  See WritableDatabase::cancel_transaction() for more information.
	 */
	void cancel_transaction();

	/** Add a new document to the database.
	 *
	 *  See WritableDatabase::add_document() for more information.
	 */
	virtual Xapian::docid add_document(const Xapian::Document & document);

	/** Delete a document in the database.
	 *
	 *  See WritableDatabase::delete_document() for more information.
	 */
	virtual void delete_document(Xapian::docid did);

	/** Delete any documents indexed by a term from the database.
	 *
	 *  See WritableDatabase::delete_document() for more information.
	 */
	virtual void delete_document(const std::string & unique_term);

	/** Replace a given document in the database.
	 *
	 *  See WritableDatabase::replace_document() for more information.
	 */
	virtual void replace_document(Xapian::docid did,
				      const Xapian::Document & document);

	/** Replace any documents matching a term.
	 *
	 *  See WritableDatabase::replace_document() for more information.
	 */
	virtual Xapian::docid replace_document(const std::string & unique_term,
					       const Xapian::Document & document);

	/** Request and later collect a document from the database.
	 *  Multiple documents can be requested with request_document(),
	 *  and then collected with collect_document().  Allows the backend
	 *  to optimise (e.g. the remote backend can start requests for all
	 *  the documents so they fetch in parallel).
	 *
	 *  If a backend doesn't support this, request_document() can be a
	 *  no-op and collect_document() the same as open_document().
	 */
	//@{
	virtual void request_document(Xapian::docid /*did*/) const { }

	virtual Xapian::Document::Internal * collect_document(Xapian::docid did) const {
	    return open_document(did);
	}
	//@}

	//////////////////////////////////////////////////////////////////
	// Introspection methods:
	// ======================

	/** Return a pointer to this object as a RemoteDatabase, or NULL.
	 *
	 *  This method is used by MultiMatch to decide whether to use a
	 *  LocalSubMatch or a RemoteSubMatch to perform a search over the
	 *  database.
	 */
	virtual RemoteDatabase * as_remotedatabase() {
	    return NULL;
	}
};

}

#endif /* OM_HGUARD_DATABASE_H */
