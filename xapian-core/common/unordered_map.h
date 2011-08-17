/* @file unordered_map.h
 * @brief Portability wrapper for the various unordered_map and hash_map versions
 */
/* Copyright (C) 2011 Olly Betts
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

#ifndef XAPIAN_INCLUDED_UNORDERED_MAP_H
#define XAPIAN_INCLUDED_UNORDERED_MAP_H

#if __cplusplus > 199711L || defined __GXX_EXPERIMENTAL_CXX0X__

// C++0x/C++1x will have <unordered_map>
#include <unordered_map>

#elif defined USE_BOOST

// Allow the user to -DUSE_BOOST.
#include <boost/unordered_map.hpp>
namespace std {
using boost::hash;
using boost::unordered_map;
}

#elif defined __GNUC__ && (__GNUC__ > 4 || __GNUC__ == 4 && __GNUC_MINOR__ >= 3)

// For GCC >= 4.3, use <tr1/unordered_map>.
#include <tr1/unordered_map>
namespace std {
using tr1::hash;
using tr1::unordered_map;
}

#elif defined __GNUC__ && __GNUC__ >= 3
// For GCC >= 3 use <ext/hash_map>.
#include <ext/hash_map>
namespace std {
using __gnu_cxx::hash_map;
using __gnu_cxx::hash;
}
#define unordered_map hash_map

#elif defined _MSC_VER
// For MSVC use <hash_map>.
#include <hash_map>

#if _MSC_VER >= 1310
// In MSVC 2003 (which is 7.1, or _MSC_VER == 1310), hash_map, etc moved to stdext.
namespace std {
using stdext::hash_map;
using stdext::hash;
}
#endif
#define unordered_map hash_map

#else

// This is what the header was called in SGI's STL.
#include <hash_map.h>
#define unordered_map hash_map

#endif

#endif // XAPIAN_INCLUDED_UNORDERED_MAP_H
