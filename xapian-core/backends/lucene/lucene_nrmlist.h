
#ifndef XAPIAN_INCLUDE_LUCENE_NRMLIST_H
#define XAPIAN_INCLUDE_LUCENE_NRMLIST_H

#include <xapian.h>

#include "backends/valuelist.h"
#include "lucene_database.h"
#include "omassert.h"

/** you can treat ValueIterator::Internal as a interface, you just
 * need to return a LuceneNrmList object to ValueIterator, then the
 * valueIterator's interface will call the function in LuceneNrmList.
 *
 * Just like pure virtual class and polymorphic
 */
class LuceneNrmList : public Xapian::ValueIterator::Internal {
    Xapian::Internal::intrusive_ptr<const LuceneDatabase> this_db;

  public:
    LuceneNrmList(Xapian::Internal::intrusive_ptr<const LuceneDatabase> this_db_);

    //never used
    Xapian::docid get_docid() const;

    //never used
    std::string get_value() const;

    //used to return doclength(calculated by norm value)
    Xapian::valueno get_valueno() const;

    bool at_end() const;

    void next();

    //never used
    void skip_to(Xapian::docid);

    //never used
    bool check(Xapian::docid did);

    //never used
    std::string get_description() const;
};

#endif
