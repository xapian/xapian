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
    Assert(ir_wt != NULL);

    return ir_wt->get_weight((*pos).positions.size(),
			     this_db->get_doclength(get_docid()));
}

///////////////////////////
// Actual database class //
///////////////////////////

TextfileDatabase::TextfileDatabase()
	: totlen(0)
{
    Assert((opened = false) == false);
    Assert((indexing = false) == false);
}

TextfileDatabase::~TextfileDatabase()
{
}

void
TextfileDatabase::open(const DatabaseBuilderParams &params)
{
    Assert(!opened); // Can only open once

    // Check validity of parameters
    Assert(params.readonly == true);
    Assert(params.paths.size() > 0);
    Assert(params.subdbs.size() == 0);
    
    // Index documents
    Assert((indexing = true) == true);

    TextfileIndexer indexer;
    indexer.set_destination(this);

    for(vector<string>::const_iterator p = params.paths.begin();
	p != params.paths.end(); p++) {
	TextfileIndexerSource source(*p);
	cout << *p << endl;
	indexer.add_source(source);
    }

    // Make sure that there's at least one document
    if(postlists.size() <= 0)
	throw OmError("Document was empty or nearly empty - nothing to search");

    Assert((opened = true) == true);
}

DBPostList *
TextfileDatabase::open_post_list(const termname & tname, RSet *rset) const
{
    Assert(opened);
    Assert(term_exists(tname));

    map<termname, TextfileTerm>::const_iterator i = postlists.find(tname);
    Assert(i != postlists.end());

    return new TextfilePostList(this, i->second);
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

void
TextfileDatabase::make_term(const termname &tname)
{
    Assert(indexing == true);
    Assert(opened == false);

    postlists[tname];  // Initialise, if not already there.
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

bool
TextfileDatabase::term_exists(const termname &tname) const
{
    Assert(opened);

#ifdef MUS_DEBUG_VERBOSE
    cout << "Looking up term `" << tname.c_str() << "'" << endl;
#endif
    map<termname, TextfileTerm>::const_iterator p = postlists.find(tname);

    if (p == postlists.end()) {
	return false;
    }
    return true;
}
