/* parsequerytest.cc: tests for parsequery.yy
 *
 * ----START-LICENCE----
 * Copyright 2000,2001 BrightStation PLC
 * Copyright 2001 Ananova Ltd
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 * USA
 * -----END-LICENCE-----
 */

#include "parsequery.h"
#include <strstream.h>

OmQuery::op default_op = OmQuery::OP_OR;

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
	    std::cout << "Query:\t`" << p->query << "'\n";
	    std::cout << "Expected:\t`" << expect << "'\n";
	    std::cout << "Got:\t\t`" << buf << std::endl;
	    fail++;
	}
	p++;
    }
    return (fail != 0);
}
