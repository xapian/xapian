/* foo.h -- interface to the libfoo library

   Copyright (C) 1996-1999 Free Software Foundation, Inc.
   Written by Gord Matzigkeit, 1996

   This file is part of GNU Libtool.

GNU Libtool is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of
the License, or (at your option) any later version.

GNU Libtool is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with GNU Libtool; see the file COPYING.  If not, a copy
can be downloaded from  http://www.gnu.org/licenses/gpl.html,
or obtained by writing to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

/* Only include this header file once. */
#ifndef _FOO_H_
#define _FOO_H_ 1

/* At some point, cygwin will stop defining __CYGWIN32__, but b19 and
 * earlier do not define __CYGWIN__.  This snippit allows us to check
 * for __CYGWIN__ reliably for both current, old, and (probable) future
 * releases.
 */
#ifdef __CYGWIN32__
#  ifndef __CYGWIN__
#    define __CYGWIN__
#  endif
#endif

/* __BEGIN_DECLS should be used at the beginning of your declarations,
   so that C++ compilers don't mangle their names.  Use __END_DECLS at
   the end of C declarations. */
#undef __BEGIN_DECLS
#undef __END_DECLS
#ifdef __cplusplus
# define __BEGIN_DECLS extern "C" {
# define __END_DECLS }
#else
# define __BEGIN_DECLS /* empty */
# define __END_DECLS /* empty */
#endif

/* LTDL_PARAMS is a macro used to wrap function prototypes, so that compilers
   that don't understand ANSI C prototypes still work, and ANSI C
   compilers can issue warnings about type mismatches. */
#undef LTDL_PARAMS
#if defined (__STDC__) || defined (_AIX) || (defined (__mips) && defined (_SYSTYPE_SVR4)) || defined(__CYGWIN__) || defined(__cplusplus)
# define LT_PARAMS(protos) protos
# define lt_ptr_t     void*
#else
# define LT_PARAMS(protos) ()
# define lt_ptr_t     char*
#endif

/* Silly constants that the functions return. */
#define HELLO_RET 0xe110
#define FOO_RET 0xf00


/* Declarations.  Note the wonderful use of the above macros. */
__BEGIN_DECLS
int foo LT_PARAMS((void));
int hello LT_PARAMS((void));
extern int nothing;
__END_DECLS

#endif /* !_FOO_H_ */
