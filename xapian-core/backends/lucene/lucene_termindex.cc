
#include "lucene_termindex.h"
//must include this file to use LOGCALL()
#include "config.h"
#include "debuglog.h"

#include <iostream>

/**
 * below include is for debug
 */
#include <cstdlib>

using namespace std;

LuceneTermIndex::LuceneTermIndex(const string & db_dir_)
        : tii_table(db_dir_),
        fnm_table(db_dir_),
        tis_table(db_dir_)
{
}

bool
LuceneTermIndex::set_filename(string prefix) {
    tii_table.set_filename(prefix);
    fnm_table.set_filename(prefix);
    tis_table.set_filename(prefix);

    return true;
}

bool
LuceneTermIndex::create_and_open_tables() {
    tii_table.open();
    //tii_table.debug_table();
    fnm_table.open();
    vector<string> field_name = fnm_table.get_field_name();
    tii_table.set_field_name(field_name);
    //TODO
    tis_table.open();
    tis_table.set_field_name(field_name);

    //for debug
    tis_table.debug_get_table();

    return true;
}

bool
LuceneTermIndex::seek(const LuceneTerm & lterm, LuceneTermInfo & result) const {
    /** .tii seems a skip list for .tis, the whole list in .tii is read in memery.
     * Here using binary search to find the index for more information
     * in .tis
     */
    int idx = tii_table.get_index_offset(lterm);

    /* Here suffix really means the whole term name */
    LOGCALL(API, bool, "LuceneTermIndex::seek", lterm.get_suffix());

    //cout << "LuceneTermIndex::seek, idx=" << idx << endl;
    const LuceneTermIndice & term_indice = tii_table.get_term_indice(idx);
    //term_indice.debug_term_indice();

    /** Firstly, fseek to the right place in .tis. Secondly, do a sequence search
     * to find the term.
     * FIXME, skip list supported in .tis, but not used yet here
     */
    bool b = tis_table.scan_to(lterm, result, term_indice);

    RETURN(b);
}

int
LuceneTermIndex::get_docfreq(const LuceneTerm & lterm) const {
    LOGCALL(API, int, "LuceneTermIndex::get_docfreq", lterm.get_suffix());

    //FIXME, search directly every time
    LuceneTermInfo terminfo;
    if (false == seek(lterm, terminfo)) {
        return 0;
    }

    RETURN(terminfo.get_docfreq());
}

