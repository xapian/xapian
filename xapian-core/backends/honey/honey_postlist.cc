
#include <config.h>

#include "honey_postlist.h"

Xapian::doccount
HoneyPostList::get_termfreq() const
{
    return 0;
}

LeafPostList*
HoneyPostList::open_nearby_postlist(const std::string & term_) const
{
    (void)term_;
    return NULL;
}
