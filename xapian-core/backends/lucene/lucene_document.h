
#ifndef XAPIAN_INCLUDED_LUCENE_DOCUMENT_H
#define XAPIAN_INCLUDED_LUCENE_DOCUMENT_H

#include "xapian/intrusive_ptr.h"
#include "backends/document.h"
#include "lucene_segdb.h"

/**
 */
class LuceneDocument : public Xapian::Document::Internal {
    friend class LuceneDatabase;
    Xapian::Internal::intrusive_ptr<const LuceneSegdb> seg_db;

    //Private constructor, only called by open_document
    LuceneDocument(Xapian::Internal::intrusive_ptr<const Xapian::Database::Internal>,
                Xapian::docid,
                Xapian::Internal::intrusive_ptr<const LuceneSegdb>);

    /**
     * If Data has been read before.
     * If true, document record is stored in string_map/int_map/long_map/float_map/double_map
     * Otherwith, these maps are empty
     **/
    bool has_read;

    /**
     * Data from .fdt, now five type are supported. First data is field number,
     * second is real data
     * TODO
     * C++ has no Object clsss like Java, so I have to use different map to store
     * different data. In Java, map<int, Object> may works, but C++ doesn't
     * 
     * Lucene has these types of data
     * 1. low order bit is one for tokenized fields
     * 2. second bit is one for fields containing binary data
     * 3. third bit is one for fields with compression option enabled (if compression is enabled, the algorithm used is ZLIB), only available for indexes until Lucene version 2.9.x
     * 4. 4th to 6th bit (mask: 0x7<<3) define the type of a numeric field:
     *   1). all bits in mask are cleared if no numeric field at all
     *   2). 1<<3: Value is Int
     *   3). 2<<3: Value is Long
     *   4). 3<<3: Value is Int as Float (as of Float.intBitsToFloat)
     *   4). 4<<3: Value is Long as Double (as of Double.longBitsToDouble)
     * More details on http://lucene.apache.org/core/3_6_2/fileformats.html
     * TODO, Now just string type of data is supported
     */
    map<int, string> string_map;
    map<int, int> int_map;
    map<int, long> long_map;
    map<int, float> float_map;
    map<int, double> double_map;

    /* Get corresponding field number to the @para field name */
    int get_fieldnum(const string & field) const;

    /* Read document record in file, and store them in maps above */
    void get_data();

  public:
    ~LuceneDocument();

    /** Virtual function from Document::Internal */
    /**
     * Called in document.h->get_data()
     * NOT USED in Lucene
     */
    //void do_get_data();

    /**
     * Read data from maps. If has not read before, get_data() first.
     * Lucene has lots of data type, so one get_data_xxx for each data type.
     * These functions can merged? Using polymorphism, or Template
     */

    /* Read data from string_map */
    std::string get_data_string(const std::string & field);

    /* Read data from int_map */
    int get_data_int(const std::string & field);

    /* Read data from long_map */
    long get_data_long(const std::string & field);

    /* Read data from float_map */
    float get_data_float(const std::string & field);

    /* Read data from double_map */
    double get_data_double(const std::string & field);
};

#endif
