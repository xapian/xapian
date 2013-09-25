
#ifndef XAPIAN_INCLUDE_LUCENE_NRMLIST_H
#define XAPIAN_INCLUDE_LUCENE_NRMLIST_H

#include <xapian.h>

#include "backends/valuelist.h"
#include "lucene_database.h"
#include "omassert.h"

/** Lucene .nrm file contains multiple informations, includes doclength/
 *  doc boost/field boost etc... all these datas are compressed into a one
 *  byte float number(one byte per document). So it is not quite suitable
 *  to use it to calculate doclength.
 *
 *  This class is not used now. We use TfIdfWeight for lucene, this weight
 *  schema does not need doclength.
 *  
 *  you can treat ValueIterator::Internal as a interface, you just
 *  need to return a LuceneNrmList object to ValueIterator, then the
 *  valueIterator's interface will call the function in LuceneNrmList.
 *
 *  Just like pure virtual class and polymorphic
 *
 *  FIXME: It is not quite suitable to extends this class from ValueIterator::Internal
 */
class LuceneNrmList : public Xapian::ValueIterator::Internal {
    Xapian::Internal::intrusive_ptr<const LuceneDatabase> this_db;

  public:
    LuceneNrmList(Xapian::Internal::intrusive_ptr<const LuceneDatabase> this_db_);

    /** No used
     */
    Xapian::docid get_docid() const;

    /** No used
     */
    std::string get_value() const;

    /** Used to return doclength(calculated by norm value)
     */
    Xapian::valueno get_valueno() const;

    bool at_end() const;

    void next();

    /** No used
     */
    void skip_to(Xapian::docid);

    /** No used
     */
    bool check(Xapian::docid did);

    /** No used
     */
    std::string get_description() const;
};

#endif
