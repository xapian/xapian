/* omdatabase.h
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

#ifndef OM_HGUARD_OMDATABASE_H
#define OM_HGUARD_OMDATABASE_H

#include <vector>
#include "omindexdoc.h"
#include "omsettings.h"

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
    /** OmWritableDatabase is a friend so that it can call is_writable().
     */
    friend OmWritableDatabase;

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
    
    private:
	/** Check whether this is a writable database.
	 *
	 *  This is used to check that an assignment to an OmWritableDatabase
	 *  is valid.
	 *
	 *  @return true if the database is writable, false otherwise.
	 *               This always returns false for an instance of
	 *               OmDatabase (as opposed to an instance of a subclass).
	 */
	virtual bool is_writable() const { return false; }

    protected:
	class Internal;

	/** Reference counted internals. */
	Internal *internal;

	/** InternalInterface is a class used internally to interact
	 *  with OmDatabase objects.
	 */
	class InternalInterface;

	friend class InternalInterface;

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

	/** Returns a string representing the database object.
	 *  Introspection method.
	 */
	virtual std::string get_description() const;
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
    private:
	/** Assignment of an OmDatabase to an OmWritableDatabase is
	 *  not allowed.  This method may get called by calling the
	 *  assignment operator from a reference to an OmDatabase,
	 *  in which case an exception will be thrown unless the
	 *
	 *  @param     other   the OmDatabase to assign.
	 *
	 *  @exception OmInvalidArgument is thrown if other is not a reference
	 *             to a writable database.
	 */
	virtual void operator=(const OmDatabase &other);

	/** Check whether this is a writable database.
	 *
	 *  This is used to check that an assignment to an OmWritableDatabase
	 *  is valid.
	 *
	 *  @return true if the database is writable, false otherwise.
	 *               This always returns true for an OmWritableDatabase.
	 */
	virtual bool is_writable() const { return true; }
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

	/** This call tells Muscat to begin a session for modifying a
	 *  database.
	 *
	 *  For efficiency reasons, when performing multiple updates to a
	 *  database it is best (indeed, almost essential) to make as many
	 *  modifications as memory will permit in a single pass through
	 *  the database.  To allow this, Muscat provides sessions for
	 *  database modifications:  before starting a set of modifications
	 *  you should begin a session, and at the end of a set of
	 *  modifications you should end the session (with end_session()).
	 *
	 *  The precise effect of begin_session() depends upon the type of
	 *  the database.  One common result is to lock the database to
	 *  protect against concurrent access: current Muscat databases
	 *  allow only one database object to be updating a database at
	 *  once: failure to observe this condition would be likely to
	 *  result in corruption of the database.  To prevent this
	 *  occuring, a lock is obtained before any modifications are made
	 *  to the database.
	 *
	 *  It is not essential to begin a session explicitly; if a session
	 *  is not in progress when an operation (such as adding a
	 *  document) is performed, one will be implicitly created for the
	 *  duration of the operation.
	 *
	 *  If multiple processes require simultaneous write access to the
	 *  database, sessions should be as short as possible to prevent
	 *  blocking other processes.  Note that this will cause severe
	 *  performance degradation.  A possible future addition is to
	 *  produce database types which allow for multiple processes to
	 *  efficiently write to a database simultaneously.
	 *
	 *  Note that Muscat's locking prevents other processes and
	 *  objects from modifying the database, but does not prevent other
	 *  threads within this process from modifying the database, via
	 *  the same object.  Appropriate concurrency controls exist such
	 *  that inter-thread conflicts will not corrupt databases, but it
	 *  is the responsibility of the application to ensure that race
	 *  conditions (such as two threads simultaneously attempting to
	 *  replace the same document) are avoided.
	 *
	 *  If an error occurs during the method, an exception will be thrown,
	 *  and a session will not be started.
	 *
	 *  @param timeout  The time to wait for a lock.  (in microseconds)
	 *                  The default of 0 means that failure to obtain a
	 *                  lock immediately will result in an exception
	 *                  being thrown: this is appropriate in cases
	 *                  where other processes are not expected to be
	 *                  writing to the database.
	 *
	 *  @exception OmDatabaseCorruptError will be thrown if the
	 *             database is in a corrupt state.
	 *
	 *  @exception OmDatabaseLockError will be thrown if a lock couldn't
	 *             be acquired on the database.
	 */
	void begin_session(om_timeout timeout = 0);

	/** End a session of modifications to the database.
	 *
	 *  This flushes any modifications made to the database, cancels any
	 *  transactions in progress, and releases any locks held on the
	 *  database.
	 *
	 *  If an error occurs, each separate addition, replacement or
	 *  deletion operation will either be fully performed or not
	 *  performed at all: it is then up to the application to work
	 *  out which operations need to be repeated.
	 *
	 *  If an error occurs during the method, an exception will be thrown,
	 *  and the session will be terminated.  If any transactions are
	 *  in progress, they will be cancelled.
	 *
	 *  @exception OmDatabaseError will be thrown if a problem occurs
	 *             while modifying the database.
	 *
	 *  @exception OmDatabaseCorruptError will be thrown if the
	 *             database is in a corrupt state.
	 *
	 *  @exception OmInvalidOperationError will be thrown if a session
	 *             is not currently in progress.
	 *
	 *  @exception OmDatabaseLockError will be thrown if a lock wasn't
	 *             currently held on the database, and locks were in
	 *             use.
	 */
	void end_session();

	/** Flush any modifications made to the database within the current
	 *  session.
	 *
	 *  This may be called at any time during a session to ensure that
	 *  the modifications which have been made are written to disk:
	 *  if the flush succeeds, all the preceding modifications will
	 *  have been written to disk.
	 *
	 *  If any of the modifications fail, an exception will be thrown and
	 *  the database will be left in a state in which each modification
	 *  has either been performed fully or has failed and not been
	 *  performed at all.
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
	 *  automatically when the session is ended, or when a sufficient
	 *  number of modifications have been made.
	 *
	 *  @exception OmDatabaseError will be thrown if a problem occurs
	 *             while modifying the database.
	 *
	 *  @exception OmDatabaseCorruptError will be thrown if the
	 *             database is in a corrupt state.
	 *
	 *  @exception OmInvalidArgumentError will be thrown if this is
	 *             called when a modification session is not in progress.
	 *
	 *  @exception OmInvalidOperationError will be thrown if this is
	 *             called at an invalid time, such as when a session is
	 *             not in progress.
	 */
	void flush();

	/** Begin a transaction.
	 *
	 *  For the purposes of Muscat, a transaction is a group of
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
	 *             called at an invalid time, such as when a session is
	 *             not in progress or when a transaction is already in
	 *             progress.
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
	 *  appear in the database; see begin_session() for more details.
	 *
	 *  As with all database modification operations, the effect is
	 *  atomic: the document will either be fully added, or the document
	 *  fails to be added and an exception is thrown (possibly at a
	 *  later time when the session is ended or flushed).
	 *
	 *  If a session is not in progress when this method is called, a
	 *  session will be created for the duration of the method.
	 *  Again, see begin_session() for more notes on sessions.
	 *
	 *  @param document The new document to be added.
	 *
	 *  @param timeout  The timeout parameter to pass to begin_session()
	 *                  if a session is not already in progress.  This
	 *                  parameter will only be used if a session has
	 *                  not already been started (see begin_session()).
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
	om_docid add_document(const OmDocumentContents & document,
			      om_timeout timeout = 0);

	/** Delete a document in the database.
	 *  FIXME: document more.
	 */
	void delete_document(om_docid did, om_timeout timeout = 0);

	/** Replace a given document in the database.
	 *  FIXME: document more.
	 */
	void replace_document(om_docid did,
			      const OmDocumentContents & document,
			      om_timeout timeout = 0);

	/** Get a document from the database.
	 *  FIXME: document more.
	 */
	OmDocumentContents get_document(om_docid did);

	/** Returns a string representing the database object.
	 *  Introspection method.
	 */
	std::string get_description() const;
};

#endif /* OM_HGUARD_OMDATABASE_H */
