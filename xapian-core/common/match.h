#ifndef _match_h_
#define _match_h_

#include "database.h"

#include <queue>
#include <stack>

class PLPCmp {
   public:
       bool operator()(const PostList *a, const PostList *b) {
           return a->get_termfreq() > b->get_termfreq();
       }
};

class Match {
    private:
        IRDatabase *DB;
   
        doccount max_msize;

        // FIXME: try using a heap instead (C++ sect 18.8)
        priority_queue<PostList*, vector<PostList*>, PLPCmp> pq;
	stack<PostList *> bq;

        PostList *merger;
    
        bool recalculate_maxweight;
    public:
        Match(IRDatabase *);
        bool add_pterm(const string &);
	bool add_bterm(const string &);
	bool add_band();
	bool add_bor();
	bool add_bandnot();
        void match();
        void set_max_msize(doccount n);
        void recalc_maxweight();
};

inline void
Match::set_max_msize(doccount n)
{
    max_msize = n;
}
#endif /* _match_h_ */
