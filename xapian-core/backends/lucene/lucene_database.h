#ifndef OM_HGUARD_LUCENE_DATABASE_H
#define OM_HGUARD_LUCENE_DATABASE_H

#include "backends/database.h"
#include "backends/valuestats.h"
#include "internaltypes.h"
#include "lucene_segmentgentable.h"
#include "lucene_segmenttable.h"
#include "lucene_stattable.h"
#include "noreturn.h"

#include <map>
#include <vector>
#include <set>

using namespace std;

/**
 * preforward declairatoin, otherwise loop include .h will occur.
 * because using LuceneDatabase in lucene_frqtable.h and using LuceneFrqtable
 * in LuceneDatabase
 */
class LuceneSegdb;

/** A backend designed for Lucene Database
 */
class LuceneDatabase : public Xapian::Database::Internal {
    std::string db_dir;

    /* File segments.gen */
    LuceneSegmentGenTable segmentgen_table;

    /* File segments_x, all segments' basic info are stored here */
    LuceneSegmentTable segment_table;

    /* File stat.xapian, statistics data for xapian */
    LuceneStatTable stat_table;

    /** Put all LuceneSegdb in a vector, and smart ptr is used
     */
    //contains all segments
    vector<Xapian::Internal::intrusive_ptr<LuceneSegdb> > seg_dbs;

    /** Init Lucene database. Just open all LuceneSegdbs in the database
     */
    void create_and_open_tables();

  public:
    LuceneDatabase(const string &db_dir);

    /* Get seg_dbs[index] */
    Xapian::Internal::intrusive_ptr<LuceneSegdb> get_segdb(unsigned int index) const;

    /**
     * Convert segment docid to external(whole db) docid.
     * Segment docid means docid in the segment, different doc in different
     * segment may have the same segment docid, but external docid is uniq
     * in the whole database.
     * external docid = biggest external docid in previous segment + segment docid
     * for example:
     * segment_0 has 100 doc, it's biggest external docid is 100, in segment_1,
     * the external docid  = 100 + segment docid
     *
     * This method doesn't support write occurrence, for example 
     * 1. call ext_did = get_ext_docid(seg_did)
     * 2. write occurs, seg_size is changed
     * 3. call get_seg_docid(ext_did), the return value may
     *    not eq ext_did
     */
    Xapian::docid get_ext_docid(Xapian::docid seg_did, int seg_idx) const;

    /**
     * Convert external docid to segment docid.
     * see get_ext_docid for external docid and segment docid
     */
    Xapian::docid get_seg_docid(Xapian::docid ext_did, unsigned int & seg_idx) const;

    /** virtual method of Database::Internal */
    //@{
    /** This interface is only for Lucene.
     * Get all field name in database
     */
    void get_fieldinfo(set<string> & field_set) const;

    /** Only for Lucene. Get iterator for .nrm files
     */
    ValueList * open_norm_lists() const;

    /** Get Database description "lucene"
     */
    string get_description() const;

    /* Get documents count in all segments */
    Xapian::doccount get_doccount() const;

    Xapian::doccount get_doccount(int segment) const;

    Xapian::docid get_lastdocid() const;
    //TODO
    totlen_t get_total_length() const;

    /* TODO totallength doesn't exits in Lucene, so avlength lacks */
    Xapian::doclength get_avlength() const;

    /** In ChertDatabase, this seems returns wdf, not doclength.
     * Test. Read doclength from .nrm, precondition:
     * 1. .nrm file exists(do not using NO_NORMS flag when indexing)
     * 2. Doc boost and field boot are not using, so the norm = 1.0 / sqrt(numTerms),
     * then doclength(numTerms) = 1 / (norm * norm).
     * See DefaultSimility::computNorm() function in Lucene souuce code for details
     *
     * Pay attenstion. norm value is compressed to one Byte, precision lost.
     * so the doclength is not accurate value
     *
     * FIXME, which field's doclength should return? Maybe we should used
     * another interface with param field name or field number
     */
    Xapian::termcount get_doclength(Xapian::docid did) const;

    /** 
     * termfreq here means how many document contains the termcount
     * TODO, there's no field number parameter, I think it should stored
     * in Query Object first, and pass it to somewhere in DB
     */
    Xapian::doccount get_termfreq(const string &tname) const;

    Xapian::termcount get_collection_freq(const string &tname) const;
    Xapian::doccount get_value_freq(Xapian::valueno slot) const;
    string get_value_lower_bound(Xapian::valueno slot) const;
    string get_value_upper_bound(Xapian::valueno slot) const;
    //Xapian::termcount get_doclength_lower_bound() const;
    //Xapian::termcount get_doclength_upper_bound() const;
    Xapian::termcount get_wdf_upper_bound(const string & term) const;
    bool term_exists(const string & tname) const;
    bool has_positions() const;

    /**
     * One Lucene DB contains lots of segments, one postlist per segment,
     * so I put all postlists in a vector
     *
     * Support multiple segments.
     * One Lucene database has lots of segments, each segment has a postlist,
     * so one Lucene database may has lots of postlists, LuceneMultiPostlist
     * is used to support this feature
     * One database has one LuceneMultiPostlist, in order to make xapian interface
     * compatible
     */
    LeafPostList * open_post_list(const string & tname) const;

    ValueList * open_value_list(Xapian::valueno slot) const;
    TermList * open_term_list(Xapian::docid did) const;
    TermList * open_allterms(const string & prefix) const;
    PositionList * open_position_list(Xapian::docid did, const string & tname) const;

    /** @param 1 is external docid, @param 2 is not used yet
     * open a Lucene Document
     */
    Xapian::Document::Internal *open_document(Xapian::docid did, bool lazy) const;

    TermList * open_spelling_termlist(const string & word) const;
    TermList * open_spelling_wordlist() const;
    Xapian::doccount get_spelling_frequency(const string & word) const;
    //void add_spelling(const string & word, Xapian::termcount freqinc) const;
    //void remove_spelling(const string & word, Xapian::termcount freqdec) const;
    TermList * open_synonym_termlist(const string & term) const;
    TermList * open_synonym_keylist(const string & prefix) const;
    //void add_synonym(const string & term, const string & synonym) const;
    //void remove_synonym(const string & term, const string & synonym) const;
    //void clear_synonyms(const string & term) const;
    string get_metadata(const string & key) const;
    TermList * open_metadata_keylist(const std::string &prefix) const;
    //void set_metadata(const string & key, const string & value);
    bool reopen();
    void close();
    //void commit();
    void cancel();
    //Xapian::docid add_document(const Xapian::Document & document);
    //void delete_document(Xapian::docid did);
    //void delete_document(const string & unique_term);
    //void replace_document(Xapian::docid did, const Xapian::Document & document);
    //Xapian::docid replace_document(const string & unique_term, const Xapian::Document & document);
    //void request_document(Xapian::docid /*did*/) const;
    //Xapian::Document::Internal * collect_document(Xapian::docid did) const;
    void write_changesets_to_fd(int fd, const std::string & start_revision, bool need_whole_db, Xapian::ReplicationInfo * info);
    string get_revision_info() const;
    string get_uuid() const;
    //void invalidate_doc_object(Xapian::Document::Internal * obj) const;
    //RemoteDatabase * as_remotedatabase();
    // @}

    ~LuceneDatabase();
};

#endif
