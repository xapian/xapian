/* omassert.h : Assert templates */

#ifndef _omassert_h_
#define _omassert_h_

#include "error.h"

#ifdef NDEBUG
const bool CHECK_PARANOID = false;
const bool CHECK_ASSERT = false;
#else
const bool CHECK_PARANOID = true;
const bool CHECK_ASSERT = true;
#endif

template<class X, class A> inline void _Assert(A assertion, const char *where)
{
    if (!assertion) throw X(where);
}

#define STRINGIZE(N) _STRINGIZE(N)
#define _STRINGIZE(N) #N

// pull this out to improve readability and so we can change it for
// all cases easily
#define ASSERT_LOCN(a) __FILE__":"STRINGIZE(__LINE__)": "#a

// paranoid checks, typically too expensive to include in debug versions
#define AssertParanoid(a) \
 _Assert<AssertionFailed>(!CHECK_PARANOID || (a), ASSERT_LOCN(a))

// assertions to put in debug builds
#define Assert(a) \
 _Assert<AssertionFailed>(!CHECK_ASSERT || (a), ASSERT_LOCN(a))

#endif /* _omassert_h_ */
