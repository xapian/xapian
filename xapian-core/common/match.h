#ifndef _match_h_
#define _match_h_

#include "database.h"
#include "omassert.h"

#include <queue>
#include <stack>
#include <vector>

class MSetItem {
    public:
        weight w;
        docid id;
        MSetItem(weight w_, docid id_) { w = w_; id = id_; }
};

typedef enum { AND, OR, FILTER, AND_NOT, AND_MAYBE, XOR } matchop;

class Match {
    private:
        IRDatabase *DB;
   
        doccount max_msize;
        int min_weight_percent;
        weight max_weight;

	stack<PostList *> q;

        PostList *merger;
	RSet *rset;
    
	bool have_added_terms;
        bool recalculate_maxweight;
    public:
        Match(IRDatabase *);
        void add_term(const termname &);
        void add_term(termid);
	bool add_op(matchop op);
	void add_oplist(matchop op, const vector<termname>&);
	void add_oplist(matchop op, const vector<termid>&);

        void match();
        void set_max_msize(doccount n);
        void set_rset(RSet *new_rset);
        weight get_max_weight();
        void set_min_weight_percent(int pcent);
        void recalc_maxweight();

        vector<MSetItem> mset;
        doccount msize;
        doccount mtotal;
};

inline void
Match::add_term(const string& termname)
{
    Match::add_term(DB->term_name_to_id(termname));
}

inline void
Match::set_rset(RSet *new_rset)
{
    Assert(!have_added_terms);
    rset = new_rset;
}

inline void
Match::set_max_msize(doccount n)
{
    max_msize = n;
}

inline void
Match::set_min_weight_percent(int pcent)
{
    min_weight_percent = pcent;
}

inline weight
Match::get_max_weight()
{
    return max_weight;
}

#endif /* _match_h_ */
