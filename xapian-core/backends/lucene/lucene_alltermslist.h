
#ifndef XAPIAN_INCLUDED_LUCENE_ALLTERMSLIST_H
#define XAPIAN_INCLUDED_LUCENE_ALLTERMSLIST_H

#include "backends/alltermslist.h"
#include "lucene_postiterator.h"

/** This class is used to visit all terms in Lucene database
 */
class LuceneAllTermsList : public AllTermsList {
    /** Coping is not allowed
     */
    LuceneAllTermsList(const LuceneAllTermsList &);

    /** Assignment is not allowed
     */
    void operator=(const LuceneAllTermsList &);

    Xapian::Internal::intrusive_ptr<const LuceneDatabase> this_db;

    /** Array Index to current segment database
     */
    unsigned int seg_idx;

    /** Current segment database
     */
    Xapian::Internal::intrusive_ptr<LuceneSegdb> seg_db;

    /** Current term information
     */
    LuceneTermInfo ti;

    /** If reaches the end
     */
    bool is_at_end;

    /** Term prefix, used to concat the term
     */
    std::string prefix;

  public:
    LuceneAllTermsList(Xapian::Internal::intrusive_ptr<const LuceneDatabase> this_db_,
                const std::string & prefix); 

    /** Move to the next term
     */
    TermList * next();

    bool at_end() const;

    /** Get the postlist for current term
     */
    Xapian::PostingIterator postlist_begin() const;

    std::string get_termname() const;

    /** No used
     */
    Xapian::doccount get_termfreq() const;

    /** No used
     */
    Xapian::termcount get_collection_freq() const;

    /** No used
     */
    TermList * skip_to(const string &);
};

#endif
