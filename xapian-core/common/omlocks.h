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

#ifdef HAVE_LIBPTHREAD
#include <pthread.h>
#include "omassert.h"

class OmLock {
    pthread_mutex_t mutex;
    public:
    	OmLock() {
	    pthread_mutex_init(&mutex, NULL);
	}
	~OmLock() { pthread_mutex_destroy(&mutex); }

	void lock() {
	    int retval = pthread_mutex_lock(&mutex);
	    Assert(retval == 0);
	}
	void unlock() {
	    int retval = pthread_mutex_unlock(&mutex);
	    Assert(retval == 0);
	}
};
#else // !HAVE_LIBPTHREAD
class OmLock {
    public:
	void lock() {}
	void unlock() {}
};
#endif // HAVE_LIBPTHREAD
#endif /* OM_HGUARD_OMLOCKS_H */
