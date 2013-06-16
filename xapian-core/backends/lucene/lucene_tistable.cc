
#include "lucene_tistable.h"
#include "debuglog.h"
#include <iostream>

/**
 * below include is for debug
 */
#include <cstdlib>

using namespace std;

LuceneTisTable::LuceneTisTable(const string & db_dir_)
        : db_dir(db_dir_),
        stream_reader(db_dir_)
{
}

bool
LuceneTisTable::set_filename(const string & prefix) {
    /* Has fixed file name suffix '.tis' */
    file_name = prefix + ".tis";
    stream_reader.set_filename(file_name);

    return true;
}

bool
LuceneTisTable::set_field_name(vector<string> field_name_) {
    field_name = field_name_;

    return true;
}

bool
LuceneTisTable::open() {
    cout << "LuceneTisTable::open"  << endl;

    stream_reader.open_stream();

    stream_reader.read_int32(ti_version);
    cout << "LuceneTisTable::open ti_version:" << ti_version << endl;
    stream_reader.read_int64(term_count);
    cout << "LuceneTisTable::open term_count:" << term_count << endl;
    stream_reader.read_int32(index_interval);
    cout << "LuceneTisTable::open index_interval:" << index_interval << endl;
    stream_reader.read_int32(skip_interval);
    cout << "LuceneTisTable::open skip_interval:" << skip_interval << endl;
    stream_reader.read_int32(max_skip_levels);
    cout << "LuceneTisTable::open max_skip_levels:" << max_skip_levels << endl;

    return true;
}

bool
LuceneTisTable::scan_to(const LuceneTerm & target, LuceneTermInfo & result,
            const LuceneTermIndice & prev) const {
    LOGCALL(API, bool, "LuceneTisTable::scan_to", target.suffix);
    LuceneTermInfo c;
    LuceneTermInfo p = prev.terminfo;
    stream_reader.seek_to(prev.index_delta);

    /** freq_delta in for loop is offset based on p_freq_delta, it's not absolute
     * data
     */
    int p_freq_delta = prev.terminfo.freq_delta;

    cout << "get_ftell=" << stream_reader.get_ftell() << 
        ", skip_interval=" << skip_interval << 
        ", index_interval=" << index_interval << endl;
    for (int i = 0; i < index_interval; ++i) {
        stream_reader.read_terminfo(c, skip_interval);

        /** c.freq_delta is offset based on p_freq_delta, it's the file offset int
         * .frq
         */
        c.freq_delta += p_freq_delta;
        p_freq_delta = c.freq_delta;

        LuceneTerm & t = c.term;
        //FIXME this code looks bad
        /* has prefix, concat it */
        if (0 != t.prefix_length) {
            string prefix = p.term.suffix.substr(0, t.prefix_length);
            t.suffix = prefix + t.suffix;
        }
        
        //c.debug_term_info();

        int r = target.compare(t);
        //find it
        if (0 == r) {
            result = c;
            RETURN(true);
        }

        p = c;
    }

    RETURN(false);
}

/**
 * below is for debug
 */
void
LuceneTisTable::debug_get_table() {
    cout << "tis-->TIVersion[" << ti_version << "],TermCount[" << 
        term_count << "],SkipInterval[" << skip_interval << "],MaxSkipInterval[" <<
        max_skip_levels << "]" << endl;
}
