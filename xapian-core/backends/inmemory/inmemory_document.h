/* inmemory_document.h: C++ class definition for accessing a inmemory document*/

#ifndef _inmemory_document_h_
#define _inmemory_document_h_

#include "irdocument.h"
#include <string>

class InMemoryDocument : public virtual IRDocument {
    friend class InMemoryDatabase;
    private:
	string doc;

	InMemoryDocument(const string &);

	// Stop copying
	InMemoryDocument(const InMemoryDocument &);
	InMemoryDocument & operator = (const InMemoryDocument &);
    public:
	IRKey get_key(keyno) const;
	IRData get_data() const;
};

#endif /* _inmemory_document_h_ */
