#include <config.h>

#include "xapian/postingsource.h"

namespace Xapian {

PostingSource::~PostingSource() { }

Xapian::weight
PostingSource::get_maxweight() const
{
    return 0;
}

Xapian::weight
PostingSource::get_weight() const
{
    return 0;
}

void
PostingSource::skip_to(Xapian::docid did, Xapian::weight w)
{
    while (!at_end() && get_docid() < did) {
	next(w);
    }
}

void
PostingSource::check(Xapian::docid did, Xapian::weight w, bool & valid)
{
    valid = true;
    skip_to(did, w);
}

std::string
PostingSource::get_description() const
{
    return "Xapian::PostingSource subclass";
}

}
