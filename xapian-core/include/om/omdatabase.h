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

class OmWritableDatabase;
class OmDatabaseGroup;

///////////////////////////////////////////////////////////////////
// OmDatabase class
// ================

/** This class is used to access a database.
 */
class OmDatabase {
    /** OmWritableDatabase is a friend so that it can call is_writable().
     */
    friend OmWritableDatabase;

    /** OmDatabaseGroup is a friend so that it can access the internals
     *  to extract the database pointer.
     */
    friend OmDatabaseGroup;

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

	/** Open a database, possibly readonly.
	 *
	 *  This may be used to open a database for writing, and is used
	 *  in this way by OmWritableDatabase.
	 *
	 *  @param type    a string describing the database type.
	 *  @param params  a vector of parameters to be used to open the
	 *                 database: meaning and number required depends
	 *                 on database type.
	 *
	 *  @exception OmInvalidArgumentError will be thrown if an invalid
	 *             argument is supplied, for example, an unknown database
	 *             type.
	 *  @exception OmOpeningError may be thrown if the database cannot
	 *             be opened.
	 *
	 */
	OmDatabase(const std::string & type,
		   const std::vector<std::string> & params,
		   bool readonly);
    public:
	/** Open a database.
	 *
	 *  @param type    a string describing the database type.
	 *  @param params  a vector of parameters to be used to open the
	 *                 database: meaning and number required depends
	 *                 on database type.
	 *
	 *  @exception OmInvalidArgumentError will be thrown if an invalid
	 *             argument is supplied, for example, an unknown database
	 *             type.
	 *  @exception OmOpeningError may be thrown if the database cannot
	 *             be opened.
	 */
	OmDatabase(const std::string & type,
		   const std::vector<std::string> & params);

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
	 *  @param type    a string describing the database type.
	 *  @param params  a vector of parameters to be used to open the
	 *                 database: meaning and number required depends
	 *                 on database type.
	 *
	 *  @exception OmInvalidArgumentError will be thrown if an invalid
	 *             argument is supplied, for example, an unknown database
	 *             type.
	 *  @exception OmOpeningError may be thrown if the database cannot
	 *             be opened.
	 */
	OmWritableDatabase(const std::string & type,
			   const std::vector<std::string> & params);

	/** Destroy this handle on the database.
	 *  If there are no copies of this object remaining, the database
	 *  will be closed, and if it is locked the lock will be released.
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

	/** Lock the database to protect against concurrent access.
	 *
	 *  Muscat databases allow only one database object to be updating
	 *  a database at once: failure to observe this condition is likely
	 *  to result in corruption of the database.  To prevent this
	 *  occuring, a lock must be obtained before any modifications are
	 *  made to the database.
	 *
	 *  Note that this is a object level lock: while it is recommended
	 *  that only one thread attempts to modify a database, it is up to
	 *  the user to enforce this.
	 *
	 *  Obtaining this lock prevents other processes from modifying the
	 *  database, but does not prevent other threads within this
	 *  process from modifying the database, via the same object.
	 *  Appropriate concurrency controls exist such that inter-thread
	 *  conflicts will not corrupt databases, but it is possible for
	 *  race conditions to exist: for example, were two threads to be
	 *  deleting the same document while another is adding a new one
	 *  the new document could be added in place of the old document,
	 *  and promptly deleted by the final thread.
	 *
	 *  A lock may either be obtained explicitly, by calling this method,
	 *  or will be obtained implicitly whenever a method is called which
	 *  requires the protection of a lock.
	 *
	 *  If multiple operations are to be performed, and no other
	 *  processes require the ability to update the database, a lock
	 *  should be acquired explicitly and held for the duration of
	 *  updates.  If other processes require write access to the
	 *  database, the lock should be held for as little time as needed,
	 *  and in this situation the implicit locking mechanism may be
	 *  a desirable simplification of the code's logic.
	 *
	 *  @param timeout  The time to wait for a lock.  (in microseconds)
	 *                  The default of 0 means that failure to obtain a
	 *                  lock immediately will result in an exception
	 *                  being thrown: this is appropriate in cases
	 *                  where other processes are not expected to be
	 *                  writing to the database.
	 *
	 *  @exception OmDatabaseLockError will be thrown if a lock couldn't
	 *             be acquired on the database.
	 */
	void lock(om_timeout timeout = 0);

	/** Unlock the database.
	 *
	 *  This releases a lock held on the database.
	 *
	 *  @exception OmDatabaseLockError will be thrown if a lock wasn't
	 *             currently held on the database.
	 */
	void unlock();

	/** Add a new document to the database.
	 *
	 *  This method atomically adds the document: the result is either
	 *  that the document is added, or that the document fails to be
	 *  added and an exception is thrown.
	 *
	 *  If the database is not locked when this method is called, a lock
	 *  will be obtained for the duration of the method.  This may result
	 *  in an OmDatabaseLockError.
	 *
	 *  See lock() for more notes on database locks.
	 *
	 *  @param document The new document to be added.
	 *
	 *  @return         The document ID of the newly added document.
	 *
	 *  @exception OmDatabaseError will be thrown if a problem occurs
	 *             while writing to the database.
	 *
	 *  @exception OmDatabaseLockError will be thrown if a lock couldn't
	 *             be acquired or subsequently released on the database.
	 */
#if 0
	 *  @param timeout  The time to wait for a lock.  (in microseconds)
	 *                  The default of 0 means that failure to obtain a
	 *                  lock immediately will result in an exception
	 *                  being thrown: this is appropriate in cases
	 *                  where other processes are not expected to be
	 *                  writing to the database.
#endif
	om_docid add_document(const OmDocumentContents & document);

	/** Returns a string representing the database object.
	 *  Introspection method.
	 */
	std::string get_description() const;
};

///////////////////////////////////////////////////////////////////
// OmDatabaseGroup class
// =====================

/** This class encapsulates a set of databases.
 *
 *  This class is used in conjunction with an OmEnquire object.
 *
 *  @exception OmInvalidArgumentError will be thrown if an invalid
 *  argument is supplied, for example, an unknown database type.
 *
 *  @exception OmOpeningError may be thrown if the database cannot
 *  be opened (for example, a required file cannot be found).
 */
class OmDatabaseGroup {
    public:
	/** This class is used internally to hide implementation details.
	 */
	class Internal;

	/** InternalInterface is a class used internally to interact
	 *  with OmDatabaseGroup objects.
	 */
	class InternalInterface;
    private:
	Internal *internal;

	friend class InternalInterface;

    public:
	OmDatabaseGroup();
	~OmDatabaseGroup();
	OmDatabaseGroup(const OmDatabaseGroup &other);
	void operator=(const OmDatabaseGroup &other);

	/** Add a new database to use.
	 *
	 *  This call will opened a new database with the specified
	 *  parameters, and add it to the set of databases to be accessed
	 *  by this database group.
	 *
	 *  The database will always be opened read-only.
	 *
	 *  @param type    a string describing the database type.
	 *  @param params  a vector of parameters to be used to open the
	 *  database: meaning and number required depends on database type.
	 *
	 *  @exception OmInvalidArgumentError  See class documentation.
	 *  @exception OmOpeningError          See class documentation.
	 */
	void add_database(const std::string & type,
			  const std::vector<std::string> & params);

	/** Add an already opened database to the set of databases to be
	 *  accessed by this database group.
	 *
	 *  The handle on the database will be copied, so may be deleted
	 *  or reused by the caller as desired.
	 *
	 *  @param database the opened database to add.
	 */
	void add_database(const OmDatabase & database);

	/** Returns a string representing the database group object.
	 *  Introspection method.
	 */
	std::string get_description() const;
};

#endif /* OM_HGUARD_OMDATABASE_H */

