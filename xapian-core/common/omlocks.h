/* omlocks.h: class for managing pthread locks
 *
 * ----START-LICENCE----
 * Copyright 1999,2000 Dialog Corporation
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

#ifndef OM_HGUARD_OMLOCKS_H
#define OM_HGUARD_OMLOCKS_H

#include "config.h"
#include "omassert.h"

#ifdef HAVE_LIBPTHREAD

#include <pthread.h>

#include <errno.h>

//////////////////////////////////////////////////
// OmLock class
// ============
/** Representation of a mutex.
 *  The OmLock class encapsulates the basic operations on
 *  a pthread mutex (when available).  Most code wishing
 *  to take advantage of locks should use the OmLockSentry
 *  class.
 *  OmLock can be used even if it is const - const methods
 *  may need to get locks too.
 */
class OmLock {
    private:
	/// The physical pthread mutex.
	mutable pthread_mutex_t mutex;
	mutable bool islocked;

	// disallow copies
	OmLock(const OmLock &);
	void operator=(const OmLock &);

    public:
	/// The constructor, which initialises the mutex
    	OmLock() : islocked(false) {
	    pthread_mutex_init(&mutex, NULL);
	}

	/// The destructor, which destroys the mutex.
	~OmLock() { pthread_mutex_destroy(&mutex); }

	/** Acquire an exclusive lock on the mutex.  This
	 *  will not return until the mutex is locked.
	 */
	void lock() const {
	    int retval = pthread_mutex_lock(&mutex);
	    Assert(retval == 0);
	    islocked = true;
	}

	/** Release the lock.  The lock should be currently
	 *  owned by the thread calling unlock.
	 */
	void unlock() const {
	    Assert(islocked);
	    islocked = false;
	    int retval = pthread_mutex_unlock(&mutex);

	    // FIXME - remove next line
	    if(retval != 0) {
		cout << "retval:"
		     << strerror(retval)
		     << " (" << retval
		     << ")" << endl;
	    }
		    
	    Assert(retval == 0);
	}
};

//////////////////////////////////////////////////////
// OmLockSentry class
// ==================
/** Convenient automatic handling of OmLock objects.
 *  An OmLockSentry object acquires a lock (via an OmLock
 *  object) at construction and releases it at destruction.
 *  This means that the lock will be held during the object's
 *  scope, guaranteeing that the lock will be released when
 *  no longer needed.
 */
class OmLockSentry {
    private:
	/// A reference to the OmLock object.
	const OmLock &mut;

	// disallow copies
	OmLockSentry(const OmLockSentry &);
	void operator=(const OmLockSentry &);

    public:
	/// The constructor, which locks the passed in OmLock
	/// object.
    	OmLockSentry(const OmLock &mut_) : mut(mut_) {
	    mut.lock();
	}

	/// The destructor, which releases the lock.
	~OmLockSentry() {
	    try {
		mut.unlock();
	    } catch (OmAssertionError &err) {
		// catch any assertion exceptions from unlock,
		// since throwing exceptions from destructors
		// is bad
		cerr << err.get_msg() << endl;
	    }
	}
};

#else // !HAVE_LIBPTHREAD

class OmLock {
    public:
	void lock() const {}
	void unlock() const {}
};

class OmLockSentry {

    // disallow copies
    OmLockSentry(const OmLockSentry &);
    void operator=(const OmLockSentry &);

    public:
    	OmLockSentry(const OmLock &mut_) { }

	~OmLockSentry() { }
};

#endif // HAVE_LIBPTHREAD

#endif /* OM_HGUARD_OMLOCKS_H */
