#include "database.h"
#include "mergedpostlist.h"

#include <queue>

class Match {
    private:
        IRDatabase *DB;
   
        PostList *merger;
//        const int msize = 1000;

        priority_queue<PostList *> pq;

    public:
        Match(IRDatabase *);
        bool add_pterm(const string&);
        void match(void);
};
