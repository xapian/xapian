
#ifndef XAPIAN_INCLUDED_LUCENE_ALLTERMSLIST_H
#define XAPIAN_INCLUDED_LUCENE_ALLTERMSLIST_H

#include "backends/alltermslist.h"
#include "lucene_postiterator.h"

class LuceneAllTermsList : public AllTermsList {
    //Coping is not allowed
    LuceneAllTermsList(const LuceneAllTermsList &);

    //Assignment is not allowed
    void operator=(const LuceneAllTermsList &);

    Xapian::Internal::intrusive_ptr<const LuceneDatabase> this_db;

    unsigned int seg_idx;

    Xapian::Internal::intrusive_ptr<LuceneSegdb> seg_db;

    LuceneTermInfo ti;

    bool is_at_end;

    std::string prefix;

  public:
    LuceneAllTermsList(Xapian::Internal::intrusive_ptr<const LuceneDatabase> this_db_,
                const std::string & prefix); 

    TermList * next();

    bool at_end() const;

    Xapian::PostingIterator postlist_begin() const;

    std::string get_termname() const;

    /* no used */
    Xapian::doccount  get_termfreq() const;

    /* no used */
    Xapian::termcount get_collection_freq() const;

    /* no used */
    TermList * skip_to(const string &);
};

#endif
