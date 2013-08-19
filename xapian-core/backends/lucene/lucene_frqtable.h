
#ifndef XAPIAN_INCLUDED_LUCENE_FRQTABLE_H
#define XAPIAN_INCLUDED_LUCENE_FRQTABLE_H

//#include <xapian/database.h>
#include "config.h"
#include "internaltypes.h"
#include "xapian/intrusive_ptr.h"
#include "bytestream.h"
#include "api/leafpostlist.h"
#include "lucene_database.h"

#include <vector>

using namespace std;

/**
 * LuceneFrqTable is similar to ChertPostListTable in Xapian
 * The .frq file contains the lists of documents which contain each term,
 * along with the frequency of the term in that document
 *
 * FreqFile (.frq) --> <TermFreqs, SkipData> TermCount
 * TermFreqs --> <TermFreq> DocFreq
 * TermFreq --> DocDelta[, Freq?]
 * SkipData --> <<SkipLevelLength, SkipLevel> NumSkipLevels-1, SkipLevel> <SkipDatum>
 * SkipLevel --> <SkipDatum> DocFreq/(SkipInterval^(Level + 1))
 * SkipDatum --> DocSkip,PayloadLength?,FreqSkip,ProxSkip,SkipChildLevelPointer?
 * DocDelta,Freq,DocSkip,PayloadLength,FreqSkip,ProxSkip --> VInt
 * SkipChildLevelPointer --> VLong
 *
 * details on http://lucene.apache.org/core/3_6_2/fileformats.html#Frequencies
 */
class LuceneFrqTable {
    /* Database directory */
    string db_dir;

    /* .frq file name */
    string file_name;

    /* File reader for .frq */
    ByteStreamReader stream_reader;

    //This is for a specific term
    int doc_freq;

  public:
    LuceneFrqTable(const string &);
    ~LuceneFrqTable();

    bool set_filename(const string &);
    string get_filename() const;
    void set_docfreq(const int);
    string get_dbdir() const;
};

class LucenePostList : public Xapian::Internal::intrusive_base {

    /* Term name for this postlist */
    string term;

    /* Term's field number */
    int field_num;

    /** This is equal to ChertPostList::number_of_entries. In other words, It
     * means how many documents in this postlist
     */
    int doc_freq;

    int freq_delta;
    int skip_delta;
    string db_dir;
    string file_name;
    ByteStreamReader docfreq_reader;

    /* Docid of document which is visiting now */
    Xapian::docid did;

    /* wdf of document which is visiting now */
    Xapian::termcount wdf;

    /* Counter of visited documents */
    int c;

    /** Vector index for LuceneDatabase->seg_dbs, used to find segment. 
     * Segments are sequencely push_back to LuceneDatabase::seg_dbs, using seg_idx
     * to find related segment object in LuceneDatabase::seg_dbs
     */
    unsigned int seg_idx;

    /* Reached the end of postlist */
    bool is_at_end;

  public:
    LucenePostList(const string & term_, int field_num_, int doc_freq_,
                int freq_delta_, int skip_delta_, const string & db_dir,
                const string & file_name_);

    /* termfreq here means how many document contains the term */
    Xapian::doccount get_termfreq() const;

    /* virtual function realize begin */
    Xapian::docid get_docid() const;

    /**TODO, doclength is not exists in Lucene, How to get it?
     * In Chert, get_doclength() seems return wdf, see the return type,
     * Xapian::termcount, maybe it's not doclength, it's termcount
     */
    Xapian::termcount get_doclength() const;

    /* If reached to the end of the postlist */
    bool at_end() const;

    /** Visite next document in the PostList, @param is not used yet
     **/
    PostList * next(double);

    /** Skip to a specified document in the PostList, @param 2 is not used yet
     * TODO, skip list is not used yet, so when the length of postlist is big,
     * performance issues may appear
     */
    PostList * skip_to(Xapian::docid, double);

    std::string get_description() const;
    /* virtual function realize end */

    Xapian::termcount get_wdf() const;

    void set_seg_idx(int idx);
    unsigned int get_seg_idx() const;

    /* below is for debug */
    void debug_postlist() const;

    int get_field_num() const;
};

/**
 * Be composed of lots of postlists, one postlist per segment.
 * All postlits are stored in vector pls
 * To make interface compatible, this class extends from LeafPostList
 */
class LuceneMultiPostList : public LeafPostList {
    /**
     * Pointed to database
     */
    Xapian::Internal::intrusive_ptr<const LuceneDatabase> this_db;

    /**
     * one postList per segment
     */
    vector<LucenePostList *> pls;

    /**
     * Current docid which be visited, calculate from sub LucenePostList
     * It is in segment docid, not external docid
     */
    Xapian::docid c_did;

    /* vector index for current postList */
    unsigned int pls_index;

  public:
    LuceneMultiPostList(Xapian::Internal::intrusive_ptr<const LuceneDatabase>,
        const vector<LucenePostList *> &, const string &);

    /* virtual function realize begin */
    Xapian::doccount get_termfreq() const;

    /**
     * Return virtual did, not real did, virtual did is a docid
     * which contains segment index infomation
     */
    Xapian::docid get_docid() const;

    /** Find norm value in .nrm. Using class variable term to find field,
     * then find norm in .nrm
     */
    Xapian::termcount get_doclength() const;

    /* If visit to the end of PostList */
    bool at_end() const;

    /* Visit the next doc in postlist */
    PostList * next(double);

    /* Hasn't used yet */
    PostList * skip_to(Xapian::docid, double);
    std::string get_description() const;

    /**
     * Check if ext_id exists in PostList
     * @a valid is set to true if exists
     * @a valid is set to false if not exists
     */
    //PostList * check(Xapian::docid ext_did, double w_min, bool & valid);

    /* virtual function realize end*/

    /* Called by LeafPostList::get_weight() */
    Xapian::termcount get_wdf() const;

};

#endif
