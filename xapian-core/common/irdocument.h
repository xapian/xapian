/* irdocument.h : A document */

#ifndef _irdocument_h_
#define _irdocument_h_

#include "omtypes.h"

// A key in a document
class IRKey {
    public:
	unsigned int value;  // FIXME TEMPORARY
};

// The data in a document
class IRData {
    public:
	string value;
};

// A document in the database - holds keys and records
class IRDocument {
    private:
    public:
	virtual IRKey get_key(keyno) const = 0;
	virtual IRData get_data() const = 0;
};

#endif /* _irdocument_h_ */
