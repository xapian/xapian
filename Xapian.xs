#include <xapian.h>
#include <xapian/queryparser.h>
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


MODULE = Search::Xapian		PACKAGE = Search::Xapian

PROTOTYPES: ENABLE


INCLUDE: XS/Stem.xs
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
INCLUDE: XS/Stopper.xs
INCLUDE: XS/TermIterator.xs
INCLUDE: XS/PostingIterator.xs
INCLUDE: XS/PositionIterator.xs
INCLUDE: XS/WritableDatabase.xs


BOOT:
    { HV *mHvStash = gv_stashpv( "Search::Xapian", TRUE );

        newCONSTSUB( mHvStash, "OP_AND", newSViv(Query::OP_AND) );
        newCONSTSUB( mHvStash, "OP_OR", newSViv(Query::OP_OR) );
        newCONSTSUB( mHvStash, "OP_AND_NOT", newSViv(Query::OP_AND_NOT) );
        newCONSTSUB( mHvStash, "OP_XOR", newSViv(Query::OP_XOR) );
        newCONSTSUB( mHvStash, "OP_AND_MAYBE", newSViv(Query::OP_AND_MAYBE) );
        newCONSTSUB( mHvStash, "OP_FILTER", newSViv(Query::OP_FILTER) );
        newCONSTSUB( mHvStash, "OP_NEAR", newSViv(Query::OP_NEAR) );
        newCONSTSUB( mHvStash, "OP_PHRASE", newSViv(Query::OP_PHRASE) );
        newCONSTSUB( mHvStash, "OP_WEIGHT_CUTOFF", newSViv(Query::OP_WEIGHT_CUTOFF) );
        newCONSTSUB( mHvStash, "OP_ELITE_SET", newSViv(Query::OP_ELITE_SET) );

        newCONSTSUB( mHvStash, "DB_OPEN", newSViv(DB_OPEN) );
        newCONSTSUB( mHvStash, "DB_CREATE", newSViv(DB_CREATE) );
        newCONSTSUB( mHvStash, "DB_CREATE_OR_OPEN", newSViv(DB_CREATE_OR_OPEN) );
        newCONSTSUB( mHvStash, "DB_CREATE_OR_OVERWRITE", newSViv(DB_CREATE_OR_OVERWRITE) );
    }
