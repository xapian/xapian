/* omdatabase.h
 *
 * ----START-LICENCE----
 * Copyright 1999,2000,2001 BrightStation PLC
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

#ifndef OM_HGUARD_OMDATABASE_H
#define OM_HGUARD_OMDATABASE_H

#include "om/omdocument.h"
#include "om/omsettings.h"
#include "om/ompostlistiterator.h"
#include "om/omtermlistiterator.h"
#include "om/ompositionlistiterator.h"

class OmWritableDatabase;

///////////////////////////////////////////////////////////////////
// OmDatabase class
// ================

/** This class is used to access a database, or a set of databases..
 *
 *  This class is used in conjunction with an OmEnquire object.
 *
 *  @exception OmInvalidArgumentError will be thrown if an invalid
 *  argument is supplied, for example, an unknown database type.
 *
 *  @exception OmOpeningError may be thrown if the database cannot
 *  be opened (for example, a required file cannot be found).
 */
class OmDatabase {
    public:
	/** Add a new database to use.
	 *
	 *  This call will opened a new database with the specified
	 *  parameters, and add it to the set of databases to be accessed
	 *  by this database group.
	 *
	 *  The database will always be opened read-only.
	 *
	 *  @param params  an OmSettings object specifying the parameters
	 *		   to be used to open the database.
	 *
	 *  @exception OmInvalidArgumentError  See class documentation.
	 *  @exception OmOpeningError          See class documentation.
	 */
	void add_database(const OmSettings &params);

	/** Add an already opened database (or set of databases) to the set
	 *  to be accessed by this object.
	 *
	 *  The handle on the database will be copied, so may be deleted
	 *  or reused by the caller as desired.
	 *
	 *  @param database the opened database to add.
	 */
	void add_database(const OmDatabase & database);
    
    public: // FIXME protected:
	class Internal;

	/** Reference counted internals. */
	Internal *internal;

	/** InternalInterface is a class used internally to interact
	 *  with OmDatabase objects.
	 */
	class InternalInterface;

	friend class InternalInterface;

    protected:
	/** Open a database, possibly readonly.
	 *
	 *  This may be used to open a database for writing, and is used
	 *  in this way by OmWritableDatabase.
	 *
	 *  @param params  an OmSettings object specifying the parameters
	 *		   to be used to open the database.
	 *
	 *  @param readonly readonly flag.
	 *
	 *  @exception OmInvalidArgumentError will be thrown if an invalid
	 *             argument is supplied, for example, an unknown database
	 *             type.
	 *  @exception OmOpeningError may be thrown if the database cannot
	 *             be opened.
	 *
	 */
	OmDatabase(const OmSettings &params, bool readonly);

    public:
	/** Open a database.
	 *
	 *  @param params  an OmSettings object specifying the parameters
	 *		   to be used to open the database.
	 *
	 *  @exception OmInvalidArgumentError will be thrown if an invalid
	 *             argument is supplied, for example, an unknown database
	 *             type.
	 *  @exception OmOpeningError may be thrown if the database cannot
	 *             be opened.
	 */
	OmDatabase(const OmSettings &params);

	/** Create an OmDatabase with no databases in.
	 */
	OmDatabase();

	/** Destroy this handle on the database.
	 *  If there are no copies of this object remaining, the database
	 *  will be closed.
	 */
	virtual ~OmDatabase();

        /** Copying is allowed.  The internals are reference counted, so
	 *  copying is also cheap.
	 */
	OmDatabase(const OmDatabase &other);

        /** Assignment is allowed.  The internals are reference counted,
	 *  so assignment is also cheap.
	 */
	virtual void operator=(const OmDatabase &other);

	/** Re-open the database.
	 *  This re-opens the database to the latest available versions.
	 *  It can be used either to make sure the latest results are
	 *  returned, or to recover from an OmDatabaseModifiedError.
	 */
	void reopen();

	/** Returns a string representing the database object.
	 *  Introspection method.
	 */
	virtual std::string get_description() const;

	OmPostListIterator postlist_begin(const om_termname &tname) const;
	OmPostListIterator postlist_end(const om_termname &tname) const;

	OmTermIterator termlist_begin(om_docid did) const;
	OmTermIterator termlist_end(om_docid did) const;

	OmPositionListIterator positionlist_begin(om_docid did, const om_termname &tname) const;
	OmPositionListIterator positionlist_end(om_docid did, const om_termname &tname) const;

	/** An iterator which runs across all terms in the database.
	 */
	//@{
	OmTermIterator allterms_begin() const;
	OmTermIterator allterms_end() const;
	//@}

	/// Get the number of documents in the database.
	om_doccount get_doccount() const;
	
	/// Get the average length of the documents in the database.
	om_doclength get_avlength() const;

	/// Get the number of documents in the database indexed by a given term.
	om_doccount get_termfreq(const om_termname & tname) const;

	/** Return true if and only if the term exists in the database.
	 *  This is the same as (get_termfreq(tname) != 0), but will often be
	 *  more efficient.
	 */
	bool term_exists(const om_termname & tname) const;

	/** Return the total number of occurrences of the given term.  This
	 *  is the sum of the number of ocurrences of the term in each
	 *  document: ie, the sum of the within document frequencies of the
	 *  term.
	 *
	 *  @param tname  The term whose collection frequency is being
	 *                requested.
	 */
	om_termcount get_collection_freq(const om_termname & tname) const;

	/** Get the length of a document.
	 */
	om_doclength get_doclength(om_docid did) const;

	/** Send a "keep-alive" to remote databases to stop them timing
	 *  out.
	 */
	void keep_alive();

	/** Get a document from the database, given its document id.
	 *
	 *  This method returns an OmDocument object which provides the
	 *  information about a document.
	 *
	 *  @param did   The document id for which to retrieve the data.
	 *
	 *  @return      An OmDocument object containing the document data
	 *
	 *  @exception OmDocNotFoundError      The document specified could not
	 *                                     be found in the database.
	 */
	OmDocument get_document(om_docid did) const;
};

///////////////////////////////////////////////////////////////////
// OmWritableDatabase class
// ========================

/** This class provides writable access to a database.
 *
 *  NOTE: this class is still under heavy development, and the interface
 *  is liable to change in the near future.
 */
class OmWritableDatabase : public OmDatabase {
    public:
	/** Open a database for writing.
	 *
	 *  @param params  an OmSettings object specifying the parameters
	 *		   to be used to open the database.
	 *
	 *  @exception OmInvalidArgumentError will be thrown if an invalid
	 *             argument is supplied, for example, an unknown database
	 *             type.
	 *  @exception OmOpeningError may be thrown if the database cannot
	 *             be opened.
	 */
	OmWritableDatabase(const OmSettings &params);

	/** Destroy this handle on the database.
	 *
	 *  If there are no copies of this object remaining, the database
	 *  will be closed, and if there are any sessions or transactions
	 *  in progress these will be ended.
	 */
	virtual ~OmWritableDatabase();

        /** Copying is allowed.  The internals are reference counted, so
	 *  copying is also cheap.
	 */
	OmWritableDatabase(const OmWritableDatabase &other);

        /** Assignment is allowed.  The internals are reference counted,
	 *  so assignment is also cheap.  Note that only an
	 *  OmWritableDatabase may be assigned to an OmWritableDatabase:
	 *  an attempt to assign an OmDatabase will throw an exception.
	 */
	void operator=(const OmWritableDatabase &other);

	/** Flush to disk any modifications made to the database.
	 *
	 *  For efficiency reasons, when performing multiple updates to a
	 *  database it is best (indeed, almost essential) to make as many
	 *  modifications as memory will permit in a single pass through
	 *  the database.  To ensure this, Omseek performs modifications in
	 *  "sessions".  Sessions are begun and ended implicitly, when the
	 *  database is first modified, or closed.
	 *
	 *  Flush may be called at any time during a modification session to
	 *  ensure that the modifications which have been made are written to
	 *  disk: if the flush succeeds, all the preceding modifications will
	 *  have been written to disk.
	 *
	 *  If any of the modifications fail, an exception will be thrown and
	 *  the database will be left in a state in which each separate
	 *  addition, replacement or deletion operation has either been fully
	 *  performed or not performed at all: it is then up to the
	 *  application to work out which operations need to be repeated.
	 *
	 *  If called within a transaction, this will flush database
	 *  modifications made before the transaction was begun, but will
	 *  not flush modifications made since begin_transaction() was
	 *  called.
	 *
	 *  Beware of calling flush too frequently: this will have a severe
	 *  performance cost.
	 *
	 *  Note that flush need not be called explicitly: it will be called
	 *  automatically when the database is closed, or when a sufficient
	 *  number of modifications have been made.
	 *
	 *  @exception OmDatabaseError will be thrown if a problem occurs
	 *             while modifying the database.
	 *
	 *  @exception OmDatabaseCorruptError will be thrown if the
	 *             database is in a corrupt state.
	 *
	 *  @exception OmDatabaseLockError will be thrown if a lock couldn't
	 *             be acquired on the database.
	 */
	void flush();

	/** Begin a transaction.
	 *
	 *  For the purposes of Omseek, a transaction is a group of
	 *  modifications to the database which are grouped together such
	 *  that either all or none of them will succeed.  Even in the case
	 *  of a power failure, this characteristic should be preserved (as
	 *  long as the filesystem isn't corrupted, etc).
	 *
	 *  Transactions are only available with certain access methods,
	 *  and as you might expect will generally have a fairly high
	 *  performance cost.
	 *
	 *  A transaction may only be begun within a session, see
	 *  begin_session().
	 * 
	 *  @exception OmUnimplementedError will be thrown if transactions
	 *             are not available for this database type.
	 *
	 *  @exception OmInvalidOperationError will be thrown if this is
	 *             called at an invalid time, such as when a transaction
	 *             is already in progress.
	 */
	void begin_transaction();

	/** This ends the transaction currently in progress, commiting the
	 *  modifications made to the database.
	 *
	 *  If this completes successfully, all the database modifications
	 *  made during the transaction will have been committed to the
	 *  database.
	 *
	 *  If an error occurs, an exception will be thrown, and none of
	 *  the modifications made to the database during the transaction
	 *  will have been applied to the database.
	 *  
	 *  Whatever occurs, after this method the transaction will no
	 *  longer be in progress.
	 *
	 *  @exception OmDatabaseError will be thrown if a problem occurs
	 *             while modifying the database.
	 *
	 *  @exception OmDatabaseCorruptError will be thrown if the
	 *             database is in a corrupt state.
	 *
	 *  @exception OmInvalidOperationError will be thrown if a transaction
	 *             is not currently in progress.
	 *
	 *  @exception OmUnimplementedError will be thrown if transactions
	 *             are not available for this database type.
	 */
	void commit_transaction();

	/** This ends the transaction currently in progress, cancelling the
	 *  potential modifications made to the database.
	 *
	 *  If an error occurs in this method, an exception will be thrown,
	 *  but the transaction will be cancelled anyway.
	 *
	 *  @exception OmDatabaseError will be thrown if a problem occurs
	 *             while modifying the database.
	 *  
	 *  @exception OmDatabaseCorruptError will be thrown if the
	 *             database is in a corrupt state.
	 *
	 *  @exception OmInvalidOperationError will be thrown if a transaction
	 *             is not currently in progress.
	 *
	 *  @exception OmUnimplementedError will be thrown if transactions
	 *             are not available for this database type.
	 */
	void cancel_transaction();

	/** Add a new document to the database.
	 *
	 *  This method adds the specified document to the database,
	 *  returning a newly allocated document ID.
	 *
	 *  Note that this does not mean the document will immediately
	 *  appear in the database; see flush() for more details.
	 *
	 *  As with all database modification operations, the effect is
	 *  atomic: the document will either be fully added, or the document
	 *  fails to be added and an exception is thrown (possibly at a
	 *  later time when the session is ended or flushed).
	 *
	 *  If a session is not in progress when this method is called, a
	 *  session will be started.
	 *
	 *  @param document The new document to be added.
	 *
	 *  @return         The document ID of the newly added document.
	 *
	 *  @exception OmDatabaseError will be thrown if a problem occurs
	 *             while writing to the database.
	 *
	 *  @exception OmDatabaseCorruptError will be thrown if the
	 *             database is in a corrupt state.
	 *
	 *  @exception OmDatabaseLockError will be thrown if a lock couldn't
	 *             be acquired or subsequently released on the database.
	 */
	om_docid add_document(const OmDocument & document);

	/** Delete a document in the database.
	 *  FIXME: document more.
	 */
	void delete_document(om_docid did);

	/** Replace a given document in the database.
	 *  FIXME: document more.
	 */
	void replace_document(om_docid did, const OmDocument & document);

	/** Returns a string representing the database object.
	 *  Introspection method.
	 */
	std::string get_description() const;
};

#endif /* OM_HGUARD_OMDATABASE_H */
