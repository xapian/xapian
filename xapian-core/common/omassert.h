/* omassert.h: Provide Assert() and similar functions
 *
 * ----START-LICENCE----
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002 Ananova Ltd
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 * USA
 * -----END-LICENCE-----
 */

// Note: we use macros to define our assertions, since with a template
// approach the location strings typically don't get thrown away by the
// compiler.

#ifndef OM_HGUARD_OMASSERT_H
#define OM_HGUARD_OMASSERT_H

#include <config.h>

// Include the definitions of the exceptions we're going to throw
#include "om/omerror.h"

// Include utility functions
#include "utils.h"

#include <float.h> // for DBL_EPSILON

// 2nd level of stringize definition not needed for the use we put this
// to in this file (since we always use it within a macro here) but
// is required in general  (#N doesn't work outside a macro definition)
#define STRINGIZE(N) _STRINGIZE(N)
#define _STRINGIZE(N) #N

// pull this out to improve readability and so we can change it for
// all cases easily
#define ASSERT_LOCN(a) __FILE__":"STRINGIZE(__LINE__)": "#a

#ifdef MUS_DEBUG_PARANOID
// Paranoid checks, typically too expensive to include in debug versions
// for use by developers but useful to turn on when debugging OM itself

// If we want the paranoid checks, want other checks too
#ifndef MUS_DEBUG
#define MUS_DEBUG
#endif /* !MUS_DEBUG */

// NB use an else clause to avoid dangling else damage
#define AssertParanoid(a) if (a) { } else throw OmAssertionError(ASSERT_LOCN(a))
#else /* MUS_DEBUG_PARANOID */
#define AssertParanoid(a)
#endif /* MUS_DEBUG_PARANOID */

#ifdef MUS_DEBUG
#include <math.h> // for fabs() for AssertEqDouble
// Assertions to put in debug builds
// NB use an else clause to avoid dangling else damage
#define Assert(a) if (a) { } else throw OmAssertionError(ASSERT_LOCN(a))
// Keep AssertEqDouble separate so we can use an epsilon test
#define AssertEqDouble(a,b) if (fabs((a) - (b)) < DBL_EPSILON) { } else throw OmAssertionError(ASSERT_LOCN(a)" - expected equal values: had " + om_tostring(a) + " and " + om_tostring(b))
#define AssertEq(a,b) if ((a) == (b)) { } else throw OmAssertionError(ASSERT_LOCN(a)" - expected equal values: had " + om_tostring(a) + " and " + om_tostring(b))
#define AssertNe(a,b) if ((a) != (b)) { } else throw OmAssertionError(ASSERT_LOCN(a)" - expected different values: had " + om_tostring(a))
#else
#define Assert(a)
#define AssertEqDouble(a,b)
#define AssertEq(a,b)
#define AssertNe(a,b)

// CompiletimeAssert(foo) takes a constant integral expression and causes
// a compile-time error if false.  It must be used at the top-level, not
// within a function definition.
#define CompiletimeAssert(EXPR) \
struct CompileTimeCheck ## __FILE__ ## __LINE__ {	\
    int member[(EXPR)?1:-1];				\
};
#endif

#endif /* OM_HGUARD_OMASSERT_H */
