
#ifndef XAPIAN_INCLUDED_LUCENE_TISTABLE_H
#define XAPIAN_INCLUDED_LUCENE_TISTABLE_H

#include "config.h"
#include "bytestream.h"

class LuceneTisTable {
    string db_dir;
    string file_name;

    vector<string> field_name;
    int ti_version;
    long long term_count;
    int index_interval;
    int skip_interval;
    int max_skip_levels;
    ByteStreamReader stream_reader;

  public:
    LuceneTisTable(const string &);
    bool set_filename(const string & prefix);
    bool set_field_name(vector<string>);
    bool open();
    /**
     * LuceneTerm &, target
     * LuceneTermInfo &, result 
     * const LuceneTermInfo &, prev term info in tii
     * const long long &, file offset in tis
     */
    bool scan_to(const LuceneTerm &, LuceneTermInfo &, 
                const LuceneTermIndice &) const;

    //below is for debug
    void debug_get_table();
};

#endif
