#ifndef _match_h_
#define _match_h_

#include "database.h"
#include "postlist.h"

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
    
        bool recalculate_maxweight;
    public:
        Match(IRDatabase *);
        bool add_term(const termname &);
        bool add_term(termid);
	bool add_op(matchop op);
	bool add_oplist(matchop op, const vector<termname>&);
	bool add_oplist(matchop op, const vector<termid>&);

        void match();
        void set_max_msize(doccount n);
        weight get_max_weight();
        void set_min_weight_percent(int pcent);
        void recalc_maxweight();

        vector<MSetItem> mset;
        doccount msize;
        doccount mtotal;
};

inline bool
Match::add_term(const string& termname)
{
    return Match::add_term(DB->term_name_to_id(termname));
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
