/* textfile_database.cc: interface to text file access */

#include <stdio.h>

#include "omassert.h"
#include "textfile_database.h"
#include "textfile_indexer.h"
#include "textfile_document.h"

#include <string>
#include <vector>
#include <map>
#include <list>

//////////////
// Postlist //
//////////////

weight
TextfilePostList::get_weight() const
{
    Assert(started);
    Assert(!at_end());

    return ir_wt->get_weight((*pos).positions.size(),
			     this_db->get_doclength(get_docid()));
}

///////////////////////////
// Actual database class //
///////////////////////////

TextfileDatabase::TextfileDatabase()
{
    Assert((opened = false) == false);
    Assert((indexing = false) == false);
}

TextfileDatabase::~TextfileDatabase()
{
    close();
}

void TextfileDatabase::open(const string &pathname, bool readonly)
{
    Assert(readonly == true);
    Assert(opened == false); // Can only open once

    // Initialise
    totlen = 0;

    // Index document
    Assert((indexing = true) == true);

    TextfileIndexerSource source(pathname);
    TextfileIndexer indexer;

    indexer.set_destination(this);
    indexer.add_source(source);

    // Make sure that there's at least one document
    if(postlists.size() <= 0)
	throw OmError("Document was empty or nearly empty - nothing to search");

    path = pathname;
    Assert((opened = true) == true);
}

void TextfileDatabase::close()
{
}

DBPostList *
TextfileDatabase::open_post_list(const termname & tname, RSet *rset) const
{
    Assert(opened);
    Assert(term_exists(tname));

    map<termname, TextfileTerm>::const_iterator i = postlists.find(tname);
    Assert(i != postlists.end());

    return new TextfilePostList(root, this, i->second, tname, rset);
}

TermList *
TextfileDatabase::open_term_list(docid did) const
{
    Assert(opened);
    Assert(did > 0 && did <= termlists.size());

    return new TextfileTermList(this, termlists[did - 1]);
}

IRDocument *
TextfileDatabase::open_document(docid did) const
{
    Assert(opened);
    Assert(did > 0 && did <= doclists.size());

    return new TextfileDocument(doclists[did - 1]);
}


termid
TextfileDatabase::add_term(const termname &tname)
{
    throw OmError("TextfileDatabase.add_term() not implemented");
}

docid
TextfileDatabase::add_doc(IRDocument &doc)
{
    throw OmError("TextfileDatabase.add_doc() not implemented");
}

void
TextfileDatabase::add(termid tid, docid did, termpos tpos)
{
    throw OmError("TextfileDatabase.add_term() not implemented");
}

termid
TextfileDatabase::make_term(const termname &tname)
{
    Assert(indexing == true);
    Assert(opened == false);

    map<termname,termid>::const_iterator p = termidmap.find(tname);

    termid tid = 0;
    if (p == termidmap.end()) {
	tid = termvec.size() + 1;
	termvec.push_back(tname);
	termidmap[tname] = tid;
	postlists[tname];  // Initialise
    } else {
	tid = (*p).second;
    }
    return tid;
}

docid
TextfileDatabase::make_doc(const docname &dname)
{
    Assert(indexing == true);
    Assert(opened == false);

    termlists.push_back(TextfileDoc());
    doclengths.push_back(0);
    doclists.push_back(dname);

    AssertParanoid(termlists.size() == doclengths.size());

    return termlists.size();
}

void TextfileDatabase::make_posting(const termname & tname,
				    docid did,
				    termcount position)
{
    Assert(indexing == true);
    Assert(opened == false);
    Assert(postlists.find(tname) != postlists.end());
    Assert(did > 0 && did <= termlists.size());
    Assert(did > 0 && did <= doclengths.size());

    // Make the posting
    TextfilePosting posting;
    posting.tname = tname;
    posting.did = did;
    posting.positions.push_back(position);

    // Now record the posting
    postlists[tname].add_posting(posting);
    termlists[did - 1].add_posting(posting);
    doclengths[did - 1] += posting.positions.size();
    totlen += posting.positions.size();
}

termid
TextfileDatabase::term_name_to_id(const termname &tname) const
{
    Assert(opened);

    printf("Looking up term `%s': ", tname.c_str());
    map<termname,termid>::const_iterator p = termidmap.find(tname);

    termid tid = 0;
    if (p == termidmap.end()) {
	tid = 0;
    } else {
	tid = (*p).second;
    }
    return tid;
}
