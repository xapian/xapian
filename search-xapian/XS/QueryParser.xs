MODULE = Search::Xapian 		PACKAGE = Search::Xapian::QueryParser

PROTOTYPES: ENABLE

QueryParser *
new0()
    CODE:
	RETVAL = new QueryParser();
    OUTPUT:
	RETVAL

void
QueryParser::set_stemmer(stemmer)
    Stem * stemmer
    CODE:
	THIS->set_stemmer(*stemmer);

void
QueryParser::set_stemming_strategy(strategy)
    int strategy
    CODE:
	THIS->set_stemming_strategy(static_cast<QueryParser::stem_strategy>(strategy));

void
QueryParser::set_stopper(stopper)
    Stopper * stopper
    CODE:
	// FIXME: no corresponding SvREFCNT_dec(), but a leak seems better than
	// a SEGV!
	SvREFCNT_inc(ST(1));
	THIS->set_stopper(stopper);

void
QueryParser::set_default_op(op)
    int op
    CODE:
	THIS->set_default_op(static_cast<Query::op>(op));

int
QueryParser::get_default_op()
    CODE:
	RETVAL = static_cast<int>(THIS->get_default_op());
    OUTPUT:
	RETVAL

void
QueryParser::set_database(database)
    Database * database
    CODE:
	THIS->set_database(*database);

Query *
QueryParser::parse_query(q, flags = QueryParser::FLAG_DEFAULT)
    string q
    int flags
    CODE:
	try {
	    RETVAL = new Query(THIS->parse_query(q,flags));
	} catch (const Error &error) {
	    croak( "Exception: %s", error.get_msg().c_str() );
	}
    OUTPUT:
	RETVAL

void
QueryParser::add_prefix(string field, string prefix)

void
QueryParser::add_boolean_prefix(string field, string prefix)

TermIterator *
QueryParser::stoplist_begin()
    CODE:
	RETVAL = new TermIterator(THIS->stoplist_begin());
    OUTPUT:
	RETVAL

TermIterator *
QueryParser::stoplist_end()
    CODE:
	RETVAL = new TermIterator(THIS->stoplist_end());
    OUTPUT:
	RETVAL

TermIterator *
QueryParser::unstem_begin(term)
    string term
    CODE:
	RETVAL = new TermIterator(THIS->unstem_begin(term));
    OUTPUT:
	RETVAL

TermIterator *
QueryParser::unstem_end(term)
    string term
    CODE:
	RETVAL = new TermIterator(THIS->unstem_end(term));
    OUTPUT:
	RETVAL

string
QueryParser::get_corrected_query_string()
    CODE:
    try {
	RETVAL = THIS->get_corrected_query_string();
    } catch (const Error &error) {
	croak( "Exception: %s", error.get_msg().c_str() );
    }
    OUTPUT:
	RETVAL

string
QueryParser::get_description()

void
QueryParser::add_valuerangeprocessor(ValueRangeProcessor * vrproc)
    CODE:
	// FIXME: no corresponding SvREFCNT_dec(), but a leak seems better than
	// a SEGV!
	SvREFCNT_inc(ST(1));
	THIS->add_valuerangeprocessor(vrproc);

void
QueryParser::DESTROY()
