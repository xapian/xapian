MODULE = Search::Xapian		PACKAGE = Search::Xapian::Settings		

PROTOTYPES: ENABLE

OmSettings *
new1();
    CODE:
        RETVAL = new OmSettings();
    OUTPUT:
        RETVAL

OmSettings *
new2(other);
    OmSettings * other
    CODE:
        RETVAL = new OmSettings(* other);
    OUTPUT:
        RETVAL
 
void
OmSettings::set1(key, value)
    string *    key
    char *      value
    CODE:
        THIS->set(*key, value);

void
OmSettings::set2(key, value)
    string *    key
    int         value
    CODE:
        THIS->set(*key, value);

void
OmSettings::set3(key, value)
    string *    key
    double      value
    CODE:
        THIS->set(*key, value);

string
OmSettings::get(string * key)
    CODE:
        THIS->get(*key);

int
OmSettings::get_int(string * key)
    CODE:
        THIS->get_int(*key);

bool
OmSettings::get_bool(string * key)
    CODE:
        THIS->get_bool(*key);

double
OmSettings::get_real(string * key)
    CODE:
        THIS->get_real(*key);

string
OmSettings::get_description()

void
OmSettings::DESTROY()
