#include "database.h"

typedef unsigned int weight;

class Match {
    private:
        IRDatabase DB;
        vector<PostListIterator> postlist;

    public:
        int add_pterm(const string&);
        void match(void);
};
