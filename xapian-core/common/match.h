#include "database.h"

class Match {
    private:
        IRDatabase DB;
	list<PostListIterator*> postlist;

    public:
        int add_pterm(const string&);
        void match(void);
};
