#ifdef __cplusplus
extern "C" {
#endif
#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"
#ifdef __cplusplus
}
#endif

#include <om/om.h>
#include <string>

using namespace std;

MODULE = Search::Xapian		PACKAGE = Search::Xapian::Settings		

PROTOTYPES: ENABLE

OmSettings *
OmSettings::new()
 
void
OmSettings::set(key, value)
    string *    key
    string *    value
    CODE:
        THIS->set(*key, *value);

void
OmSettings::DESTROY()


MODULE = Search::Xapian		PACKAGE = Search::Xapian::Database		

PROTOTYPES: ENABLE

OmDatabase *
OmDatabase::new(params)
    OmSettings * params
    CODE:
        RETVAL = new OmDatabase(* params);
    OUTPUT:
        RETVAL

void
OmDatabase::DESTROY()


MODULE = Search::Xapian  		PACKAGE = Search::Xapian::Query		

PROTOTYPES: ENABLE

OmQuery *
OmQuery::new(term)
    om_termname term

string
OmQuery::get_description()

void
OmQuery::DESTROY()


MODULE = Search::Xapian		PACKAGE = Search::Xapian::Document		

PROTOTYPES: ENABLE

OmDocument *
OmDocument::new()

string
OmDocument::get_data()

void
OmDocument::DESTROY()


MODULE = Search::Xapian 	PACKAGE = Search::Xapian::MatchSetIterator		

PROTOTYPES: ENABLE

OmMSetIterator *
OmMSetIterator::new()

OmMSetIterator *
OmMSetIterator::inc()
    CODE:
        RETVAL = new OmMSetIterator();
        *RETVAL = ++(*THIS);
    OUTPUT:
        RETVAL


om_docid
OmMSetIterator::get_docid()
    CODE:
        *THIS;

om_percent
OmMSetIterator::get_percent()

OmDocument *
OmMSetIterator::get_document()
    CODE:
        RETVAL = new OmDocument();
        *RETVAL = THIS->get_document();
    OUTPUT:
        RETVAL

void
OmMSetIterator::DESTROY()


MODULE = Search::Xapian		PACKAGE = Search::Xapian::MatchSet		

PROTOTYPES: ENABLE

OmMSet *
OmMSet::new()

om_doccount
OmMSet::get_matches_estimated()

OmMSetIterator *
OmMSet::begin()
    CODE:
        RETVAL = new OmMSetIterator();
        *RETVAL = THIS->begin();
    OUTPUT:
        RETVAL

OmMSetIterator *
OmMSet::end()
    CODE:
        RETVAL = new OmMSetIterator();
        *RETVAL = THIS->end();
    OUTPUT:
        RETVAL

om_doccount
OmMSet::size()

void
OmMSet::DESTROY()

    
MODULE = Search::Xapian		PACKAGE = Search::Xapian::Enquire		

PROTOTYPES: ENABLE

OmEnquire *
OmEnquire::new(databases)
    OmDatabase * databases
    CODE:
        RETVAL = new OmEnquire(* databases);
    OUTPUT:
        RETVAL

void
OmEnquire::set_query(query)
    OmQuery *    query
    CODE:
        THIS->set_query(* query);

OmMSet *
OmEnquire::get_mset(first, maxitems)
    om_doccount first
    om_doccount maxitems
    CODE:
        RETVAL = new OmMSet();
        *RETVAL = THIS->get_mset(first, maxitems);
    OUTPUT:
        RETVAL

void
OmEnquire::DESTROY()
