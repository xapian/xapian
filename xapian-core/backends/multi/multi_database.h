/* multi_database.h: C++ class definition for multiple database access */

#ifndef _multi_database_h_
#define _multi_database_h_

#include "omassert.h"
#include "database.h"
#include <stdlib.h>
#include <vector>

class MultiTerm {
    friend class MultiDatabase;
    private:
	MultiTerm(termname name_new) {
	    name = name_new;
	}
    public:
	termname name;
};


class MultiDatabase : public virtual IRDatabase {
    private:
	mutable termid max_termid;
	mutable map<termname, termid> termidmap;
	mutable vector<MultiTerm> termvec;

	vector<IRDatabase *> databases;
	bool opened;
    public:
	MultiDatabase();
	~MultiDatabase();

	termid term_name_to_id(const termname &) const;
	termname term_id_to_name(termid) const;

	termid add_term(const termname &);
	docid add_doc(IRDocument &);
	void add(termid, docid, termpos);

	void open(const string &pathname, bool readonly) {
	    throw OmError("open() not valid for MultiDatabase\n");
	}

	// MultiDatabase will take care of closing and deleting the
	// database.  FIXME find appropriate structure to make this
	// implicit in the interface.
	void open_subdatabase(IRDatabase *,
			      const string &pathname, bool readonly);
	void close();

	PostList * open_post_list(termid id) const;
	TermList * open_term_list(docid id) const;
};

#endif /* _sleepy_database_h_ */
