/* omtypes.h : Common types used */

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

#ifdef __cplusplus
#include <string>
typedef string termname;
#endif

#endif /* _omtypes_h_ */
