/** @file
 * @brief functions for reading and writing different width words
 */
/* Copyright (C) 2016,2018,2019 Olly Betts
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

#ifndef XAPIAN_INCLUDED_WORDACCESS_H
#define XAPIAN_INCLUDED_WORDACCESS_H

#ifndef PACKAGE
# error config.h must be included first in each C++ source file
#endif

#include <cstdint>
#include <type_traits>
#include <cstring>

#include "alignment_cast.h"
#include "omassert.h"

#ifndef WORDS_BIGENDIAN

#if HAVE_DECL__BYTESWAP_USHORT || HAVE_DECL__BYTESWAP_ULONG
# include <stdlib.h>
#endif

inline uint16_t do_bswap(uint16_t value) {
# if HAVE_DECL___BUILTIN_BSWAP16
    return __builtin_bswap16(value);
# elif HAVE_DECL__BYTESWAP_USHORT
    return _byteswap_ushort(value);
# else
    return (value << 8) | (value >> 8);
# endif
}

inline uint32_t do_bswap(uint32_t value) {
# if HAVE_DECL___BUILTIN_BSWAP32
    return __builtin_bswap32(value);
# elif HAVE_DECL__BYTESWAP_ULONG
    return _byteswap_ulong(value);
# else
    return (value << 24) |
	   ((value & 0xff00) << 8) |
	   ((value >> 8) & 0xff00) |
	   (value >> 24);
# endif
}

inline uint64_t do_bswap(uint64_t value) {
# if HAVE_DECL___BUILTIN_BSWAP64
    return __builtin_bswap64(value);
# elif HAVE_DECL__BYTESWAP_UINT64
    return _byteswap_uint64(value);
# else
    return (value << 56) |
	   ((value & 0xff00) << 40) |
	   ((value & 0xff0000) << 24) |
	   ((value & 0xff000000) << 8) |
	   ((value >> 8) & 0xff000000) |
	   ((value >> 24) & 0xff0000) |
	   ((value >> 40) & 0xff00) |
	   (value >> 56);
# endif
}

#endif

template<typename UINT>
inline UINT
do_aligned_read(const unsigned char * ptr)
{
    UINT value = *alignment_cast<const UINT*>(ptr);
#ifndef WORDS_BIGENDIAN
    value = do_bswap(value);
#endif
    return value;
}

template<typename T, typename UINT>
inline void
do_aligned_write(unsigned char * ptr, T value)
{
    if (std::is_signed<T>::value) {
	AssertRel(value, >=, 0);
    }
    if (sizeof(T) > sizeof(UINT)) {
	AssertEq(value, T(UINT(value)));
    }
    UINT v = UINT(value);
#ifndef WORDS_BIGENDIAN
    v = do_bswap(v);
#endif
    *alignment_cast<UINT*>(ptr) = v;
}

template<typename UINT>
inline UINT
do_unaligned_read(const unsigned char * ptr)
{
    UINT value;
    memcpy(&value, ptr, sizeof(UINT));
#ifndef WORDS_BIGENDIAN
    value = do_bswap(value);
#endif
    return value;
}

template<typename T, typename UINT>
inline void
do_unaligned_write(unsigned char * ptr, T value)
{
    if (std::is_signed<T>::value) {
	AssertRel(value, >=, 0);
    }
    if (sizeof(T) > sizeof(UINT)) {
	AssertEq(value, T(UINT(value)));
    }
    UINT v = UINT(value);
#ifndef WORDS_BIGENDIAN
    v = do_bswap(v);
#endif
    memcpy(ptr, &v, sizeof(UINT));
}

inline uint32_t
aligned_read4(const unsigned char *ptr)
{
    return do_aligned_read<uint32_t>(ptr);
}

inline uint32_t
unaligned_read4(const unsigned char *ptr)
{
    return do_unaligned_read<uint32_t>(ptr);
}

inline uint16_t
aligned_read2(const unsigned char *ptr)
{
    return do_aligned_read<uint16_t>(ptr);
}

inline uint16_t
unaligned_read2(const unsigned char *ptr)
{
    return do_unaligned_read<uint16_t>(ptr);
}

template<typename T>
inline void
aligned_write4(unsigned char *ptr, T value)
{
    do_aligned_write<T, uint32_t>(ptr, value);
}

template<typename T>
inline void
unaligned_write4(unsigned char *ptr, T value)
{
    do_unaligned_write<T, uint32_t>(ptr, value);
}

template<typename T>
inline void
aligned_write2(unsigned char *ptr, T value)
{
    do_aligned_write<T, uint16_t>(ptr, value);
}

template<typename T>
inline void
unaligned_write2(unsigned char *ptr, T value)
{
    do_unaligned_write<T, uint16_t>(ptr, value);
}

#endif // XAPIAN_INCLUDED_WORDACCESS_H
