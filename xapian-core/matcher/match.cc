#include "match.h"

int
Match::add_pterm(const string& termname)
{
    // FIXME: const discarded...
    termid id = DB.term_name_to_id((char*)termname.c_str());

    if (!id) return 0;
   
    PostListIterator *plist = new PostListIterator;
    plist->open(id);
    postlist.push_back(plist);
}

void
Match::match(void)
{
    bool more = true;
    docid current = 0;
    map<docid,weight> m;
   
    while (more) {
        more = false;
       
        list<PostListIterator*>::iterator i = postlist.begin();
        list<PostListIterator*>::iterator end = postlist.end();
        while (i != end) {
	    PostListIterator *plist = *i;
            if (!plist->at_end()) {
 	        docid d = plist->get_docid();
	        // FIXME: don't always want to advance each posting iterator...
	        m[d]++;
                plist->next();
	    } else {
	        // i.erase();
	    }
            i++;
	}
        more = postlist.size() > 0;
    }

    map<docid,weight>::iterator i = m.begin();
    map<docid,weight>::iterator end = m.end();

    while (i != end) {
        cout << (*i).first << "\t" << (*i).second << endl;
        i++;
    }
}
