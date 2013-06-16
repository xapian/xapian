
#ifndef XAPIAN_INCLUDED_LUCENE_SEGMENTTABLE_H
#define XAPIAN_INCLUDED_LUCENE_SEGMENTTABLE_H

#include "bytestream.h"

class LuceneSegmentPart;

class LuceneSegmentTable {
    string db_dir;
    string file_name;
    ByteStreamReader stream_reader;

    //segment_gen 's data
    int format;
    long long version;
    int name_counter;
    int seg_count;
    //using vector instead
    LuceneSegmentPart * segment_part;
    map<string, string> commit_user_data;
    long long checksum;

  public:
    LuceneSegmentTable(const string &);
    bool open();
    bool set_filename(long long file_suffix);
    string get_seg_name(int part_num);

    int get_seg_count();
    Xapian::doccount get_doccount() const;
    /**
     * Get docid base for segment[seg_idx]
     */
    Xapian::docid get_didbase(int seg_idx) const;

    /**
     * Get segment docid and segment index
     */
    Xapian::docid get_didbase_and_segidx(Xapian::docid ext_did,
                unsigned int & seg_idx) const;

    //for debug
    void debug_get_table();
};

class LuceneSegmentPart {
    friend class LuceneSegmentTable;
    string seg_version;
    string seg_name;
    int seg_size;
    long long del_gen;
    int doc_store_offset;
    char has_single_normfile;
    int num_field;
    char is_compoundfile;
    int del_count;
    char has_proxy;
    map<string, string> diagnostics;
    char has_vectors;

  public:
};

#endif
