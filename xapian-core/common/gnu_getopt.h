/** @file gnu_getopt.h
 * @brief Wrappers to allow GNU getopt to be used cleanly from C++ code.
 */
/* Copyright (C) 2004,2009,2010 Olly Betts
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301
 * USA
 */

#ifndef XAPIAN_INCLUDED_GNU_GETOPT_H
#define XAPIAN_INCLUDED_GNU_GETOPT_H

// We need to include a header to get __GLIBC__ defined.  Hopefully <cctype>
// is a safe bet.
#include <cctype>

#define GNU_GETOPT_INTERFACE_VERSION 2
#if defined __GLIBC__ && __GLIBC__ >= 2
# include <gnu-versions.h>
# if _GNU_GETOPT_INTERFACE_VERSION == GNU_GETOPT_INTERFACE_VERSION
#  define USE_GLIBC_GNUGETOPT
# endif
#endif

#ifdef USE_GLIBC_GNUGETOPT

#include <getopt.h>

inline int
gnu_getopt(int argc_, char *const *argv_, const char *shortopts_) {
    return getopt(argc_, argv_, shortopts_);
}

inline int
gnu_getopt_long(int argc_, char *const *argv_, const char *shortopts_,
		const struct option *longopts_, int *optind_) {
    return getopt_long(argc_, argv_, shortopts_, longopts_, optind_);
}

inline int
gnu_getopt_long_only(int argc_, char *const *argv_, const char *shortopts_,
		     const struct option *longopts_, int *optind_) {
    return getopt_long_only(argc_, argv_, shortopts_, longopts_, optind_);
}

#else

#ifdef __CYGWIN__
// Cygwin has __declspec(dllimport) magic on optarg, etc, so just pull in the
// header there rather than trying to duplicate that.
# include <getopt.h>
#else
extern "C" {
extern char *optarg;
extern int optind;
extern int opterr;
extern int optopt;
}

struct option {
    const char *name;
    int has_arg;
    int * flag;
    int val;
};

# define no_argument		0
# define required_argument	1
# define optional_argument	2
#endif

// For internal use only.
int
gnu_getopt_internal_(int, char *const *, const char *, const struct option *,
		     int *, int);

inline int
gnu_getopt(int argc_, char *const *argv_, const char *shortopts_) {
    return gnu_getopt_internal_(argc_, argv_, shortopts_,
			    reinterpret_cast<const struct option *>(0),
			    reinterpret_cast<int *>(0), 0);
}

inline int
gnu_getopt_long(int argc_, char *const *argv_, const char *shortopts_,
		const struct option *longopts_, int *optind_) {
    return gnu_getopt_internal_(argc_, argv_, shortopts_, longopts_, optind_, 0);
}

inline int
gnu_getopt_long_only(int argc_, char *const *argv_, const char *shortopts_,
		     const struct option *longopts_, int *optind_) {
    return gnu_getopt_internal_(argc_, argv_, shortopts_, longopts_, optind_, 1);
}
#endif

#endif
