// boolean AND NOT of two posting lists

#ifndef _andnotpostlist_h_
#define _andnotpostlist_h_

#include "database.h"
#include "orpostlist.h"

class AndNotPostList : public virtual OrPostList {
    private:
        void advance_to_next_match();
    public:
	doccount get_termfreq() const;

	docid  get_docid() const;

	void   next();
	void   skip_to(docid);
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

inline bool
AndNotPostList::at_end() const
{
    return lhead == 0;
}

#endif /* _andnotpostlist_h_ */
