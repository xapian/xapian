/* omtypes.h : Common types used
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

#ifndef OM_HGUARD_OMTYPES_H
#define OM_HGUARD_OMTYPES_H

typedef unsigned int om_termid;   // Type for term id's.
typedef unsigned int om_docid;    // Type for document id's.  Start at 1.
typedef unsigned int om_termpos;  // Type for term positions within documents.  Start at 1.

typedef om_docid om_doccount;     // Type for counts of documents
typedef om_termid om_termcount;   // Type for counts of terms (eg, wdf, termfreq)

typedef double om_doclength;      // Type for (normalised) lengths of documents
typedef unsigned long long om_totlength; // Type for sum of lengths of documents

typedef unsigned int om_keyno;    // Type for referring to key in document

typedef double om_weight;

#ifdef __cplusplus
#include <string>
typedef string om_termname;
typedef string om_docname;
#endif

#endif /* OM_HGUARD_OMTYPES_H */
