%{
/* xapian-headers.i: Getting SWIG to parse Xapian's C++ headers.
 *
 * Copyright 2006,2011 Olly Betts
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

/* A class which can usefully be subclassed in the target language. */
%define SUBCLASSABLE(NS, CLASS)
    %ignore NS::CLASS::clone;
    %ignore NS::CLASS::serialise;
    %ignore NS::CLASS::unserialise;
    %#ifdef XAPIAN_SWIG_DIRECTORS
    %feature(director) NS::CLASS;
    %#endif
%enddef

/* A class which is only useful to wrap if the target language allows
 * subclassing of wrapped classes (what SWIG calls "director support".
 */
#ifdef XAPIAN_SWIG_DIRECTORS
#define SUBCLASSABLE_ONLY(NS, CLASS) SUBCLASSABLE(NS, CLASS)
#else
#define SUBCLASSABLE_ONLY(NS, CLASS) %ignore NS::CLASS;
#endif

#ifdef SWIGTCL
/* Tcl needs copy constructors it seems. */
%define STANDARD_IGNORES(NS, CLASS)
    %ignore NS::CLASS::internal;
    %ignore NS::CLASS::CLASS(Internal*);
    %ignore NS::CLASS::operator=;
%enddef
#else
%define STANDARD_IGNORES(NS, CLASS)
    %ignore NS::CLASS::internal;
    %ignore NS::CLASS::CLASS(Internal*);
    %ignore NS::CLASS::operator=;
    %ignore NS::CLASS::CLASS(const CLASS &);
%enddef
#endif

/* We use %ignore and %extend rather than %rename on operator* so that any
 * pattern rename used to match local naming conventions applies to
 * DEREF_METHOD.
 */
%define INPUT_ITERATOR_METHODS(NS, CLASS, RET_TYPE, DEREF_METHOD)
    STANDARD_IGNORES(NS, CLASS)
    %ignore NS::CLASS::operator++;
    %ignore NS::CLASS::operator*;
    %extend NS::CLASS {
	bool equals(const NS::CLASS & o) const { return *self == o; }
	RET_TYPE DEREF_METHOD() const { return **self; }
	NEXT(RET_TYPE, NS::CLASS)
    }
%enddef

%define BIDIRECTIONAL_ITERATOR_METHODS(NS, CLASS, RET_TYPE, DEREF_METHOD)
    INPUT_ITERATOR_METHODS(NS, CLASS, RET_TYPE, DEREF_METHOD)
    %ignore NS::CLASS::operator--;
    %extend NS::CLASS {
	PREV(RET_TYPE, NS::CLASS)
    }
%enddef

/* Ignore these for all classes: */
%ignore operator==;
%ignore operator!=;
%ignore difference_type;
%ignore iterator_category;
%ignore value_type;
%ignore max_size;
%ignore swap;
%ignore iterator;
%ignore const_iterator;
%ignore size_type;

/* These methods won't throw exceptions. */
%exception Xapian::major_version "$action"
%exception Xapian::minor_version "$action"
%exception Xapian::revision "$action"
%exception Xapian::version_string "$action"
%include <xapian.h>

/* We don't wrap the version macros - they're useful for compile time checks
 * in C++ code, but for a scripting language, the version functions tell us
 * the version of Xapian we're actually using, which is more interesting than
 * the one the bindings were built against.
 */
/* %include <xapian/version.h> */

/* Types are needed by most of the other headers. */
%include <xapian/types.h>

/* The Error subclasses are handled separately for languages where we wrap
 * them. */
/* %include <xapian/error.h> */

/* ErrorHandler isn't currently wrapped. */
/* %include <xapian/errorhandler.h> */

/* Currently wrapped via declarations in xapian.i: */
/* %include <xapian/dbfactory.h> */

INPUT_ITERATOR_METHODS(Xapian, PositionIterator, Xapian::termpos, get_termpos)
%include <xapian/positioniterator.h>

%ignore Xapian::DocIDWrapper;
INPUT_ITERATOR_METHODS(Xapian, PostingIterator, Xapian::docid, get_docid)
%include <xapian/postingiterator.h>

INPUT_ITERATOR_METHODS(Xapian, TermIterator, std::string, get_term)
%include <xapian/termiterator.h>

/* FIXME: Not all languages fully ignore ValueIteratorEnd_ */
#undef ValueIteratorEnd_
%ignore ValueIterator(const ValueIteratorEnd_ &);
%ignore operator=(const ValueIteratorEnd_ &);
%ignore operator==(const ValueIterator &, const ValueIteratorEnd_ &);
%ignore operator==(const ValueIteratorEnd_ &, const ValueIterator &);
%ignore operator==(const ValueIteratorEnd_ &, const ValueIteratorEnd_ &);
%ignore operator!=(const ValueIterator &, const ValueIteratorEnd_ &);
%ignore operator!=(const ValueIteratorEnd_ &, const ValueIterator &);
%ignore operator!=(const ValueIteratorEnd_ &, const ValueIteratorEnd_ &);
%ignore Xapian::ValueIteratorEnd_;
INPUT_ITERATOR_METHODS(Xapian, ValueIterator, std::string, get_value)
%include <xapian/valueiterator.h>
#define ValueIteratorEnd_ ValueIterator

STANDARD_IGNORES(Xapian, Document)
%include <xapian/document.h>

STANDARD_IGNORES(Xapian, Query)
#if defined SWIGCSHARP || defined SWIGJAVA || defined SWIGPERL || \
    defined SWIGPYTHON || defined SWIGRUBY
// C#, Java, Perl, Python and Ruby wrap these "by hand" to give a nicer API
// than SWIG gives by default.
%ignore Xapian::Query::MatchAll;
%ignore Xapian::Query::MatchNothing;
#endif
%include <xapian/query.h>

// Suppress warning that Xapian::Internal::intrusive_base is unknown.
%warnfilter(SWIGWARN_TYPE_UNDEFINED_CLASS) Xapian::StemImplementation;
SUBCLASSABLE_ONLY(Xapian, StemImplementation)
#ifndef XAPIAN_SWIG_DIRECTORS
%ignore Xapian::Stem::Stem(Xapian::StemImplementation *);
#endif
STANDARD_IGNORES(Xapian, Stem)
%ignore Xapian::Stem::Stem();
%include <xapian/stem.h>

STANDARD_IGNORES(Xapian, TermGenerator)
%ignore Xapian::TermGenerator::operator=;
/* Ignore forms which use Utf8Iterator, as we don't wrap that class. */
%ignore Xapian::TermGenerator::index_text(const Xapian::Utf8Iterator &);
%ignore Xapian::TermGenerator::index_text(const Xapian::Utf8Iterator &, Xapian::termcount);
%ignore Xapian::TermGenerator::index_text(const Xapian::Utf8Iterator &, Xapian::termcount, const std::string &);
%ignore Xapian::TermGenerator::index_text_without_positions(const Xapian::Utf8Iterator &);
%ignore Xapian::TermGenerator::index_text_without_positions(const Xapian::Utf8Iterator &, Xapian::termcount);
%ignore Xapian::TermGenerator::index_text_without_positions(const Xapian::Utf8Iterator &, Xapian::termcount, const std::string &);
%ignore Xapian::TermGenerator::TermGenerator(const TermGenerator &);
%include <xapian/termgenerator.h>

STANDARD_IGNORES(Xapian, MSet)
#ifdef SWIGJAVA
// For compatibility with the original JNI wrappers.
%rename("getElement") Xapian::MSet::operator[];
#else
%ignore Xapian::MSet::operator[];
#endif
%extend Xapian::MSet {
    Xapian::docid get_docid(Xapian::doccount i) const {
	return *(*self)[i];
    }

    Xapian::Document get_document(Xapian::doccount i) const {
	return (*self)[i].get_document();
    }

    Xapian::MSetIterator get_hit(Xapian::doccount i) const {
	return (*self)[i];
    }

    Xapian::percent get_document_percentage(Xapian::doccount i) const {
	return self->convert_to_percent((*self)[i]);
    }
}
STANDARD_IGNORES(Xapian, ESet)
%ignore Xapian::ESet::operator[];
STANDARD_IGNORES(Xapian, RSet)

STANDARD_IGNORES(Xapian, Enquire)

BIDIRECTIONAL_ITERATOR_METHODS(Xapian, MSetIterator, Xapian::docid, get_docid)
BIDIRECTIONAL_ITERATOR_METHODS(Xapian, ESetIterator, std::string, get_term)

SUBCLASSABLE(Xapian, MatchDecider)

#ifdef XAPIAN_TERMITERATOR_PAIR_OUTPUT_TYPEMAP
/* Instantiating the template we're going to use avoids SWIG wrapping uses
 * of it in SwigValueWrapper.
 */
%template() std::pair<Xapian::TermIterator, Xapian::TermIterator>;

%extend Xapian::Enquire {
    /* This returns start and end iterators, then a typemap iterates between
     * those and returns an array of strings in the target language.
     */
    std::pair<Xapian::TermIterator, Xapian::TermIterator>
    get_matching_terms(const Xapian::MSetIterator & item) const {
	return std::make_pair($self->get_matching_terms_begin(item),
			      $self->get_matching_terms_end(item));
    }
}
#endif

/* We don't wrap ErrorHandler, so ignore the optional ErrorHandler parameter.
 */
%ignore Enquire(const Database &, ErrorHandler *);

%include <xapian/enquire.h>

SUBCLASSABLE(Xapian, ExpandDecider)
%ignore Xapian::ExpandDeciderAnd::ExpandDeciderAnd(const ExpandDecider *, const ExpandDecider *);
/* FIXME: %extend ExpandDeciderFilterTerms so it can be constructed from an
 * array of strings (or whatever the equivalent is in the target language).
 */
%ignore Xapian::ExpandDeciderFilterTerms;
%include <xapian/expanddecider.h>

SUBCLASSABLE(Xapian, KeyMaker)
%include <xapian/keymaker.h>

SUBCLASSABLE(Xapian, Stopper)
SUBCLASSABLE(Xapian, ValueRangeProcessor)
STANDARD_IGNORES(Xapian, QueryParser)
%ignore Xapian::QueryParser::QueryParser(const QueryParser &);
%include <xapian/queryparser.h>

%include <xapian/valuesetmatchdecider.h>

/* Xapian::Weight isn't usefully subclassable via the bindings, as clone()
 * needs to be implemented for it to be usable for weighting a search.  But
 * there are several supplied weighting schemes implemented in C++ which can
 * usefully be used via the bindings so we wrap those.
 */
STANDARD_IGNORES(Xapian, Weight)
/* The copy constructor isn't implemented, but is protected rather than
 * private to work around a compiler bug, so we ignore it explicitly.
 */
%ignore Xapian::Weight::Weight(const Weight &);
%ignore Xapian::Weight::clone;
%ignore Xapian::Weight::clone_;
%ignore Xapian::Weight::init_;
%ignore Xapian::Weight::serialise;
%ignore Xapian::Weight::unserialise;
%include <xapian/weight.h>

STANDARD_IGNORES(Xapian, Registry)
%include <xapian/registry.h>

/* We don't wrap Xapian's Unicode support as other languages usually already
 * have their own Unicode support. */
/* %include <xapian/unicode.h> */

SUBCLASSABLE(Xapian, Compactor)
%include <xapian/compactor.h>

SUBCLASSABLE(Xapian, PostingSource)
%ignore Xapian::PostingSource::register_matcher_;
%include <xapian/postingsource.h>

SUBCLASSABLE(Xapian, MatchSpy)
%ignore Xapian::MatchSpy::serialise_results;
%include <xapian/matchspy.h>

STANDARD_IGNORES(Xapian, Database)
STANDARD_IGNORES(Xapian, WritableDatabase)
%ignore Xapian::WritableDatabase::WritableDatabase(Database::Internal *);
%ignore Xapian::Database::get_document_lazily_;
%ignore Xapian::DB_CREATE;
%ignore Xapian::DB_CREATE_OR_OPEN;
%ignore Xapian::DB_CREATE_OR_OVERWRITE;
%ignore Xapian::DB_OPEN;
%include <xapian/database.h>

%constant int DB_CREATE = Xapian::DB_CREATE;
%constant int DB_CREATE_OR_OPEN = Xapian::DB_CREATE_OR_OPEN;
%constant int DB_CREATE_OR_OVERWRITE = Xapian::DB_CREATE_OR_OVERWRITE;
%constant int DB_OPEN = Xapian::DB_OPEN;
