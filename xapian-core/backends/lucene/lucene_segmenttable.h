
#ifndef XAPIAN_INCLUDED_LUCENE_SEGMENTTABLE_H
#define XAPIAN_INCLUDED_LUCENE_SEGMENTTABLE_H

#include "bytestream.h"

class LuceneSegmentPart;

class LuceneSegmentTable {
    friend class LuceneDatabase;

    string db_dir;
    string file_name;
    ByteStreamReader stream_reader;

    //segment_gen's data
    int format;
    long long version;
    int name_counter;
    int seg_count;

    /* Segments info vector */
    vector<Xapian::Internal::intrusive_ptr<LuceneSegmentPart> > segments;
    map<string, string> commit_user_data;
    long long checksum;

  public:
    LuceneSegmentTable(const string &);
    bool open();
    bool set_filename(long long file_suffix);
    string get_seg_name(int part_num);

    int get_seg_count();

    /** Get document count in all segments */
    Xapian::doccount get_doccount() const;

    /* Get document count in one segment */
    Xapian::doccount get_doccount(int segment) const;

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

class LuceneSegmentPart : public Xapian::Internal::intrusive_base {
    friend class LuceneSegmentTable;
    string seg_version;

    /* File name prefix in this segment */
    string seg_name;

    /* How many documents in this segment */
    int seg_size;

    long long del_gen;
    int doc_store_offset;
    char has_single_normfile;

    /* How many fields in this segment */
    int num_field;
    
    /* is .cfs file used */
    char is_compoundfile;

    /* How many Documents deleted in this segment */
    int del_count;
    char has_proxy;
    map<string, string> diagnostics;
    char has_vectors;

  public:
    string get_seg_name() const;

    int get_seg_size() const;
};

#endif
