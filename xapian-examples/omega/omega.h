#define FERRET 1

#include "database.h"
#include "postlist.h"
#include "termlist.h"
#include "irdocument.h"
#include "match.h"
#include "stem.h"
#include "da_database.h"

#include "config.h"
#define FX_VERSION_STRING "1.4 24/02/1998 (" PACKAGE "-" VERSION ")"

extern FILE *page_fopen(const string &page);

extern string db_name;
extern char dash_chr;
extern int have_query;
extern char *fmt, *fmtfile;

extern DADatabase database;
extern Match *matcher;

extern map<string, string> option;

extern const string default_db_name;
