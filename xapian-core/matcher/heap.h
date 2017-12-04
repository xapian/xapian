/** @file heap.h
 * @brief C++ STL heap implementation with extensions
 *
 * Adapted from libc++'s <algorithm>, with the following additions:
 *
 * * replace() - pop() followed by push(), but as efficient as just pop()
 *   (i.e. at most 2 * log(N) compares rather than at most 3 * log(N))
 * * siftdown() - sink adjusted entry to restore heap invariant
 *
 * Complexity:
 *
 * make() : At most 3*N comparisons
 * pop()/replace()/siftdown() : At most 2*log(N) comparisons
 * push() : At most log(N) comparisons (O(1) average)
 * sort() : At most 2*N*log(N) comparisons
 */

// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses:
//
// ==============================================================================
// libc++ License
// ==============================================================================
//
// The libc++ library is dual licensed under both the University of Illinois
// "BSD-Like" license and the MIT license.  As a user of this code you may choose
// to use it under either license.  As a contributor, you agree to allow your code
// to be used under both.
//
// Full text of the relevant licenses is included below.
//
// ==============================================================================
//
// University of Illinois/NCSA
// Open Source License
//
// Copyright (c) 2009-2016 by the contributors listed in CREDITS.TXT
//
// All rights reserved.
//
// Developed by:
//
//     LLVM Team
//
//     University of Illinois at Urbana-Champaign
//
//     http://llvm.org
//
// Permission is hereby granted, free of charge, to any person obtaining a copy of
// this software and associated documentation files (the "Software"), to deal with
// the Software without restriction, including without limitation the rights to
// use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
// of the Software, and to permit persons to whom the Software is furnished to do
// so, subject to the following conditions:
//
//     * Redistributions of source code must retain the above copyright notice,
//       this list of conditions and the following disclaimers.
//
//     * Redistributions in binary form must reproduce the above copyright notice,
//       this list of conditions and the following disclaimers in the
//       documentation and/or other materials provided with the distribution.
//
//     * Neither the names of the LLVM Team, University of Illinois at
//       Urbana-Champaign, nor the names of its contributors may be used to
//       endorse or promote products derived from this Software without specific
//       prior written permission.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
// FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
// CONTRIBUTORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS WITH THE
// SOFTWARE.
//
// ==============================================================================
//
// Copyright (c) 2009-2014 by the contributors listed in CREDITS.TXT
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.

namespace Heap {

// push_heap

template <class _Compare, class _RandomAccessIterator>
void
sift_up_(_RandomAccessIterator first, _RandomAccessIterator last, _Compare comp,
          typename iterator_traits<_RandomAccessIterator>::difference_type len)
{
    typedef typename iterator_traits<_RandomAccessIterator>::value_type value_type;
    if (len > 1)
    {
        len = (len - 2) / 2;
        _RandomAccessIterator ptr = first + len;
        if (comp(*ptr, *--last))
        {
            value_type t(std::move(*last));
            do
            {
                *last = std::move(*ptr);
                last = ptr;
                if (len == 0)
                    break;
                len = (len - 1) / 2;
                ptr = first + len;
            } while (comp(*ptr, t));
            *last = std::move(t);
        }
    }
}

template <class _RandomAccessIterator, class _Compare>
inline
void
push(_RandomAccessIterator first, _RandomAccessIterator last, _Compare comp)
{
    sift_up_(first, last, comp, last - first);
}

// pop_heap

template <class _Compare, class _RandomAccessIterator>
void
sift_down_(_RandomAccessIterator first, _Compare comp,
            typename iterator_traits<_RandomAccessIterator>::difference_type len,
            _RandomAccessIterator start)
{
    typedef typename iterator_traits<_RandomAccessIterator>::difference_type difference_type;
    typedef typename iterator_traits<_RandomAccessIterator>::value_type value_type;
    // left-child of start is at 2 * start + 1
    // right-child of start is at 2 * start + 2
    difference_type child = start - first;

    if (len < 2 || (len - 2) / 2 < child)
        return;

    child = 2 * child + 1;
    _RandomAccessIterator child_i = first + child;

    if ((child + 1) < len && comp(*child_i, *(child_i + 1))) {
        // right-child exists and is greater than left-child
        ++child_i;
        ++child;
    }

    // check if we are in heap-order
    if (comp(*child_i, *start))
        // we are, start is larger than it's largest child
        return;

    value_type top(std::move(*start));
    do
    {
        // we are not in heap-order, swap the parent with it's largest child
        *start = std::move(*child_i);
        start = child_i;

        if ((len - 2) / 2 < child)
            break;

        // recompute the child based off of the updated parent
        child = 2 * child + 1;
        child_i = first + child;

        if ((child + 1) < len && comp(*child_i, *(child_i + 1))) {
            // right-child exists and is greater than left-child
            ++child_i;
            ++child;
        }

        // check if we are in heap-order
    } while (!comp(*child_i, top));
    *start = std::move(top);
}

template <class _Compare, class _RandomAccessIterator>
inline
void
pop_heap_(_RandomAccessIterator first, _RandomAccessIterator last, _Compare comp,
           typename iterator_traits<_RandomAccessIterator>::difference_type len)
{
    if (len > 1)
    {
        swap(*first, *--last);
        sift_down_(first, comp, len - 1, first);
    }
}

template <class _RandomAccessIterator, class _Compare>
inline
void
pop(_RandomAccessIterator first, _RandomAccessIterator last, _Compare comp)
{
    pop_heap_(first, last, comp, last - first);
}

template <class _Compare, class _RandomAccessIterator>
inline
void
replace_heap_(_RandomAccessIterator first, _Compare comp,
              typename iterator_traits<_RandomAccessIterator>::difference_type len)
{
    sift_down_(first, comp, len, first);
}

// Replace the tip of the heap then call replace() to restore the invariant.
template <class _RandomAccessIterator, class _Compare>
inline void
replace(_RandomAccessIterator first, _RandomAccessIterator last, _Compare comp)
{
    replace_heap_(first, comp, last - first);
}

template <class _Compare, class _RandomAccessIterator>
inline void
siftdown_heap_(_RandomAccessIterator first,
               _RandomAccessIterator elt, _Compare comp,
               typename iterator_traits<_RandomAccessIterator>::difference_type len)
{
    sift_down_(first, comp, len, elt);
}

// Replace an element with a "worse" one (in _Compare terms) and call siftdown_heap()
// to restore the invariant.
template <class _RandomAccessIterator, class _Compare>
inline
void
siftdown(_RandomAccessIterator first, _RandomAccessIterator last,
         _RandomAccessIterator elt, _Compare comp)
{
    siftdown_heap_(first, elt, comp, last - first);
}

// make_heap

template <class _RandomAccessIterator, class _Compare>
void
make(_RandomAccessIterator first, _RandomAccessIterator last, _Compare comp)
{
    typedef typename iterator_traits<_RandomAccessIterator>::difference_type difference_type;
    difference_type n = last - first;
    if (n > 1)
    {
        // start from the first parent, there is no need to consider children
        for (difference_type start = (n - 2) / 2; start >= 0; --start)
        {
            sift_down_(first, comp, n, first + start);
        }
    }
}

}
