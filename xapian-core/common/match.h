#include "database.h"
#include "mergedpostlist.h"

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
    public:
        Match(IRDatabase *);
        bool add_pterm(const string &);
	bool add_bterm(const string &);
	bool add_band(void);
	bool add_bor(void);
	bool add_bandnot(void);
        void match(void);
        void set_max_msize(doccount n);
};

inline void
Match::set_max_msize(doccount n)
{
    max_msize = n;
}
