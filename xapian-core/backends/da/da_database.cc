/* da_database.cc: C++ class for datype access routines */

#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <math.h>
#include <string>
#include <vector>
#include <algorithm>

#include "database.h"
#include "da_database.h"
#include "daread.h"

DAPostList::DAPostList(struct postings *pl, doccount tf, doccount size)
{
    termfreq = tf;
    postlist = pl;
    dbsize = size;

    weight_initialised = false;

    DAreadpostings(postlist, 0, 0);
}

DAPostList::~DAPostList()
{
    DAclosepostings(postlist);
}

/* This is the biggie */
weight DAPostList::get_weight() const
{
    Assert(!at_end());
    doccount wdf;
    weight wt;

    if (!weight_initialised) {
	weight_initialised = true;

	termweight = (dbsize - termfreq + 0.5) / (termfreq + 0.5);
	if (termweight < 2) {
	    // if size and/or termfreq is estimated we can get termweight <= 0
	    // so handle this gracefully
	    if (termweight <= 1e-6) termweight = 1e-6;
	    termweight = termweight / 2 + 1;
	}
	termweight = log(termweight);

	printf("(dbsize, termfreq) = (%4d, %4d)\t=> termweight = %f\n",
	       dbsize, termfreq, termweight);
    }

    wdf = postlist->wdf;

//    printf("(wdf, termweight)  = (%4d, %4.2f)", wdf, termweight);

    const double k = 1;
    // FIXME - precalculate this freq score for several values of wdf - may
    // remove much computation.
    wt = (double) wdf / (k + wdf);
//    printf("(freq score %4.2f)", wt);

    wt *= termweight;

//    printf("\t=> weight = %f\n", wt);

    return wt;
}

void DAPostList::next()
{
    Assert(!at_end());
    DAreadpostings(postlist, 0, 0);
}

void DAPostList::skip_to(docid id)
{
    Assert(!at_end());
    Assert(id >= docid(postlist->Doc));
    //printf("%p:From %d skip_to ", this, postlist->Doc);
    if(id != docid(postlist->Doc)) DAreadpostings(postlist, 0, id);
    //printf("%d - get_id %d\n", id, postlist->Doc);
}



DATermList::DATermList(DADatabase *db, struct termvec *tv)
{
    readterms(tv);
    while(tv->term != 0) {
	char *term = (char *)tv->term;
	termid id;
	// FIXME - Next line is inefficient - checks for term in term list
	id = db->term_name_to_id(string(term + 1, (unsigned)term[0] - 1));
	ids.push_back(id);
	readterms(tv);
    }
    losetermvec(tv);

    sort(ids.begin(), ids.end());

    pos = ids.begin();
}



DADatabase::DADatabase()
{
    DA_r = NULL;
    DA_t = NULL;
    max_termid = 0;
    opened = false;
}

DADatabase::~DADatabase()
{
    close();
}

void
DADatabase::open(const string &pathname, bool readonly)
{
    DADatabase::close();

    string filename_r = pathname + "/R";
    string filename_t = pathname + "/T";

    DA_r = DAopen((byte *)(filename_r.c_str()), DARECS);
    if(DA_r == NULL)
	throw OpeningError(string("When opening ") + filename_r + ": " + strerror(errno));

    DA_t = DAopen((byte *)(filename_t.c_str()), DATERMS);
    if(DA_t == NULL) {
	DAclose(DA_r);
	DA_r = NULL;
	throw OpeningError(string("When opening ") + filename_t + ": " + strerror(errno));
    }

    dbsize = DA_r->itemcount;

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
    Assert(opened);
    Assert(id > 0 && id <= termvec.size());

    termname name = term_id_to_name(id);

    struct postings * postlist;
    postlist = DAopenpostings(&(termvec[id - 1].ti), DA_t);

    DAPostList * pl = new DAPostList(postlist, termvec[id - 1].ti.freq, dbsize);
    return pl;
}

TermList * DADatabase::open_term_list(docid id)
{
    Assert(opened);

    struct termvec *tv = maketermvec();
    int found = DAgettermvec(DA_r, id, tv);

    if(found == 0) throw RangeError("Docid not found");

    openterms(tv);

    DATermList *tl = new DATermList(this, tv);
    return tl;
}

termid
DADatabase::term_name_to_id(const termname &name)
{
    Assert(opened);
    //printf("Looking up term `%s': ", name.c_str());

    map<termname,termid>::iterator p = termidmap.find(name);

    termid id = 0;
    if (p == termidmap.end()) {
	int len = name.length();
	if(len > 255) throw RangeError("Termid not found");
	byte * k = (byte *) malloc(len + 1);
	if(k == NULL) throw OmError(strerror(ENOMEM));
	k[0] = len + 1;
	name.copy((char*)(k + 1), len);

	struct terminfo ti;
	int found = DAterm(k, &ti, DA_t);
	free(k);

	if(found == 0) {
	    //printf("Not in collection\n");
	} else {
	    id = termvec.size() + 1;
	    //printf("Adding as ID %d\n", id);
	    termvec.push_back(DATerm(&ti, name));
	    termidmap[name] = id;
	}
    } else {
	id = (*p).second;
	//printf("found, ID %d\n", id);
    }
    return id;
}

termname
DADatabase::term_id_to_name(termid id)
{
    Assert(opened);
    Assert(id > 0 && id <= termvec.size());
    //printf("Looking up termid %d: name = `%s'\n", id, termvec[id - 1].name.c_str());
    return termvec[id - 1].name;
}
