%module xapian
%{
/* perl.i: SWIG interface file for the Perl bindings
 *
 * Copyright (C) 2009 Kosei Moriyama
 * Copyright (C) 2011,2012,2013 Olly Betts
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

/* The XS Search::Xapian never wrapped these, and they're now deprecated. */
#define XAPIAN_BINDINGS_SKIP_DEPRECATED_DB_FACTORIES

%include ../xapian-head.i

/* Rename function next() to increment() since the keyword "next" is already
 * used in Perl. */
%rename(increment) *::next();
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
%constant int OP_VALUE_RANGE = Xapian::Query::OP_VALUE_RANGE;
%constant int OP_VALUE_GE = Xapian::Query::OP_VALUE_GE;
%constant int OP_VALUE_LE = Xapian::Query::OP_VALUE_LE;
%constant int FLAG_BOOLEAN = Xapian::QueryParser::FLAG_BOOLEAN;
%constant int FLAG_PHRASE = Xapian::QueryParser::FLAG_PHRASE;
%constant int FLAG_LOVEHATE = Xapian::QueryParser::FLAG_LOVEHATE;
%constant int FLAG_BOOLEAN_ANY_CASE = Xapian::QueryParser::FLAG_BOOLEAN_ANY_CASE;
%constant int FLAG_WILDCARD = Xapian::QueryParser::FLAG_WILDCARD;
%constant int FLAG_PURE_NOT = Xapian::QueryParser::FLAG_PURE_NOT;
%constant int FLAG_PARTIAL = Xapian::QueryParser::FLAG_PARTIAL;
%constant int FLAG_SPELLING_CORRECTION = Xapian::QueryParser::FLAG_SPELLING_CORRECTION;
%constant int FLAG_SYNONYM = Xapian::QueryParser::FLAG_SYNONYM;
%constant int FLAG_AUTO_SYNONYMS = Xapian::QueryParser::FLAG_AUTO_SYNONYMS;
%constant int FLAG_AUTO_MULTIWORD_SYNONYMS = Xapian::QueryParser::FLAG_AUTO_MULTIWORD_SYNONYMS;
%constant int FLAG_DEFAULT = Xapian::QueryParser::FLAG_DEFAULT;
%constant int STEM_NONE = Xapian::QueryParser::STEM_NONE;
%constant int STEM_SOME = Xapian::QueryParser::STEM_SOME;
%constant int STEM_ALL = Xapian::QueryParser::STEM_ALL;
%constant int FLAG_SPELLING = Xapian::TermGenerator::FLAG_SPELLING;

/* Xapian::Enquire */
%feature("shadow") Xapian::Enquire::get_mset
%{
sub get_mset {
  my $self = $_[0];
  my $nargs = scalar(@_);
  if( $nargs == 4 ) {
    my $type = ref( $_[2] );
    if ( $type eq 'Search::Xapian::RSet' ) {
      # get_mset(first, max, rset)
      splice @_, 2, 0, (0); # insert checkatleast
    }
  }
  return Search::Xapianc::Enquire_get_mset( @_ );
}
%}

%feature("shadow") Xapian::Enquire::set_query
%{
sub set_query {
  my $self = shift;
  my $query = shift;
  if( ref( $query ) ne 'Search::Xapian::Query' ) {
    $query = Search::Xapian::Query->new( $query, @_ );
    Search::Xapianc::Enquire_set_query( $self, $query );
    return;
  }
  my $nargs = scalar(@_);
  if( $nargs > 1) {
    use Carp;
    Carp::carp( "USAGE: \$enquire->set_query(\$query) or \$enquire->set_query(\$query, \$length)" );
    exit;
  }
  Search::Xapianc::Enquire_set_query( $self, $query, @_ );
}
%}

%feature("shadow") Xapian::Enquire::set_sort_by_key
%{
sub set_sort_by_key {
    my $self = $_[0];
    my $sorter = $_[1];
    $self{_sorter} = $sorter;
    Search::Xapianc::Enquire_set_sort_by_key( @_ );
}
%}

%feature("shadow") Xapian::Enquire::set_sort_by_key_then_relevance
%{
sub set_sort_by_key_then_relevance {
    my $self = $_[0];
    my $sorter = $_[1];
    $self{_sorter} = $sorter;
    Search::Xapianc::Enquire_set_sort_by_key_then_relevance( @_ );
}
%}

%feature("shadow") Xapian::Enquire::set_sort_by_relevance_then_key
%{
sub set_sort_by_relevance_then_key {
    my $self = $_[0];
    my $sorter = $_[1];
    $self{_sorter} = $sorter;
    Search::Xapianc::Enquire_set_sort_by_relevance_then_key( @_ );
}
%}

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

bool equal(Xapian::ESetIterator * that) {
    return ((*self) == (*that));
}

bool nequal(Xapian::ESetIterator * that) {
    return ((*self) != (*that));
}
}

/* Xapian::MSet */
%extend Xapian::MSet {
Xapian::MSetIterator FETCH(int index) {
    return ((*self)[index]);
}
}

/* Xapian::MSetIterator */
%extend Xapian::MSetIterator {
bool equal(Xapian::MSetIterator * that) {
     return ((*self) == (*that));
}

bool nequal(Xapian::MSetIterator * that) {
     return ((*self) != (*that));
}
}

/* Xapian::PositionIterator */
%extend Xapian::PositionIterator {
bool equal1(Xapian::PositionIterator * that) {
     return ((*self) == (*that));
}

bool nequal1(Xapian::PositionIterator * that) {
     return ((*self) != (*that));
}
}

/* Xapian::PostingIterator */
%extend Xapian::PostingIterator {
bool equal(Xapian::PostingIterator * that) {
     return ((*self) == (*that));
}

bool nequal(Xapian::PostingIterator * that) {
     return ((*self) != (*that));
}
}

/* Xapian::Query */
%feature("shadow") Xapian::Query::Query
%{
sub new {
  my $class = shift;
  my $query;

  if( @_ <= 1 ) {
    $query = Search::Xapianc::new_Query(@_);
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
      $query = Search::Xapianc::new_Query( @_ );
    } elsif( $op == 9 ) { # FIXME: OP_SCALE_WEIGHT
      if( @_ != 3 ) {
        Carp::croak( "USAGE: $class->new(OP_SCALE_WEIGHT, QUERY, FACTOR)" );
      }
      $query = Search::Xapianc::new_Query( @_ );
    } elsif( $op == 11 || $op == 12 ) { # FIXME: OP_VALUE_GE, OP_VALUE_LE; eliminate hardcoded literals
      if( @_ != 3 ) {
        Carp::croak( "USAGE: $class->new(OP_VALUE_[GL]E, VALNO, LIMIT)" );
      }
      $query = Search::Xapianc::new_Query( @_ );
    } else {
      shift @_;
      $query = Search::Xapian::newN( $op, \@_ );
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

        if ( sv_isa(sv, "Search::Xapian::Query")) {
            Xapian::Query *q;
            SWIG_ConvertPtr(sv, (void **)&q, SWIGTYPE_p_Xapian__Query, 0);
            return *q;
        }

        if ( SvOK(sv) ) {
            STRLEN len;
            const char * ptr = SvPV(sv, len);
            return Xapian::Query(string(ptr, len));
        }

        croak( "USAGE: Search::Xapian::Query->new(OP, @TERMS_OR_QUERY_OBJECTS)" );
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
  my $qp = Search::Xapianc::new_QueryParser();

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
    Search::Xapianc::QueryParser_set_stopper( @_ );
}
%}

%feature("shadow") Xapian::QueryParser::add_valuerangeprocessor
%{
sub add_valuerangeprocessor {
    my ($self, $vrproc) = @_;
    push @{$self{_vrproc}}, $vrproc;
    Search::Xapianc::QueryParser_add_valuerangeprocessor( @_ );
}
%}

/* Xapian::SimpleStopper */
%feature("shadow") Xapian::SimpleStopper::SimpleStopper
%{
sub new {
    my $class = shift;
    my $stopper = Search::Xapianc::new_SimpleStopper();

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

%extend Xapian::TermIterator {
bool equal(Xapian::TermIterator * that) {
     return ((*self) == (*that));
}

bool nequal(Xapian::TermIterator * that) {
     return ((*self) != (*that));
}
}

/* Xapian::ValueIterator */
%extend Xapian::ValueIterator {
bool equal(Xapian::ValueIterator * that) {
     return ((*self) == (*that));
}

bool nequal(Xapian::ValueIterator * that) {
     return ((*self) != (*that));
}
}

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
    $self = Search::Xapianc::new3_WritableDatabase(@_);
  } else {
    $self = Search::Xapianc::new_WritableDatabase(@_);
  }
  bless $self, $pkg if defined($self);
}
%}

%inline %{
Xapian::WritableDatabase * new3_WritableDatabase() {
        try {
	    return new Xapian::WritableDatabase(Xapian::InMemory::open());
        }
        catch (const Xapian::Error &error) {
            croak( "Exception: %s", error.get_msg().c_str() );
        }
}
%}

%include util.i
%include except.i
%include ../xapian-headers.i
