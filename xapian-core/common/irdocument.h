/* irdocument.h : A document */

#ifndef _irdocument_h_
#define _irdocument_h_

#include "omtypes.h"

// A key in a document
class IRKey {
    public:
	unsigned int value;  // FIXME TEMPORARY
};

// A record in a document
class IRRec {
    public:
	unsigned int value;  // FIXME TEMPORARY
};

// A document in the database - holds keys and records
class IRDocument {
    private:
    public:
	virtual ~IRDocument() { return; }
	virtual IRKey get_key(keyno) const = 0;
	virtual IRRec get_rec(recno) const = 0;
};

#endif /* _irdocument_h_ */
