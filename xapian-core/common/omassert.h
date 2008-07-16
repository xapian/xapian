/** @file omassert.h
 * @brief Various assertion macros.
 */
/* Copyright (C) 2007 Olly Betts
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
#define CompileTimeAssert(COND)\
    do {\
	typedef int xapian_compile_time_check_[(COND) ? 1 : -1];\
    } while (0)

#ifndef XAPIAN_DEBUG
// The configure script should always define XAPIAN_DEBUG if it defines
// XAPIAN_DEBUG_PARANOID.
# ifdef XAPIAN_DEBUG_PARANOID
#  error XAPIAN_DEBUG_PARANOID defined without XAPIAN_DEBUG
# endif
#else

#include <xapian/error.h>

#include "utils.h" // For om_to_string() and within_DBL_EPSILON().

#define XAPIAN_ASSERT_LOCATION__(LINE,MSG) __FILE__":"#LINE": "#MSG
#define XAPIAN_ASSERT_LOCATION_(LINE,MSG) XAPIAN_ASSERT_LOCATION__(LINE,MSG)
#define XAPIAN_ASSERT_LOCATION(MSG) XAPIAN_ASSERT_LOCATION_(__LINE__,MSG)

// Expensive (or potentially expensive) assertions can be marked as "Paranoid"
// - these can be disabled separately from other assertions to allow a build
// with assertions which still has good performance.
#ifdef XAPIAN_DEBUG_PARANOID
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
	    xapian_assertion_msg += om_tostring(A);\
	    xapian_assertion_msg += " and ";\
	    xapian_assertion_msg += om_tostring(B);\
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
	    xapian_assertion_msg += om_tostring(A);\
	    xapian_assertion_msg += " and ";\
	    xapian_assertion_msg += om_tostring(B);\
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

#endif // XAPIAN_INCLUDED_OMASSERT_H
