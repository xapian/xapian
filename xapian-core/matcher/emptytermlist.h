// empty term list (for degenerated trees)

#ifndef _emptytermlist_h_
#define _emptytermlist_h_

#include "termlist.h"

class EmptyTermList : public virtual TermList {
    public:
	termcount get_approx_size() const;
	weight get_weight() const;
	termid get_termid() const;
	termcount get_wdf() const;
	doccount get_termfreq() const;

	TermList *next();
	bool      at_end() const;
};

inline termcount
EmptyTermList::get_approx_size() const
{
    return 0;
}

inline weight
EmptyTermList::get_weight() const
{
    Assert(0); // no terms
    return 0;
}

inline termid
EmptyTermList::get_termid() const
{
    Assert(0); // no terms
    return 0;
}

inline termcount
EmptyTermList::get_wdf() const
{
    Assert(0); // no terms
    return 0;
}

inline doccount
EmptyTermList::get_termfreq() const
{
    Assert(0); // no terms
    return 0;
}

inline TermList *
EmptyTermList::next()
{
    return NULL;
}

inline bool
EmptyTermList::at_end() const
{
    return true;
}

#endif /* _emptytermlist_h_ */
