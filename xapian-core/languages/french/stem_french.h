/* stem_french.h: header for french stemming algorithm.
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
/* Version 2: see http://open.muscat.com/ for further information */

#ifndef OM_HGUARD_STEM_FRENCH_H
#define OM_HGUARD_STEM_FRENCH_H

/* Make header file work when included from C++ */
#ifdef __cplusplus
extern "C" {
#endif

struct french_stemmer;

extern struct french_stemmer * setup_french_stemmer();

extern const char * french_stem(struct french_stemmer * z, const char * q, int i0, int i1);

extern void closedown_french_stemmer(struct french_stemmer * z);


/* To set up the stemming process:

       struct french_stemmer * z = setup_french_stemmer();

   to use it:

       const char * p = french_stem(z, q, i0, i1);

   The word to be stemmed is in byte address q offsets i0 to i1
   inclusive (i.e. from q[i0] to q[i1]). The stemmed result is the
   C string at address p.

   To close down the stemming process:

   closedown_french_stemmer(z);

*/

#ifdef __cplusplus
}
#endif

#endif /* OM_HGUARD_STEM_FRENCH_H */
