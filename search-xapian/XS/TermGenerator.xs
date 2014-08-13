MODULE = Search::Xapian			PACKAGE = Search::Xapian::TermGenerator

PROTOTYPES: ENABLE

TermGenerator *
new0()
    CODE:
	RETVAL = new TermGenerator();
    OUTPUT:
	RETVAL

void
TermGenerator::set_stemmer(stemmer)
    Stem * stemmer
    CODE:
	THIS->set_stemmer(*stemmer);

void
TermGenerator::set_stopper0(stopper)
    Stopper * stopper
    CODE:
	THIS->set_stopper(stopper);

void
TermGenerator::set_database(db)
    WritableDatabase * db
    CODE:
	THIS->set_database(*db);

void
TermGenerator::set_document(Document * doc)
    CODE:
	THIS->set_document(*doc);

Document *
TermGenerator::get_document()
    CODE:
	RETVAL = new Document(THIS->get_document());
    OUTPUT:
	RETVAL

void
TermGenerator::index_text(text, weight = 1, prefix = NO_INIT)
    string text
    termcount weight
    string prefix

int
TermGenerator::set_flags(int toggle, int mask = 0)
    CODE:
	RETVAL = THIS->set_flags(TermGenerator::flags(toggle),
				 TermGenerator::flags(mask));
    OUTPUT:
	RETVAL

void
TermGenerator::index_text_without_positions(text, weight = 1, prefix = NO_INIT)
    string text
    termcount weight
    string prefix

void
TermGenerator::increase_termpos(termcount delta = 100)

termcount
TermGenerator::get_termpos()

void
TermGenerator::set_termpos(termcount termpos)

string
TermGenerator::get_description()

void
TermGenerator::DESTROY()
    CODE:
       {
          dSP;
          ENTER;
          SAVETMPS;
          PUSHMARK( sp);
          XPUSHs( ST(0) );
          PUTBACK;
          perl_call_method("_delete_subrefs", G_DISCARD);
          SPAGAIN; 
          FREETMPS; 
          LEAVE;
       }
       delete THIS;
