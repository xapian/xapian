/** @file base.h
 * @brief Reference-counted pointers
 */
/* Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002 Ananova Ltd
 * Copyright 2002,2003,2004,2007 Olly Betts
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

#ifndef XAPIAN_INCLUDED_BASE_H
#define XAPIAN_INCLUDED_BASE_H

#include <xapian/deprecated.h>

namespace Xapian {
namespace Internal {

/** @internal Reference counted internal classes should inherit from RefCntBase.
 *
 * This gives the object a reference count used by RefCntPtr.
 */
class RefCntBase {
    /* Note: We never delete a pointer to a subclass of RefCntBase using
     * a RefCntBase *, so we don't need a virtual destructor here.
     */
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

	typedef unsigned int ref_count_t;

	/** The actual reference count.  It's mutable so we can have reference
	 *  counting work with const pointers.
	 */
	mutable ref_count_t ref_count;
};

/** @internal A reference-counted pointer.  Can be used with any
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
	/** Make a RefCntPtr for an object which may already
	 *  have reference counted pointers.
	 *
	 *  You usually pass in a newly created object, or an object may pass
	 *  in "this" to get a RefCntPtr to itself to pass to other classes.
	 *  (e.g. a database might pass a newly created postlist a reference
	 *  counted pointer to itself.)
	 */
	RefCntPtr(T *dest_);
	RefCntPtr();
	RefCntPtr(const RefCntPtr &other);
	void operator=(const RefCntPtr &other);
	void operator=(T *dest_);
	~RefCntPtr();

	template <class U>
	RefCntPtr(const RefCntPtr<U> &other);
};

template <class T>
inline RefCntPtr<T>::RefCntPtr(T *dest_) : dest(dest_)
{
    if (dest) ++dest->ref_count;
}

template <class T>
inline RefCntPtr<T>::RefCntPtr() : dest(0)
{
}

template <class T>
inline RefCntPtr<T>::RefCntPtr(const RefCntPtr &other) : dest(other.dest)
{
    if (dest) ++dest->ref_count;
}

template <class T>
inline void RefCntPtr<T>::operator=(const RefCntPtr &other) {
    operator=(other.dest);
}

template <class T>
inline void RefCntPtr<T>::operator=(T *dest_) {
    // copy the new dest in before we delete the old to avoid a small
    // window in which dest points to a deleted object
    // FIXME: if pointer assignment isn't atomic, we ought to use locking...
    T *old_dest = dest;
    dest = dest_;
    // Increment the new before we decrement the old so that if dest == dest_
    // we don't delete the pointer.
    //
    // Note that if dest == dest_, either both are NULL (in which case we
    // aren't reference counting), or we're already reference counting the
    // object, in which case ref_count is non-zero at this point.  So we
    // won't accidentally delete an untracked object by doing this.
    if (dest) ++dest->ref_count;
    if (old_dest && --old_dest->ref_count == 0) delete old_dest;
}

template <class T>
inline RefCntPtr<T>::~RefCntPtr()
{
    if (dest && --dest->ref_count == 0) {
	// zero before we delete to avoid a small window in which dest points
	// to a deleted object
	// FIXME: if pointer assignment isn't atomic, we ought to use locking...
	T * condemned = dest;
	dest = 0;
	delete condemned;
    }
}

template <class T>
template <class U>
inline
RefCntPtr<T>::RefCntPtr(const RefCntPtr<U> &other)
	: dest(other.get())
{
    if (dest) ++dest->ref_count;
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

}
}

#endif /* XAPIAN_INCLUDED_BASE_H */
