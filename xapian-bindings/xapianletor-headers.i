%{
/* xapianletor-headers.i: Getting SWIG to parse Xapian's C++ headers.
 *
 * Copyright 2004,2006,2011,2012,2013,2014,2015,2016,2017,2019 Olly Betts
 * Copyright 2014 Assem Chelli
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
%}

/* Ignore anything ending in an underscore, which is for internal use only: */
%rename("$ignore", regexmatch$name="_$") "";

/* For other languages, SWIG already renames operator() suitably. */
#if defined SWIGJAVA || defined SWIGPHP || defined SWIGTCL
%rename(apply) *::operator();
#elif defined SWIGCSHARP
%rename(Apply) *::operator();
#endif

/* Ignore these for all classes: */
%ignore operator==;
%ignore operator!=;
%ignore operator<;
%ignore operator>;
%ignore operator<=;
%ignore operator>=;
%ignore operator+;
%ignore difference_type;
%ignore iterator_category;
%ignore value_type;
%ignore max_size;
%ignore swap;
%ignore iterator;
%ignore const_iterator;
%ignore size_type;
%ignore unserialise(const char **, const char *);
%ignore release();

%include <xapian-letor.h>

/* Types are needed by most of the other headers. */
%include <xapian/types.h>

%include <xapian/visibility.h>

/* The Error subclasses are handled separately for languages where we wrap
 * them. */
/* %include <xapian-letor/letor_error.h> */

SUBCLASSABLE(Xapian, Feature)
STANDARD_IGNORES(Xapian, Feature)
%include <xapian-letor/feature.h>

STANDARD_IGNORES(Xapian, FeatureList)
%include <xapian-letor/featurelist.h>

STANDARD_IGNORES(Xapian, FeatureVector)
%include <xapian-letor/featurevector.h>

SUBCLASSABLE(Xapian, Ranker)
STANDARD_IGNORES(Xapian, Ranker)
// Suppress warning that Xapian::Internal::opt_intrusive_base is unknown.
%warnfilter(SWIGWARN_TYPE_UNDEFINED_CLASS) Xapian::Ranker;
%include <xapian-letor/ranker.h>

SUBCLASSABLE(Xapian, Scorer)
STANDARD_IGNORES(Xapian, Scorer)
// Suppress warning that Xapian::Internal::opt_intrusive_base is unknown.
%warnfilter(SWIGWARN_TYPE_UNDEFINED_CLASS) Xapian::Scorer;
%include <xapian-letor/scorer.h>
