
#ifndef XAPIAN_INCLUDED_LUCENE_SEGMENTTABLE_H
#define XAPIAN_INCLUDED_LUCENE_SEGMENTTABLE_H

#include "bytestream.h"

class LuceneSegmentPart;

/** Meta information for database, file format is:
 *  Segments --> Format, Version, NameCounter, SegCount, <SegVersion,
 *  SegName, SegSize, DelGen, DocStoreOffset, [DocStoreSegment, DocStoreIsCompoundFile], 
 *  HasSingleNormFile, NumField, NormGenNumField, IsCompoundFile, DeletionCount,
 *  HasProx, Diagnostics, HasVectors>SegCount, CommitUserData, Checksum
 *
 *  more details see http://lucene.apache.org/core/3_6_2/fileformats.html
 */
class LuceneSegmentTable {
    friend class LuceneDatabase;

    /** Database directory
     */
    string db_dir;

    /** File name for segments_X
     */
    string file_name;

    /** File reader
     */
    ByteStreamReader stream_reader;

    /** Format is -9 (SegmentInfos.FORMAT_DIAGNOSTICS) in lucene3.6.2
     */
    int format;

    /** Version counts how often the index has been changed by adding or
     *  deleting documents.
     */
    long long version;

    /** NameCounter is used to generate names for new segment files
     */
    int name_counter;

    /** How many segment database
     */
    int seg_count;

    /** Segments infos
     */
    vector<Xapian::Internal::intrusive_ptr<LuceneSegmentPart> > segments;

    map<string, string> commit_user_data;

    /** File checksum
     */
    long long checksum;

  public:
    LuceneSegmentTable(const string &);
    bool open();
    bool set_filename(long long file_suffix);
    string get_seg_name(int part_num);

    int get_seg_count();

    /** Get document count in all segments
     */
    Xapian::doccount get_doccount() const;

    /** Get document count in one segment
     */
    Xapian::doccount get_doccount(int segment) const;

    /** Get docid base for segment[seg_idx]
     */
    Xapian::docid get_didbase(int seg_idx) const;

    /** Get segment docid and segment index
     */
    Xapian::docid get_didbase_and_segidx(Xapian::docid ext_did,
                unsigned int & seg_idx) const;

    /** Just for debug
     */
    void debug_get_table();
};

/** Meta informations for a particular segment
 */
class LuceneSegmentPart : public Xapian::Internal::intrusive_base {
    friend class LuceneSegmentTable;

    /** SegVersion is the code version that created the segment.
     */ 
    string seg_version;

    /* File name prefix in this segment */
    string seg_name;

    /* How many documents in this segment */
    int seg_size;

    /** DelGen is the generation count of the separate deletes file.
     *  If this is -1, there are no separate deletes. If it is 0, this
     *  is a pre-2.1 segment and you must check filesystem for the
     *  existence of _X.del. Anything above zero means there are separate
     *  deletes (_X_N.del). 
     *
     *  TODO Not supported now.
     */
    long long del_gen;
    int doc_store_offset;

    /** If HasSingleNormFile is 1, then the field norms are written as
     *  a single joined file (with extension .nrm); if it is 0 then each
     *  field's norms are stored as separate .fN files.
     *
     *  TODO Just HasSingleNormFile=1 is supported
     */
    char has_single_normfile;

    /* How many fields in this segment */
    int num_field;
    
    /** IsCompoundFile records whether the segment is written as a
     *  compound file or not. If this is -1, the segment is not a compound
     *  file. If it is 1, the segment is a compound file. Else it is 0,
     *  which means we check filesystem to see if _X.cfs exists
     */
    char is_compoundfile;

    /* How many Documents deleted in this segment */
    int del_count;
    char has_proxy;

    /** The Diagnostics Map is privately written by IndexWriter, as a
     *  debugging aid, for each segment it creates. It includes metadata
     *  like the current Lucene version, OS, Java version, why the segment
     *  was created (merge, flush, addIndexes), etc. 
     */
    map<string, string> diagnostics;

    /** HasVectors is 1 if this segment stores term vectors, else it's 0.
     */
    char has_vectors;

  public:
    string get_seg_name() const;

    int get_seg_size() const;
};

#endif
