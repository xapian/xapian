/* da_record.cc: C++ class for reading DA records
 *
 * ----START-LICENCE----
 * -----END-LICENCE-----
 */

#include "irdocument.h"
#include "da_record.h"
#include "daread.h"
#include "damuscat.h"

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
    unsigned char *pos = (unsigned char *)rec->p;
    unsigned int len = LOF(pos, 0);
    data.value = string((char *)pos + LWIDTH + 3, len - LWIDTH - 3);
    return data;
}
