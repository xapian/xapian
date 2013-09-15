
#ifndef XAPIAN_INCLUDED_LUCENE_TERM_H
#define XAPIAN_INCLUDED_LUCENE_TERM_H

#include <string>

using namespace std;

/**
 * Lucene Term, include three properties,
 * 1. Prefix length
 * 2. Term suffix
 * 3. Field number which the term belongs to
 *
 * See details on http://lucene.apache.org/core/3_6_2/fileformats.html
 */
class LuceneTerm {
    friend class ByteStreamReader;
    friend class LuceneTermInfo;
    friend class LuceneTiiTable;
    friend class LuceneTisTable;

    /* begin this line is for debug */
    friend class LuceneTermIndice;
    /* end this line is for debug */

    /**
     * Term prefix length, more details http://lucene.apache.org/core/3_6_2/fileformats.html
     * Lucene writes strings as UTF-8 encoded bytes. First the length, in bytes,
     * is written as a VInt, followed by the bytes.
     * string --> VInt, Chars
     **/
    int prefix_length;

    /**
     * Term content, sometimes it's term suffix, but other time it's the whole term content,
     * so the name 'suffix' is not quit suitable here, FIXME
     */
    string suffix;

    /**
     * Lucene Term has two properties, @1 is term name, @2 is field number
     */
    int field_num;

  public:
    LuceneTerm()
        : prefix_length(0), suffix(string()), field_num(-1) {}

    /**
     * Term compare, using term name(suffix) and field number(field_num)
     */
    int compare(const LuceneTerm &) const;

    /* Return suffix */
    string get_suffix() const;

    /* Return field_num */
    int get_field_num() const;

    //just for debug
    void set_field_num(int);

    /* Set suffix */
    void set_suffix(const string &);
};

/**
 * More information about one Lucene term.
 *
 * See details on http://lucene.apache.org/core/3_6_2/fileformats.html
 */
class LuceneTermInfo {
    friend class ByteStreamReader;
    friend class LuceneTermIndice;
    friend class LuceneTiiTable;
    friend class LuceneTisTable;

    /* Lucene term */
    LuceneTerm term;

    /* Document frequency. It means how many document contains this term */
    int doc_freq;

    /**
     * File offset in .frq file, which contains the list of docs of this term
     * along with frequency(within document frequency, wdf)
     * Relative position or absolute position?
     **/
    int freq_delta;

    /**
     * File offset in .prx file, which stores position information about where
     * this term occurs in the index
     */
    int prox_delta;

    int skip_delta;

  public:
    LuceneTermInfo()
        : doc_freq(0), freq_delta(0), prox_delta(0), skip_delta(0) {}

    int get_docfreq() const;
    int get_freqdelta() const;
    int get_proxdelta() const;
    int get_skipdelta() const;
    LuceneTerm get_term() const;

    /**
     * below is for debug
     */
    void debug_term_info() const;
};

class LuceneTermIndice {
    friend class LuceneTiiTable;
    friend class LuceneTisTable;

    LuceneTermInfo terminfo;
    long long index_delta;

    /**
     * below is for debug
     */
  public:
    void debug_term_indice() const;
};

#endif
