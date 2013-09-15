
#include "lucene_term.h"
#include <iostream>

using namespace std;

int
LuceneTerm::compare(const LuceneTerm & term) const {
    if (field_num == term.field_num) {
        return suffix.compare(term.suffix);
    }

    if (field_num > term.field_num) {
        return 1;
    } else {
        return -1;
    }
}

string
LuceneTerm::get_suffix() const {
    return suffix;
}

int
LuceneTerm::get_field_num() const {
    return field_num;
}

int
LuceneTermInfo::get_docfreq() const {
    return doc_freq;
}

int
LuceneTermInfo::get_freqdelta() const {
    return freq_delta;
}

int
LuceneTermInfo::get_proxdelta() const {
    return prox_delta;
}

int
LuceneTermInfo::get_skipdelta() const {
    return skip_delta;
}

LuceneTerm
LuceneTermInfo::get_term() const {
    return term;
}


/**
 * below is for debug
 */
void
LuceneTerm::set_field_num(int field_num_) {
    field_num = field_num_;
}

void
LuceneTerm::set_suffix(const string &text) {
    suffix = text;
}

void
LuceneTermInfo::debug_term_info() const {
    const LuceneTerm & t = term;

    cout << "LuceneTermInfo-->doc_freq[" << doc_freq << "],freq_delta[" <<
        freq_delta << "],prox_delta[" << prox_delta << "],skip_delta[" <<
        skip_delta << "],suffix[" << t.suffix << "],field_num[" <<
        t.field_num << "]" << endl;
}

void
LuceneTermIndice::debug_term_indice() const {
    const LuceneTermInfo & ti = terminfo;
    const LuceneTerm & t = ti.term;

    cout << "LuceneTermIndice-->doc_freq[" << ti.doc_freq << "],freq_delta[" <<
        ti.freq_delta << "],prox_delta[" << ti.prox_delta << "], skip_delta[" <<                                                  
        ti.skip_delta << "],suffix[" << t.suffix << "],field_num[" <<   
        t.field_num << "],index_delta[" << index_delta << "]" << endl;
}

