/** @file
 * @brief Arithmetic operations with overflow checks
 *
 * The operations are implemented with compiler builtins or equivalent where
 * possible, so the overflow check will typically just require a check of the
 * processor's overflow or carry flag.
 */
/* Copyright (C) 2018,2022,2023 Olly Betts
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
 */

#ifndef XAPIAN_INCLUDED_OVERFLOW_H
#define XAPIAN_INCLUDED_OVERFLOW_H

#ifndef PACKAGE
# error config.h must be included first in each C++ source file
#endif

#include <type_traits>

#if !HAVE_DECL___BUILTIN_ADD_OVERFLOW || !HAVE_DECL___BUILTIN_SUB_OVERFLOW
# if HAVE_DECL__ADDCARRY_U32 || HAVE_DECL__ADDCARRY_U64 || \
     HAVE_DECL__SUBBORROW_U32 || HAVE_DECL__SUBBORROW_U64
#  include <intrin.h>
# endif
#endif

/** Addition with overflow checking.
 *
 *  Add @a a and @a b in infinite precision, and store the result in
 *  @a res.
 *
 *  Where possible, compiler built-ins or intrinsics are used to try to ensure
 *  minimal overhead from the overflow check.
 *
 *  Currently only supported when types involved are unsigned.
 *
 *  @return true if the result can be represented exactly in @a res, false
 *	    otherwise.
 */
template<typename T1, typename T2, typename R>
typename std::enable_if<std::is_unsigned<T1>::value &&
			std::is_unsigned<T2>::value &&
			std::is_unsigned<R>::value, bool>::type
add_overflows(T1 a, T2 b, R& res) {
#if HAVE_DECL___BUILTIN_ADD_OVERFLOW
    return __builtin_add_overflow(a, b, &res);
#else
    // Use a local variable to test for overflow so we can use auto and get
    // a type which will at least hold each of the inputs, and so we don't need
    // to worry if res could be modified by another thread between us setting
    // and testing it.
    auto r = a + b;
    res = static_cast<R>(r);
    // Overflow is only possible if the result type is the same width as or
    // narrower than at least one of the input types.
    //
    // We've overflowed if r doesn't fit in type R, or if the result is less
    // then an input.
    typedef decltype(r) r_type;
    return (sizeof(R) <= sizeof(T1) || sizeof(R) <= sizeof(T2)) &&
	   (r_type(res) != r || r < r_type(b));
#endif
}

#if !HAVE_DECL___BUILTIN_ADD_OVERFLOW
// Only use the addcarry intrinsics where the builtins aren't available.
// GCC and clang support both, but _addcarry_u64() uses `unsigned long
// long` instead of `unsigned __int64` and the two types aren't compatible.
# if HAVE_DECL__ADDCARRY_U32
template<>
inline bool
add_overflows<unsigned,
	      unsigned,
	      unsigned>(unsigned a,
			unsigned b,
			unsigned& res) {
    return _addcarry_u32(0, a, b, &res) != 0;
}
# endif

# if HAVE_DECL__ADDCARRY_U64
template<>
inline bool
add_overflows<unsigned __int64,
	      unsigned __int64,
	      unsigned __int64>(unsigned __int64 a,
				unsigned __int64 b,
				unsigned __int64& res) {
    return _addcarry_u64(0, a, b, &res) != 0;
}
# endif
#endif

/** Subtraction with overflow checking.
 *
 *  Subtract @a b from @a a in infinite precision, and store the result in
 *  @a res.
 *
 *  Where possible, compiler built-ins or intrinsics are used to try to ensure
 *  minimal overhead from the overflow check.
 *
 *  Currently only supported when types involved are unsigned.
 *
 *  @return true if the result can be represented exactly in @a res, false
 *	    otherwise.
 */
template<typename T1, typename T2, typename R>
typename std::enable_if<std::is_unsigned<T1>::value &&
			std::is_unsigned<T2>::value &&
			std::is_unsigned<R>::value, bool>::type
sub_overflows(T1 a, T2 b, R& res) {
#if HAVE_DECL___BUILTIN_ADD_OVERFLOW
    return __builtin_sub_overflow(a, b, &res);
#else
    // Use a local variable to test for overflow so we can use auto and get
    // a type which will at least hold each of the inputs, and so we don't need
    // to worry if res could be modified by another thread between us setting
    // and testing it.
    auto r = a - b;
    res = static_cast<R>(r);
    // We've overflowed if r doesn't fit in type R, or if the subtraction
    // wrapped.
    typedef decltype(r) r_type;
    return r_type(res) != r || r > r_type(a);
#endif
}

#if !HAVE_DECL___BUILTIN_SUB_OVERFLOW
// Only use the subborrow intrinsics where the builtins aren't available.
// GCC and clang support both, but _subborrow_u64() uses `unsigned long
// long` instead of `unsigned __int64` and the two types aren't compatible.
# if HAVE_DECL__SUBBORROW_U32
template<>
inline bool
sub_overflows<unsigned,
	      unsigned,
	      unsigned>(unsigned a,
			unsigned b,
			unsigned& res) {
    return _subborrow_u32(0, a, b, &res) != 0;
}
# endif

# if HAVE_DECL__SUBBORROW_U64
template<>
inline bool
sub_overflows<unsigned __int64,
	      unsigned __int64,
	      unsigned __int64>(unsigned __int64 a,
				unsigned __int64 b,
				unsigned __int64& res) {
    return _subborrow_u64(0, a, b, &res) != 0;
}
# endif
#endif

/** Multiplication with overflow checking.
 *
 *  Multiply @a a and @a b in infinite precision, and store the result in
 *  @a res.
 *
 *  Where possible, compiler built-ins or intrinsics are used to try to ensure
 *  minimal overhead from the overflow check.
 *
 *  Currently only supported when types involved are unsigned.
 *
 *  @return true if the result can be represented exactly in @a res, false
 *	    otherwise.
 */
template<typename T1, typename T2, typename R>
typename std::enable_if<std::is_unsigned<T1>::value &&
			std::is_unsigned<T2>::value &&
			std::is_unsigned<R>::value, bool>::type
mul_overflows(T1 a, T2 b, R& res) {
#if HAVE_DECL___BUILTIN_MUL_OVERFLOW
    return __builtin_mul_overflow(a, b, &res);
#else
    // Use a local variable to test for overflow so we can use auto and get
    // a type which will at least hold each of the inputs, and so we don't need
    // to worry if res could be modified by another thread between us setting
    // and testing it.
    auto r = a * b;
    res = static_cast<R>(r);
    // We've overflowed if r doesn't fit in type R, or if the multiplication
    // wrapped.
    typedef decltype(r) r_type;
    return r_type(res) != r || (a != 0 && r / r_type(a) != r_type(b));
#endif
}

#endif // XAPIAN_INCLUDED_OVERFLOW_H
