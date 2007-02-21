// Disable any deprecation warnings for Xapian methods/functions/classes.
 #define XAPIAN_DEPRECATED(D) D
#include <xapian.h>
#include <string>
#include <vector>

#ifdef __cplusplus
extern "C" {
#endif
#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"
#ifdef __cplusplus
}
#endif

using namespace std;
using namespace Xapian;

/* PerlStopper class
 *
 * Make operator() call Perl $OBJECT->stop_word
 */

class PerlStopper : public Stopper {
    public:
	PerlStopper(SV * obj) { SV_stopper_ref = newRV_inc(obj); }
	~PerlStopper() { sv_2mortal(SV_stopper_ref); }
	bool operator()(const string &term) {
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
    private:
	SV *callback;
    public:
	perlMatchDecider(SV *func) {
	    callback = newSVsv(func);
	}

	~perlMatchDecider() {
	    SvREFCNT_dec(callback);
	}

	int operator()(const Xapian::Document &doc) const {
	    dSP;
	    Document *pdoc;

	    ENTER;
	    SAVETMPS;

	    PUSHMARK(SP);

	    pdoc = new Document();
	    *pdoc = doc;

	    SV *arg;
	    arg = sv_newmortal();
	    sv_setref_pv(arg, "Search::Xapian::Document", (void *)pdoc);
	    XPUSHs(arg);

	    PUTBACK;

	    int count = call_sv(callback, G_SCALAR);

	    SPAGAIN;
	    if (count != 1)
		croak("callback function should return 1 value, got %d", count);

	    SV *decide_result = POPs;
	    int decide_actual_result = SvIV(decide_result);

	    PUTBACK;

	    FREETMPS;
	    LEAVE;

	    return decide_actual_result;
	}
};

MODULE = Search::Xapian		PACKAGE = Search::Xapian

PROTOTYPES: ENABLE


INCLUDE: XS/BM25Weight.xs
INCLUDE: XS/BoolWeight.xs
INCLUDE: XS/Database.xs
INCLUDE: XS/Document.xs
INCLUDE: XS/Enquire.xs
INCLUDE: XS/MSet.xs
INCLUDE: XS/MSetIterator.xs
INCLUDE: XS/ESet.xs
INCLUDE: XS/ESetIterator.xs
INCLUDE: XS/RSet.xs
INCLUDE: XS/Query.xs
INCLUDE: XS/QueryParser.xs
INCLUDE: XS/SimpleStopper.xs
INCLUDE: XS/Stem.xs
INCLUDE: XS/Stopper.xs
INCLUDE: XS/TermIterator.xs
INCLUDE: XS/TradWeight.xs
INCLUDE: XS/PostingIterator.xs
INCLUDE: XS/PositionIterator.xs
INCLUDE: XS/ValueIterator.xs
INCLUDE: XS/WritableDatabase.xs
INCLUDE: XS/Weight.xs


BOOT:
    {
	HV *mHvStash = gv_stashpv( "Search::Xapian", TRUE );

        newCONSTSUB( mHvStash, "OP_AND", newSViv(Query::OP_AND) );
        newCONSTSUB( mHvStash, "OP_OR", newSViv(Query::OP_OR) );
        newCONSTSUB( mHvStash, "OP_AND_NOT", newSViv(Query::OP_AND_NOT) );
        newCONSTSUB( mHvStash, "OP_XOR", newSViv(Query::OP_XOR) );
        newCONSTSUB( mHvStash, "OP_AND_MAYBE", newSViv(Query::OP_AND_MAYBE) );
        newCONSTSUB( mHvStash, "OP_FILTER", newSViv(Query::OP_FILTER) );
        newCONSTSUB( mHvStash, "OP_NEAR", newSViv(Query::OP_NEAR) );
        newCONSTSUB( mHvStash, "OP_PHRASE", newSViv(Query::OP_PHRASE) );
        newCONSTSUB( mHvStash, "OP_ELITE_SET", newSViv(Query::OP_ELITE_SET) );

        newCONSTSUB( mHvStash, "DB_OPEN", newSViv(DB_OPEN) );
        newCONSTSUB( mHvStash, "DB_CREATE", newSViv(DB_CREATE) );
        newCONSTSUB( mHvStash, "DB_CREATE_OR_OPEN", newSViv(DB_CREATE_OR_OPEN) );
        newCONSTSUB( mHvStash, "DB_CREATE_OR_OVERWRITE", newSViv(DB_CREATE_OR_OVERWRITE) );

        newCONSTSUB( mHvStash, "ENQ_DESCENDING", newSViv(Enquire::DESCENDING) );
        newCONSTSUB( mHvStash, "ENQ_ASCENDING", newSViv(Enquire::ASCENDING) );
        newCONSTSUB( mHvStash, "ENQ_DONT_CARE", newSViv(Enquire::DONT_CARE) );

        newCONSTSUB( mHvStash, "FLAG_BOOLEAN", newSViv(QueryParser::FLAG_BOOLEAN) );
        newCONSTSUB( mHvStash, "FLAG_PHRASE", newSViv(QueryParser::FLAG_PHRASE) );
        newCONSTSUB( mHvStash, "FLAG_LOVEHATE", newSViv(QueryParser::FLAG_LOVEHATE) );
	newCONSTSUB( mHvStash, "FLAG_BOOLEAN_ANY_CASE", newSViv(QueryParser::FLAG_BOOLEAN_ANY_CASE) );
	newCONSTSUB( mHvStash, "FLAG_WILDCARD", newSViv(QueryParser::FLAG_WILDCARD) );

        newCONSTSUB( mHvStash, "STEM_NONE", newSViv(QueryParser::STEM_NONE) );
        newCONSTSUB( mHvStash, "STEM_SOME", newSViv(QueryParser::STEM_SOME) );
        newCONSTSUB( mHvStash, "STEM_ALL", newSViv(QueryParser::STEM_ALL) );
    }
