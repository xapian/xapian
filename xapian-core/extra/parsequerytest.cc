/* parsequerytest.cc: Tests of omqueryparser
 *
 * ----START-LICENCE----
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2001,2002 Ananova Ltd
 * Copyright 2002 Olly Betts
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

#include <config.h>
#include "omparsequery.h"
#include <iostream>
#include <string>

using namespace std;

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
    { "AT&T M&S", "(Rat&t:(pos=1) OR Rm&s:(pos=2))" },
    { "E.T. N.A.T.O AB.C.", "(Ret:(pos=1) OR Rnato:(pos=2) OR Rab:(pos=3) OR c:(pos=4))" },
    { "author:orwell animal farm", "(Aorwel:(pos=1) OR anim:(pos=2) OR farm:(pos=3))" },
    { NULL, NULL }
};

int
main(void)
{
    OmQueryParser qp;
    qp.set_stemming_options("english");
    qp.prefixes.insert(make_pair("author", "A"));
    test *p = tests;
    int succeed = 0, fail = 0;
    while (p->query) {
	string expect = string("OmQuery(") + p->expect + ')';
	string parsed;
	try {
	    parsed = qp.parse_query(p->query).get_description();
	} catch (const OmError &e) {
	    parsed = e.get_msg();
	} catch (const char *s) {
	    parsed = s;
	} catch (...) {
	    parsed = "Unknown exception!";
	}
	if (parsed == expect) {
	    succeed++;
	} else {
	    cout << "Query:\t`" << p->query << "'\n";
	    cout << "Expected:\t`" << expect << "'\n";	    
	    cout << "Got:\t\t`" << parsed << "'\n";
	    fail++;
	}
	p++;
    }
    return (fail != 0);
}
