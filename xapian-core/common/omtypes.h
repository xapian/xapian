/* omtypes.h : Common types used */

#ifndef _omtypes_h_
#define _omtypes_h_

typedef unsigned int termid;   // Type for term id's
typedef unsigned int docid;    // Type for document id's
typedef unsigned int termpos;  // Type for term positions within documents

typedef docid doccount;        // Type for counts of documents
typedef termid termcount;      // Type for counts of terms (eg, wdf, termfreq)

typedef double doclength;      // Type for (averaged) lengths of documents

typedef double weight;

#ifdef __cplusplus
typedef string termname;
#endif

#endif /* _omtypes_h_ */
