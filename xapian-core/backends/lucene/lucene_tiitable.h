
#ifndef XAPIAN_INCLUDED_LUCENE_TIITABLE_H
#define XAPIAN_INCLUDED_LUCENE_TIITABLE_H

#include "bytestream.h"

/**
 * Read the whole .tii file into memery
 */
class LuceneTiiTable {
    string db_dir;
    string file_name;

    unsigned int ti_version;
    unsigned long long index_term_count;
    unsigned int index_interval;
    unsigned int skip_interval;
    unsigned int max_skip_levels;
    vector<LuceneTermIndice> term_indices;
    vector<string> field_name;
    ByteStreamReader stream_reader;

  public:
    LuceneTiiTable(const string &);
    bool set_filename(const string &);
    bool open();
    bool set_field_name(vector<string>);
    int get_index_offset(const LuceneTerm &) const;
    const LuceneTermIndice & get_term_indice(int) const;

    //for dubug
    void debug_table();
};

#endif
