// Disable any deprecation warnings for Xapian methods/functions/classes.
#define XAPIAN_DEPRECATED(D) D
#include <xapian.h>
#include <string>
#include <vector>

// Stop Perl headers from even thinking of doing '#define bool char' or
// '#define bool int', which they would do with compilers other than GCC.
#define HAS_BOOL

extern "C" {
#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"
}

/* Perl's embed.h defines get_context, but that mangles
 * Xapian::Error::get_context(). */
#ifdef get_context
# undef get_context
#endif

using namespace std;
using namespace Xapian;

// For some classes, we want extra slots to keep references to set objects in.

struct Enquire_perl {
    Enquire real_obj;
    SV * sorter;

    Enquire_perl(const Xapian::Database & db) : real_obj(db), sorter(NULL) { }

    void ref_sorter(SV * sv) {
	SvREFCNT_inc(sv);
	swap(sv, sorter);
	SvREFCNT_dec(sv);
    }

    ~Enquire_perl() {
	SvREFCNT_dec(sorter);
	sorter = NULL;
    }
};

struct QueryParser_perl {
    QueryParser real_obj;
    SV * stopper;
    vector<SV *> vrps;

    QueryParser_perl() : real_obj(), stopper(NULL) { }

    void ref_stopper(SV * sv) {
	SvREFCNT_inc(sv);
	swap(sv, stopper);
	SvREFCNT_dec(sv);
    }

    void ref_vrp(SV * sv) {
	SvREFCNT_inc(sv);
	vrps.push_back(sv);
    }

    ~QueryParser_perl() {
	SvREFCNT_dec(stopper);
	stopper = NULL;
	vector<SV *>::const_iterator i;
	for (i = vrps.begin(); i != vrps.end(); ++i) {
	    SvREFCNT_dec(*i);
	}
	vrps.clear();
    }
};

struct TermGenerator_perl {
    TermGenerator real_obj;
    SV * stopper;

    TermGenerator_perl() : real_obj(), stopper(NULL) { }

    void ref_stopper(SV * sv) {
	SvREFCNT_inc(sv);
	swap(sv, stopper);
	SvREFCNT_dec(sv);
    }

    ~TermGenerator_perl() {
	SvREFCNT_dec(stopper);
	stopper = NULL;
    }
};

#define XAPIAN_PERL_NEW(CLASS, PARAMS) (&((new CLASS##_perl PARAMS)->real_obj))
#define XAPIAN_PERL_CAST(CLASS, OBJ) ((CLASS##_perl*)(void*)(OBJ))
#define XAPIAN_PERL_REF(CLASS, OBJ, MEMB, SV) XAPIAN_PERL_CAST(CLASS, OBJ)->ref_##MEMB(SV)
#define XAPIAN_PERL_DESTROY(CLASS, OBJ) delete XAPIAN_PERL_CAST(CLASS, OBJ)

extern void handle_exception(void);

/* PerlStopper class
 *
 * Make operator() call Perl $OBJECT->stop_word
 */

class PerlStopper : public Stopper {
    public:
	PerlStopper(SV * obj) { SV_stopper_ref = newRV_inc(obj); }
	~PerlStopper() { sv_2mortal(SV_stopper_ref); }
	bool operator()(const string &term) const {
	    dSP ;

	    ENTER ;
	    SAVETMPS ;

	    PUSHMARK(SP);
	    PUSHs(SvRV(SV_stopper_ref));
	    PUSHs(sv_2mortal(newSVpv(term.data(), term.size())));
	    PUTBACK ;

	    int count = call_method("stop_word", G_SCALAR);

	    SPAGAIN ;

	    if (count != 1)
		croak("callback function should return 1 value, got %d", count);

	    // Breaks with SvTRUE(POPs) ?!?!?!
	    bool r = SvTRUE(SP[0]);
	    POPs ;

	    PUTBACK ;
	    FREETMPS ;
	    LEAVE ;

	    return r;
	}

    private:
	SV * SV_stopper_ref;
};

class perlMatchDecider : public Xapian::MatchDecider {
    SV *callback;

  public:
    perlMatchDecider(SV *func) {
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

	SV *arg = sv_newmortal();

	Document *pdoc = new Document(doc);
	sv_setref_pv(arg, "Search::Xapian::Document", (void *)pdoc);
	XPUSHs(arg);

	PUTBACK;

	int count = call_sv(callback, G_SCALAR);

	SPAGAIN;
	if (count != 1)
	    croak("callback function should return 1 value, got %d", count);

	int decide_actual_result = POPi;

	PUTBACK;

	FREETMPS;
	LEAVE;

	return decide_actual_result;
    }
};

class perlExpandDecider : public Xapian::ExpandDecider {
    SV *callback;

  public:
    perlExpandDecider(SV *func) {
	callback = newSVsv(func);
    }

    ~perlExpandDecider() {
	SvREFCNT_dec(callback);
    }

    bool operator()(const string &term) const {
	dSP;

	ENTER;
	SAVETMPS;

	PUSHMARK(SP);

	XPUSHs(sv_2mortal(newSVpv(term.data(), term.size())));

	PUTBACK;

	int count = call_sv(callback, G_SCALAR);

	SPAGAIN;
	if (count != 1)
	    croak("callback function should return 1 value, got %d", count);

	int decide_actual_result = POPi;

	PUTBACK;

	FREETMPS;
	LEAVE;

	return decide_actual_result;
    }
};

MODULE = Search::Xapian		PACKAGE = Search::Xapian

PROTOTYPES: ENABLE

string
sortable_serialise(double value)

double
sortable_unserialise(string value)

const char *
version_string()

int
major_version()

int
minor_version()

int
revision()

INCLUDE: XS/BM25Weight.xs
INCLUDE: XS/BoolWeight.xs
INCLUDE: XS/Database.xs
INCLUDE: XS/Document.xs
INCLUDE: XS/Enquire.xs
INCLUDE: XS/MSet.xs
INCLUDE: XS/MSetIterator.xs
INCLUDE: XS/ESet.xs
INCLUDE: XS/Error.xs
INCLUDE: XS/ESetIterator.xs
INCLUDE: XS/RSet.xs
INCLUDE: XS/MultiValueSorter.xs
INCLUDE: XS/Query.xs
INCLUDE: XS/QueryParser.xs
INCLUDE: XS/SimpleStopper.xs
INCLUDE: XS/Stem.xs
INCLUDE: XS/Stopper.xs
INCLUDE: XS/TermGenerator.xs
INCLUDE: XS/TermIterator.xs
INCLUDE: XS/TradWeight.xs
INCLUDE: XS/PostingIterator.xs
INCLUDE: XS/PositionIterator.xs
INCLUDE: XS/ValueIterator.xs
INCLUDE: XS/WritableDatabase.xs
INCLUDE: XS/Weight.xs

INCLUDE: XS/DateValueRangeProcessor.xs
INCLUDE: XS/NumberValueRangeProcessor.xs
INCLUDE: XS/StringValueRangeProcessor.xs

BOOT:
    {
	HV *mHvStash = gv_stashpv( "Search::Xapian", TRUE );
#define ENUM_CONST(P, C) newCONSTSUB( mHvStash, (char*)#P, newSViv(C) )

	ENUM_CONST(OP_AND, Query::OP_AND);
	ENUM_CONST(OP_OR, Query::OP_OR);
	ENUM_CONST(OP_AND_NOT, Query::OP_AND_NOT);
	ENUM_CONST(OP_XOR, Query::OP_XOR);
	ENUM_CONST(OP_AND_MAYBE, Query::OP_AND_MAYBE);
	ENUM_CONST(OP_FILTER, Query::OP_FILTER);
	ENUM_CONST(OP_NEAR, Query::OP_NEAR);
	ENUM_CONST(OP_PHRASE, Query::OP_PHRASE);
	ENUM_CONST(OP_VALUE_RANGE, Query::OP_VALUE_RANGE);
	ENUM_CONST(OP_SCALE_WEIGHT, Query::OP_SCALE_WEIGHT);
	ENUM_CONST(OP_ELITE_SET, Query::OP_ELITE_SET);
	ENUM_CONST(OP_VALUE_GE, Query::OP_VALUE_GE);
	ENUM_CONST(OP_VALUE_LE, Query::OP_VALUE_LE);

	ENUM_CONST(DB_OPEN, DB_OPEN);
	ENUM_CONST(DB_CREATE, DB_CREATE);
	ENUM_CONST(DB_CREATE_OR_OPEN, DB_CREATE_OR_OPEN);
	ENUM_CONST(DB_CREATE_OR_OVERWRITE, DB_CREATE_OR_OVERWRITE);

	ENUM_CONST(ENQ_DESCENDING, Enquire::DESCENDING);
	ENUM_CONST(ENQ_ASCENDING, Enquire::ASCENDING);
	ENUM_CONST(ENQ_DONT_CARE, Enquire::DONT_CARE);

	ENUM_CONST(FLAG_BOOLEAN, QueryParser::FLAG_BOOLEAN);
	ENUM_CONST(FLAG_PHRASE, QueryParser::FLAG_PHRASE);
	ENUM_CONST(FLAG_LOVEHATE, QueryParser::FLAG_LOVEHATE);
	ENUM_CONST(FLAG_BOOLEAN_ANY_CASE, QueryParser::FLAG_BOOLEAN_ANY_CASE);
	ENUM_CONST(FLAG_WILDCARD, QueryParser::FLAG_WILDCARD);
	ENUM_CONST(FLAG_PURE_NOT, QueryParser::FLAG_PURE_NOT);
	ENUM_CONST(FLAG_PARTIAL, QueryParser::FLAG_PARTIAL);
	ENUM_CONST(FLAG_SPELLING_CORRECTION, QueryParser::FLAG_SPELLING_CORRECTION);
	ENUM_CONST(FLAG_SYNONYM, QueryParser::FLAG_SYNONYM);
	ENUM_CONST(FLAG_AUTO_SYNONYMS, QueryParser::FLAG_AUTO_SYNONYMS);
	ENUM_CONST(FLAG_AUTO_MULTIWORD_SYNONYMS, QueryParser::FLAG_AUTO_SYNONYMS);
	ENUM_CONST(FLAG_DEFAULT, QueryParser::FLAG_DEFAULT);

	ENUM_CONST(STEM_NONE, QueryParser::STEM_NONE);
	ENUM_CONST(STEM_SOME, QueryParser::STEM_SOME);
	ENUM_CONST(STEM_ALL, QueryParser::STEM_ALL);

	ENUM_CONST(FLAG_SPELLING, TermGenerator::FLAG_SPELLING);

	ENUM_CONST(BAD_VALUENO, BAD_VALUENO);
    }
