
#ifndef XAPIAN_INCLUDED_LUCENE_SEGMENTGENTABLE_H
#define XAPIAN_INCLUDED_LUCENE_SEGMENTGENTABLE_H

#include "bytestream.h"

class LuceneSegmentPart;

class LuceneSegmentGenTable {
    /** Database directory
     */
    string db_dir;

    /** File name for segment.gen
     */
    string file_name;

    /** File reader
     */
    ByteStreamReader stream_reader;

    int version;
    long long generationA;
    long long generationB;

  public:
    LuceneSegmentGenTable(const string &);
    bool open();

    int get_version();
    long long get_generationA();
    long long get_generationB();

    /** Just for debug
     */
    void debug_get_table();
};

#endif
