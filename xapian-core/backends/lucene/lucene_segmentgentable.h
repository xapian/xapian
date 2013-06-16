
#ifndef XAPIAN_INCLUDED_LUCENE_SEGMENTGENTABLE_H
#define XAPIAN_INCLUDED_LUCENE_SEGMENTGENTABLE_H

#include "bytestream.h"

class LuceneSegmentPart;

class LuceneSegmentGenTable {
    string db_dir;
    string file_name;
    ByteStreamReader stream_reader;

    //segment_gen 's data
    int version;
    long long generationA;
    long long generationB;

  public:
    LuceneSegmentGenTable(const string &);
    bool open();

    int get_version();
    long long get_generationA();
    long long get_generationB();

    //for debug
    void debug_get_table();
};

#endif
