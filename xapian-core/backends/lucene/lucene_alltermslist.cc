
#include <config.h>

#include "lucene_alltermslist.h"

#include "debuglog.h"
#include "omassert.h"

#include <string>

using Xapian::Internal::intrusive_ptr;
using namespace std;
using namespace Xapian;

LuceneAllTermsList::LuceneAllTermsList(intrusive_ptr<const LuceneDatabase> this_db_,
            const string & prefix_)
        : this_db(this_db_),
          seg_idx(0),
          is_at_end(true),
          prefix(prefix_)
{
    seg_db = this_db->get_segdb(seg_idx);
    if (intrusive_ptr<LuceneSegdb>(NULL) != seg_db) {
        is_at_end = false;
    }
}

TermList *
LuceneAllTermsList::next()
{
    if (is_at_end)
        return NULL;

    if (seg_db->at_end()) {
        seg_db = this_db->get_segdb(++seg_idx);
        if (intrusive_ptr<LuceneSegdb>(NULL) == seg_db) {
            is_at_end = true;
            return NULL;
        }
    }

    seg_db->next_term();
    ti = seg_db->get_current_ti();

    return NULL;
}

bool
LuceneAllTermsList::at_end() const
{
    return is_at_end;
}

Xapian::PostingIterator
LuceneAllTermsList::postlist_begin() const
{
    LOGCALL(DB, PostList, "LuceneAllTermsList::postlist_begin", NO_ARGS);
    LucenePostIterator * pl = new LucenePostIterator(this_db, seg_db, ti, seg_idx);

    return PostingIterator(pl);
}

string
LuceneAllTermsList::get_termname() const
{
    LOGCALL(DB, string, "LuceneAllTermsList::get_termname", NO_ARGS);
    RETURN(ti.get_term().get_suffix());
}

Xapian::doccount
LuceneAllTermsList::get_termfreq() const
{
    Assert(false);
    return Xapian::doccount(0);
}

Xapian::termcount
LuceneAllTermsList::get_collection_freq() const
{
    Assert(false);
    return Xapian::termcount(0);
}

TermList *
LuceneAllTermsList::skip_to(const string & name)
{
    Assert(false);
    (void)name;
    return NULL;
}
