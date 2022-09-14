/** @file
 * @brief Template used by configure to generate xapian/version.h
 *
 * (For portability, files run through $CXXCPP must have extension .c .cc or .C)
 */
#include <config.h>
const char * dummy[] = {
"/** @file",
" * @brief Define preprocessor symbols for the library version",
" */",
"// Copyright (C) 2002-2022 Olly Betts",
"//",
"// This program is free software; you can redistribute it and/or",
"// modify it under the terms of the GNU General Public License as",
"// published by the Free Software Foundation; either version 2 of the",
"// License, or (at your option) any later version.",
"//",
"// This program is distributed in the hope that it will be useful,",
"// but WITHOUT ANY WARRANTY; without even the implied warranty of",
"// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the",
"// GNU General Public License for more details.",
"//",
"// You should have received a copy of the GNU General Public License",
"// along with this program; if not, write to the Free Software",
"// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA",
"",
"#ifndef XAPIAN_INCLUDED_VERSION_H",
"#define XAPIAN_INCLUDED_VERSION_H",
"",
// Disabled for now, since str.h is used by omega, and includes visibility.h
// which includes version.h.  (FIXME)
//"#if !defined XAPIAN_INCLUDED_XAPIAN_H && !defined XAPIAN_LIB_BUILD",
//"# error @@Never use <xapian/version.h> directly; include <xapian.h> instead.@@",
//"#endif",
//"",
#ifdef __GNUC__
// We used to check __GXX_ABI_VERSION here which was helpful in the GCC 3 days,
// but ABI versions 2 and up are compatible aside from obscure corner cases,
// and GCC now defaults to using the latest ABI version it supports.  The
// result is that this check was no longer useful enough to justify the noise.
//
// We still check that the setting of _GLIBCXX_DEBUG matches since that
// introduces ABI-like incompatibilities.
//
// After preprocessing with "g++ -E" or similar (which will expand macros,
// strip comments such as this block, etc) we remove lines starting with a
// '#', remove blank lines, and collapse multiple spaces.  And we strip out
// double quotes, then replace '@@' with '"', and drop ',' at the end of a
// line (the purpose of the ',' is to prevent certain preprocessors from
// concatenating literal strings).
//
// So for lines we want in the output, we quote parts of the line which we
// don't want substituting, and use @@ where we really want " in the output.
"#ifdef __GNUC__",
"#if __GNUC__ < 3 || (__GNUC__ == 3 && __GNUC_MINOR__ == 0)",
"#error Xapian no longer supports GCC < 3.1",
"#else",
// _GLIBCXX_DEBUG is supported by GCC 3.4 and later so we only need to check
// it for those versions.
#if (__GNUC__ == 3 && __GNUC_MINOR__ >= 4) || __GNUC__ >= 4
#ifdef _GLIBCXX_DEBUG
"#ifndef _GLIBCXX_DEBUG",
"#error This library was compiled with _GLIBCXX_DEBUG defined, but you",
"#error have not specified this flag.  The settings must match or your",
"#error program will not work correctly.",
"#endif",
#else
"#ifdef _GLIBCXX_DEBUG",
"#error You are compiling with _GLIBCXX_DEBUG defined, but the library",
"#error was not compiled with this flag.  The settings must match or your",
"#error program will not work correctly.",
"#endif",
#endif
#endif
"#endif",
"#endif",
"",
#elif defined _MSC_VER
// When building the library with MSVC, generate preprocessor code to check
// that the same setting of _DEBUG is used for building applications as was
// used for building the library.
"#ifdef _MSC_VER",
#ifdef _DEBUG
"#ifndef _DEBUG",
"#error This library was compiled with _DEBUG defined, but you",
"#error have not specified this flag.  The settings must match or your",
"#error program will not work correctly.",
"#endif",
#else
"#ifdef _DEBUG",
"#error You are compiling with _DEBUG defined, but the library",
"#error was not compiled with this flag.  The settings must match or your",
"#error program will not work correctly.",
"#endif",
#endif
"#endif",
"",
#endif
#ifdef XAPIAN_ENABLE_VISIBILITY
"/// The library was compiled with GCC's -fvisibility=hidden option.",
"#define XAPIAN_ENABLE_VISIBILITY",
"",
#endif
"/// The version of Xapian as a C string literal.",
"#define XAPIAN_VERSION ", STRING_VERSION,
"",
"/** The major component of the Xapian version.",
" *  E.g. for Xapian 1.0.14 this would be: 1",
" */",
"#define XAPIAN_MAJOR_VERSION ", MAJOR_VERSION,
"",
"/** The minor component of the Xapian version.",
" *  E.g. for Xapian 1.0.14 this would be: 0",
" */",
"#define XAPIAN_MINOR_VERSION ", MINOR_VERSION,
"",
"/** The revision component of the Xapian version.",
" *  E.g. for Xapian 1.0.14 this would be: 14",
" */",
"#define XAPIAN_REVISION ", REVISION,
"",
"/// Base (signed) type for Xapian::docid and related types.",
"#define XAPIAN_DOCID_BASE_TYPE ", XAPIAN_DOCID_BASE_TYPE,
"",
"/// Base (signed) type for Xapian::termcount and related types.",
"#define XAPIAN_TERMCOUNT_BASE_TYPE ", XAPIAN_TERMCOUNT_BASE_TYPE,
"",
"/// Base (signed) type for Xapian::termpos.",
"#define XAPIAN_TERMPOS_BASE_TYPE ", XAPIAN_TERMPOS_BASE_TYPE,
"",
"/// Type for returning total document length.",
"#define XAPIAN_TOTALLENGTH_TYPE ", XAPIAN_REVISION_TYPE,
"",
"/// Underlying type for Xapian::rev.",
"#define XAPIAN_REVISION_TYPE ", XAPIAN_REVISION_TYPE,
"",
"/// XAPIAN_HAS_CHERT_BACKEND Defined if the chert backend is enabled.",
#ifdef XAPIAN_HAS_CHERT_BACKEND
"#define XAPIAN_HAS_CHERT_BACKEND 1",
#else
"/* #undef XAPIAN_HAS_CHERT_BACKEND */",
#endif
"",
"/// XAPIAN_HAS_GLASS_BACKEND Defined if the glass backend is enabled.",
#ifdef XAPIAN_HAS_GLASS_BACKEND
"#define XAPIAN_HAS_GLASS_BACKEND 1",
#else
"/* #undef XAPIAN_HAS_GLASS_BACKEND */",
#endif
"",
"/// XAPIAN_HAS_INMEMORY_BACKEND Defined if the inmemory backend is enabled.",
#ifdef XAPIAN_HAS_INMEMORY_BACKEND
"#define XAPIAN_HAS_INMEMORY_BACKEND 1",
#else
"/* #undef XAPIAN_HAS_INMEMORY_BACKEND */",
#endif
"",
"/// XAPIAN_HAS_REMOTE_BACKEND Defined if the remote backend is enabled.",
#ifdef XAPIAN_HAS_REMOTE_BACKEND
"#define XAPIAN_HAS_REMOTE_BACKEND 1",
#else
"/* #undef XAPIAN_HAS_REMOTE_BACKEND */",
#endif
"",
"/// XAPIAN_AT_LEAST(A,B,C) checks for xapian-core >= A.B.C - use like so:",
"///",
"/// @code",
"/// #if XAPIAN_AT_LEAST(1,4,2)",
"/// /* Code needing features needing Xapian >= 1.4.2. */",
"/// #endif",
"/// @endcode",
"///",
"/// Added in Xapian 1.4.2.",
"#define XAPIAN_AT_LEAST(A,B,C) \\",
"    (XAPIAN_MAJOR_VERSION > (A) || \\",
"     (XAPIAN_MAJOR_VERSION == (A) && \\",
"      (XAPIAN_MINOR_VERSION > (B) || \\",
"       (XAPIAN_MINOR_VERSION == (B) && XAPIAN_REVISION >= (C)))))",
"",
"/// We support move semantics when we're confident the compiler supports it.",
"///",
"/// C++11 move semantics are very useful in threaded code that wants to",
"/// hand-off Xapian objects to worker threads, but in this case it's very",
"/// unhelpful for availability of these semantics to vary by compiler as it",
"/// quietly leads to a build with non-threadsafe behaviour.",
"///",
"/// User code can #define XAPIAN_MOVE_SEMANTICS to force this on, and will",
"/// then get a compilation failure if the compiler lacks suitable support.",
"#ifndef XAPIAN_MOVE_SEMANTICS",
"# if __cplusplus >= 201103L || \\",
"     (defined _MSC_VER && _MSC_VER >= 1900) || \\",
"     defined XAPIAN_LIB_BUILD",
"#  define XAPIAN_MOVE_SEMANTICS",
"# endif",
"#endif",
"",
"#endif /* XAPIAN_INCLUDED_VERSION_H */"
};
