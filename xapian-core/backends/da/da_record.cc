/* da_record.cc: C++ class for reading DA records
 *
 * ----START-LICENCE----
 * Copyright 1999 Dialog Corporation
 * 
 * This program is free software; you can redistribute it and/or 
 * modify it under the terms of the GNU General Public License as 
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 * USA
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
