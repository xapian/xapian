/* da_record.cc: C++ class for reading DA records */

#include "irdocument.h"
#include "da_record.h"
#include "daread.h"

DADocument::DADocument(struct record *rec_new) {
    rec = rec_new;
    pos = rec->p;
}

DADocument::~DADocument() {
    loserecord(rec);
}

IRKey
DADocument::get_key(keyno id) const
{
    printf("Record at %p, size %d\n", rec->p, rec->size);
}

IRRec
DADocument::get_rec(recno id) const
{
}
