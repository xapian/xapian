
#ifndef XAPIAN_INCLUDED_LUCENE_STATTABLE_H
#define XAPIAN_INCLUDED_LUCENE_STATTABLE_H

//xapian.h must be included, in order to use Xapian namespace
#include <xapian.h>

#include <fstream>

class LuceneStatTable {
    Xapian::termcount wdf_upper_bound;

    //Database directory
    std::string db_dir;

    //Use file stream to read data
    std::ifstream fin;

  public:
    LuceneStatTable(const std::string & db_dir_);

    Xapian::termcount get_wdf_upper_bound() const;

    //debug this table
    void debug_get_table() const;
};

#endif
