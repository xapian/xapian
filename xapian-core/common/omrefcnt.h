/* omrefcnt.h: Reference-counted pointers
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

#ifndef OM_HGUARD_OMREFCNT_H
#define OM_HGUARD_OMREFCNT_H

#include "omlocks.h"

/** Reference counted objects should inherit from
 *  OmRefCntBase.  This gives the object a reference count
 *  and a lock used by OmRefCntPtr.
 */
class OmRefCntBase {
    private:
	typedef unsigned int ref_count_t;

	/// The actual reference count
	mutable ref_count_t ref_count;

	/// the lock used for synchronising increment and decrement
	OmLock ref_count_mutex;

    protected:
	/** The copy constructor.
	 *
	 *  This is protected since it'll only be used by derived classes,
	 *  which should only rarely need copying (this is, after all, a
	 *  refcount implementation).  Sometimes it's needed, though,
	 *  since OmLock objects can't be copied.
	 */
	OmRefCntBase(const OmRefCntBase &other)
		: ref_count(0), ref_count_mutex() {};

    public:
	/** Return the current ref count.
	 *
	 *  This is only rarely useful.  One use is for copy-on-write.
	 */
	ref_count_t ref_count_get()
	{
	    return ref_count;
	}

	/// The constructor, which initialises the ref_count to 0.
	OmRefCntBase() : ref_count(0) {};

	/** Increase reference count from 0 to 1, used when first making an
	 *  OmRefCntPtr out of a pointer.
	 */
	void ref_start() const;
	
	/// Increase the reference count, used when copying an OmRefCntPtr.
	void ref_increment() const;

	/** Decrease the reference count.  In addition, return true if the
	 *  count has decreased to zero, meaning that this object should
	 *  be deleted.
	 */
	bool ref_decrement() const;
};

/** The actual reference-counted pointer.  Can be used with any
 *  class derived from OmRefCntBase, as long as it is allocated
 *  on the heap by new (not new[]!).
 */
template <class T>
class OmRefCntPtr {
    private:
	T *dest;

    public:
	T *operator->() const;
	T &operator*() const;
	T *get() const;
	OmRefCntPtr(T *dest_ = 0);
	OmRefCntPtr(const OmRefCntPtr &other);
	void operator=(const OmRefCntPtr &other);
	~OmRefCntPtr();
};

inline void OmRefCntBase::ref_start() const
{
    OmLockSentry locksentry(ref_count_mutex);
    Assert(ref_count == 0);
    ref_count += 1;
}

inline void OmRefCntBase::ref_increment() const
{
    OmLockSentry locksentry(ref_count_mutex);
    ref_count += 1;
}

inline bool OmRefCntBase::ref_decrement() const
{
    OmLockSentry locksentry(ref_count_mutex);
    ref_count -= 1;
    return (ref_count == 0);
}

template <class T>
inline OmRefCntPtr<T>::OmRefCntPtr(T *dest_) : dest(dest_)
{
    if (dest) {
	dest->ref_start();
    }
}

template <class T>
inline OmRefCntPtr<T>::OmRefCntPtr(const OmRefCntPtr &other) : dest(other.dest)
{
    dest->ref_increment();
}

template <class T>
inline void OmRefCntPtr<T>::operator=(const OmRefCntPtr &other) {
    if (dest && dest->ref_decrement()) {
	delete dest;
    };
    dest = other.dest;
    if (dest) {
	dest->ref_increment();
    };
}

template <class T>
inline OmRefCntPtr<T>::~OmRefCntPtr()
{
    if (dest && dest->ref_decrement()) {
	delete dest;
	dest = 0;
    }
}

template <class T>
inline T *OmRefCntPtr<T>::operator->() const
{
    return dest;
}

template <class T>
inline T &OmRefCntPtr<T>::operator*() const
{
    return *dest;
}

template <class T>
inline T *OmRefCntPtr<T>::get() const
{
    return dest;
}

#endif /* OM_HGUARD_OMREFCNT_H */
