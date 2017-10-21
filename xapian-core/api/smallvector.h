/** @file smallvector.h
 * @brief Custom vector implementations using small vector optimisation
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

#include <algorithm>
#include <cstddef> // For std::size_t
#include <cstring> // For std::memcpy
#include <type_traits>

namespace Xapian {

/** Suitable for "simple" type T.
 *
 *  If sizeof(T) > 2 * sizeof(T*) this isn't going to work, and it's not very
 *  useful when sizeof(T) == 2 * sizeof(T*) as you can only store a single
 *  element inline.
 *
 *  Offers optional Copy-On-Write functionality - if COW is true, then copying
 *  a Vec with external data only makes a copy of that data if you attempt
 *  to modify it.  Current COW is only supported for integral types T.
 */
template<typename T,
	 bool COW = false,
	 typename = typename std::enable_if<!COW ||
					    std::is_integral<T>::value>::type>
class Vec {
    std::size_t c;

    static constexpr std::size_t INTERNAL_CAPACITY = 2 * sizeof(T*) / sizeof(T);

    union {
	T v[INTERNAL_CAPACITY];
	struct {
	    T* b;
	    T* e;
	};
    } u;

    struct Vec_to_copy {
	const Vec& ref;
	explicit Vec_to_copy(const Vec& o) : ref(o) {}
    };

  public:
    typedef std::size_t size_type;

    typedef const T* const_iterator;

    typedef T* iterator;

    Vec() : c(0) { }

    // Prevent inadvertent copying.
    Vec(const Vec&) = delete;

    Vec(const Vec_to_copy& o) {
	do_copy_from(o.ref);
    }

    // Prevent inadvertent copying.
    void operator=(const Vec&) = delete;

    void operator=(const Vec_to_copy& o) {
	clear();
	do_copy_from(o.ref);
    }

    Vec_to_copy copy() const {
	return Vec_to_copy(*this);
    }

    Vec(Vec&& o) noexcept : Vec() {
	std::memcpy(&u, &o.u, sizeof(u));
	std::swap(c, o.c);
    }

    void operator=(Vec&& o) {
	clear();
	u = o.u;
	std::swap(c, o.c);
    }

    explicit Vec(size_type n) : c(0) {
	reserve(n);
    }

    ~Vec() {
	clear();
    }

    size_type size() const {
	return is_external() ? u.e - u.b : c;
    }

    size_type capacity() const {
	return is_external() ? c : INTERNAL_CAPACITY;
    }

    bool empty() const {
	return is_external() ? u.b == u.e : c == 0;
    }

    void reserve(size_type n) {
	if (n > capacity()) {
	    do_reserve(n);
	}
    }

    const_iterator cbegin() const {
	return is_external() ? u.b : u.v;
    }

    const_iterator cend() const {
	return is_external() ? u.e : u.v + c;
    }

    const_iterator begin() const {
	return cbegin();
    }

    const_iterator end() const {
	return cend();
    }

    iterator begin() {
	// FIXME: This is a bit eager - non-const begin() is often invoked when
	// no modification is needed, but doing it lazily is a bit tricky as
	// the pointer will change when we COW.
	if (COW && is_external() && u.b[-1] > 0) {
	    do_cow();
	}
	return is_external() ? u.b : u.v;
    }

    iterator end() {
	if (COW && is_external() && u.b[-1] > 0) {
	    do_cow();
	}
	return is_external() ? u.e : u.v + c;
    }

    void push_back(T elt) {
	auto cap = capacity();
	if (size() == cap) {
	    do_reserve(cap * 2);
	}
	if (c >= INTERNAL_CAPACITY) {
	    if (COW && u.b[-1] > 0)
		do_cow();
	    *(u.e++) = elt;
	} else {
	    u.v[c++] = elt;
	}
    }

    void pop_back() {
	if (is_external()) {
	    --u.e;
	} else {
	    --c;
	}
    }

    void clear() {
	if (is_external())
	    do_free();
	c = 0;
    }

    void erase(const_iterator it) {
	auto end_it = end();
	while (true) {
	    T* p = const_cast<T*>(it);
	    ++it;
	    if (it == end_it)
		break;
	    *p = *it;
	}
	if (is_external()) {
	    --u.e;
	} else {
	    --c;
	}
    }

    void insert(const_iterator pos, const T& elt) {
	push_back(T());
	T* p = const_cast<T*>(end());
	while (--p != pos) {
	    *p = p[-1];
	}
	*(const_cast<T*>(pos)) = elt;
    }

    const T& operator[](size_type idx) const {
	return begin()[idx];
    }

    T& operator[](size_type idx) {
	if (COW && is_external() && u.b[-1] > 0) {
	    do_cow();
	}
	return const_cast<T&>(begin()[idx]);
    }

    const T& front() const {
	return *(begin());
    }

    const T& back() const {
	return end()[-1];
    }

  protected:
    void do_free() {
	if (!COW || u.b[-1] == 0)
	    delete [] (u.b - COW);
	else
	    --u.b[-1];
    }

    void do_reserve(size_type n) {
	// Logic error or size_t wrapping.
	if (rare(COW ? n < c : n <= c))
	    throw std::bad_alloc();
	T* blk = new T[n + COW];
	if (COW)
	    *blk++ = 0;
	if (is_external()) {
	    u.e = std::copy(u.b, u.e, blk);
	    do_free();
	} else {
	    u.e = std::copy(u.v, u.v + c, blk);
	}
	u.b = blk;
	c = n;
    }

    void do_cow() {
	T* blk = new T[c + 1];
	*blk++ = 0;
	u.e = std::copy(u.b, u.e, blk);
	--u.b[-1];
	u.b = blk;
    }

    void do_copy_from(const Vec& o) {
	if (!o.is_external()) {
	    u = o.u;
	    c = o.c;
	} else if (COW) {
	    u = o.u;
	    c = o.c;
	    ++u.b[-1];
	} else {
	    T* blk = new T[o.c];
	    u.e = std::copy(o.u.b, o.u.e, blk);
	    u.b = blk;
	    c = o.c;
	}
    }

    /// Return true if storage is external to the object.
    bool is_external() const noexcept {
	return c > INTERNAL_CAPACITY;
    }
};

template<typename T>
using VecCOW = Vec<T, true>;

class SmallVector_ {
    std::size_t c;

    void * p[2];

#ifndef _MSC_VER
    static constexpr std::size_t INTERNAL_CAPACITY = sizeof(p) / sizeof(*p);
#else
    static constexpr std::size_t INTERNAL_CAPACITY = 2; // Argh!
#endif

  public:
    SmallVector_() : c(0) { }

    // Prevent inadvertent copying.
    SmallVector_(const SmallVector_&) = delete;

    // Prevent inadvertent copying.
    void operator=(const SmallVector_&) = delete;

    SmallVector_(SmallVector_&& o) noexcept : SmallVector_() {
	std::memcpy(p, o.p, sizeof(p));
	std::swap(c, o.c);
    }

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
	}
    }

  protected:
    void do_free();

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
	    do_reserve(cap * 2);
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

/** Vector of Xapian PIMPL internal objects.
 *
 *  A notable feature is that if the vector holds <= 2 objects, there's no
 *  extra storage - the two internal pointers are held in the two
 *  pointers which otherwise point to the first and just after the last
 *  element.
 *
 *  This means that for the fairly common cases of pair-wise Query operators
 *  and Database objects with one or two subdatabases, we use less space than
 *  std::vector<Xapian::Foo::Internal*> would.
 */
template<typename TI>
class SmallVectorI : public SmallVector_ {
  public:
    typedef std::size_t size_type;

    typedef TI*const* const_iterator;

    // Create an empty SmallVectorI.
    SmallVectorI() : SmallVector_() { }

    // Create an empty SmallVectorI with n elements reserved.
    explicit SmallVectorI(size_type n) : SmallVector_(n) { }

    SmallVectorI(SmallVectorI&& o) noexcept : SmallVector_(o) { }

    ~SmallVectorI() {
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
	    if ((*i) && --(*i)->_refs == 0)
		delete *i;

	do_clear();
    }

    void push_back(TI* elt) {
	// Cast away potential const-ness in TI.  We can only try to modify an
	// element after casting to TI*, so this is const-safe overall.
	do_push_back(const_cast<void*>(static_cast<const void*>(elt)));
	if (elt)
	    ++elt->_refs;
    }

    TI* operator[](size_type idx) const {
	return begin()[idx];
    }

    TI* front() const {
	return *(begin());
    }

    TI* back() const {
	return end()[-1];
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
class SmallVector : public SmallVectorI<typename T::Internal> {
    typedef SmallVectorI<typename T::Internal> super;

  public:
    typedef std::size_t size_type;

    class const_iterator {
	typename super::const_iterator ptr;

      public:
	const_iterator() : ptr(nullptr) { }

	explicit const_iterator(typename super::const_iterator ptr_)
	    : ptr(ptr_) { }

	const_iterator & operator++() { ++ptr; return *this; }

	const_iterator operator++(int) { return const_iterator(ptr++); }

	T operator*() const {
	    return T(*ptr);
	}

	T operator[](size_type idx) const {
	    return T(ptr[idx]);
	}

	bool operator==(const const_iterator& o) const { return ptr == o.ptr; }

	bool operator!=(const const_iterator& o) const { return !(*this == o); }

	const_iterator operator+(int n) { return const_iterator(ptr + n); }

	const_iterator operator-(int n) { return const_iterator(ptr - n); }
    };

    // Create an empty SmallVector.
    SmallVector() { }

    // Create an empty SmallVector with n elements reserved.
    explicit SmallVector(size_type n) : super(n) { }

    const_iterator begin() const {
	return const_iterator(super::begin());
    }

    const_iterator end() const {
	return const_iterator(super::end());
    }

    using super::push_back;

    void push_back(const T & elt) {
	push_back(elt.internal.get());
    }

    T operator[](size_type idx) const {
	return begin()[idx];
    }

    T front() const {
	return *(begin());
    }

    T back() const {
	return end()[-1];
    }
};

}

#endif // XAPIAN_INCLUDED_SMALLVECTOR_H
