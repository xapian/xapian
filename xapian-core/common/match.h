#include "database.h"
#include "mergedpostlist.h"

#include <queue>

class PLPCmp {
   public:
       bool operator()(const PostList *a, const PostList *b) {
           return a->get_termfreq() > b->get_termfreq();
       }
};

class Match {
    private:
        IRDatabase *DB;
   
        PostList *merger;
//        const int msize = 1000;

        // FIXME: try using a heap instead (C++ sect 18.8)
        priority_queue<PostList*, vector<PostList*>, PLPCmp> pq;

    public:
        Match(IRDatabase *);
        bool add_pterm(const string&);
        void match(void);
};
