// boolean OR of two posting lists

#ifndef _orpostlist_h_
#define _orpostlist_h_

#include "database.h"

class OrPostList : public virtual PostList {
    protected:
        PostList *l, *r;
        docid lhead, rhead;
    public:
	doccount get_termfreq() const;

	docid  get_docid() const;
	weight get_weight() const;

	void   next();
	void   skip_to(docid);
	bool   at_end() const;

        OrPostList(PostList *, PostList *);
        ~OrPostList();
};

inline doccount
OrPostList::get_termfreq() const
{
    // this is actually the maximum possible frequency for the union of
    // the terms
    return l->get_termfreq() + r->get_termfreq();
}

inline docid
OrPostList::get_docid() const
{
    if (!rhead) {
        return lhead;
    } else if (!lhead) {
        return rhead;
    }
   
    return min(lhead, rhead);
}

inline weight
OrPostList::get_weight() const
{
    return 1;
}

inline bool
OrPostList::at_end() const
{
    return lhead == 0 && rhead == 0;
}

#endif /* _orpostlist_h_ */
