// AND of two posting lists

#ifndef _andpostlist_h_
#define _andpostlist_h_

#include "database.h"

class AndPostList : public virtual PostList {
    protected:
        PostList *l, *r;
    private:
        docid head;

        void advance_to_next_match();
    public:
	doccount get_termfreq() const;

	docid  get_docid() const;
	weight get_weight() const;

	void   next();
	void   skip_to(docid);
	bool   at_end() const;

        AndPostList(PostList *l, PostList *r);
        ~AndPostList();
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
    return head;
}

// only called if we are doing a probabilistic AND
inline weight
AndPostList::get_weight() const
{
    return l->get_weight() + r->get_weight();
}

inline bool
AndPostList::at_end() const
{
    return head == 0;
}

#endif /* _andpostlist_h_ */
