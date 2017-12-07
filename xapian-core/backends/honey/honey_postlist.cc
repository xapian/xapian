
#include <config.h>

#include "honey_postlist.h"

Xapian::doccount
HoneyPostList::get_termfreq() const
{
    return 0;
}

Xapian::doccount
HoneyPostList::get_termfreq_min() const
{
    return 0;
}

Xapian::doccount
HoneyPostList::get_termfreq_max() const
{
    return 0;
}

Xapian::doccount
HoneyPostList::get_termfreq_est() const
{
    return 0;
}

double
HoneyPostList::get_weight() const
{
    return 0;
}

double
HoneyPostList::recalc_maxweight()
{
    return 0;
}

TermFreqs
HoneyPostList::get_termfreq_est_using_stats(
    const Xapian::Weight::Internal& stats) const
{
    (void)stats;
    return TermFreqs();
}

Xapian::termcount
HoneyPostList::count_matching_subqs() const
{
    return 0;
}

void
HoneyPostList::gather_position_lists(OrPositionList* orposlist)
{
    (void)orposlist;
}

LeafPostList*
HoneyPostList::open_nearby_postlist(const std::string & term_) const
{
    (void)term_;
    return NULL;
}
