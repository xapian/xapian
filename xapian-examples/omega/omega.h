#define FERRET 1

#include "om.h"

#include "config.h"
#define FX_VERSION_STRING "1.4+ (" PACKAGE "-" VERSION ")"

#include <map>
#include <vector>
#include <stdio.h>

extern FILE *page_fopen(const string &page);

extern string db_name;
extern string fmt, fmtfile;

extern IRDatabase *database;
extern Match *matcher;
extern RSet *rset;

extern map<string, string> option;

extern const string default_db_name;

#ifdef FERRET
extern vector<int> dlist;
#endif

class ExpandDeciderFerret : public virtual ExpandDecider {
    public:
	bool want_term(const termname& tname) const {
	    // only suggest 4 or more letter words for now to
	    // avoid italian problems FIXME: fix this at index time
	    if (tname.length() <= 3) return false;

	    // also avoid terms with a prefix and with a space in
	    if (isupper(tname[0]) || tname.find(' ') != string::npos)
		return false;

	    // and terms in the query already
	    // FIXME - implement this
	    //return ExpandDeciderNotQuery::want_term(tname);
	    return true;
	}
};
