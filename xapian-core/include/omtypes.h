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

#ifndef _omtypes_h_
#define _omtypes_h_

typedef unsigned int termid;   // Type for term id's
typedef unsigned int docid;    // Type for document id's
typedef unsigned int termpos;  // Type for term positions within documents

typedef docid doccount;        // Type for counts of documents
typedef termid termcount;      // Type for counts of terms (eg, wdf, termfreq)

typedef double doclength;      // Type for (normalised) lengths of documents
typedef unsigned long long totlength; // Type for sum of lengths of documents

typedef unsigned int keyno;    // Type for referring to key in document

typedef double weight;

/* Type of a database */
enum _om_database_type {
    OM_DBTYPE_NULL,
    OM_DBTYPE_DA,
    OM_DBTYPE_INMEMORY,
    OM_DBTYPE_SLEEPY,
    OM_DBTYPE_MULTI
};

typedef enum _om_database_type om_database_type;

#ifdef __cplusplus
#include <string>
typedef string termname;
typedef string docname;
#endif

#endif /* _omtypes_h_ */
