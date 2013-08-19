
#ifndef XAPIAN_INCLUDED_LUCENE_FNMTABLE_H
#define XAPIAN_INCLUDED_LUCENE_FNMTABLE_H

#include "bytestream.h"
#include <vector>
#include <map>

/** The low-order bit is one for indexed fields, and zero for non-indexed fields.
 * The second lowest-order bit is one for fields that have term vectors stored,
 *   and zero for fields without term vectors.
 * If the fifth lowest-order bit is set (0x10), norms are omitted for the indexed
 *   field.
 * If the sixth lowest-order bit is set (0x20), payloads are stored for the indexed
 *   field.
 * If the seventh lowest-order bit is set (0x40), term frequencies and positions
 *   omitted for the indexed field.
 * If the eighth lowest-order bit is set (0x80), positions are omitted for the
 *   indexed field.
 */
class FnmBitsInfo {
  public:
    bool has_indexed_field;
    bool has_term_vector;
    bool has_norm;
    bool has_payload;
    bool has_freq_pos;
    bool has_position;

    FnmBitsInfo();
};

class LuceneFnmTable {
    friend class LuceneSegdb;

    /* Lucene database directory */
    string db_dir;

    /* .fnm file name */
    string file_name;
    
    /* version number */
    int fnm_version;

    /* How many fields in this segment table */
    int fields_count;

    /**
     * Field names, field number is the vector index
     * For example, field_name[0]'s field number is 0
     * field_name[1]'s field number is 1, etc...
     **/
    vector<string> field_name;

    /**
     * Field attributes, more details http://lucene.apache.org/core/3_6_2/fileformats.html#field_data
     *
     * 1. low order bit is one for tokenized fields
     * 2. second bit is one for fields containing binary data
     * 3. third bit is one for fields with compression option enabled (if compression is enabled, the algorithm used is ZLIB), only available for indexes until Lucene version 2.9.x
     * 4. 4th to 6th bit (mask: 0x7<<3) define the type of a numeric field:
     *   1). all bits in mask are cleared if no numeric field at all
     *   2). 1<<3: Value is Int
     *   3). 2<<3: Value is Long
     *   4): 3<<3: Value is Int as Float (as of Float.intBitsToFloat)
     *   5): 4<<3: Value is Long as Double (as of Double.longBitsToDouble)
     **/
    vector<char> field_bits;

    /** Transfer field_bits infomation to a class FnmBItsInfo, for
     * reading easily
     */
    vector<FnmBitsInfo> field_bitsinfo;

    /**
     * Map for field_name --> field_number
     * First data is field name, second is field number
     * Used for LuceneDocument.get_data_xxx(), mapping field name to field number
     */
    map<string, int> field_map;

    /* .fnm file reader */
    ByteStreamReader stream_reader;

    /* Transfer from field_bits to field_bitsinfo */
    void transfer_bits();

  public:
    LuceneFnmTable(const string &db_dir);

    /* Set file_name */
    bool set_filename(const string &);

    /* Open .fnm file */
    void open();

    /* Return field_name */
    vector<string> get_field_name() const;

    /** Get related field number to @param field name
     * FIXME, field number should a non-negative, so using unsigned 
     * int instead
     */
    int get_field_num(const string & field_name) const;

    /* below is for debug */
    void debug_get_table();
};

#endif
