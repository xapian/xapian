/* omassert.h : Provide Assert() and similar functions
 *
 * ----START-LICENCE----
 * ----END-LICENCE----
 */

// Note: we use macros to define our assertions, since with a template
// approach the location strings typically don't get thrown away by the
// compiler.

#ifndef _omassert_h_
#define _omassert_h_

#include "config.h"

// Include the definitions of the exceptions we're going to throw
#include "omerror.h"

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
#define AssertParanoid(a) if (a) { } else throw AssertionFailed(ASSERT_LOCN(a))
#else /* MUS_DEBUG_PARANOID */
#define AssertParanoid(a)
#endif /* MUS_DEBUG_PARANOID */

#ifdef MUS_DEBUG
// Assertions to put in debug builds
// NB use an else clause to avoid dangling else damage
#define Assert(a) if (a) { } else throw AssertionFailed(ASSERT_LOCN(a))
#else
#define Assert(a)
#endif


#endif /* _omassert_h_ */
