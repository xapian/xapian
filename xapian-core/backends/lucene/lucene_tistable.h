
#ifndef XAPIAN_INCLUDED_LUCENE_TISTABLE_H
#define XAPIAN_INCLUDED_LUCENE_TISTABLE_H

#include "bytestream.h"

/** File format:
 *  TermInfoFile (.tis)--> TIVersion, TermCount, IndexInterval,
 *    SkipInterval, MaxSkipLevels, TermInfos
 *  TermInfos --> <TermInfo>^TermCount
 *  TermInfo --> <Term, DocFreq, FreqDelta, ProxDelta, SkipDelta>
 *  Term --> <PrefixLength, Suffix, FieldNum>
 *
 *  Part of the term dictionary, stores term info.
 *
 *  This file is sorted by Term. Terms are ordered first lexicographically
 *  (by UTF16 character code) by the term's field name, and within that
 *  lexicographically (by UTF16 character code) by the term's text.
 */
class LuceneTisTable {
    /** Database Directory
     */
    string db_dir;

    /** .tis file
     */
    string file_name;

    /** File reader
     */
    ByteStreamReader stream_reader;

    vector<string> field_name;

    /** TIVersion names the version of the format of this file and is equal
     *  to TermInfosWriter.FORMAT_CURRENT.
     */
    int ti_version;

    long long term_count;
    int index_interval;

    /** SkipInterval is the fraction of TermDocs stored in skip tables. It
     *  is used to accelerate TermDocs.skipTo(int). Larger values result in
     *  smaller indexes, greater acceleration, but fewer accelerable cases,
     *  while smaller values result in bigger indexes, less acceleration (in
     *  case of a small value for MaxSkipLevels) and more accelerable cases.
     */
    int skip_interval;

    /** MaxSkipLevels is the max. number of skip levels stored for each Term
     *  in the .frq file. A low value results in smaller indexes but less
     *  acceleration, a larger value results in slighly larger indexes but
     *  greater acceleration. See format of .frq file for more information
     *  about skip levels.
     */
    int max_skip_levels;

    /** pti means previous Term info, this is used for LuceneAllTermsList
     */
    LuceneTermInfo pti;

    /** How many terms are visited, used for LuceneAllTermsList
     */
    long long counter;

  public:
    LuceneTisTable(const string &);
    bool set_filename(const string & prefix);
    bool set_field_name(vector<string>);
    bool open();

    /** LuceneTerm &, target
     *  LuceneTermInfo &, result 
     *  const LuceneTermInfo &, prev term info in tii
     *  const long long &, file offset in tis
     */
    bool scan_to(const LuceneTerm &, LuceneTermInfo &, 
                const LuceneTermIndice &) const;

    /** Move ByteStreamReader to the next term
     */
    void next();

    /** If ByteStreamReader points to the end of file
     */
    bool at_end() const;

    LuceneTermInfo get_current_ti() const;

    /** below is for debug
     */
    void debug_get_table();
};

#endif
