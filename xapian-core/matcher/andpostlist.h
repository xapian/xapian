// boolean AND of two posting lists

#ifndef _andpostlist_h_
#define _andpostlist_h_

#include "database.h"
#include "orpostlist.h"

class AndPostList : public virtual OrPostList {
    private:
        void advance_to_next_match();
    public:
	doccount get_termfreq() const;

	docid  get_docid() const;

	void   next();
	void   skip_to(docid);

        AndPostList(PostList *l, PostList *r);
};

inline doccount
AndPostList::get_termfreq() const
{
    // this is actually the maximum possible frequency for the intersection of
    // the terms
    return min(l->get_termfreq(), r->get_termfreq());
}

inline docid
AndPostList::get_docid() const
{
    return lhead; // lhead and rhead are always equal between method calls
}

#endif /* _andpostlist_h_ */
