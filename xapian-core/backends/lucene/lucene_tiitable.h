
#ifndef XAPIAN_INCLUDED_LUCENE_TIITABLE_H
#define XAPIAN_INCLUDED_LUCENE_TIITABLE_H

#include "bytestream.h"

/** File format:
 *  TermInfoIndex (.tii)--> TIVersion, IndexTermCount, IndexInterval,
 *    SkipInterval, MaxSkipLevels, TermIndices 
 *  TermIndices --> <TermInfo, IndexDelta>^IndexTermCount
 *
 *  The index into the Term Infos file.
 *  This contains every IndexInterval th entry from the .tis file,
 *  along with its location in the "tis" file. This is designed to
 *  be read entirely into memory and used to provide random access
 *  to the "tis" file. 
 *
 *  Read the whole .tii file into memery
 */
class LuceneTiiTable {
    string db_dir;
    string file_name;

    /** TIVersion names the version of the format of this file and is
     *  equal to TermInfosWriter.FORMAT_CURRENT. 
     */
    unsigned int ti_version;
    unsigned long long index_term_count;
    unsigned int index_interval;

    /** SkipInterval is the fraction of TermDocs stored in skip tables.
     *  It is used to accelerate TermDocs.skipTo(int). Larger values
     *  result in smaller indexes, greater acceleration, but fewer
     *  accelerable cases, while smaller values result in bigger indexes,
     *  less acceleration (in case of a small value for MaxSkipLevels)
     *  and more accelerable cases.
     */
    unsigned int skip_interval;

    /** MaxSkipLevels is the max. number of skip levels stored for
     * each term in the .frq file. A low value results in smaller
     * indexes but less acceleration, a larger value results in slighly
     * larger indexes but greater acceleration. See format of .frq file
     * for more information about skip levels.
     */
    unsigned int max_skip_levels;
    vector<LuceneTermIndice> term_indices;
    vector<string> field_name;

    /** File reader
     */
    ByteStreamReader stream_reader;

  public:
    LuceneTiiTable(const string &);
    bool set_filename(const string &);
    bool open();
    bool set_field_name(vector<string>);
    int get_index_offset(const LuceneTerm &) const;
    const LuceneTermIndice & get_term_indice(int) const;

    /** For dubug
     */
    void debug_table();
};

#endif
