/* textfile_document.cc: C++ class for storing textfile documents */

#include "textfile_document.h"

TextfileDocument::TextfileDocument(const string &doc_new)
	: doc(doc_new)
{ return; }

IRKey
TextfileDocument::get_key(keyno id) const
{
    IRKey key;
    key.value = 0;
    return key;
}

IRData
TextfileDocument::get_data() const
{
    IRData data;
    data.value = doc;
    return data;
}
