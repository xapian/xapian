/* autoptr.h: An auto pointer implementation
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

#ifndef OM_HGUARD_AUTOPTR_H
#define OM_HGUARD_AUTOPTR_H

template <class T>
struct AutoPtrRef;

/** The actual auto pointer.  Can be used with any pointer allocated
 *  by operator new (not new[]!).
 */
template <class T>
class AutoPtr {
    private:
	typedef AutoPtrRef<T> thisAutoPtrRef;
    public:
	typedef T element_type;

	explicit AutoPtr(T *p = 0) throw();

	AutoPtr(AutoPtr &other) throw();

	template <class U>
	AutoPtr(AutoPtr<U> &other) throw();

	AutoPtr &operator=(AutoPtr &other) throw();

	template <class U>
	AutoPtr &operator=(AutoPtr<U> &other) throw();

	AutoPtr &operator=(thisAutoPtrRef other) throw();

	~AutoPtr() throw();

	T &operator*() const throw();
	T *operator->() const throw();
	T *get() const throw();
	T *release() throw();
	void reset(T *p = 0) throw();

	AutoPtr(thisAutoPtrRef other) throw() : dest(other.ap.release()) { }
	template <class U>
	operator AutoPtrRef<U>() throw() {
		AutoPtrRef<U> ref;
		ref.ap = *this;
		return ref;
	}
	template <class U>
	operator AutoPtr<U>() throw();
    private:
	T *dest;
};

/** A helper class used by AutoPtr
 */
template <class T>
struct AutoPtrRef {
    AutoPtr<T> ap;
};

template <class T>
AutoPtr<T>::AutoPtr<T>(T *p) throw()
	: dest(p) { }

template <class T>
AutoPtr<T>::AutoPtr<T>(AutoPtr &other) throw()
	: dest(other.release()) { }

template <class T>
template <class U>
AutoPtr<T>::AutoPtr<T>(AutoPtr<U> &other) throw()
	: dest(other.release()) { }

template <class T>
AutoPtr<T> &AutoPtr<T>::operator=(AutoPtr<T> &other) throw()
{
    reset(other.release());
    return *this;
}

template <class T>
template <class U>
AutoPtr<T> &AutoPtr<T>::operator=(AutoPtr<U> &other) throw()
{
    reset(other.release());
    return *this;
}

template <class T>
AutoPtr<T> &AutoPtr<T>::operator=(AutoPtr<T>::thisAutoPtrRef other) throw()
{
    reset(other.ap.release());
    return *this;
}

template <class T>
AutoPtr<T>::~AutoPtr() throw()
{
    delete dest;
}

template <class T>
T &AutoPtr<T>::operator*() const throw()
{
    return *get();
}

template <class T>
T *AutoPtr<T>::operator->() const throw()
{
    return get();
}

template <class T>
T *AutoPtr<T>::get() const throw()
{
    return dest;
}

template <class T>
T *AutoPtr<T>::release() throw()
{
    T *temp = get();
    dest = 0;
    return temp;
}

template <class T>
void AutoPtr<T>::reset(T *p) throw()
{
    if (get() != p) {
	delete get();
    }
    dest = p;
}

#if 0
template <class T>
AutoPtr<T>::AutoPtr(AutoPtrRef<T> ref) throw()
	: dest(ref.ap.release()) { }
#endif

#if 0
template <class T>
template <class U>
AutoPtr<T>::operator AutoPtr<T>::AutoPtrRef<U>() throw()
{
    AutoPtrRef<U> ar;
    ar.ap = *this;
    return ar;
}
#endif

template <class T>
template <class U>
AutoPtr<T>::operator AutoPtr<U>() throw()
{
    return AutoPtr<U>(*this);
}

#endif /* OM_HGUARD_AUTOPTR_H */
