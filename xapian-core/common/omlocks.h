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

class OmLock {
    mutable pthread_mutex_t mutex;

    // disallow copies
    OmLock(const OmLock &);
    void operator=(const OmLock &);

    public:
    	OmLock() {
	    pthread_mutex_init(&mutex, NULL);
	}
	~OmLock() { pthread_mutex_destroy(&mutex); }

	void lock() const {
	    int retval = pthread_mutex_lock(&mutex);
	    Assert(retval == 0);
	}
	void unlock() const {
	    int retval = pthread_mutex_unlock(&mutex);
	    Assert(retval == 0);
	}
};

class OmLockSentry {
    const OmLock &mut;

    // disallow copies
    OmLockSentry(const OmLockSentry &);
    void operator=(const OmLockSentry &);

    public:
    	OmLockSentry(const OmLock &mut_) : mut(mut_) {
	    mut.lock();
	}

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
