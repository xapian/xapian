#include "database.h"
#include "mergedpostlist.h"

class Match {
    private:
        IRDatabase *DB;
        PostList *merger;
//        const int msize = 1000;

    public:
        Match(IRDatabase *);
        bool add_pterm(const string&);
        void match(void);
};
