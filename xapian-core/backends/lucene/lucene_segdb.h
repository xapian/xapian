#ifndef OM_HGUARD_LUCENE_SEGMENT_DATABASE_H
#define OM_HGUARD_LUCENE_SEGMENT_DATABASE_H

#include "backends/valuestats.h"
#include "internaltypes.h"
#include "xapian/intrusive_ptr.h"
#include "noreturn.h"
#include "lucene_termindex.h"
#include "lucene_frqtable.h"
#include "lucene_fdtxtable.h"

using namespace std;

class LuceneSegdb : public Xapian::Internal::intrusive_base {
    string db_dir;
    /* File name prefix, such as _0, _1 */
    string prefix;

    /* For .tii and .tis */
    LuceneTermIndex index_reader;

    /* For .frq */
    LuceneFrqTable frq_table;

    /* For .fdt and .fdx */
    LuceneFdtxTable fdtx_table;

    /* For .fnm */
    LuceneFnmTable fnm_table;

  public:
    LuceneSegdb(const string &, const string &);

    /* Set all tables' name in this segment */
    bool set_filename();

    /* Open all tables in this segment */
    bool create_and_open_tables();

    /**
     * Get termfreq for a term.
     * Termfreq here means how many document contains this term.
     * Xapian's termfreq = Lucene's docfreq
     */
    Xapian::doccount get_termfreq(const LuceneTerm &) const;

    /* Open a term's postlist */
    LucenePostList * open_post_list(const LuceneTerm &) const;

    /**
     * Read document record.
     * All the @param maps are used to store document records
     */
    void get_record(Xapian::docid, map<int, string> &, map<int, int> &,
        map<int, long> &, map<int, float> &, map<int, double> &) const;

    /* get field name in this segment */
    void get_fieldinfo(set<string> & field_info) const;

    /** Construct LuceneTerm from query string.
     * Now, Lucene query string must fit this format, field_name:term.
     * For example, user_name:white, this query means I want to seach
     * 'white' in field 'user_name'
     *
     * Actually, this function is used to parse the query, change field
     * name to field number
     */
    void get_luceneterm(const string & str, LuceneTerm & lterm) const;

    /* Find the field number related to @param field */
    int get_fieldnum(const string & field) const;
};

#endif
