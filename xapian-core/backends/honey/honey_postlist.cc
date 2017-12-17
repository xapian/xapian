
#include <config.h>

#include "honey_postlist.h"

#include <string>

using namespace std;

Xapian::doccount
HoneyPostList::get_termfreq() const
{
    return 0; // TODO0
}

LeafPostList*
HoneyPostList::open_nearby_postlist(const std::string& term_) const
{
    (void)term_;
    return NULL; // TODO1
}

Xapian::docid
HoneyPostList::get_docid() const
{
    return 0; // TODO0
}

Xapian::termcount
HoneyPostList::get_wdf() const
{
    return 0; // TODO0
}

bool
HoneyPostList::at_end() const
{
    return true; // TODO0
}

PositionList*
HoneyPostList::read_position_list()
{
    return 0; // TODO1
}

PositionList*
HoneyPostList::open_position_list() const
{
    return 0; // TODO1
}

PostList*
HoneyPostList::next(double w_min)
{
    (void)w_min;
    return 0; // TODO0
}

PostList*
HoneyPostList::skip_to(Xapian::docid did, double w_min)
{
    (void)did;
    (void)w_min;
    return 0; // TODO0
}

PostList*
HoneyPostList::check(Xapian::docid did, double w_min, bool& valid)
{
    (void)did;
    (void)w_min;
    (void)valid;
    return 0; // TODO0
}

std::string
HoneyPostList::get_description() const
{
    string desc = "HoneyPostList(";
    desc += term;
    desc += ')';
    return desc;
}
