
#include <config.h>
#include "debuglog.h"

#include "lucene_database.h"
#include "lucene_segdb.h"
#include "lucene_document.h"
#include "lucene_nrmlist.h"
#include "lucene_alltermslist.h"

#include <xapian/error.h>
#include <xapian/valueiterator.h>
#include <common/omassert.h>
#include "backends/multi/multi_postlist.h"

#include <sys/types.h>

#include <algorithm>
#include <string>
#include <iostream>

using namespace std;
using namespace Xapian;
using Xapian::Internal::intrusive_ptr;

LuceneDatabase::LuceneDatabase(const string &lucene_dir) 
        : db_dir(lucene_dir),
          segmentgen_table(db_dir),
          segment_table(db_dir),
          stat_table(db_dir)

{
    LOGCALL_CTOR(DB, "LuceneDatabase", lucene_dir);

    create_and_open_tables();

    return;
}

LuceneDatabase::~LuceneDatabase()
{
    LOGCALL_DTOR(DB, "LuceneDatabase");
}

intrusive_ptr<LuceneSegdb>
LuceneDatabase::get_segdb(unsigned int index) const {
    if (index >= seg_dbs.size())
        return intrusive_ptr<LuceneSegdb>(NULL);
    return seg_dbs[index];
}

/** Return the sum of doc count in all segments */
Xapian::doccount
LuceneDatabase::get_doccount() const
{
    LOGCALL(DB, Xapian::doccount, "LuceneDatabase::get_doccount", NO_ARGS);
    Xapian::doccount count = segment_table.get_doccount();

    RETURN(Xapian::doccount(count));
}

Xapian::doccount
LuceneDatabase::get_doccount(int segment) const {
    LOGCALL(DB, Xapian::doccount, "LuceneDatabase::get_doccount", segment);
    Xapian::doccount count = segment_table.get_doccount(segment);

    RETURN(count);
}

/**
 * TODO lastdocid doesn't exists in Lucene
 */
Xapian::docid
LuceneDatabase::get_lastdocid() const
{
    LOGCALL(DB, Xapian::docid, "(not realized)LuceneDatabase::get_lastdocid", NO_ARGS);
    Assert(false);

    RETURN(Xapian::docid(0));
}

/**
 * TODO there's no total_length(doc length) in lucene, this length is stored in 
 * postlist.DB(with key '\0') in xapian, so... should I caculate this data by using 
 * copydatabase tool and store it in somewhere
*/
//TODO
totlen_t
LuceneDatabase::get_total_length() const
{
    LOGCALL(DB, totlen_t, "(not realized)LuceneDatabase::get_total_length", NO_ARGS);
    Assert(false);

    RETURN(0);
}

Xapian::doclength
LuceneDatabase::get_avlength() const
{
    LOGCALL(DB, Xapian::doclength, "(not realized)LuceneDatabase::get_avlength", NO_ARGS);
    Assert(false);

    RETURN(0);
}

Xapian::termcount
LuceneDatabase::get_doclength(Xapian::docid did) const
{
    LOGCALL(DB, Xapian::termcount, "(not realized)LuceneDatabase::get_doclength", did);
    Assert(false);
    (void)did;

    RETURN(0);
}

Xapian::doccount
LuceneDatabase::get_termfreq(const string & term) const
{
    LOGCALL(DB, Xapian::doccount, "(not realized) LuceneDatabase::get_termfreq", term);
    Assert(!term.empty());

    LuceneTerm lterm;
    vector<intrusive_ptr<LuceneSegdb> >::const_iterator i;
    Xapian::doccount doc_freq = 0;
    for (i = seg_dbs.begin(); i != seg_dbs.end(); i++) {
        (*i)->get_luceneterm(term, lterm);
        doc_freq += (*i)->get_termfreq(lterm);
    }

    RETURN(doc_freq);
}

//TODO
Xapian::termcount
LuceneDatabase::get_collection_freq(const string & term) const
{
    LOGCALL(DB, Xapian::termcount, "(not realized)LuceneDatabase::get_collection_freq", term);
    Assert(false);
    (void)term;

    RETURN(0);
}

//TODO
Xapian::doccount
LuceneDatabase::get_value_freq(Xapian::valueno slot) const
{
    LOGCALL(DB, Xapian::doccount, "(not realized)LuceneDatabase::get_value_freq", slot);
    Assert(false);
    (void)slot;

    RETURN(0);
}

//TODO
std::string
LuceneDatabase::get_value_lower_bound(Xapian::valueno slot) const
{
    LOGCALL(DB, std::string, "(not realized)LuceneDatabase::get_value_lower_bound", slot);
    Assert(false);
    (void)slot;

    RETURN(std::string());
}

//TODO
std::string
LuceneDatabase::get_value_upper_bound(Xapian::valueno slot) const
{
    LOGCALL(DB, std::string, "(not realized)LuceneDatabase::get_value_upper_bound", slot);
    Assert(false);
    (void)slot;

    RETURN(std::string());
}

//TODO
/*
Xapian::termcount
LuceneDatabase::get_doclength_lower_bound() const
{
    LOGCALL(DB, Xapian::termcount, "(not realized)LuceneDatabase::get_doclength_lower_bound",
                NO_ARGS);
    return 1;
}
*/

//TODO
/*
Xapian::termcount
LuceneDatabase::get_doclength_upper_bound() const
{
    LOGCALL(DB, Xapian::termcount, "(not realized)LuceneDatabase::get_doclength_upper_bound",
                NO_ARGS);
    return 1;
}
*/

Xapian::termcount
LuceneDatabase::get_wdf_upper_bound(const string & term) const
{
    LOGCALL(DB, Xapian::termcount, "LuceneDatabase::get_wdf_upper_bound", term);
    (void)term;

    RETURN(stat_table.get_wdf_upper_bound());
}

//TODO
bool
LuceneDatabase::term_exists(const string & term) const
{
    LOGCALL(DB, bool, "(not realized)LuceneDatabase::term_exists", term);
    Assert(false);
    (void)term;

    RETURN(false);
}

//TODO
bool
LuceneDatabase::has_positions() const
{
    LOGCALL(DB, bool, "(not realized)LuceneDatabase::has_positions", NO_ARGS);
    Assert(false);

    RETURN(false);
}

LeafPostList *
LuceneDatabase::open_post_list(const string & term) const
{
    LOGCALL(DB, LeafPostList*, "LuceneDatabase::open_post_list", term);

    LuceneTerm lterm;
    vector<LucenePostList *> pls;
    for (unsigned int i = 0; i < seg_dbs.size(); ++i) {
        seg_dbs[i]->get_luceneterm(term, lterm);
        LucenePostList * postlist = seg_dbs[i]->open_post_list(lterm);
        if (NULL != postlist) {
            //seg_idx is index for vector LuceneDatabase::seg_dbs
            postlist->set_seg_idx(i);
            pls.push_back(postlist);
        }
    }

    intrusive_ptr<const LuceneDatabase> ptrtothis(this);
    //temporary variable pls, pass it as a reference, memery leak?
    RETURN(new LuceneMultiPostList(ptrtothis, pls, term));
}

//TODO
ValueList *
LuceneDatabase::open_value_list(Xapian::valueno slot) const
{
    LOGCALL(DB, ValueList *, "(not realized)LuceneDatabase::open_value_list", slot);
    Assert(false);
    (void)slot;

    RETURN(NULL);
}

//TODO
TermList *
LuceneDatabase::open_term_list(Xapian::docid did) const
{
    LOGCALL(DB, TermList *, "(not realized)LuceneDatabase::open_term_list", did);
    Assert(false);
    (void)did;

    RETURN(NULL);
}

TermList *
LuceneDatabase::open_allterms(const string & prefix) const
{
    LOGCALL(DB, TermList *, "(not realized)LuceneDatabase::open_allterms", NO_ARGS);

    RETURN(new LuceneAllTermsList(intrusive_ptr<const LuceneDatabase>(this), prefix));
}

//TODO
PositionList *
LuceneDatabase::open_position_list(Xapian::docid did, const string & term) const
{
    LOGCALL(DB, PositionList *, "(not realized)LuceneDatabase::open_position_list",
                term);
    (void)did;
    (void)term;

    RETURN(NULL);
}

//TODO
Xapian::Document::Internal *
LuceneDatabase::open_document(Xapian::docid ext_did, bool lazy) const
{
    LOGCALL(DB, Xapian::Document::Internal *, "LuceneDatabase::open_document",
                ext_did | lazy);

    intrusive_ptr<const Database::Internal> ptrtothis(this);
    unsigned int seg_idx = 0;
    Xapian::docid seg_did = get_seg_docid(ext_did, seg_idx);
    LOGLINE(DB, "LuceneDatabase::open_document, seg_idx=" << seg_idx);

    Assert(seg_idx < segment_table.get_seg_count());

    //just avoid -Werror=unused-parameter
    lazy = lazy;
    RETURN(new LuceneDocument(ptrtothis, seg_did, seg_dbs[seg_idx]));
}

//TODO
TermList *
LuceneDatabase::open_spelling_termlist(const string & word) const
{
    LOGCALL(DB, TermList *, "(not realized)LuceneDatabase::open_spelling_termlist",
                word);
    Assert(false);
    (void)word;

    RETURN(NULL);
}

//TODO
TermList *
LuceneDatabase::open_spelling_wordlist() const
{
    LOGCALL(DB, TermList *, "(not realized)LuceneDatabase::open_spelling_wordlist",
                NO_ARGS);
    Assert(false);

    RETURN(NULL);
}

//TODO
Xapian::doccount
LuceneDatabase::get_spelling_frequency(const string & word) const
{
    LOGCALL(DB, Xapian::doccount, "(not realized)LuceneDatabase::get_spelling_frequency",
                word);
    Assert(false);
    (void)word;

    RETURN(Xapian::doccount(0));
}

//TODO
TermList *
LuceneDatabase::open_synonym_termlist(const string & term) const
{
    LOGCALL(DB, TermList *, "(not realized)LuceneDatabase::open_synonym_termlist",
                term);
    Assert(false);
    (void)term;

    RETURN(NULL);
}

//TODO
TermList *
LuceneDatabase::open_synonym_keylist(const string & prefix) const
{
    LOGCALL(DB, TermList *, "(not realized)LuceneDatabase::open_synonym_keylist",
                prefix);
    Assert(false);
    (void)prefix;

    return 0;
}

//TODO
string
LuceneDatabase::get_metadata(const string & key) const
{
    LOGCALL(DB, string, "(not realized)LuceneDatabase::get_metadata", key);
    (void)key;

    RETURN(string());
}

//TODO
TermList *
LuceneDatabase::open_metadata_keylist(const std::string &prefix) const
{
    LOGCALL(DB, TermList *, "(not realized)LuceneDatabase::open_metadata_keylist",
                NO_ARGS);
    Assert(false);
    (void)prefix;

    RETURN(NULL);
}

//TODO
string
LuceneDatabase::get_revision_info() const
{
    LOGCALL(DB, string, "(not realized)LuceneDatabase::get_revision_info",
                NO_ARGS);
    Assert(false);

    RETURN(string());
}

//TODO
string
LuceneDatabase::get_uuid() const
{
    LOGCALL(DB, string, "(not realized)LuceneDatabase::get_uuid", NO_ARGS);
    Assert(false);

    RETURN(string());
}

//TODO
bool
LuceneDatabase::reopen()
{
    LOGCALL(DB, bool, "(not realized)LuceneDatabase::reopen", NO_ARGS);
    Assert(false);

    RETURN(false);
}

//TODO
void
LuceneDatabase::close()
{
    LOGCALL_VOID(DB, "(not realized)LuceneDatabase::close", NO_ARGS);
}

//TODO
void
LuceneDatabase::cancel()
{
    LOGCALL_VOID(DB, "(not realized)LuceneDatabase::cancel", NO_ARGS);
}

//TODO
void 
LuceneDatabase::write_changesets_to_fd(int fd, 
            const string &revision,
            bool need_whole_db,
            ReplicationInfo *info)
{
    LOGCALL_VOID(DB, "(not realized)LuceneDatabase::write_changesets_to_fd",
                fd | revision | need_whole_db | info);
    fd = fd;
    revision.data();
    need_whole_db = need_whole_db;
    info = info;
}

//Support multiple segments
void
LuceneDatabase::create_and_open_tables()
{
    LOGCALL(DB, void, "LuceneDatabase::create_and_open_tables", NO_ARGS);
    stat_table.debug_get_table();

    segmentgen_table.open();
    //Segmentgen_table.debug_get_table();

    //Choose generationA for test
    //TODO Actually, generation is chooseen from generationA and generationB
    long long generation = segmentgen_table.get_generationA();
    segment_table.set_filename(generation);
    segment_table.open();
    segment_table.debug_get_table();

    int seg_count = segment_table.get_seg_count();
    for (int i = 0; i < seg_count; ++i) {
        //File name prefix stores in segment table, for example _0
        string prefix = segment_table.get_seg_name(i);
        intrusive_ptr<LuceneSegdb> s_db(new LuceneSegdb(db_dir, segment_table.segments[i]));
        //Pay attention to the sequence, first segment in the front of vector
        seg_dbs.push_back(s_db);
    }

    //open all the LuceneSegdb
    vector<intrusive_ptr<LuceneSegdb> >::const_iterator i;
    int idx = 0;
    for (i = seg_dbs.begin(); i != seg_dbs.end(); i++, idx++) {
        (*i)->create_and_open_tables();
    }
}

Xapian::docid
LuceneDatabase::get_ext_docid(Xapian::docid seg_did, int seg_idx) const {
    LOGCALL(DB, Xapian::docid, "LuceneDatabase::get_ext_docid", seg_did |
        seg_idx);

    Xapian::docid base = segment_table.get_didbase(seg_idx);

    RETURN(seg_did + base);
}

Xapian::docid
LuceneDatabase::get_seg_docid(Xapian::docid ext_did,
            unsigned int & seg_idx) const {
    LOGCALL(DB, Xapian::docid, "LuceneDatabase::get_seg_docid", ext_did |
        seg_idx);

    Xapian::docid base = segment_table.get_didbase_and_segidx(ext_did, seg_idx);

    RETURN(ext_did - base);
}

void
LuceneDatabase::get_fieldinfo(set<string> & field_set) const {
    LOGCALL(DB, void, "LuceneDatabase::get_fieldinfo", field_set.size());

    vector<intrusive_ptr<LuceneSegdb> >::const_iterator it;
    for (it = seg_dbs.begin(); it != seg_dbs.end(); ++it) {
        set<string> seg_field;
        (*it)->get_fieldinfo(seg_field);
        field_set.insert(seg_field.begin(), seg_field.end());
    }

    LOGLINE(DB, "LuceneDatabase::get_fieldinfo, size=" << field_set.size());
}

ValueList *
LuceneDatabase::open_norm_lists() const {
    LOGCALL(DB, ValueList *, "no used", NO_ARGS);

    //Deletion is executed where doing ValueIterator destruction
    //return new LuceneNrmList(intrusive_ptr<const LuceneDatabase>(this));
    return NULL;
}

string
LuceneDatabase::get_description() const {
    LOGCALL(DB, string, "LuceneDatabase::get_description", NO_ARGS);

    RETURN("lucene");
}
