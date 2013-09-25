
#include <config.h>

#include "lucene_nrmlist.h"

#include "debuglog.h"

using namespace std;
using Xapian::Internal::intrusive_ptr;

//make cursor to the beginning of norm lists
LuceneNrmList::LuceneNrmList(intrusive_ptr<const LuceneDatabase> this_db_)
        : this_db(this_db_)
{
}

Xapian::docid
LuceneNrmList::get_docid() const
{
    LOGCALL(DB, Xapian::docid, "LuceneNrmList::get_docid", NO_ARGS);
    Assert(false);

    RETURN(Xapian::docid(0));
}

string
LuceneNrmList::get_value() const
{
    LOGCALL(DB, string, "LuceneNrmList::get_value", NO_ARGS);
    Assert(false);

    RETURN(string());
}

Xapian::valueno
LuceneNrmList::get_valueno() const
{
    LOGCALL(DB, Xapian::valueno, "LuceneNrmList::get_valueno", NO_ARGS);
    Assert(false);

    RETURN(Xapian::valueno(0));
}

bool
LuceneNrmList::at_end() const
{
    LOGCALL(DB, bool, "LuceneNrmList::at_end", NO_ARGS);
    Assert(false);
    
    RETURN(false);
}

void
LuceneNrmList::next()
{
    LOGCALL(DB, void, "LuceneNrmList::next", NO_ARGS);
    Assert(false);
}

void
LuceneNrmList::skip_to(Xapian::docid did)
{
    LOGCALL(DB, void, "LuceneNrmList::skip_to", did);
    Assert(false);
    (void)did;
}

bool
LuceneNrmList::check(Xapian::docid did)
{
    LOGCALL(DB, bool, "LuceneNrmList::check", did);
    Assert(false);
    (void)did;

    RETURN(false);
}

string
LuceneNrmList::get_description() const
{
    LOGCALL(DB, string, "LuceneNrmList::get_description", NO_ARGS);
    Assert(false);

    RETURN(string());
}

