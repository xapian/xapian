#include "parsequery.h"
#include <strstream.h>

typedef struct {
    const char *query;
    const char *expect;
} test;

static test tests[] = {
    { "om-example", "(om PHRASE 2 exampl)" },
    { "size_t", "(size PHRASE 2 t)" },
    { "muscat -wine", "(muscat AND_NOT wine)" },
    { "a- grade", "(a- OR grade)" },
    { "gtk+ -gimp", "(gtk+ AND_NOT gimp)" },
    { "c++ -c--", "(c++ AND_NOT c--)" },
    { "\"c++ standard\"", "(c++ PHRASE 2 standard)" },
    { NULL, NULL }
};

int
main(int argc, char **argv)
{
    char buf[10240]; // Very big (though we're also bounds checked)
    QueryParser qp;
    qp.set_stemming_options("english");
    // Use ostrstream (because ostringstream often doesn't exist)
    test *p = tests;
    int succeed = 0, fail = 0;
    while (p->query) {
	ostrstream ost(buf, sizeof(buf));
	ost << qp.parse_query(p->query) << '\0';
	string expect = string("OmQuery(") + p->expect + ')';
	if (buf == expect) {
	    succeed++;
	} else {
	    cout << "Query:\t`" << p->query << "'\n";
	    cout << "Expected:\t`" << expect << "'\n";	    
	    cout << "Got:\t\t`" << buf << "'\n";
	    fail++;
	}
	p++;
    }
    return (fail != 0);
}
