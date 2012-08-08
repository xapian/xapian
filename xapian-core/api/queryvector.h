/** @file queryvector.h
 * @brief Append only vector of Query objects
 */
/* Copyright (C) 2012 Olly Betts
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

#ifndef XAPIAN_INCLUDED_QUERYVECTOR_H
#define XAPIAN_INCLUDED_QUERYVECTOR_H

#include "xapian/query.h"

namespace Xapian {

/** Vector of Query objects.
 *
 *  A notable feature is that if the vector holds <= 2 objects, there's no
 *  extra storage - the two Query::Internal pointers are held in the two
 *  pointers which otherwise point to the first and just after the last
 *  element.
 *
 *  This means that for the fairly common pair-wise operator case, we use less
 *  space than a std::vector<Query> would.
 */
class QueryVector {
  public:
    class const_iterator {
	void * const * ptr;

      public:
	const_iterator() { }

	const_iterator(void * const * ptr_) : ptr(ptr_) { }

	const_iterator & operator++() { ++ptr; return *this; }

	const_iterator operator++(int) { return const_iterator(ptr++); }

	Query operator*() const {
	    return Query(*static_cast<Query::Internal*>(*ptr));
	}

	Query operator[](size_t idx) const {
	    return Query(*static_cast<Query::Internal*>(ptr[idx]));
	}

	bool operator==(const const_iterator & o) const { return ptr == o.ptr; }

	bool operator!=(const const_iterator & o) const { return !(*this == o); }

	const_iterator operator+(int n) { return const_iterator(ptr + n); }
    };

    // Create an empty QueryVector.
    QueryVector() : c(0) { }

    // Create an empty QueryVector with n elements reserved.
    explicit QueryVector(size_t n) {
	if (n <= sizeof(p) / sizeof(*p)) {
	    c = 0;
	} else {
	    c = n;
	    p[1] = p[0] = static_cast<void*>(new void* [n]);
	}
    }

    ~QueryVector() {
	clear();
    }

    const_iterator begin() const {
	return const_iterator(c > sizeof(p) / sizeof(*p) ?
			      static_cast<void * const *>(p[0]) :
			      p);
    }

    const_iterator end() const {
	return const_iterator(c > sizeof(p) / sizeof(*p) ?
			      static_cast<void * const *>(p[1]) :
			      p + c);
    }

    size_t size() const {
	return c > sizeof(p) / sizeof(*p) ?
	       static_cast<void**>(p[1]) - static_cast<void**>(p[0]) : c;
    }

    size_t capacity() const {
	return c > sizeof(p) / sizeof(*p) ? c : sizeof(p) / sizeof(*p);
    }

    bool empty() const {
	return c > sizeof(p) / sizeof(*p) ? p[0] == p[1] : c == 0;
    }

    void clear() {
	for (const_iterator i = begin(); i != end(); ++i)
	    if ((*i).internal.get() && (*i).internal->_refs == 0)
		delete (*i).internal.get();

	if (c > sizeof(p) / sizeof(*p))
	    delete [] static_cast<Query::Internal**>(p[0]);

	c = 0;
    }

    void reserve(size_t n) {
	if (n > sizeof(p) / sizeof(*p) && n > c) {
	    void ** blk = new void* [n];
	    if (c > sizeof(p) / sizeof(*p)) {
		std::copy(static_cast<void **>(p[0]),
			  static_cast<void **>(p[1]),
			  blk);
		p[1] = blk +
		    (static_cast<void**>(p[1]) - static_cast<void**>(p[0]));
		delete [] static_cast<void**>(p[0]);
	    } else {
		std::copy(p, p + c, blk);
		p[1] = blk + c;
	    }
	    p[0] = blk;
	    c = n;
	}
    }

    void push_back(const Query & q) {
	if (size() == capacity()) {
	    reserve(capacity() * 2);
	}
	if (q.internal.get())
	    ++q.internal->_refs;
	if (c >= sizeof(p) / sizeof(*p)) {
	    void ** e = static_cast<void **>(p[1]);
	    *e++ = static_cast<void*>(q.internal.get());
	    p[1] = static_cast<void*>(e);
	} else {
	    p[c++] = q.internal.get();
	}
    }

    Query operator[](size_t idx) const {
	return begin()[idx];
    }

  private:
    size_t c;
    void * p[2];
};

}

#endif // XAPIAN_INCLUDED_QUERYVECTOR_H
