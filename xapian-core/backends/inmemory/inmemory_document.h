/* textfile_document.h: C++ class definition for accessing a textfile document*/

#ifndef _textfile_document_h_
#define _textfile_document_h_

#include "irdocument.h"
#include <string>

class TextfileDocument : public virtual IRDocument {
    friend class TextfileDatabase;
    private:
	string doc;

	TextfileDocument(const string &);

	// Stop copying
	TextfileDocument(const TextfileDocument &);
	TextfileDocument & operator = (const TextfileDocument &);
    public:
	IRKey get_key(keyno) const;
	IRData get_data() const;
};

#endif /* _textfile_document_h_ */
