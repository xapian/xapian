#include "parsequery.h"
#include <strstream.h>

typedef struct {
    const char *query;
    const char *expect;
} test;

static test tests[] = {
    { "om-example", "(om:(pos=1) PHRASE 2 exampl:(pos=2))" },
    { "size_t", "(size:(pos=1) PHRASE 2 t:(pos=2))" },
    { "muscat -wine", "(muscat:(pos=1) AND_NOT wine:(pos=2))" },
    { "a- grade", "(a-:(pos=1) OR grade:(pos=2))" },
    { "gtk+ -gimp", "(gtk+:(pos=1) AND_NOT gimp:(pos=2))" },
    { "c++ -c--", "(c++:(pos=1) AND_NOT c--:(pos=2))" },
    { "\"c++ standard\"", "(c++:(pos=1) PHRASE 2 standard:(pos=2))" },
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
