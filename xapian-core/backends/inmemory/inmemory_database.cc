/* textfile_database.cc: interface to text file access */

#include <stdio.h>

#include "omassert.h"
#include "textfile_database.h"

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
			     this_db->get_normlength(get_docid()));
}

///////////////////////////
// Actual database class //
///////////////////////////

TextfileDatabase::TextfileDatabase()
{
    Assert((opened = false) == false);
}

TextfileDatabase::~TextfileDatabase() {
    close();
}

void TextfileDatabase::open(const string &pathname, bool readonly) {
    Assert(readonly == true);
    Assert(opened == false); // Can only open once

    // Initialise
    totlen = 0;

    // Index document
    termname word = "thou";
    docid did = make_doc();
    termcount position = 1;

    make_posting(make_term(word), did, position++);
    make_posting(make_term("things"), did, position++);
    make_posting(make_term(word), did, position++);
    did = make_doc();
    position = 1;
    make_posting(make_term(word), did, position++);

    opened = true;
}

void TextfileDatabase::close() {
}

void TextfileDatabase::make_posting(termid tid, docid did, termcount position) {
    Assert(tid > 0 && tid <= postlists.size());
    Assert(did > 0 && did <= termlists.size());
    Assert(did > 0 && did <= doclengths.size());

    // Make the posting
    TextfilePosting posting;
    posting.tid = tid;
    posting.did = did;
    posting.positions.push_back(position);

    // Now record the posting
    postlists[tid - 1].add_posting(posting);
    termlists[did - 1].add_posting(posting);
    doclengths[did - 1] += posting.positions.size();
    totlen += posting.positions.size();
}

DBPostList *
TextfileDatabase::open_post_list(termid tid) const {
    Assert(opened);
    Assert(tid > 0 && tid <= postlists.size());

    return new TextfilePostList(root, this, postlists[tid - 1]);
}

TermList *
TextfileDatabase::open_term_list(docid did) const {
    Assert(opened);
    Assert(did > 0 && did <= termlists.size());

    return new TextfileTermList(termlists[did - 1]);
}

IRDocument *
TextfileDatabase::open_document(docid did) const {
    Assert(opened);

    throw OmError("TextfileDatabase.open_document() not yet implemented");
    return NULL;
}


termid
TextfileDatabase::add_term(const termname &tname) {
    throw OmError("TextfileDatabase.add_term() not implemented");
}

docid
TextfileDatabase::add_doc(IRDocument &doc) {
    throw OmError("TextfileDatabase.add_doc() not implemented");
}

void
TextfileDatabase::add(termid tid, docid did, termpos tpos) {
    throw OmError("TextfileDatabase.add_term() not implemented");
}

termid
TextfileDatabase::make_term(const termname &tname)
{
    map<termname,termid>::const_iterator p = termidmap.find(tname);

    termid tid = 0;
    if (p == termidmap.end()) {
	tid = termvec.size() + 1;
	termvec.push_back(tname);
	termidmap[tname] = tid;
	postlists.push_back(TextfileTerm());
    } else {
	tid = (*p).second;
    }
    return tid;
}

docid
TextfileDatabase::make_doc()
{
    termlists.push_back(TextfileDoc());
    doclengths.push_back(0);

    AssertParanoid(termlists.size() == doclengths.size());

    return termlists.size();
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
