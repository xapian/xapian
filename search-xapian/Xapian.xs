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
        try {
            RETVAL = new OmDatabase(* params);
        }
        catch (const OmError &error) {
            croak( "Exception: %s", error.get_msg().c_str() );
        }
    OUTPUT:
        RETVAL

void
OmDatabase::DESTROY()


MODULE = Search::Xapian		PACKAGE = Search::Xapian::Database::Writable

PROTOTYPES: ENABLE

OmWritableDatabase *
OmWritableDatabase::new(params)
    OmSettings * params
    CODE:
        try {
            RETVAL = new OmWritableDatabase(* params);
        }
        catch (const OmError &error) {
            croak( "Exception: %s", error.get_msg().c_str() );
        }
    OUTPUT:
        RETVAL

om_docid
OmWritableDatabase::add_document(document)
    OmDocument * document
    CODE:
        RETVAL = THIS->add_document(*document);
    OUTPUT:
        RETVAL

void
OmWritableDatabase::DESTROY()


MODULE = Search::Xapian  		PACKAGE = Search::Xapian::Query		

PROTOTYPES: ENABLE

OmQuery *
OmQuery::new(term)
    om_termname term

string
OmQuery::get_description()

void
OmQuery::DESTROY()


MODULE = Search::Xapian 		PACKAGE = Search::Xapian::Document		

PROTOTYPES: ENABLE

OmDocument *
OmDocument::new()

string
OmDocument::get_data()
    CODE:
        RETVAL = THIS->get_data();
    OUTPUT:
        RETVAL

void
OmDocument::set_data(data)
    string * data
    CODE:
        THIS->set_data(*data);

void
OmDocument::add_posting(tname, tpos)
    om_termname  tname
    om_termpos   tpos

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
        RETVAL = THIS->operator*();
    OUTPUT:
        RETVAL

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


MODULE = Search::Xapian		PACKAGE = Search::Xapian::Stem		

PROTOTYPES: ENABLE

OmStem *
OmStem::new(language)
    string *     language
    CODE:
        RETVAL = new OmStem(*language);
    OUTPUT:
        RETVAL

string
OmStem::stem_word(word)
    string *     word
    CODE:
        RETVAL = THIS->stem_word(*word);
    OUTPUT:
        RETVAL

void
OmStem::DESTROY()
