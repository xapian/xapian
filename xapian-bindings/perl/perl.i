%module xapian
%{
/* perl.i: SWIG interface file for the Perl bindings
 *
 * Copyright (C) 2009 Kosei Moriyama
 * Copyright (C) 2011,2012,2013,2015,2016,2019,2020 Olly Betts
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

%begin %{
// Older Perl headers contain things which cause warnings with more recent
// C++ compilers.  There's nothing we can really do about them, so just
// suppress them.
#ifdef __clang__
# pragma clang diagnostic push
# pragma clang diagnostic ignored "-Wreserved-user-defined-literal"
#elif defined __GNUC__
// Warning added in GCC 4.8 and we don't support anything older.
# pragma GCC diagnostic push
# pragma GCC diagnostic ignored "-Wliteral-suffix"
#endif

extern "C" {
#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"
}

#ifdef __clang__
# pragma clang diagnostic pop
#elif defined __GNUC__
# pragma GCC diagnostic pop
#endif
%}

/* The XS Xapian never wrapped these, and they're now deprecated. */
#define XAPIAN_BINDINGS_SKIP_DEPRECATED_DB_FACTORIES

%include ../xapian-head.i

/* "next" is a keyword in Perl. */
%rename(increment_weight) *::next(double min_wt);

/* Wrapping constant values. */
%constant int OP_AND = Xapian::Query::OP_AND;
%constant int OP_OR = Xapian::Query::OP_OR;
%constant int OP_AND_NOT = Xapian::Query::OP_AND_NOT;
%constant int OP_XOR = Xapian::Query::OP_XOR;
%constant int OP_AND_MAYBE = Xapian::Query::OP_AND_MAYBE;
%constant int OP_FILTER = Xapian::Query::OP_FILTER;
%constant int OP_NEAR = Xapian::Query::OP_NEAR;
%constant int OP_PHRASE = Xapian::Query::OP_PHRASE;
%constant int OP_VALUE_RANGE = Xapian::Query::OP_VALUE_RANGE;
%constant int OP_SCALE_WEIGHT = Xapian::Query::OP_SCALE_WEIGHT;
%constant int OP_ELITE_SET = Xapian::Query::OP_ELITE_SET;
%constant int OP_VALUE_GE = Xapian::Query::OP_VALUE_GE;
%constant int OP_SYNONYM = Xapian::Query::OP_SYNONYM;
%constant int OP_MAX = Xapian::Query::OP_MAX;
%constant int OP_WILDCARD = Xapian::Query::OP_WILDCARD;
%constant int OP_VALUE_LE = Xapian::Query::OP_VALUE_LE;
%constant int OP_INVALID = Xapian::Query::OP_INVALID;
%constant int ENQ_ASCENDING = Xapian::Enquire::ASCENDING;
%constant int ENQ_DESCENDING = Xapian::Enquire::DESCENDING;
%constant int ENQ_DONT_CARE = Xapian::Enquire::DONT_CARE;
%constant int FLAG_ACCUMULATE = Xapian::QueryParser::FLAG_ACCUMULATE;
%constant int FLAG_BOOLEAN = Xapian::QueryParser::FLAG_BOOLEAN;
%constant int FLAG_FUZZY = Xapian::QueryParser::FLAG_FUZZY;
%constant int FLAG_NO_POSITIONS = Xapian::QueryParser::FLAG_NO_POSITIONS;
%constant int FLAG_PHRASE = Xapian::QueryParser::FLAG_PHRASE;
%constant int FLAG_LOVEHATE = Xapian::QueryParser::FLAG_LOVEHATE;
%constant int FLAG_BOOLEAN_ANY_CASE = Xapian::QueryParser::FLAG_BOOLEAN_ANY_CASE;
%constant int FLAG_WILDCARD = Xapian::QueryParser::FLAG_WILDCARD;
%constant int FLAG_WILDCARD_GLOB = Xapian::QueryParser::FLAG_WILDCARD_GLOB;
%constant int FLAG_WILDCARD_MULTI = Xapian::QueryParser::FLAG_WILDCARD_MULTI;
%constant int FLAG_WILDCARD_SINGLE = Xapian::QueryParser::FLAG_WILDCARD_SINGLE;
%constant int FLAG_PURE_NOT = Xapian::QueryParser::FLAG_PURE_NOT;
%constant int FLAG_PARTIAL = Xapian::QueryParser::FLAG_PARTIAL;
%constant int FLAG_SPELLING_CORRECTION = Xapian::QueryParser::FLAG_SPELLING_CORRECTION;
%constant int FLAG_SYNONYM = Xapian::QueryParser::FLAG_SYNONYM;
%constant int FLAG_AUTO_SYNONYMS = Xapian::QueryParser::FLAG_AUTO_SYNONYMS;
%constant int FLAG_AUTO_MULTIWORD_SYNONYMS = Xapian::QueryParser::FLAG_AUTO_MULTIWORD_SYNONYMS;
%constant int FLAG_CJK_NGRAM = Xapian::QueryParser::FLAG_CJK_NGRAM;
%constant int FLAG_CJK_WORDS = Xapian::QueryParser::FLAG_CJK_WORDS;
%constant int FLAG_DEFAULT = Xapian::QueryParser::FLAG_DEFAULT;
%constant int STEM_NONE = Xapian::QueryParser::STEM_NONE;
%constant int STEM_SOME = Xapian::QueryParser::STEM_SOME;
%constant int STEM_SOME_FULL_POS = Xapian::QueryParser::STEM_SOME_FULL_POS;
%constant int STEM_ALL = Xapian::QueryParser::STEM_ALL;
%constant int STEM_ALL_Z = Xapian::QueryParser::STEM_ALL_Z;
%constant int FLAG_SPELLING = Xapian::TermGenerator::FLAG_SPELLING;
// FLAG_CJK_NGRAM already set above from QueryParser (values match).
%constant int WILDCARD_LIMIT_ERROR = Xapian::Query::WILDCARD_LIMIT_ERROR;
%constant int WILDCARD_LIMIT_FIRST = Xapian::Query::WILDCARD_LIMIT_FIRST;
%constant int WILDCARD_LIMIT_MOST_FREQUENT = Xapian::Query::WILDCARD_LIMIT_MOST_FREQUENT;

/* Xapian::Enquire */
%feature("shadow") Xapian::Enquire::get_mset
%{
sub get_mset {
  my $self = $_[0];
  my $nargs = scalar(@_);
  if( $nargs == 4 ) {
    my $type = ref( $_[2] );
    if ( $type eq 'Xapian::RSet' ) {
      # get_mset(first, max, rset)
      splice @_, 2, 0, (0); # insert checkatleast
    }
  }
  return Xapianc::Enquire_get_mset( @_ );
}
%}

%feature("shadow") Xapian::Enquire::set_query
%{
sub set_query {
  if (ref($_[1]) ne 'Xapian::Query') {
    push @_, Xapian::Query->new(splice @_, 1);
  }
  Xapianc::Enquire_set_query(@_);
}
%}

%feature("shadow") Xapian::Enquire::set_sort_by_key
%{
sub set_sort_by_key {
    my $self = $_[0];
    my $sorter = $_[1];
    $self{_sorter} = $sorter;
    Xapianc::Enquire_set_sort_by_key( @_ );
}
%}

%feature("shadow") Xapian::Enquire::set_sort_by_key_then_relevance
%{
sub set_sort_by_key_then_relevance {
    my $self = $_[0];
    my $sorter = $_[1];
    $self{_sorter} = $sorter;
    Xapianc::Enquire_set_sort_by_key_then_relevance( @_ );
}
%}

%feature("shadow") Xapian::Enquire::set_sort_by_relevance_then_key
%{
sub set_sort_by_relevance_then_key {
    my $self = $_[0];
    my $sorter = $_[1];
    $self{_sorter} = $sorter;
    Xapianc::Enquire_set_sort_by_relevance_then_key( @_ );
}
%}

/* Xapian::Enquire */
%extend Xapian::Enquire {
// For compatibility with Search::Xapian.
Xapian::MSet get_mset(Xapian::doccount first,
		      Xapian::doccount maxitems,
		      const Xapian::MatchDecider* mdecider) {
    return $self->get_mset(first, maxitems, 0, NULL, mdecider);
}
}

/* Xapian::ESet */
%extend Xapian::ESet {
Xapian::ESetIterator FETCH(int index) {
    return ((*self)[index]);
}
}

/* Xapian::ESetIterator */
%extend Xapian::ESetIterator {
std::string get_termname() {
    return self->operator*();
}
}

/* Xapian::MSet */
%extend Xapian::MSet {
Xapian::MSetIterator FETCH(int index) {
    return ((*self)[index]);
}
}

/* Xapian::Query */
%feature("shadow") Xapian::Query::Query
%{
sub new {
  my $class = shift;
  my $query;

  if( @_ <= 1 ) {
    $query = Xapianc::new_Query(@_);
  } else {
    use Carp;
    my $op = $_[0];
    if( $op !~ /^\d+$/ ) {
	Carp::croak( "USAGE: $class->new('term') or $class->new(OP, <args>)" );
    }
    if( $op == 8 ) { # FIXME: 8 is OP_VALUE_RANGE; eliminate hardcoded literal
      if( @_ != 4 ) {
	Carp::croak( "USAGE: $class->new(OP_VALUE_RANGE, VALNO, START, END)" );
      }
      $query = Xapianc::new_Query( @_ );
    } elsif( $op == 9 ) { # FIXME: OP_SCALE_WEIGHT
      if( @_ != 3 ) {
	Carp::croak( "USAGE: $class->new(OP_SCALE_WEIGHT, QUERY, FACTOR)" );
      }
      $query = Xapianc::new_Query( @_ );
    } elsif( $op == 11 || $op == 12 ) { # FIXME: OP_VALUE_GE, OP_VALUE_LE; eliminate hardcoded literals
      if( @_ != 3 ) {
	Carp::croak( "USAGE: $class->new(OP_VALUE_[GL]E, VALNO, LIMIT)" );
      }
      $query = Xapianc::new_Query( @_ );
    } else {
      shift @_;
      $query = Xapian::newN( $op, \@_ );
    }
  }
  return $query;
}
%}

%typemap(in) SV ** {
	AV *tempav;
	I32 len;
	int i;
	SV  **tv;
	if (!SvROK($input))
	    croak("Argument $argnum is not a reference.");
	if (SvTYPE(SvRV($input)) != SVt_PVAV)
	    croak("Argument $argnum is not an array.");
	tempav = (AV*)SvRV($input);
	len = av_len(tempav);
	$1 = (SV **) malloc((len+2)*sizeof(SV *));
	for (i = 0; i <= len; i++) {
	    tv = av_fetch(tempav, i, 0);
	    $1[i] = *tv;
	}
	$1[i] = NULL;
};

%typemap(freearg) SV ** {
	free($1);
}

%{
class XapianSWIGQueryItor {
    AV * array;

    int i;

  public:
    XapianSWIGQueryItor() { }

    void begin(AV * array_) {
	array = array_;
	i = 0;
    }

    void end(int n) {
	i = n;
    }

    XapianSWIGQueryItor & operator++() {
	++i;
	return *this;
    }

    Xapian::Query operator*() const {
	SV **svp = av_fetch(array, i, 0);
	if( svp == NULL )
	    croak("Unexpected NULL returned by av_fetch()");
	SV *sv = *svp;

	if (!SvOK(sv)) {
	    croak("USAGE: Xapian::Query->new(OP, @TERMS_OR_QUERY_OBJECTS)");
	}

	Xapian::Query *q;
	if (SWIG_ConvertPtr(sv, (void**)&q,
			    SWIGTYPE_p_Xapian__Query, 0) == SWIG_OK) {
	    return *q;
	}

	STRLEN len;
	const char * ptr = SvPV(sv, len);
	return Xapian::Query(string(ptr, len));
    }

    bool operator==(const XapianSWIGQueryItor & o) {
	return i == o.i;
    }

    bool operator!=(const XapianSWIGQueryItor & o) {
	return !(*this == o);
    }

    typedef std::input_iterator_tag iterator_category;
    typedef Xapian::Query value_type;
    typedef Xapian::termcount_diff difference_type;
    typedef Xapian::Query * pointer;
    typedef Xapian::Query & reference;
};

%}

%inline %{
Xapian::Query * newN(int op_, SV *q_) {
    Xapian::Query::op op = (Xapian::Query::op)op_;
    XapianSWIGQueryItor b, e;

    AV *q = (AV *) SvRV(q_);

    b.begin(q);
    e.end(av_len(q) + 1);

    try {
	return new Xapian::Query(op, b, e);
    } catch (const Xapian::Error &error) {
	croak( "Exception: %s", error.get_msg().c_str() );
    }
}
%}

/* Xapian::QueryParser */
%feature("shadow") Xapian::QueryParser::QueryParser
%{
sub new {
  my $class = shift;
  my $qp = Xapianc::new_QueryParser();

  bless $qp, $class;
  $qp->set_database(@_) if scalar(@_) == 1;

  return $qp;
}
%}

%feature("shadow") Xapian::QueryParser::set_stopper
%{
sub set_stopper {
    my ($self, $stopper) = @_;
    $self{_stopper} = $stopper;
    Xapianc::QueryParser_set_stopper( @_ );
}
%}

%feature("shadow") Xapian::QueryParser::add_rangeprocessor
%{
sub add_rangeprocessor {
    my ($self, $rproc) = @_;
    push @{$self{_rproc}}, $rproc;
    Xapianc::QueryParser_add_rangeprocessor( @_ );
}
%}

/* Xapian::SimpleStopper */
%feature("shadow") Xapian::SimpleStopper::SimpleStopper
%{
sub new {
    my $class = shift;
    my $stopper = Xapianc::new_SimpleStopper();

    bless $stopper, $class;
    foreach (@_) {
	$stopper->add($_);
    }

    return $stopper;
}
%}

%extend Xapian::SimpleStopper {
bool stop_word(std::string term) {
     return (*self)(term);
}
}

/* Xapian::Stem */
%extend Xapian::Stem {
std::string stem_word(std::string word) {
	    return (*self)(word);
}
}

/* Xapian::TermIterator */
%rename(get_termname) Xapian::TermIterator::get_term;

/* Xapian::WritableDatabase */
%rename(replace_document_by_term) \
	Xapian::WritableDatabase::replace_document(const std::string &,
						   const Xapian::Document &);
%rename(delete_document_by_term) \
	Xapian::WritableDatabase::delete_document(const std::string &);

%feature("shadow") Xapian::WritableDatabase::WritableDatabase
%{
sub new {
  my $pkg = shift;
  my $self;
  if( scalar(@_) == 0 ) {
    # For compatibility with Search::Xapian
    @_ = ('', $Xapianc::DB_BACKEND_INMEMORY);
  }
  $self = Xapianc::new_WritableDatabase(@_);
  bless $self, $pkg if defined($self);
}
%}

%define SUB_CLASS(NS, CLASS)
%{
class perl##CLASS : public NS::CLASS {
    SV* callback;

  public:
    perl##CLASS(SV* func) {
	callback = newSVsv(func);
    }

    ~perl##CLASS() {
	SvREFCNT_dec(callback);
    }

    bool operator()(const std::string &term) const {
	dSP;

	ENTER;
	SAVETMPS;

	PUSHMARK(SP);

	SV* arg = sv_newmortal();
	sv_setpvn(arg, term.data(), term.size());
	XPUSHs(arg);
	PUTBACK;

	int count = call_sv(callback, G_SCALAR);

	SPAGAIN;
	if (count != 1)
	    croak("callback function should return 1 value, got %d", count);

	bool result = POPi;

	PUTBACK;

	FREETMPS;
	LEAVE;

	return result;
    }
};
%}

%enddef

SUB_CLASS(Xapian, ExpandDecider)
SUB_CLASS(Xapian, Stopper)

%{
class perlMatchDecider : public Xapian::MatchDecider {
    SV* callback;

  public:
    perlMatchDecider(SV* func) {
	callback = newSVsv(func);
    }

    ~perlMatchDecider() {
	SvREFCNT_dec(callback);
    }

    bool operator()(const Xapian::Document &doc) const {
	dSP;

	ENTER;
	SAVETMPS;

	PUSHMARK(SP);

	XPUSHs(SWIG_NewPointerObj(const_cast<Xapian::Document*>(&doc),
				  SWIGTYPE_p_Xapian__Document, 0));
	PUTBACK;

	int count = call_sv(callback, G_SCALAR);

	SPAGAIN;
	if (count != 1)
	    croak("callback function should return 1 value, got %d", count);

	bool result = POPi;

	PUTBACK;

	FREETMPS;
	LEAVE;

	return result;
    }
};
%}

%{
class perlStemImplementation : public Xapian::StemImplementation {
    SV* callback;

  public:
    perlStemImplementation(SV* func) {
	callback = newSVsv(func);
    }

    ~perlStemImplementation() {
	SvREFCNT_dec(callback);
    }

    std::string operator()(const std::string& word) {
	dSP;

	ENTER;
	SAVETMPS;

	PUSHMARK(SP);

	SV* arg = sv_newmortal();
	sv_setpvn(arg, word.data(), word.size());
	XPUSHs(arg);
	PUTBACK;

	int count = call_sv(callback, G_SCALAR);

	SPAGAIN;
	if (count != 1)
	    croak("callback function should return 1 value, got %d", count);

	SV* sv = POPs;
	STRLEN len;
	const char* ptr = SvPV(sv, len);
	std::string result(ptr, len);

	PUTBACK;

	FREETMPS;
	LEAVE;

	return result;
    }

    std::string get_description() const {
	return "perlStemImplementation()";
    }
};
%}

%{
class perlKeyMaker : public Xapian::KeyMaker {
    SV* callback;

  public:
    perlKeyMaker(SV* func) {
	callback = newSVsv(func);
    }

    ~perlKeyMaker() {
	SvREFCNT_dec(callback);
    }

    std::string operator()(const Xapian::Document &doc) const {
	dSP;

	ENTER;
	SAVETMPS;

	PUSHMARK(SP);

	XPUSHs(SWIG_NewPointerObj(const_cast<Xapian::Document*>(&doc),
				  SWIGTYPE_p_Xapian__Document, 0));
	PUTBACK;

	int count = call_sv(callback, G_SCALAR);

	SPAGAIN;
	if (count != 1)
	    croak("callback function should return 1 value, got %d", count);

	SV* sv = POPs;
	STRLEN len;
	const char* ptr = SvPV(sv, len);
	std::string result(ptr, len);

	PUTBACK;

	FREETMPS;
	LEAVE;

	return result;
    }
};
%}

%{
class perlRangeProcessor : public Xapian::RangeProcessor {
    SV* callback;

  public:
    perlRangeProcessor(SV* func) {
	callback = newSVsv(func);
    }

    ~perlRangeProcessor() {
	SvREFCNT_dec(callback);
    }

    Xapian::Query operator()(const std::string& begin, const std::string& end) {
	dSP;

	ENTER;
	SAVETMPS;

	PUSHMARK(SP);
	EXTEND(SP, 2);
	SV* arg = sv_newmortal();
	sv_setpvn(arg, begin.data(), begin.size());
	PUSHs(arg);
	arg = sv_newmortal();
	sv_setpvn(arg, end.data(), end.size());
	PUSHs(arg);
	PUTBACK;

	int count = call_sv(callback, G_SCALAR);

	SPAGAIN;
	if (count != 1)
	    croak("callback function should return 1 value, got %d", count);

	// Allow the function to return a string or Query object.
	SV* sv = POPs;
	if (!SvOK(sv))
	    croak("function must return a string or Query object");

	Xapian::Query result;
	Xapian::Query* q;
	if (SWIG_ConvertPtr(sv, (void**)&q,
			    SWIGTYPE_p_Xapian__Query, 0) == SWIG_OK) {
	    result = *q;
	} else {
	    STRLEN len;
	    const char* ptr = SvPV(sv, len);
	    result = Xapian::Query(string(ptr, len));
	}

	PUTBACK;

	FREETMPS;
	LEAVE;

	return result;
    }
};
%}

%{
class perlFieldProcessor : public Xapian::FieldProcessor {
    SV* callback;

  public:
    perlFieldProcessor(SV* func) {
	callback = newSVsv(func);
    }

    ~perlFieldProcessor() {
	SvREFCNT_dec(callback);
    }

    Xapian::Query operator()(const std::string &str) {
	dSP;

	ENTER;
	SAVETMPS;

	PUSHMARK(SP);

	SV* arg = sv_newmortal();
	sv_setpvn(arg, str.data(), str.size());
	XPUSHs(arg);
	PUTBACK;

	int count = call_sv(callback, G_SCALAR);

	SPAGAIN;
	if (count != 1)
	    croak("callback function should return 1 value, got %d", count);

	// Allow the function to return a string or Query object.
	SV* sv = POPs;
	if (!SvOK(sv))
	    croak("function must return a string or Query object");

	Xapian::Query result;
	Xapian::Query* q;
	if (SWIG_ConvertPtr(sv, (void**)&q,
			    SWIGTYPE_p_Xapian__Query, 0) == SWIG_OK) {
	    result = *q;
	} else {
	    STRLEN len;
	    const char* ptr = SvPV(sv, len);
	    result = Xapian::Query(string(ptr, len));
	}

	PUTBACK;

	FREETMPS;
	LEAVE;

	return result;
    }
};
%}

%{
class perlMatchSpy : public Xapian::MatchSpy {
    SV* callback;

  public:
    perlMatchSpy(SV* func) {
	callback = newSVsv(func);
    }

    ~perlMatchSpy() {
	SvREFCNT_dec(callback);
    }

    void operator()(const Xapian::Document &doc, double wt) {
	dSP;

	ENTER;
	SAVETMPS;

	PUSHMARK(SP);
	EXTEND(SP, 2);
	PUSHs(SWIG_NewPointerObj(const_cast<Xapian::Document*>(&doc),
				 SWIGTYPE_p_Xapian__Document, 0));
	mPUSHn(wt);
	PUTBACK;

	(void)call_sv(callback, G_VOID);

	SPAGAIN;
	PUTBACK;

	FREETMPS;
	LEAVE;
    }
};
%}

%define SUB_CLASS_TYPEMAPS(NS, CLASS)

%typemap(typecheck, precedence=100) NS::CLASS * {
    SV* sv = $input;
    void* ptr;
    if (SWIG_ConvertPtr(sv, &ptr, $descriptor(NS::CLASS *), 0) == SWIG_OK) {
	(void)ptr;
	$1 = 1;
    } else {
	/* The docs in perlapi for call_sv say:
	 *
	 *    [T]he SV may be any of a CV, a GV, a reference to a CV, a
	 *    reference to a GV or "SvPV(sv)" will be used as the name of the
	 *    sub to call.
	 *
	 * To make overloading work helpfully, we don't allow passing the name
	 * of a sub.  Search::Xapian did in some cases, but it seems unlikely
	 * anyone relied on this.
	 */
	svtype t = SvTYPE(sv);
	if (t == SVt_RV) {
	    t = SvTYPE(SvRV(sv));
	}
	$1 = (t == SVt_PVCV || t == SVt_PVGV);
    }
}
%typemap(in) NS::CLASS * {
    SV* sv = $input;
    if (SWIG_ConvertPtr(sv, (void**)&$1,
			$descriptor(NS::CLASS *), 0) != SWIG_OK) {
	$1 = new perl##CLASS(sv);
    }
}

%enddef
SUB_CLASS_TYPEMAPS(Xapian, MatchDecider)
SUB_CLASS_TYPEMAPS(Xapian, ExpandDecider)
SUB_CLASS_TYPEMAPS(Xapian, Stopper)
SUB_CLASS_TYPEMAPS(Xapian, StemImplementation)
SUB_CLASS_TYPEMAPS(Xapian, KeyMaker)
SUB_CLASS_TYPEMAPS(Xapian, RangeProcessor)
SUB_CLASS_TYPEMAPS(Xapian, FieldProcessor)
SUB_CLASS_TYPEMAPS(Xapian, MatchSpy)

%include except.i
%include ../xapian-headers.i
%include extra.i
