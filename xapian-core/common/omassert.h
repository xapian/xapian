/* omassert.h : Assert templates */

#ifndef _omassert_h_
#define _omassert_h_

#include "error.h"

// 2nd level not need here (since we're using this in a macro) but
// is required in general
#define STRINGIZE(N) _STRINGIZE(N)
#define _STRINGIZE(N) #N

// pull this out to improve readability and so we can change it for
// all cases easily
#define ASSERT_LOCN(a) __FILE__":"STRINGIZE(__LINE__)": "#a

// use macros - with a template approach the where strings typically don't get
// thrown away by the compiler

#ifdef NDEBUG
#define AssertParanoid(a)
#define Assert(a)
#else
// NB use an else clause to avoid dangling else damage

// Paranoid checks, typically too expensive to include in debug versions
// for use by developers but useful to turn on when debugging OM itself
#define AssertParanoid(a) if (a) { } else throw AssertionFailed(ASSERT_LOCN(a))

// Assertions to put in debug builds
#define Assert(a) if (a) { } else throw AssertionFailed(ASSERT_LOCN(a))
#endif


#endif /* _omassert_h_ */
