/* da_database.cc: C++ class for datype access routines */

#include <string.h>
#include <errno.h>
#include <string>

#include "database.h"
#include "da_database.h"
#include "daread.h"

DAPostList::DAPostList(struct postings *pl, doccount tf) {
    termfreq = tf;
    postlist = pl;
    DAreadpostings(postlist, 0, 0);
}

DAPostList::~DAPostList() {
    DAclosepostings(postlist);
}

doccount DAPostList::get_termfreq() {
    return termfreq;
}

docid DAPostList::get_docid() {
    if(at_end()) throw OmError("Attempt to access beyond end of postlist.");
    return postlist->Doc;
}

weight DAPostList::get_weight() {
    if(at_end()) throw OmError("Attempt to access beyond end of postlist.");
    return postlist->wdf;
}

void DAPostList::next() {
    if(at_end()) throw OmError("Attempt to access beyond end of postlist.");
    DAreadpostings(postlist, 0, 0);
}

void DAPostList::skip_to(docid id) {
    if(at_end()) throw OmError("Attempt to access beyond end of postlist.");
    DAreadpostings(postlist, 0, id);
}

bool DAPostList::at_end() {
    if(postlist->Doc == MAXINT) return true;
    return false;
}




DADatabase::DADatabase()
{
    DA_r = NULL;
    DA_t = NULL;
    max_termid = 0;
    opened = false;
}

void
DADatabase::open(string pathname, bool readonly)
{
    DADatabase::close();

    string filename_r = pathname + "/R";
    string filename_t = pathname + "/T";

    DA_r = DAopen((byte *)(filename_r.c_str()), DARECS);
    if(DA_r == NULL)
	throw OpeningError(string("Opening ") + filename_r + ": " + strerror(errno));

    DA_t = DAopen((byte *)(filename_t.c_str()), DATERMS);
    if(DA_t == NULL) {
	DAclose(DA_r);
	DA_r = NULL;
	throw OpeningError(string("Opening ") + filename_t + ": " + strerror(errno));
    }
    opened = true;

    return;
}

void
DADatabase::close()
{
    if(DA_r != NULL) {
	DAclose(DA_r);
	DA_r = NULL;
    }
    if(DA_t != NULL) {
	DAclose(DA_t);
	DA_t = NULL;
    }
    opened = false;
}

PostList * DADatabase::open_post_list(termid id)
{
    if(!opened) throw OmError("DADatabase not opened.");

    termname name = term_id_to_name(id);

    int len = name.length();
    if(len > 126) throw OmError("Term too long for DA implementation.");
    byte * k = (byte *) malloc(len + 1);
    if(k == NULL) throw OmError(strerror(ENOMEM));
    k[0] = len + 1;
    memcpy(k + 1, name.c_str(), len);

    struct terminfo ti;
    int found = DAterm(k, 0, &ti, DA_t);

    if(found == 0) throw RangeError("Termid not found");

    struct postings * postlist;
    postlist = DAopenpostings(&ti, DA_t);

    DAPostList * pl = new DAPostList(postlist, ti.freq);
    return pl;
}

TermList * DADatabase::open_term_list(termid id)
{
    if(!opened) throw OmError("DADatabase not opened.");
    return NULL;
}

termid
DADatabase::term_name_to_id(termname name)
{
    termid id;

    id = termidmap[name];
    if (!id) {
	id = termidvec.size() + 1;
//	printf("Adding term `%s' as ID %d\n", name.c_str(), id);
	termidvec.push_back(name);
	termidmap[name] = id;
    }
//    printf("Looking up term `%s': ID = %d\n", name.c_str(), id);
    return id;
}

termname
DADatabase::term_id_to_name(termid id)
{
    if (id <= 0 || id > termidvec.size()) throw RangeError("invalid termid");
//    printf("Looking up termid %d: name = `%s'\n", id, termidvec[id - 1].c_str());
    return termidvec[id - 1];
}
