/* refcnt.h: Reference-counted pointers
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

#ifndef OM_HGUARD_REFCNT_H
#define OM_HGUARD_REFCNT_H

#include "omlocks.h"

/** Reference counted objects should inherit from
 *  RefCntBase.  This gives the object a reference count
 *  and a lock used by RefCntPtr.
 */
class RefCntBase {
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
	RefCntBase(const RefCntBase &other)
		: ref_count(0), ref_count_mutex() { }

	/** Dummy class, used simply to make the private constructor
	 *  different.
	 */
	class RefCntPtrToThis {};

    public:
	/** Return the current ref count.
	 *
	 *  This is only rarely useful.  One use is for copy-on-write.
	 */
	ref_count_t ref_count_get() const
	{
	    return ref_count;
	}

	/// The constructor, which initialises the ref_count to 0.
	RefCntBase() : ref_count(0) { }

	/** Increase reference count from 0 to 1, used when first making an
	 *  RefCntPtr out of a pointer.
	 */
	void ref_start() const;

	/// Increase the reference count, used when copying an RefCntPtr.
	void ref_increment() const;

	/** Decrease the reference count.  In addition, return true if the
	 *  count has decreased to zero, meaning that this object should
	 *  be deleted.
	 */
	bool ref_decrement() const;
};

/** The actual reference-counted pointer.  Can be used with any
 *  class derived from RefCntBase, as long as it is allocated
 *  on the heap by new (not new[]!).
 */
template <class T>
class RefCntPtr {
    friend T;
    private:
	T *dest;

    public:
	/** Make an RefCntPtr for an object which may already
	 *  have reference counted pointers.  This should only
	 *  be called by the object itself, to pass references
	 *  to objects which it creates and which depend on it.
	 *  Everything else should already have a refcntptr.
	 *
	 *  (eg, a database might pass a newly created postlist
	 *  a reference counted pointer to itself.)
	 */
	RefCntPtr(RefCntBase::RefCntPtrToThis, T *dest_);

	T *operator->() const;
	T &operator*() const;
	T *get() const;
	RefCntPtr(T *dest_ = 0);
	RefCntPtr(const RefCntPtr &other);
	void operator=(const RefCntPtr &other);
	~RefCntPtr();

	template <class U>
	RefCntPtr(const RefCntPtr<U> &other);
};

inline void RefCntBase::ref_start() const
{
    OmLockSentry locksentry(ref_count_mutex);
    Assert(ref_count == 0);
    ref_count += 1;
}

inline void RefCntBase::ref_increment() const
{
    OmLockSentry locksentry(ref_count_mutex);
    ref_count += 1;
}

inline bool RefCntBase::ref_decrement() const
{
    OmLockSentry locksentry(ref_count_mutex);
    ref_count -= 1;
    return (ref_count == 0);
}



template <class T>
inline RefCntPtr<T>::RefCntPtr(RefCntBase::RefCntPtrToThis, T *dest_)
	: dest(dest_)
{
    Assert(dest != 0);
    Assert(dest->ref_count_get() != 0);
    dest->ref_increment();
}

template <class T>
inline RefCntPtr<T>::RefCntPtr(T *dest_) : dest(dest_)
{
    if (dest) {
	dest->ref_start();
    }
}

template <class T>
inline RefCntPtr<T>::RefCntPtr(const RefCntPtr &other) : dest(other.dest)
{
    if (dest) {
	Assert(dest->ref_count_get() != 0);
	dest->ref_increment();
    }
}

template <class T>
inline void RefCntPtr<T>::operator=(const RefCntPtr &other) {
    if (dest && dest->ref_decrement()) {
	delete dest;
    }
    dest = other.dest;
    if (dest) {
	Assert(dest->ref_count_get() != 0);
	dest->ref_increment();
    }
}

template <class T>
inline RefCntPtr<T>::~RefCntPtr()
{
    if (dest && dest->ref_decrement()) {
	delete dest;
	dest = 0;
    }
}

template <class T>
template <class U>
inline
RefCntPtr<T>::RefCntPtr(const RefCntPtr<U> &other)
	: dest(other.get())
{
    if (dest) {
	Assert(dest->ref_count_get() != 0);
	dest->ref_increment();
    }
}

template <class T>
inline T *RefCntPtr<T>::operator->() const
{
    return dest;
}

template <class T>
inline T &RefCntPtr<T>::operator*() const
{
    return *dest;
}

template <class T>
inline T *RefCntPtr<T>::get() const
{
    return dest;
}

#endif /* OM_HGUARD_REFCNT_H */
