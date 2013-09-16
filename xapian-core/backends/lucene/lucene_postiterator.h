
#ifndef XAPIAN_INCLUDED_LUCENEPOSTITERATOR_H
#define XAPIAN_INCLUDED_LUCENEPOSTITERATOR_H

#include <xapian.h>

#include "api/postlist.h"
#include "lucene_term.h"
#include "lucene_segdb.h"

//typedef Xapian::PostingIterator::Internal PostList;

class LucenePostIterator: public PostList {
    /* used for caculted external docid */
    Xapian::Internal::intrusive_ptr<const LuceneDatabase> this_db;

    Xapian::Internal::intrusive_ptr<const LuceneSegdb> seg_db;

    Xapian::Internal::intrusive_ptr<LucenePostList> pl;

    LuceneTermInfo ti;

    /* used for caculted external docid */
    unsigned int seg_idx;

  public:
    LucenePostIterator(Xapian::Internal::intrusive_ptr<const LuceneDatabase> this_db_,
                Xapian::Internal::intrusive_ptr<const LuceneSegdb> seg_db_,
                const LuceneTermInfo & ti_, unsigned int seg_idx);

    Xapian::termcount get_wdf() const;

    Xapian::docid get_docid() const;

    Xapian::PostingIterator::Internal * next(double);

    bool at_end() const;

    /* no used */
    Xapian::doccount get_termfreq_min() const;

    /* no used */
    Xapian::doccount get_termfreq_max() const;

    /* no used */
    Xapian::doccount get_termfreq_est() const;

    /* no used */
    double get_maxweight() const;

    /* no used */
    Xapian::termcount get_doclength() const;

    /* no used */
    double get_weight() const;

    /* no used */
    double recalc_maxweight();

    /* no used */
    Xapian::PostingIterator::Internal * skip_to(Xapian::docid, double);

    /* no used */
    std::string get_description() const;
};

#endif
