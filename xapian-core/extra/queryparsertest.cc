/* parsequerytest.cc: Tests of omqueryparser
 *
 * ----START-LICENCE----
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2001,2002 Ananova Ltd
 * Copyright 2002,2003 Olly Betts
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
#include "queryparser.h"
#include <iostream>
#include <string>

using namespace std;

Xapian::Query::op default_op = Xapian::Query::OP_OR;

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
    { "Call to undefined function: imagecreate()", "(Rcall:(pos=1) OR to:(pos=2) OR undefin:(pos=3) OR function:(pos=4) OR imagecr:(pos=5))" },
    { "mysql_fetch_row(): supplied argument is not a valid MySQL result resource", "((mysql:(pos=1) PHRASE 3 fetch:(pos=2) PHRASE 3 row:(pos=3)) OR suppli:(pos=4) OR argument:(pos=5) OR is:(pos=6) OR not:(pos=7) OR a:(pos=8) OR valid:(pos=9) OR Rmysql:(pos=10) OR result:(pos=11) OR resourc:(pos=12))" },
    { "php date() nedelands", "(php:(pos=1) OR date:(pos=2) OR nedeland:(pos=3))" },
    // Not the obvious parse, but not unreasonable
    { "DVD+RW", "(Rrw:(pos=2) AND_MAYBE Rdvd:(pos=1))" },
    { "wget domein --http-user", "(wget:(pos=1) OR domein:(pos=2) OR (http:(pos=3) PHRASE 2 user:(pos=4)))" },
    { "@home problemen", "(home:(pos=1) OR problemen:(pos=2))" },
    { "'ipacsum'", "ipacsum:(pos=1)" },
    { "canal + ", "canal:(pos=1)" },
    // Perhaps "." should be a "phrase-maker" too when used like this...
    { "/var/run/mysqld/mysqld.sock", "((var:(pos=1) PHRASE 4 run:(pos=2) PHRASE 4 mysqld:(pos=3) PHRASE 4 mysqld:(pos=4)) OR sock:(pos=5))" },
    { "\"QSI-161 drivers\"", "(Rqsi:(pos=1) PHRASE 3 R161:(pos=2) PHRASE 3 driver:(pos=3))" },
    { "\"e-cube\" barebone", "((e:(pos=1) PHRASE 2 cube:(pos=2)) OR barebon:(pos=3))" },
    { "\"./httpd: symbol not found: dlopen\"", "(httpd:(pos=1) PHRASE 5 symbol:(pos=2) PHRASE 5 not:(pos=3) PHRASE 5 found:(pos=4) PHRASE 5 dlopen:(pos=5))" },
    // These are currently parse errors, but shouldn't be:
    { "ERROR 2003: Can't connect to MySQL server on 'localhost' (10061)", NULL },
    { "behuizing 19\" inch", NULL },
    { "553 sorry, that domain isn't in my list of allowed rcpthosts (#5.7.1)", NULL },
    { NULL, NULL }
};

int
main(void)
{
    Xapian::QueryParser qp;
    qp.set_stemming_options("english");
    qp.prefixes.insert(pair<string, string>("author", "A"));
    test *p = tests;
    int succeed = 0, fail = 0;
    while (p->query) {
	if (1 && !p->expect) {
	    ++p;
	    continue;
	}	
	string expect, parsed;
	if (p->expect) expect = string("Xapian::Query(") + p->expect + ')';
	try {
	    parsed = qp.parse_query(p->query).get_description();
	} catch (const Xapian::Error &e) {
	    parsed = e.get_msg();
	} catch (const char *s) {
	    parsed = s;
	} catch (...) {
	    parsed = "Unknown exception!";
	}
	if (parsed == expect) {
	    //cout << "OK\t`" << p->query << "'" << endl;
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
