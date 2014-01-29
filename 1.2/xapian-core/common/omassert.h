/** @file omassert.h
 * @brief Various assertion macros.
 */
/* Copyright (C) 2007,2008,2009,2013 Olly Betts
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

/* The "om" prefix is a historical vestige dating back to the "Open Muscat"
 * code base upon which Xapian is partly based.  It's preserved here only
 * to avoid colliding with the ISO C "assert.h" header.
 */

#ifndef XAPIAN_INCLUDED_OMASSERT_H
#define XAPIAN_INCLUDED_OMASSERT_H

/** Assert that a constant expression is non-zero.
 *
 *  If the assertion fails, compilation fails with an error.  There's no
 *  run-time overhead for a compile-time assertion, so we always enable
 *  them.
 *
 *  This macro must be used within a function (this is because we can only
 *  portably generate a "unique" name using __LINE__, but then if this macro
 *  is used in header files, multiple headers might be included from the same
 *  source file and have CompileTimeAssert() at the same line number.
 */
#if __cplusplus >= 201103L
// Under C++11, just use static_assert.
# define CompileTimeAssert(COND) static_assert(COND, #COND)
#else
# define CompileTimeAssert(COND)\
    do {\
	typedef int xapian_compile_time_check_[(COND) ? 1 : -1];\
	xapian_compile_time_check_ xapian_compile_time_check_var_;\
	(void)xapian_compile_time_check_var_;\
    } while (0)
#endif

#ifndef XAPIAN_ASSERTIONS
// The configure script should always define XAPIAN_ASSERTIONS if it defines
// XAPIAN_ASSERTIONS_PARANOID.
# ifdef XAPIAN_ASSERTIONS_PARANOID
#  error XAPIAN_ASSERTIONS_PARANOID defined without XAPIAN_ASSERTIONS
# endif
#else

#include <xapian/error.h>

#include "str.h"
#include "utils.h" // For within_DBL_EPSILON().

#define XAPIAN_ASSERT_LOCATION__(LINE,MSG) __FILE__":"#LINE": "#MSG
#define XAPIAN_ASSERT_LOCATION_(LINE,MSG) XAPIAN_ASSERT_LOCATION__(LINE,MSG)
#define XAPIAN_ASSERT_LOCATION(MSG) XAPIAN_ASSERT_LOCATION_(__LINE__,MSG)

// Expensive (or potentially expensive) assertions can be marked as "Paranoid"
// - these can be disabled separately from other assertions to allow a build
// with assertions which still has good performance.
#ifdef XAPIAN_ASSERTIONS_PARANOID
# define AssertParanoid(COND) Assert(COND)
# define AssertRelParanoid(A,REL,B) AssertRel(A,REL,B)
# define AssertEqParanoid(A,B) AssertEq(A,B)
# define AssertEqDoubleParanoid(A,B) AssertEqDouble(A,B)
#endif

/** Assert that condition COND is non-zero.
 *
 *  If this assertion fails, Xapian::AssertionError() will be thrown.
 */
#define Assert(COND) \
    do {\
	if (rare(!(COND)))\
	    throw Xapian::AssertionError(XAPIAN_ASSERT_LOCATION(COND));\
    } while (0)

/** Assert that A REL B is non-zero.
 *
 *  The intended usage is that REL is ==, !=, <=, <, >=, or >.
 *
 *  If this assertion fails, Xapian::AssertionError() will be thrown, and the
 *  exception message will include the values of A and B.
 */
#define AssertRel(A,REL,B) \
    do {\
	if (rare(!((A) REL (B)))) {\
	    std::string xapian_assertion_msg(XAPIAN_ASSERT_LOCATION(A REL B));\
	    xapian_assertion_msg += " : values were ";\
	    xapian_assertion_msg += str(A);\
	    xapian_assertion_msg += " and ";\
	    xapian_assertion_msg += str(B);\
	    throw Xapian::AssertionError(xapian_assertion_msg);\
	}\
    } while (0)

/** Assert that A == B.
 *
 *  This is just a wrapper for AssertRel(A,==,B) and is provided partly for
 *  historical reasons (we've have AssertEq() for ages) and partly because
 *  it's convenient to have a shorthand for the most common relation which
 *  we want to assert.
 */
#define AssertEq(A,B) AssertRel(A,==,B)

/// Assert two values differ by DBL_EPSILON or more.
#define AssertEqDouble(A,B) \
    do {\
	using Xapian::Internal::within_DBL_EPSILON;\
	if (rare(within_DBL_EPSILON(A,B))) {\
	    std::string xapian_assertion_msg(XAPIAN_ASSERT_LOCATION(within_DBL_EPSILON(A,B)));\
	    xapian_assertion_msg += " : values were ";\
	    xapian_assertion_msg += str(A);\
	    xapian_assertion_msg += " and ";\
	    xapian_assertion_msg += str(B);\
	    throw Xapian::AssertionError(xapian_assertion_msg);\
	}\
    } while (0)

#endif

// If assertions are disabled, set the macros to expand to (void)0 so that
// we get a compiler error in this case for assertions missing a trailing
// semicolon.  This avoids one source of compile errors in debug builds
// which don't manifest in non-debug builds.

#ifndef Assert
# define Assert(COND) (void)0
# define AssertRel(A,REL,B) (void)0
# define AssertEq(A,B) (void)0
# define AssertEqDouble(A,B) (void)0
#endif

#ifndef AssertParanoid
# define AssertParanoid(COND) (void)0
# define AssertRelParanoid(A,REL,B) (void)0
# define AssertEqParanoid(A,B) (void)0
# define AssertEqDoubleParanoid(A,B) (void)0
#endif

/** A "compile-time" assertion.
 *
 *  STATIC_ASSERT must be used inside a function (not at the top level, or
 *  in a class definition).
 *
 *  COND must be a compile-time constant expression.
 *
 *  If COND is false, the compiler will try to compile an array with negative
 *  length, which is invalid (we don't use 0 as the length, since GCC allows
 *  zero length arrays as an extension).  We name the array
 *  "xapian_static_assert_failed" since this won't clash with any sanely named
 *  variable, and it's likely to appear in the compiler error message and so
 *  indicate to the developer what went wrong.
 */
#define STATIC_ASSERT(COND) \
    do { \
	char xapian_static_assert_failed[(COND) ? 1 : -1]; \
	(void)xapian_static_assert_failed; \
    } while (0)

/** Assert at compile-time that type TYPE is unsigned. */
#define STATIC_ASSERT_UNSIGNED_TYPE(TYPE) \
    STATIC_ASSERT(static_cast<TYPE>(-1) > 0)

/** Assert at compile-time that integer type T1 can hold any value which
 *  integer type T2 can.
 *
 *  NB Doesn't currently work reliably for type bool (which is typically
 *  incorrectly treated as equivalent to unsigned char or unsigned int).
 *
 *  If T1 is unsigned, T2 must be unsigned and have no more bits.
 *
 *  If T1 is signed, then T2 must have no more bits (if also signed) or
 *  fewer bits (if unsigned).
 */
#define STATIC_ASSERT_TYPE_DOMINATES(T1, T2) \
    STATIC_ASSERT(static_cast<T1>(-1) > 0 ? \
	(static_cast<T2>(-1) > 0 && sizeof(T1) >= sizeof(T2)) : \
	(sizeof(T1) >= sizeof(T2) + (static_cast<T2>(-1) > 0)))

#endif // XAPIAN_INCLUDED_OMASSERT_H
