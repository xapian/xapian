/* multi_termlist.h: C++ class definition for multiple database access */

#ifndef _multi_termlist_h_
#define _multi_termlist_h_

#include "omassert.h"
#include "termlist.h"
#include "database.h"
#include <stdlib.h>
#include <set>
#include <vector>
#include <list>

class MultiTermList : public virtual DBTermList {
    friend class MultiDatabase;
    private:
	TermList *tl;
	const IRDatabase *termdb;
	const IRDatabase *rootdb;
	double termfreq_factor;

	MultiTermList(TermList *tl,
		      const IRDatabase *termdb,
		      const IRDatabase *rootdb);
    public:
	termcount get_approx_size() const;

	ExpandBits get_weighting() const; // Gets weight info of current term
	const termname get_termname() const;
	termcount get_wdf() const; // Number of occurences of term in current doc
	doccount get_termfreq() const;  // Number of docs indexed by term
	TermList * next();
	bool   at_end() const;

	~MultiTermList();
};

inline MultiTermList::MultiTermList(TermList *tl_new,
				    const IRDatabase *termdb_new,
				    const IRDatabase *rootdb_new)
	: tl(tl_new), termdb(termdb_new), rootdb(rootdb_new)
{
    termfreq_factor = ((double)(rootdb->get_doccount())) /
		      (termdb->get_doccount());
#ifdef DEBUG
    cout << "Approximation factor for termfrequency: " <<
	    termfreq_factor << endl;
#endif /* DEBUG */
}

inline MultiTermList::~MultiTermList()
{
    delete tl;
}

inline termcount
MultiTermList::get_approx_size() const
{
    return tl->get_approx_size();
}

inline ExpandBits
MultiTermList::get_weighting() const {
    return tl->get_weighting();
}

inline const termname
MultiTermList::get_termname() const
{
    return tl->get_termname();
}

inline termcount MultiTermList::get_wdf() const
{
    return tl->get_wdf();
}

inline doccount MultiTermList::get_termfreq() const
{
    // Approximate term frequency
    return (doccount) (tl->get_termfreq() * termfreq_factor);
}

inline TermList * MultiTermList::next()
{
    return tl->next();
}

inline bool MultiTermList::at_end() const
{
    return tl->at_end();
}

#endif /* _multi_termlist_h_ */
