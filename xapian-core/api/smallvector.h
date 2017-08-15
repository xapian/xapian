/** @file smallvector.h
 * @brief Append only vector of Xapian PIMPL objects
 */
/* Copyright (C) 2012,2013,2014,2017 Olly Betts
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#ifndef XAPIAN_INCLUDED_SMALLVECTOR_H
#define XAPIAN_INCLUDED_SMALLVECTOR_H

#include <cstddef> // For std::size_t

namespace Xapian {

class SmallVector_ {
    std::size_t c;

    void * p[2];

    static const std::size_t INTERNAL_CAPACITY = sizeof(p) / sizeof(*p);

  public:
    SmallVector_() : c(0) { }

    explicit SmallVector_(std::size_t n) : c(0) {
	reserve(n);
    }

    std::size_t size() const {
	if (!is_external())
	    return c;
	void * const * b = static_cast<void * const *>(p[0]);
	void * const * e = static_cast<void * const *>(p[1]);
	return e - b;
    }

    std::size_t capacity() const {
	return is_external() ? c : INTERNAL_CAPACITY;
    }

    bool empty() const {
	return is_external() ? p[0] == p[1] : c == 0;
    }

    void reserve(std::size_t n) {
	if (n > INTERNAL_CAPACITY && n > c) {
	    do_reserve(n);
	    c = n;
	}
    }

    void do_free();

  protected:
    void do_reserve(std::size_t n);

    void do_clear() {
	if (is_external())
	    do_free();
	c = 0;
    }

    /// Return true if storage is external to the object.
    bool is_external() const {
	return c > INTERNAL_CAPACITY;
    }

    void * const * do_begin() const {
	return is_external() ? static_cast<void * const *>(p[0]) : p;
    }

    void * const * do_end() const {
	return is_external() ? static_cast<void * const *>(p[1]) : p + c;
    }

    void do_push_back(void* elt) {
	std::size_t cap = capacity();
	if (size() == cap) {
	    cap *= 2;
	    do_reserve(cap);
	    c = cap;
	}
	if (c >= INTERNAL_CAPACITY) {
	    void ** e = static_cast<void **>(p[1]);
	    *e++ = elt;
	    p[1] = static_cast<void*>(e);
	} else {
	    p[c++] = elt;
	}
    }
};

/** Vector of Xapian PIMPL objects.
 *
 *  A notable feature is that if the vector holds <= 2 objects, there's no
 *  extra storage - the two internal pointers are held in the two
 *  pointers which otherwise point to the first and just after the last
 *  element.
 *
 *  This means that for the fairly common cases of pair-wise Query operators
 *  and Database objects with one or two subdatabases, we use less space than
 *  std::vector<Xapian::Foo> would.
 */
template<typename T>
class SmallVector : public SmallVector_ {
  public:
    typedef std::size_t size_type;
    class const_iterator {
	void * const * ptr;

      public:
	const_iterator() { }

	explicit const_iterator(void * const * ptr_) : ptr(ptr_) { }

	const_iterator & operator++() { ++ptr; return *this; }

	const_iterator operator++(int) { return const_iterator(ptr++); }

	T operator*() const {
	    return T(static_cast<typename T::Internal*>(*ptr));
	}

	T operator[](size_type idx) const {
	    return T(static_cast<typename T::Internal*>(ptr[idx]));
	}

	bool operator==(const const_iterator& o) const { return ptr == o.ptr; }

	bool operator!=(const const_iterator& o) const { return !(*this == o); }

	const_iterator operator+(int n) { return const_iterator(ptr + n); }
    };

    // Create an empty SmallVector.
    SmallVector() : SmallVector_() { }

    // Create an empty SmallVector with n elements reserved.
    explicit SmallVector(size_type n) : SmallVector_(n) { }

    ~SmallVector() {
	clear();
    }

    const_iterator begin() const {
	return const_iterator(do_begin());
    }

    const_iterator end() const {
	return const_iterator(do_end());
    }

    void clear() {
	for (const_iterator i = begin(); i != end(); ++i)
	    if ((*i).internal.get() && --(*i).internal->_refs == 0)
		delete (*i).internal.get();

	do_clear();
    }

    void push_back(const T & elt) {
	do_push_back(static_cast<void*>(elt.internal.get()));
	if (elt.internal.get())
	    ++elt.internal->_refs;
    }

    T operator[](size_type idx) const {
	return begin()[idx];
    }
};

}

#endif // XAPIAN_INCLUDED_SMALLVECTOR_H
