
#include <config.h>

#include "debuglog.h"
#include "lucene_postiterator.h"
#include "omassert.h"

using Xapian::Internal::intrusive_ptr;

LucenePostIterator::LucenePostIterator(intrusive_ptr<const LuceneDatabase> this_db_,
            intrusive_ptr<const LuceneSegdb> seg_db_,
            const LuceneTermInfo & ti_, unsigned int seg_idx_)
        : this_db(this_db_),
          seg_db(seg_db_),
          ti(ti_),
          seg_idx(seg_idx_)
{
    intrusive_ptr<LucenePostList> postlist(seg_db->open_postlist_directly(ti));
    pl = postlist;
}

Xapian::termcount
LucenePostIterator::get_wdf() const
{
    LOGCALL(DB, Xapian::termcount, "LucenePostIterator::get_wdf", NO_ARGS);

    RETURN(pl->get_wdf());
}

Xapian::docid
LucenePostIterator::get_docid() const
{
    LOGCALL(DB, Xapian::docid, "LucenePostIterator::get_docid", NO_ARGS);
    
    RETURN(this_db->get_ext_docid(pl->get_docid(), seg_idx));
    //RETURN(pl->get_docid());
}


PostList *
LucenePostIterator::next(double d)
{
    LOGCALL(DB, PostList *, "LucenePostIterator::next", NO_ARGS);

    RETURN(pl->next(d));
}

bool
LucenePostIterator::at_end() const
{
    LOGCALL(DB, bool, "lucene_postiterator::at_end", NO_ARGS);

    RETURN(pl->at_end());
}

Xapian::doccount
LucenePostIterator::get_termfreq_min() const
{
    Assert(false);
    return 0;
}

Xapian::doccount
LucenePostIterator::get_termfreq_max() const
{
    Assert(false);
    return 0;
}

Xapian::doccount
LucenePostIterator::get_termfreq_est() const
{
    Assert(false);
    return 0;
}

double
LucenePostIterator::get_maxweight() const
{
    Assert(false);
    return 0;
}

Xapian::termcount
LucenePostIterator::get_doclength() const
{
    Assert(false);
    return 0;
}

double
LucenePostIterator::get_weight() const
{
    Assert(false);
    return 0;
}

double
LucenePostIterator::recalc_maxweight()
{
    Assert(false);
    return 0;
}

Xapian::PostingIterator::Internal *
LucenePostIterator::skip_to(Xapian::docid did, double d)
{
    Assert(false);
    (void)did;
    (void)d;
    return NULL;
}

std::string
LucenePostIterator::get_description() const
{
    Assert(false);
    return std::string();
}
