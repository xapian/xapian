/* stem_italian.h: header for italian stemming algorithm.
 * 
 * ----START-LICENCE----
 * Copyright 1999,2000 Dialog Corporation
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

#ifndef _stem_italian_h_
#define _stem_italian_h_

/* Make header file work when included from C++ */
#ifdef __cplusplus
extern "C" {
#endif

struct italian_stemmer
{
    char * p;
    int p_size;
    int k;

    int j;
    int posV;
    int pos2;
    struct pool * irregulars;

};

extern struct italian_stemmer * setup_italian_stemmer();

extern char * italian_stem(struct italian_stemmer * z, char * q, int i0, int i1);

extern void closedown_italian_stemmer(struct italian_stemmer * z);


/* To set up the stemming process:

       struct italian_stemmer * z = setup_italian_stemmer();

   to use it:

       char * p = italian_stem(z, q, i0, i1);

   The word to be stemmed is in byte address q offsets i0 to i1
   inclusive (i.e. from q[i0] to q[i1]). The stemmed result is the
   C string at address p.

   To close down the stemming process:

   closedown_italian_stemmer(z);

*/

#ifdef __cplusplus
}
#endif

#endif /* _stem_italian_h_ */
