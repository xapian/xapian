// AND NOT of two posting lists

#ifndef _andnotpostlist_h_
#define _andnotpostlist_h_

#include "database.h"
#include "branchpostlist.h"

class AndNotPostList : public virtual BranchPostList {
    private:
        docid lhead, rhead;
        PostList *advance_to_next_match();
    public:
	doccount get_termfreq() const;

	docid  get_docid() const;
	weight get_weight() const;

        PostList *next();
        PostList *skip_to(docid);
	bool   at_end() const;

        AndNotPostList(PostList *l, PostList *r);
};

inline doccount
AndNotPostList::get_termfreq() const
{
    // this is actually the maximum possible frequency
    return l->get_termfreq();
}

inline docid
AndNotPostList::get_docid() const
{
    return lhead;
}

// only called if we are doing a probabilistic AND NOT
inline weight
AndNotPostList::get_weight() const
{
    return l->get_weight();
}

inline bool
AndNotPostList::at_end() const
{
    return lhead == 0;
}

#endif /* _andnotpostlist_h_ */
