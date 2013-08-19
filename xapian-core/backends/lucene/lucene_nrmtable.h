
#ifndef XAPIAN_INCLUDED_LUCENE_NRMTABLE_H
#define XAPIAN_INCLUDED_LUCENE_NRMTABLE_H

#include "bytestream.h"
#include <string>

#define LOAD_WHOLE_NORM 1

using namespace std;

/** A separate norm file is created when the norm values of an existing
 * segment are modified. When field N is modified, a separate norm file
 * .sN is created, to maintain the norm values for that field.
 *
 * TODO, just single .nrm file is supported now, no separate files
 */
class LuceneNrmTable {

    /* Lucene database directory */
    string db_dir;

    /* .nrm file name */
    string file_name;

    /* version number */
    int nrm_version;

#ifdef LOAD_WHOLE_NORM
    char * norms;
#else
    /** File reader.
     * Actually, this reader is not just for stream reading, random seek
     * is used too
     */
    ByteStreamReader reader;
#endif

    /* Documents count */
    int seg_size;

  public:
    LuceneNrmTable(const string & db_dir);

    ~LuceneNrmTable();

    /* set file name */
    void set_filename(const string & prefix);

    /* Open .nrm file */
    void open();

    /* Read Norm value from file */
    float get_norm(Xapian::docid did, int field_num) const;

    void set_seg_size(int seg_size);

    int get_seg_size() const;

    /** Static function for decodeNorm. This function should be moved
     * to a util class, not necessary now
     *
     * These are converted to an IEEE single float value as follows:
     * If the byte is zero, use a zero float.
     * Otherwise, set the sign bit of the float to zero;
     * add 48 to the exponent and use this as the float's exponent;
     * map the mantissa to the high-order 3 bits of the float's mantissa; and
     * set the low-order 21 bits of the float's mantissa to zero.
     */
    float decode_norm(char b) const;
};

/* inline this function seems no effect for optimizing performance
inline float decode_norm(char b) {
    //Use union to convert int to float
    union {
        int i;
        float f;
    } d;

    if (0 == b)
        return 0.0f;
    d.i = (b & 0xff) << (24 - 3);
    d.i += (63 - 15) << 24;
    return d.f;
}
*/

#endif
