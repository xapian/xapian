/** @file
 * @brief Arithmetic operations with overflow checks
 *
 * The operations are implemented with compiler builtins or equivalent where
 * possible, so the overflow check will typically just require a check of the
 * processor's overflow or carry flag.
 */
/* Copyright (C) 2018 Olly Betts
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
    // Use a local variable to test for overflow so we don't need to worry if
    // res could be modified by another thread between us setting and testing
    // it.
    R r = R(a) + R(b);
    res = r;
    return (sizeof(R) <= sizeof(T1) || sizeof(R) <= sizeof(T2)) && r < R(b);
#endif
}

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
    // Use a local variable to test for overflow so we don't need to worry if
    // res could be modified by another thread between us setting and testing
    // it.
    R r = R(a) * R(b);
    res = r;
    return sizeof(R) < sizeof(T1) + sizeof(T2) && a != 0 && T2(r / R(a)) != b;
#endif
}

#endif // XAPIAN_INCLUDED_OVERFLOW_H
