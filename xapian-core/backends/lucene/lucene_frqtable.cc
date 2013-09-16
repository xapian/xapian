
#include <config.h>

#include "lucene_frqtable.h"

#include "debuglog.h"
#include "lucene_segdb.h"
#include "omassert.h"

#include <iostream>

using namespace std;
using Xapian::Internal::intrusive_ptr;

LuceneFrqTable::LuceneFrqTable(const string & db_dir_)
        :db_dir(db_dir_),
        stream_reader(db_dir_)
{
}

LuceneFrqTable::~LuceneFrqTable() {
    LOGCALL_DTOR(DB, "LuceneFrqTable");
}

bool
LuceneFrqTable::set_filename(const string & prefix) {
    file_name = prefix + ".frq";
    stream_reader.set_filename(file_name);

    return true;
}

string
LuceneFrqTable::get_filename() const {
    return file_name;
}

string
LuceneFrqTable::get_dbdir() const {
    return db_dir;
}

LucenePostList::LucenePostList(const string & term_, int field_num_,
            int doc_freq_, int freq_delta_, int skip_delta_, const string &
            db_dir_, const string & file_name_)
        : term(term_),
        field_num(field_num_),
        doc_freq(doc_freq_),
        freq_delta(freq_delta_),
        skip_delta(skip_delta_),
        db_dir(db_dir_),
        file_name(file_name_),
        docfreq_reader(db_dir_, file_name_),
        did(0),
        wdf(-1),
        c(0),
        seg_idx(-1),
        is_at_end(false)
{
    LOGCALL_CTOR(API, "LucenePostList::LucenePostList", term_ | doc_freq |
                freq_delta | skip_delta | file_name);

    term = term_;
    
    docfreq_reader.seek_to(freq_delta);

    /* debug
    debug_postlist();
    */
}

LucenePostList::~LucenePostList()
{
}

Xapian::doccount
LucenePostList::get_termfreq() const {
    LOGCALL(API, Xapian::doccount, "LucenePostList::get_termfreq", file_name);

    RETURN(doc_freq);
}

Xapian::docid
LucenePostList::get_docid() const {
    //LOGCALL(API, Xapian::docid, "LucenePostList::get_docid", did);
    return did;
}

Xapian::termcount
LucenePostList::get_doclength() const {
    LOGCALL(API, Xapian::termcount, "(not realized)LucenePostList::get_doclength", NO_ARGS);
    Assert(false);

    RETURN(Xapian::termcount(0));
}

bool
LucenePostList::at_end() const {
    return is_at_end;
}

PostList *
LucenePostList::next(double data) {
    LOGCALL(API, PostList *, "LucenePostList::next", file_name | data );
    (void)data;

    if (c >= doc_freq) {
        is_at_end = true;
        return NULL;
    }

    int doc_delta = 0;
    int freq = 0;
    docfreq_reader.read_did_and_freq(doc_delta, freq);

    did = did + doc_delta;
    wdf = freq;
    ++c;

    //LOGLINE(API, "LucenePostList::next did=" << did << ", wdf=" << wdf <<
                //", doc_delta=" << doc_delta);

    return NULL;
}

//TODO using skip list
PostList *
LucenePostList::skip_to(Xapian::docid desire_did,
            double data) {
    LOGCALL(API, PostList *, "LucenePostList::skip_to", desire_did| data);

    //just a simple realization, not using skiplist
    while (true) {
        next(data);
        if (at_end()) {
            break;
        }

        did = get_docid();
        //Docids in postlist are ordered
        if (desire_did <= did) {
            break;
        }
    }

    RETURN(NULL);
}

std::string
LucenePostList::get_description() const {
    LOGCALL(API, std::string, "(not realized)LucenePostList::get_description", NO_ARGS);

    RETURN(std::string());
}

Xapian::termcount
LucenePostList::get_wdf() const {
    return wdf;
}

void
LucenePostList::set_seg_idx(int idx) {
    seg_idx = idx;
}

unsigned int
LucenePostList::get_seg_idx() const {
    return seg_idx;
}

int
LucenePostList::get_field_num() const {
    return field_num;
}

LuceneMultiPostList::LuceneMultiPostList(intrusive_ptr<const LuceneDatabase> this_db_,
            const vector<LucenePostList *> & pls_,
            const string & term_)
        : LeafPostList(term_),
        this_db(this_db_),
        pls(pls_),
        c_did(0),
        pls_index(0)
{
    LOGCALL_CTOR(API, "LuceneMultiPostList", pls_);
}

Xapian::doccount
LuceneMultiPostList::get_termfreq() const {
    LOGCALL(API, Xapian::doccount, "LuceneMultiPostList::get_termfreq", pls.size());

    vector<LucenePostList *>::const_iterator i;
    Xapian::doccount c = 0;
    for (i = pls.begin(); i != pls.end(); ++i) {
        c += (*i)->get_termfreq();
    }

    RETURN(c);
}

Xapian::docid
LuceneMultiPostList::get_docid() const {
    LOGCALL(API, Xapian::docid, "LuceneMultiPostList::get_docid", term |
                pls_index | (unsigned int)this);

    //c_did setted in LuceneMultiPostList::next()
    //Caculate ext_did, which include segment index infomation
    LucenePostList * postlist = pls[pls_index];

    LOGLINE(API, "LuceneMultiPostList::get_docid, c_did=" << c_did <<
                ", seg_idx=" << postlist->get_seg_idx());

    //Using Lucene's docid caculation method
    RETURN(this_db->get_ext_docid(c_did, postlist->get_seg_idx()));
    
    //Like Xapian's docid caculation method
    //RETURN(c_did * pls.size() + postlist->get_seg_idx());
}

Xapian::termcount
LuceneMultiPostList::get_doclength() const {
    //LOGCALL(API, Xapian::termcount, "(not realized) LuceneMultiPostList::get_doclength",
    //            term | c_did);

    //Field number
    int field = pls[pls_index]->get_field_num();
    //Segment number
    int segment = pls[pls_index]->get_seg_idx();

    Xapian::Internal::intrusive_ptr<LuceneSegdb> seg_db = this_db->get_segdb(segment);
    Xapian::termcount dl = seg_db->get_doclength(c_did, field);

    return dl;
    //RETURN(dl);
}

PostList *
LuceneMultiPostList::next(double data) {
    LOGCALL(API, PostList *, "LuceneMultiPostList::next", term
                | pls_index | pls.size() | data | (unsigned int)this);

    if (pls_index >= pls.size()) {
        return NULL;
    }

    for (; pls_index < pls.size(); ++pls_index) {
        LucenePostList * postlist = pls[pls_index];
        postlist->next(data);
        if (! postlist->at_end()) {
            c_did = postlist->get_docid();
            break;
        }
    }

    RETURN(NULL);
}

bool
LuceneMultiPostList::at_end() const {
    //LOGCALL(API, bool, "LuceneMultiPostList::at_end", NO_ARGS);

    if (pls_index >= pls.size()) {
        return true;
        //RETURN(true);
    }

    return false;
    //RETURN(false);
}

string
LuceneMultiPostList::get_description() const {
    return string();
}

PostList *
LuceneMultiPostList::skip_to(Xapian::docid ext_did, double data) {
    LOGCALL(API, PostList *, "LuceneMultiPostList::skip_to", ext_did | data);

    /* FIXME, if next() is not called before, get_docid() will get a unknown result */
    //No need to find in postlist
    Xapian::docid c_ext_did = get_docid();
    if (ext_did <= c_ext_did || at_end())
        RETURN(NULL);

    //Convert ext_did to within segment did, and get segment index
    unsigned int seg_idx = 0;
    Xapian::docid seg_did = this_db->get_seg_docid(ext_did, seg_idx);

    for (; pls_index < pls.size(); pls_index++) {
        if (seg_idx == pls[pls_index]->get_seg_idx()) {
            break;
        }
    }

    if (at_end()) {
        //Xapian::docid is unsigned, but did=0 is available in Lucene
        //c_did = 0;
        RETURN(NULL);
    }

    LOGLINE(API, "LuceneMultiPostList::skip_to, pls_index=" << pls_index <<
                ", seg_idx=" << seg_idx);

    pls[pls_index]->skip_to(seg_did, data);
    //Skip to the first doc which after pls[pls_index]
    if (pls[pls_index]->at_end()) {
        pls_index++;
        //Skip to the first doc after pls[pls_index]
        for (; pls_index < pls.size(); pls_index++) {
            pls[pls_index]->skip_to(0, data);
            if (! pls[pls_index]->at_end()) {
                break;
            }
        }
    }

    if (! at_end()) {
        c_did = pls[pls_index]->get_docid();
    }

    LOGLINE(API, "LuceneMultiPostList::skip_to, c_did=" << c_did);

    return NULL;
}

/*
PostList *
LuceneMultiPostList::check(Xapian::docid ext_did, double w_min,
            bool & valid) {
    LOGCALL(API, PostList *, "LuceneMultiPostList::check", ext_did | w_min
                | valid);

    valid = false;
    skip_to(ext_did, w_min);
    if (at_end()) {
        valid = true;
        RETURN(NULL);
    }

    Xapian::docid new_did = get_docid();
    if (ext_did == new_did)
      valid = true;

    RETURN(NULL);
}
*/

Xapian::termcount
LuceneMultiPostList::get_wdf() const {
    //LOGCALL(API, Xapian::termcount, "LuceneMultiPostList::get_wdf", NO_ARGS);

    LucenePostList * postlist = pls[pls_index];

    return postlist->get_wdf();
    //RETURN(postlist->get_wdf());
}

/**
 * below is for debug
 */
void
LucenePostList::debug_postlist() const {
    //SkipDelta is only stored if DocFreq is not smaller than SkipInterval
    cout << "ftetll:" << docfreq_reader.get_ftell() << 
        " freq_delta:" << freq_delta <<
        " skip_delta:" << skip_delta << 
        " doc_freq:" << doc_freq << endl;
    cout << "LucenePostList::debug_postlist->";
    for (int i = 0; i < doc_freq; ++i) {
        int doc_delta = 0;
        int freq = 0;
        docfreq_reader.read_did_and_freq(doc_delta, freq);
        cout << "doc_delta[" << doc_delta <<
            "],freq[" << freq << "],";
    }

    cout << endl;
}
