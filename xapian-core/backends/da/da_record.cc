/* da_record.cc: C++ class for reading DA records */

#include "irdocument.h"
#include "da_record.h"
#include "daread.h"

DADocument::DADocument(struct record *rec_new) {
    rec = rec_new;
}

DADocument::~DADocument() {
    loserecord(rec);
}

IRKey
DADocument::get_key(keyno id) const
{
    printf("Record at %p, size %d\n", rec->p, rec->size);
    IRKey key;
    key.value = 0;
    return key;
}

IRData
DADocument::get_data() const
{
    IRData data;
    data.value = string((char *)rec->p, rec->size);
    return data;
}
