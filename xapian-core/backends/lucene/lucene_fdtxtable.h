
#ifndef XAPIAN_INCLUDED_LUCENE_FDTXTABLE_H
#define XAPIAN_INCLUDED_LUCENE_FDTXTABLE_H

#include "bytestream.h"

class LuceneSegdb;

class LuceneFdtxTable {
    friend class LuceneSegdb;

    string db_dir;

    /** File name for .fdx
     */
    string fdx_filename;

    /** File reader for .fdx
     */
    ByteStreamReader fdx_reader;

    /** File name for .fdt
     */
    string fdt_filename;
    
    /** File reader for .fdt
     */
    ByteStreamReader fdt_reader;

    void set_fdx_filename(const string &);
    void set_fdt_filename(const string &);

  public:
    LuceneFdtxTable(const string &);
    ~LuceneFdtxTable();

    void set_filename(const string &);

    /** Open .fdt and .fdx files
     */
    bool open();

    /** Read document content from .fdt and .fdx, and store them in thess maps
     *  @para 2: date type is string
     *  @para 3: data type is int
     *  @para 4: data type is long
     *  @para 5: data type is float
     *  @para 6: data type is double
     *
     *  TODO
     *  Lucene has other data type, like binary data, which is not supported here
     */
    void get_record(Xapian::docid , map<int, string> &, map<int, int> &,
        map<int, long> &, map<int, float> &, map<int, double>) const;
};

#endif
