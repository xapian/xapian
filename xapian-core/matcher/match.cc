#include "match.h"

Match::Match(IRDatabase *database)
{
    DB = database;
    merger = NULL;
}

bool
Match::add_pterm(const string& termname)
{
    termid id = DB->term_name_to_id(termname);

    if (!id) return false;

    PostList *postlist = DB->open_post_list(id);
   
    pq.push(postlist);
   
    return true;
}

#define MSIZE 100

typedef struct {
    weight w;
    docid id;
} msetitem;

void
Match::match(void)
{    
    if (!merger) {
        if (pq.empty()) return; // No terms in query

        PostList *tmp;
       
        // build a tree balanced by the term frequencies
        // (similar to a huffman encoding tree)
        while (true) {
	    tmp = pq.top();
	    pq.pop();
	    if (pq.empty()) break;
	    tmp = new MergedPostList(pq.top(), tmp);
	    pq.pop();
	    pq.push(tmp);
	}
       
        merger = tmp;
    }

    docid msize = 0, mtotal = 0;
    weight min = 0;
    msetitem mset[MSIZE];

    while (!merger->at_end()) {
        weight w = merger->get_weight();
        
        if (w > min) {
	    docid id = merger->get_docid();
	    docid i;
	    for (i = 0; i < msize; i++) {
	        if (mset[i].w < w) break;
	    }

	    if (i == msize) {
	        if (msize < MSIZE) {
		    mset[msize].id = id;
		    mset[msize].w = w;
	            msize++;
		}
	    } else {
	        int len;
	        if (msize < MSIZE) msize++;
	        len = msize - i - 1;
	        memmove(mset + i + 1, mset + i, len * sizeof(msetitem));
	        mset[i].id = id;
	        mset[i].w = w;	        
	    }
	    if (msize == MSIZE) min = mset[MSIZE - 1].w;
	}
        mtotal++;
        merger->next();
    }

    cout << "msize = " << msize << ", mtotal = " << mtotal << endl;
    for (docid i = 0; i < msize; i++) {
        cout << mset[i].id << "\t" << mset[i].w << endl;
    }
}
