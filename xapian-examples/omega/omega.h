#define FERRET 1

#include "database.h"
#include "postlist.h"
#include "termlist.h"
#include "irdocument.h"
#include "match.h"
#include "stem.h"
#include "multi_database.h"
#include "rset.h"
#include "expand.h"

#include "config.h"
#define FX_VERSION_STRING "1.4+ (" PACKAGE "-" VERSION ")"

#include <map>
#include <vector>
#include <stdio.h>

extern FILE *page_fopen(const string &page);

extern string db_name;
extern string fmt, fmtfile;

extern MultiDatabase database;
extern Match *matcher;
extern RSet *rset;

extern map<string, string> option;

extern const string default_db_name;

#ifdef FERRET
extern vector<int> dlist;
#endif
