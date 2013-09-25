
#ifndef XAPIAN_INCLUDED_LUCENEPOSTITERATOR_H
#define XAPIAN_INCLUDED_LUCENEPOSTITERATOR_H

#include <xapian.h>

#include "api/postlist.h"
#include "lucene_term.h"
#include "lucene_segdb.h"

/** This class is used for visiting all datas in a particular postlist
 */
class LucenePostIterator: public PostList {
    /** Some datas used for calculating is stored in this_db
     */
    Xapian::Internal::intrusive_ptr<const LuceneDatabase> this_db;

    Xapian::Internal::intrusive_ptr<const LuceneSegdb> seg_db;

    Xapian::Internal::intrusive_ptr<LucenePostList> pl;

    /** Current term's information
     */
    LuceneTermInfo ti;

    /* For calculating external docid
     */
    unsigned int seg_idx;

  public:
    LucenePostIterator(Xapian::Internal::intrusive_ptr<const LuceneDatabase> this_db_,
                Xapian::Internal::intrusive_ptr<const LuceneSegdb> seg_db_,
                const LuceneTermInfo & ti_, unsigned int seg_idx);

    Xapian::termcount get_wdf() const;

    Xapian::docid get_docid() const;

    Xapian::PostingIterator::Internal * next(double);

    bool at_end() const;

    /** No used
     */
    Xapian::doccount get_termfreq_min() const;

    /** No used
     */
    Xapian::doccount get_termfreq_max() const;

    /** No used
     */
    Xapian::doccount get_termfreq_est() const;

    /** No used
     */
    double get_maxweight() const;

    /** No used
     */
    Xapian::termcount get_doclength() const;

    /** No used
     */
    double get_weight() const;

    /** No used
     */
    double recalc_maxweight();

    /** No used
     */
    Xapian::PostingIterator::Internal * skip_to(Xapian::docid, double);

    /** No used
     */
    std::string get_description() const;
};

#endif
