
#include <config.h>

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
        stream_reader(db_dir_),
        ti_version(0),
        term_count(0),
        index_interval(0),
        skip_interval(0),
        max_skip_levels(0),
        counter(0)
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
    LOGCALL(DB, bool, "LuceneTisTable::open", NO_ARGS);

    stream_reader.open_stream();

    stream_reader.read_int32(ti_version);
    stream_reader.read_int64(term_count);
    stream_reader.read_int32(index_interval);
    stream_reader.read_int32(skip_interval);
    stream_reader.read_int32(max_skip_levels);

    return true;
}

bool
LuceneTisTable::scan_to(const LuceneTerm & target, LuceneTermInfo & result,
            const LuceneTermIndice & prev) const {
    //LOGCALL(API, bool, "LuceneTisTable::scan_to", target.suffix);
    LuceneTermInfo c;
    LuceneTermInfo p = prev.terminfo;
    stream_reader.seek_to(prev.index_delta);

    /** freq_delta in for loop is offset based on p_freq_delta, it's not absolute
     * data
     */
    int p_freq_delta = prev.terminfo.freq_delta;

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
        
        int r = target.compare(t);
        //find it
        if (0 == r) {
            result = c;
            return true;
        }

        p = c;
    }

    return false;
}

void
LuceneTisTable::next()
{
    LOGCALL(DB, void, "LuceneTisTable::next", NO_ARGS);

    LuceneTermInfo cti;
    stream_reader.read_terminfo(cti, skip_interval);
    cti.freq_delta += pti.freq_delta;
    LuceneTerm & t = cti.term;
    if (0 != cti.term.prefix_length) {
        t.suffix = pti.term.suffix.substr(0, t.prefix_length) + t.suffix;
    }

    pti = cti;
    ++counter;
}

bool
LuceneTisTable::at_end() const
{
    if (counter >= term_count)
        return true;

    return false;
}

LuceneTermInfo
LuceneTisTable::get_current_ti() const
{
    return pti;
}

/** below is for debug
 */
void
LuceneTisTable::debug_get_table() {
    cout << "tis-->TIVersion[" << ti_version << "],TermCount[" << 
        term_count << "],SkipInterval[" << skip_interval << "],MaxSkipInterval[" <<
        max_skip_levels << "]" << endl;
}
