/* da_record.cc: C++ class for reading DA records */

#include "da_database.h"
#include "da_record.h"
#include "daread.h"

DADocument::DADocument(struct record *rec_new) {
    rec = rec_new;
    printf("Record at %p, size %d\n", rec->p, rec->size);

    pos = rec->p;
}

DADocument::~DADocument() {
    printf("Deleting\n");
    loserecord(rec);
}
