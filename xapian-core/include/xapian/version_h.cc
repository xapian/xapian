// version_h.cc - template used by configure to generating xapian/version.h
// (for portability, files run through $CXXCPP must have extension .c .cc or .C)
#include <config.h>
const char * dummy = {
"// version.h: Define preprocesor symbols for the library version.",
"// If using GCC, also check the C++ ABI version is compatible with that used",
"// to build the library.",
"//",
"// Copyright (C) 2002,2004,2005,2006 Olly Betts",
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
#ifdef __GNUC__
// When building the library with GCC, generate preprocessor code to check that
// any version of GCC used to build applications has a matching C++ ABI. This
// means that users get a nice explanatory error message rather than a
// confusing link failure (or worse a program which builds but crashes).
// Another benefit is that the check happens near the start of compilation of
// the first source file which uses Xapian in the user's application, rather
// than during the first attempt to link with Xapian.
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
#define V2(A,B) J2(A,B)
#define J2(A,B) g++ A##.##B
#define V3(A,B,C) J3(A,B,C)
#define J3(A,B,C) g++ A##.##B##.##C
"",
"#ifdef __GNUC__",
#ifdef __GXX_ABI_VERSION
// GCC 3.1 reports ABI version 100 (same as 3.0), but this should actually have
// been 101!  So use hard-coded version number checks for 3.0 and 3.1.
#if __GNUC__ == 3 && __GNUC_MINOR__ == 0
"#if !(__GNUC__ == 3 && __GNUC_MINOR__ == 0)",
#elif __GNUC__ == 3 && __GNUC_MINOR__ == 1
"#if !(__GNUC__ == 3 && __GNUC_MINOR__ == 1)",
#else
"#if !defined(__GXX_ABI_VERSION) || __GXX_ABI_VERSION != ",__GXX_ABI_VERSION,
#endif
#elif __GNUC__ == 2 && __GNUC_MINOR__ == 96
"#if !(__GNUC__ == 2 && __GNUC_MINOR__ == 96)",
#else
"#if __GNUC__ > 2 || (__GNUC__ == 2 && __GNUC_MINOR__ >= 96)",
#endif
"#error The C++ ABI version of compiler you are using does not match",
"#error that of the compiler used to build the library. The versions",
"#error must match or your program will not work correctly.",
#ifdef __GNUC_PATCHLEVEL__
"#error The Xapian library was built with ",V3(__GNUC__,__GNUC_MINOR__,__GNUC_PATCHLEVEL__),
#else
"#error The Xapian library was built with ",V2(__GNUC__,__GNUC_MINOR__),
#endif
"#endif",
"#endif",
#endif
"",
"#include <xapian/deprecated.h>",
"",
"#define XAPIAN_VERSION ",STRING_VERSION,
"#define XAPIAN_MAJOR_VERSION ",MAJOR_VERSION,
"#define XAPIAN_MINOR_VERSION ",MINOR_VERSION,
"#define XAPIAN_REVISION ",REVISION,
"",
"namespace Xapian {",
"/** Report the version string of the library which the program is linked with.",
"  *",
"  * This may be different to the version compiled against (given by",
"  * XAPIAN_VERSION) if shared libraries are being used.",
"  */",
"const char * version_string();",
"",
"/** For compatibility with Xapian 0.9.5 and earlier.",
" *",
" *  @deprecated This function is now deprecated, use Xapian::version_string()",
" *  instead.",
" */",
"XAPIAN_DEPRECATED(const char * xapian_version_string());",
"",
"/** Report the major version of the library which the program is linked to.",
"  * This may be different to the version compiled against (given by",
"  * XAPIAN_MAJOR_VERSION) if shared libraries are being used.",
"  */",
"int major_version();",
"",
"/** For compatibility with Xapian 0.9.5 and earlier.",
" *",
" *  @deprecated This function is now deprecated, use Xapian::major_version()",
" *  instead.",
" */",
"XAPIAN_DEPRECATED(int xapian_major_version());",
"",
"/** Report the minor version of the library which the program is linked to.",
"  * This may be different to the version compiled against (given by",
"  * XAPIAN_MINOR_VERSION) if shared libraries are being used.",
"  */",
"int minor_version();",
"",
"/** For compatibility with Xapian 0.9.5 and earlier.",
" *",
" *  @deprecated This function is now deprecated, use Xapian::minor_version()",
" *  instead.",
" */",
"XAPIAN_DEPRECATED(int xapian_minor_version());",
"",
"/** Report the revision of the library which the program is linked to.",
"  * This may be different to the version compiled against (given by",
"  * XAPIAN_REVISION) if shared libraries are being used.",
"  */",
"int revision();",
"",
"/** For compatibility with Xapian 0.9.5 and earlier.",
" *",
" *  @deprecated This function is now deprecated, use Xapian::revision()",
" *  instead.",
" */",
"XAPIAN_DEPRECATED(int xapian_revision());",
"}",
"",
#ifdef XAPIAN_HAS_FLINT_BACKEND 
"#define XAPIAN_HAS_FLINT_BACKEND 1",
#else
"/* #undef XAPIAN_HAS_FLINT_BACKEND */",
#endif
#ifdef XAPIAN_HAS_QUARTZ_BACKEND 
"#define XAPIAN_HAS_QUARTZ_BACKEND 1",
#else
"/* #undef XAPIAN_HAS_QUARTZ_BACKEND */",
#endif
#ifdef XAPIAN_HAS_INMEMORY_BACKEND 
"#define XAPIAN_HAS_INMEMORY_BACKEND 1",
#else
"/* #undef XAPIAN_HAS_INMEMORY_BACKEND */",
#endif
#ifdef XAPIAN_HAS_MUSCAT36_BACKEND 
"#define XAPIAN_HAS_MUSCAT36_BACKEND 1",
#else
"/* #undef XAPIAN_HAS_MUSCAT36_BACKEND */",
#endif
#ifdef XAPIAN_HAS_REMOTE_BACKEND 
"#define XAPIAN_HAS_REMOTE_BACKEND 1",
#else
"/* #undef XAPIAN_HAS_REMOTE_BACKEND */",
#endif
"",
"#endif"
};
