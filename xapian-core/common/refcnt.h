/* refcnt.h: Reference-counted pointers
 *
 * ----START-LICENCE----
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002 Ananova Ltd
 * Copyright 2002,2003 Olly Betts
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

#include "omdebug.h"

/** Reference counted objects should inherit from
 *  RefCntBase.  This gives the object a reference count used by RefCntPtr.
 */
class RefCntBase {
    private:
	typedef unsigned int ref_count_t;

	/// The actual reference count
	mutable ref_count_t ref_count;

    protected:
	/** The copy constructor.
	 *
	 *  This is protected since it'll only be used by derived classes,
	 *  which should only rarely need copying (this is, after all, a
	 *  refcount implementation).  Sometimes it's needed, though,
	 *  since we need to zero ref_count in the copy.
	 */
	RefCntBase(const RefCntBase &) : ref_count(0) { }

    public:
	/// The constructor, which initialises the ref_count to 0.
	RefCntBase() : ref_count(0) { }

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
    private:
	T *dest;

    public:
	T *operator->() const;
	T &operator*() const;
	T *get() const;
	/** Make an RefCntPtr for an object which may already
	 *  have reference counted pointers.  You usually pass in
	 *  a newly created object, or an object may pass in "this"
	 *   to get a RefCntPtr to itself to pass to other classes.
	 *
	 *  (eg, a database might pass a newly created postlist
	 *  a reference counted pointer to itself.)
	 */
	RefCntPtr(T *dest_);
	RefCntPtr();
	RefCntPtr(const RefCntPtr &other);
	void operator=(const RefCntPtr &other);
	~RefCntPtr();

	template <class U>
	RefCntPtr(const RefCntPtr<U> &other);
};

inline void RefCntBase::ref_increment() const
{
    ++ref_count;
}

inline bool RefCntBase::ref_decrement() const
{
    Assert(ref_count != 0);
    --ref_count;
    return (ref_count == 0);
}

template <class T>
inline RefCntPtr<T>::RefCntPtr(T *dest_) : dest(dest_)
{
    if (dest) dest->ref_increment();
}

template <class T>
inline RefCntPtr<T>::RefCntPtr() : dest(0)
{
}

template <class T>
inline RefCntPtr<T>::RefCntPtr(const RefCntPtr &other) : dest(other.dest)
{
    if (dest) dest->ref_increment();
}

template <class T>
inline void RefCntPtr<T>::operator=(const RefCntPtr &other) {
    // check if we're assigning a pointer to itself
    if (dest == other.dest) return;
    
    // copy the new dest in before we delete the old to avoid a small
    // window in which dest points to a deleted object
    // FIXME: if pointer assignment isn't atomic, we ought to use locking...
    T *old_dest = dest;
    dest = other.dest;
    if (dest) dest->ref_increment();
    if (old_dest && old_dest->ref_decrement()) delete old_dest;
}

template <class T>
inline RefCntPtr<T>::~RefCntPtr()
{
    if (dest && dest->ref_decrement()) {
	// zero before we delete to avoid a small window in which dest points
	// to a deleted object
	// FIXME: if pointer assignment isn't atomic, we ought to use locking...
	T *old_dest = dest;
	dest = 0;
	delete old_dest;
    }
}

template <class T>
template <class U>
inline
RefCntPtr<T>::RefCntPtr(const RefCntPtr<U> &other)
	: dest(other.get())
{
    if (dest) dest->ref_increment();
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
