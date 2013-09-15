
#include <config.h>
#include "debuglog.h"
#include <xapian/error.h>
#include <xapian/valueiterator.h>
#include <omassert.h>
#include <sys/types.h>
#include "lucene_segdb.h"

using namespace Xapian;
using Xapian::Internal::intrusive_ptr;

LuceneSegdb::LuceneSegdb(const string & db_dir_,
            intrusive_ptr<LuceneSegmentPart> seg_part_)
        : db_dir(db_dir_),
        prefix(""),
        index_reader(db_dir),
        frq_table(db_dir),
        fdtx_table(db_dir),
        fnm_table(db_dir),
        nrm_table(db_dir),
        seg_part(seg_part_)
{
    LOGCALL_CTOR(DB, "LuceneSegdb", db_dir_);
}

bool
LuceneSegdb::set_filename() {
    LOGCALL(DB, bool, "LuceneSegdb::set_filename", NO_ARGS);

    index_reader.set_filename(prefix);
    frq_table.set_filename(prefix);
    fdtx_table.set_filename(prefix);
    fnm_table.set_filename(prefix);
    nrm_table.set_filename(prefix);

    return true;
}

bool
LuceneSegdb::create_and_open_tables() {
    prefix = seg_part->get_seg_name();
    set_filename();

    /* Changing function name to open() is better */
    index_reader.create_and_open_tables();
    fdtx_table.open();
    fnm_table.open();
    nrm_table.open();
    nrm_table.set_seg_size(seg_part->get_seg_size());

    fnm_table.debug_get_table();

    return true;
}

Xapian::doccount
LuceneSegdb::get_termfreq(const LuceneTerm & lterm) const
{
    LOGCALL(DB, Xapian::doccount, "LuceneSegdb::get_termfreq",
        lterm.get_suffix() | lterm.get_field_num());

    RETURN(index_reader.get_docfreq(lterm));
}

LucenePostList *
LuceneSegdb::open_post_list(const LuceneTerm & lterm) const
{
    LOGCALL(DB, LucenePostList *, "LuceneSegdb::open_post_list", lterm.get_suffix() |
                lterm.get_field_num() | prefix);

    LuceneTermInfo term_info;
    bool b = index_reader.seek(lterm, term_info);
    if (false == b) {
        LOGLINE(API, "LuceneDatabase::open_post_list index_reader.seek false");
        RETURN(NULL);
    }

    LOGLINE(API, "LuceneDatabase::open_post_list index_reader.seek true");
    //should return a LucenePostList from frq_table, LucenePostList extends LeafPostList?

    RETURN(new LucenePostList(lterm.get_suffix(), lterm.get_field_num(),
                    term_info.get_docfreq(), term_info.get_freqdelta(),
                    term_info.get_skipdelta(), frq_table.get_dbdir(),
                    frq_table.get_filename()));
}

void
LuceneSegdb::get_record(Xapian::docid did, map<int, string> & string_map,
            map<int, int> & int_map, map<int, long> & long_map,
            map<int, float> & float_map, map<int, double> & double_map) const {
    LOGCALL(DB, string, "LuceneSegdb::get_record", did);

    fdtx_table.get_record(did, string_map, int_map, long_map, float_map,
        double_map);
}

LucenePostList *
LuceneSegdb::open_postlist_directly(LuceneTermInfo & term_info) const
{
    LOGCALL(DB, LucenePostList *, "LuceneSegdb::open_postlist_directly",
                term_info.get_term().get_suffix());

    LuceneTerm term = term_info.get_term();

    RETURN(new LucenePostList(term.get_suffix(), term.get_field_num(),
                    term_info.get_docfreq(), term_info.get_freqdelta(),
                    term_info.get_skipdelta(), frq_table.get_dbdir(),
                    frq_table.get_filename()));
}

void
LuceneSegdb::get_fieldinfo(set<string> & field_info) const {
    LOGCALL(DB, void, "LuceneSegdb::get_fieldinfo", field_info.size());

    /* .fnm stores field informations */
    const vector<string> field_name = fnm_table.get_field_name();
    vector<string>::const_iterator it = field_name.begin();
    for (; it != field_name.end(); ++it) {
        field_info.insert(*it);
    }

    return ;
}

void
LuceneSegdb::get_luceneterm(const string & query, LuceneTerm & lterm) const {
    LOGCALL(DB, void, "LuceneSegdb::get_luceneterm", query);

    /* Query format must fit this foramt field_name:term */
    size_t pos = query.find(":");
    Assert(pos != string::npos);

    string field = query.substr(0, pos);
    lterm.set_suffix(query.substr(pos + 1));

    int fn = fnm_table.get_field_num(field);

    /* No match, query string format is wrong, or no related field, abort() */
    if (-1 == fn) {
        throw Xapian::DatabaseError("LuceneSegdb::get_luceneterm, no such field");
    }

    lterm.set_field_num(fn);
    LOGLINE(DB, "LuceneSegdb::get_luceneterm, field=" << field << ", name=" <<
                lterm.get_suffix() << ", fn=" << fn);
}

int
LuceneSegdb::get_fieldnum(const string & field) const {
    LOGCALL(DB, int, "LuceneSegdb::get_fieldnum", field);

    int field_num = fnm_table.get_field_num(field);
    if (-1 == field_num) {
        throw Xapian::DatabaseError("LuceneSegdb::get_filednum, field is not"\
                    "match .fnm field name");
    }

    RETURN(field_num);
}

Xapian::termcount
LuceneSegdb::get_doclength(Xapian::docid did, int field_num) const {
    LOGCALL(DB, Xapian::termcount, "LuceneSegdb::get_doclength", did | field_num);

    float norm = nrm_table.get_norm(did, field_num);

    RETURN(1 / (norm * norm));
}

void
LuceneSegdb::next_term()
{
    index_reader.next_term();
}

bool
LuceneSegdb::at_end() const
{
    return index_reader.at_end();
}

LuceneTermInfo
LuceneSegdb::get_current_ti() const
{
    return index_reader.get_current_ti();
}
