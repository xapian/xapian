#ifdef __cplusplus
extern "C" {
#endif
#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"
#ifdef __cplusplus
}
#endif

#include <om/om.h>
#include <string>

using namespace std;


MODULE = Search::Xapian		PACKAGE = Search::Xapian

PROTOTYPES: ENABLE


INCLUDE: XS/Database.xs
INCLUDE: XS/Document.xs
INCLUDE: XS/Enquire.xs
INCLUDE: XS/MSet.xs
INCLUDE: XS/MSetIterator.xs
INCLUDE: XS/Query.xs
INCLUDE: XS/Settings.xs
INCLUDE: XS/TermIterator.xs
INCLUDE: XS/WritableDatabase.xs


BOOT:
    { HV *mHvStash = gv_stashpv( "Search::Xapian", TRUE );
        newCONSTSUB( mHvStash, "OP_AND", newSViv(OmQuery::OP_AND) );
        newCONSTSUB( mHvStash, "OP_OR", newSViv(OmQuery::OP_OR) );
        newCONSTSUB( mHvStash, "OP_AND_NOT", newSViv(OmQuery::OP_AND_NOT) );
        newCONSTSUB( mHvStash, "OP_XOR", newSViv(OmQuery::OP_XOR) );
        newCONSTSUB( mHvStash, "OP_AND_MAYBE", newSViv(OmQuery::OP_AND_MAYBE) );
        newCONSTSUB( mHvStash, "OP_FILTER", newSViv(OmQuery::OP_FILTER) );
        newCONSTSUB( mHvStash, "OP_NEAR", newSViv(OmQuery::OP_NEAR) );
        newCONSTSUB( mHvStash, "OP_PHRASE", newSViv(OmQuery::OP_PHRASE) );
        newCONSTSUB( mHvStash, "OP_WEIGHT_CUTOFF", newSViv(OmQuery::OP_WEIGHT_CUTOFF) );
        newCONSTSUB( mHvStash, "OP_ELITE_SET", newSViv(OmQuery::OP_ELITE_SET) );
    }