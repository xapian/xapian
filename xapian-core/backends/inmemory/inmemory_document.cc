/* inmemory_document.cc: C++ class for storing inmemory documents
 *
 * ----START-LICENCE----
 * -----END-LICENCE-----
 */

#include "inmemory_document.h"

InMemoryDocument::InMemoryDocument(const string &doc_new)
	: doc(doc_new)
{ return; }

IRKey
InMemoryDocument::get_key(keyno id) const
{
    IRKey key;
    key.value = 0;
    return key;
}

IRData
InMemoryDocument::get_data() const
{
    IRData data;
    data.value = doc;
    return data;
}
