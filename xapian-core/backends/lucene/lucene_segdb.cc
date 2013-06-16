
#include <config.h>
#include <xapian/error.h>
#include <xapian/valueiterator.h>
#include <omassert.h>
#include "debuglog.h"
#include <sys/types.h>
#include "lucene_segdb.h"

using namespace Xapian;

LuceneSegdb::LuceneSegdb(const string & db_dir_, const string & prefix_)
        : db_dir(db_dir_),
        prefix(prefix_),
        index_reader(db_dir),
        frq_table(db_dir),
        fdtx_table(db_dir),
        fnm_table(db_dir)
{
    LOGCALL_CTOR(DB, "LuceneSegdb", db_dir_);

    set_filename();
}

bool
LuceneSegdb::set_filename() {
    LOGCALL(DB, bool, "LuceneSegdb::set_filename", NO_ARGS);

    index_reader.set_filename(prefix);
    frq_table.set_filename(prefix);
    fdtx_table.set_filename(prefix);
    fnm_table.set_filename(prefix);

    return true;
}

bool
LuceneSegdb::create_and_open_tables() {
    /* Changing function name to open() is better */
    index_reader.create_and_open_tables();
    fdtx_table.open();
    fnm_table.open();

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

    RETURN(new LucenePostList(lterm.get_suffix(), term_info.get_docfreq(),
                    term_info.get_freqdelta(), term_info.get_skipdelta(),
                    frq_table.get_dbdir(), frq_table.get_filename()));
}

void
LuceneSegdb::get_record(Xapian::docid did, map<int, string> & string_map,
            map<int, int> & int_map, map<int, long> & long_map,
            map<int, float> & float_map, map<int, double> & double_map) const {
    LOGCALL(DB, string, "LuceneSegdb::get_record", did);

    fdtx_table.get_record(did, string_map, int_map, long_map, float_map,
        double_map);
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
LuceneSegdb::get_luceneterm(const string & str, LuceneTerm & lterm) const {
    LOGCALL(DB, void, "LuceneSegdb::get_luceneterm", str);

    /* Query format must fit field_name:term */
    size_t pos = str.find(":");
    Assert(pos != string::npos);

    string field = str.substr(0, pos);
    lterm.set_suffix(str.substr(pos + 1));

    map<string, int>::const_iterator it;
    it = fnm_table.field_map.find(field);

    /* No match, query string format is wrong, or no related field, abort() */
    if (it == fnm_table.field_map.end()) {
        throw Xapian::DatabaseError("LuceneSegdb::get_luceneterm, Query string's"\
                    "prefix not match .fnm field name");
    }

    lterm.set_field_num(it->second);
    LOGLINE(DB, "LuceneSegdb::get_luceneterm, field=" << field <<", name=" <<
                lterm.get_suffix() << ", fn=" << it->second);
}

int
LuceneSegdb::get_fieldnum(const string & field) const {
    LOGCALL(DB, int, "LuceneSegdb::get_fieldnum", field);

    map<string, int>::const_iterator it = fnm_table.field_map.find(field);
    if (it == fnm_table.field_map.end()) {
        throw Xapian::DatabaseError("LuceneSegdb::get_filednum, field is not"\
                    "match .fnm field name");
    }

    RETURN(it->second);
}
