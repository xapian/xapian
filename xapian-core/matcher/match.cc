#include "match.h"

#include <algorithm>

Match::Match(IRDatabase *database)
{
    DB = database;
    merger = NULL;
    max_msize = 1000;
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

class MSetItem {
    public:
        weight w;
        docid id;
        MSetItem(weight w_, docid id_) { w = w_; id = id_; }
};

class MSetCmp {
   public:
       bool operator()(const MSetItem &a, const MSetItem &b) {
           return a.w > b.w;
       }
};

void
Match::match(void)
{    
    if (!merger) {
        if (pq.empty()) return; // No terms in query

        PostList *tmp;
       
        // build a tree balanced by the term frequencies
        // (similar to building a huffman encoding tree)
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

    doccount msize = 0, mtotal = 0;
    weight w_min = 0;
    vector<MSetItem> mset;
    int sorted_to = 0;

    // FIXME: clean all this up
    // FIXME: partial_sort?
    // FIXME: quicker to just resort whole lot than sort and merge?
    while (!merger->at_end()) {
        weight w = merger->get_weight();
        
        if (w > w_min) {
	    docid id = merger->get_docid();
	    mset.push_back(MSetItem(w,id));

	    if (mset.size() == max_msize * 2) {
	        // sort new elements
	        cout << "sorting\n";		
	        stable_sort(mset.begin() + sorted_to, mset.end(), MSetCmp());
		// merge with existing elements
	        cout << "merging\n";
                if (sorted_to) {
		    inplace_merge(mset.begin(), mset.begin() + sorted_to, mset.end(), MSetCmp());
		}
	        msize = max_msize;
	        sorted_to = msize;
		// erase elements which don't make the grade
	        mset.erase(mset.begin() + sorted_to, mset.end());
	        w_min = mset.back().w;
	        cout << "mset size = " << mset.size() << endl;
	    }
	}
        mtotal++;
        merger->next();
    }

    cout << "sorting\n";
    stable_sort(mset.begin() + sorted_to, mset.end(), MSetCmp());
    cout << "merging\n";
    if (sorted_to) {
	inplace_merge(mset.begin(), mset.begin() + sorted_to, mset.end(), MSetCmp());
    }
    msize = mset.size();
    if (max_msize < msize) {
	sorted_to = msize = max_msize;
	mset.erase(mset.begin() + sorted_to, mset.end());
	w_min = mset.back().w;
    }
    cout << "mset size = " << mset.size() << endl;

    cout << "msize = " << msize << ", mtotal = " << mtotal << endl;
#if 0
    for (docid i = 0; i < msize; i++) {
        cout << mset[i].id << "\t" << mset[i].w << endl;
    }
#endif
}
