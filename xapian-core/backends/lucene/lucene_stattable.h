
#ifndef XAPIAN_INCLUDED_LUCENE_STATTABLE_H
#define XAPIAN_INCLUDED_LUCENE_STATTABLE_H

#include <xapian.h>

#include <fstream>

/** This clsss is for file stat.xapian, which includes statistics
 *  datas for Xapian, these datas are calculated in
 *  lucene-copydatabase.cc.
 *  Now, only wdf_upper_bound is in this file.
 */
class LuceneStatTable {
    Xapian::termcount wdf_upper_bound;

    /** Database directory
     */
    std::string db_dir;

    /** Use file stream to read data
     */
    std::ifstream fin;

  public:
    LuceneStatTable(const std::string & db_dir_);

    Xapian::termcount get_wdf_upper_bound() const;

    /** Debug this table
     */
    void debug_get_table() const;
};

#endif
