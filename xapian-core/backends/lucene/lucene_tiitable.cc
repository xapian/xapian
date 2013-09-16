
#include <config.h>

#include "lucene_tiitable.h"

#include "debuglog.h"

#include <iostream>

using namespace std;

LuceneTiiTable::LuceneTiiTable(const string & db_dir_)
        : db_dir(db_dir_),
        stream_reader(db_dir)
{
    LOGCALL_CTOR(DB, "LuceneTiiTable", db_dir);
    file_name = "";
}

bool
LuceneTiiTable::set_filename(const string & prefix) {
    /* Has fixed suffix name '.tii' */
    file_name = prefix + ".tii";
    stream_reader.set_filename(file_name);

    return true;
}

/** TermInfoIndex (.tii)--> TIVersion, IndexTermCount, IndexInterval, SkipInterval, MaxSkipLevels, TermIndices
 * TIVersion --> UInt32
 * IndexTermCount --> UInt64
 * IndexInterval --> UInt32
 * SkipInterval --> UInt32
 * TermIndices --> <TermInfo, IndexDelta> IndexTermCount
 * IndexDelta --> VLong
 *
 * More details on http://lucene.apache.org/core/3_6_2/fileformats.html#tii
 */
bool
LuceneTiiTable::open() {
    LOGCALL(DB, bool, "LuceneTiiTable::open", NO_ARGS);

    stream_reader.open_stream();

    /* Just a version number */
    stream_reader.read_uint32(ti_version);

    /* How many terms in .tii file */
    stream_reader.read_uint64(index_term_count);
    stream_reader.read_uint32(index_interval);

    /** SkipInterval is the fraction of TermDocs stored in skip tables. It is 
     * used to accelerate TermDocs.skipTo(int). Larger values result in smaller
     * indexes, greater acceleration, but fewer accelerable cases, while smaller
     * values result in bigger indexes, less acceleration (in case of a small
     * value for MaxSkipLevels) and more accelerable cases.
     */
    stream_reader.read_uint32(skip_interval);

    /** MaxSkipLevels is the max. number of skip levels stored for each term in
     * the .frq file. A low value results in smaller indexes but less acceleration,
     * a larger value results in slighly larger indexes but greater acceleration.
     * See format of .frq file for more information about skip levels.
     */
    stream_reader.read_uint32(max_skip_levels);

    //FIXME this code looks bad
    LuceneTerm p_term;
    long long p_index_delta = 0;
    int p_freq_delta = 0;
    LuceneTermIndice indice;
    for (unsigned long long i = 0; i < index_term_count; ++i) {
        stream_reader.read_terminfo(indice.terminfo, skip_interval);
        stream_reader.read_vint64(indice.index_delta);
        /* Caculate indice.index_delta, index_delta is not absolute data */
        indice.index_delta += p_index_delta;

        /* So do freq_delta, it's not absolute data */
        indice.terminfo.freq_delta += p_freq_delta;

        /** Change prefix string to unprefix string, in order to binary search in memery.
         * So here, term.suffix means the whole string, not suffix
         * */
        LuceneTerm & c_term = indice.terminfo.term;
        int pl = c_term.prefix_length;
        /* Has prefix, concate string */
        if (pl >= 0) {
            string & prev_string = p_term.suffix;
            string prefix = prev_string.substr(0, pl);
            c_term.suffix = prefix + c_term.suffix;
        }
        term_indices.push_back(indice);

        p_term = c_term;
        p_index_delta = indice.index_delta;
        p_freq_delta = indice.terminfo.freq_delta;
    }

    return true;
}

bool
LuceneTiiTable::set_field_name(vector<string> field_name_) {
    field_name = field_name_;

    return true;
}

/* The whole list in .tii is read in memery, so do binary search */
int
LuceneTiiTable::get_index_offset(const LuceneTerm & term) const {
    LOGCALL(API, int, "LuceneTiiTable::get_index_offset", term.suffix);

    //Binary search
    int lo = 0;
    int hi = term_indices.size();
    int mid = 0;
    int delta = 0;
    while (hi >= lo) {
        mid = (lo + hi) >> 1;
        const LuceneTerm & t = term_indices[mid].terminfo.term;
        delta = term.compare(t);
        if (delta < 0)
          hi = mid - 1;
        else if (delta > 0)
          lo = mid + 1;
        else
          return mid;
    }

    return hi;
}

const LuceneTermIndice &
LuceneTiiTable::get_term_indice(int idx) const {
    return term_indices[idx];
}

//below for debug
void
LuceneTiiTable::debug_table() {
    cout << file_name << ".tii table-->" << "TIVersion(" << ti_version << "),IndexTermCount:(" <<
        index_term_count << "),IndexInterval:(" << index_interval << 
        "),SkipInterval(" << skip_interval << "),MaxSkipLevels(" <<
        max_skip_levels << "),TermIndices:<" << endl;

    vector<LuceneTermIndice>::iterator it = term_indices.begin();
    for (; it != term_indices.end(); ++it) {
        LuceneTermInfo & ti = (*it).terminfo;
        LuceneTerm & t = ti.term;
        cout << "Term[PrefixLength(" << t.prefix_length << "),suffix(" <<
            t.suffix << "),FieldNum(" << t.field_num << "],DocFreq(" <<
            ti.doc_freq << "),FreqDelta(" << ti.freq_delta << "),ProxDelta(" <<
            ti.prox_delta << "),SkipDelta(" << ti.skip_delta << "],IndexDelta(" <<
            (*it).index_delta << ")" << endl;
    }

    return ;
}
