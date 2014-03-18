/* queryparsertest.cc: Tests of Xapian::QueryParser
 *
 * Copyright (C) 2002,2003,2004,2005,2006,2007,2008,2009,2010,2011,2013 Olly Betts
 * Copyright (C) 2007,2009 Lemur Consulting Ltd
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301
 * USA
 */

#include <config.h>

#include <xapian.h>

#include "cputimer.h"
#include "str.h"
#include "stringutils.h"
#include "utils.h"

#include <cmath>
#include <iostream>
#include <string>
#include <vector>

#include <stdlib.h> // For setenv() or putenv()

using namespace std;

#define TESTCASE(S) {#S, test_##S}
#define END_OF_TESTCASES {0, 0}

#include "testsuite.h"
#include "testutils.h"

struct test {
    const char *query;
    const char *expect;
};

static const test test_or_queries[] = {
    { "simple-example", "(simple:(pos=1) PHRASE 2 example:(pos=2))" },
    { "time_t", "Ztime_t:(pos=1)" },
    { "stock -cooking", "(Zstock:(pos=1) AND_NOT Zcook:(pos=2))" },
    { "foo -baz bar", "((Zfoo:(pos=1) OR Zbar:(pos=3)) AND_NOT Zbaz:(pos=2))" },
    { "d- school report", "(Zd:(pos=1) OR Zschool:(pos=2) OR Zreport:(pos=3))" },
    { "gtk+ -gnome", "(Zgtk+:(pos=1) AND_NOT Zgnome:(pos=2))" },
    { "c++ -d--", "(Zc++:(pos=1) AND_NOT Zd:(pos=2))" },
    { "Mg2+ Cl-", "(mg2+:(pos=1) OR cl:(pos=2))" },
    { "\"c++ library\"", "(c++:(pos=1) PHRASE 2 library:(pos=2))" },
    { "A&L A&RMCO AD&D", "(a&l:(pos=1) OR a&rmco:(pos=2) OR ad&d:(pos=3))" },
    { "C# vs C++", "(c#:(pos=1) OR Zvs:(pos=2) OR c++:(pos=3))" },
    { "j##", "Zj##:(pos=1)" },
    { "a#b", "(Za:(pos=1) OR Zb:(pos=2))" },
    { "O.K. U.N.C.L.E XY.Z.", "(ok:(pos=1) OR uncle:(pos=2) OR (xy:(pos=3) PHRASE 2 z:(pos=4)))" },
    { "author:orwell animal farm", "(ZAorwel:(pos=1) OR Zanim:(pos=2) OR Zfarm:(pos=3))" },
    { "author:Orwell Animal Farm", "(Aorwell:(pos=1) OR animal:(pos=2) OR farm:(pos=3))" },
    // Regression test for bug reported in 0.9.6.
    { "author:\"orwell\" title:\"animal\"", "(Aorwell:(pos=1) OR XTanimal:(pos=2))" },
    // Regression test for bug related to one reported in 0.9.6.
    { "author:(orwell) title:(animal)", "(ZAorwel:(pos=1) OR ZXTanim:(pos=2))" },
    // Regression test for bug caused by fix for previous bug.
    { "author:\"milne, a.a.\"", "(Amilne:(pos=1) PHRASE 3 Aa:(pos=2) PHRASE 3 Aa:(pos=3))" },
    { "author:\"milne a.a.\"", "(Amilne:(pos=1) PHRASE 3 Aa:(pos=2) PHRASE 3 Aa:(pos=3))" },
    // Regression test for bug reported in 0.9.7.
    { "site:/path/name", "0 * H/path/name" },
    // Regression test for bug introduced (and fixed) in SVN prior to 1.0.0.
    { "author:/path/name", "(Apath:(pos=1) PHRASE 2 Aname:(pos=2))" },
    // Feature tests for change to allow phrase generators after prefix in 1.2.4.
    { "author:/path", "ZApath:(pos=1)" },
    { "author:-Foo", "Afoo:(pos=1)" },
    { "author:/", "Zauthor:(pos=1)" },
    { "author::", "Zauthor:(pos=1)" },
    { "author:/ foo", "(Zauthor:(pos=1) OR Zfoo:(pos=2))" },
    { "author:: foo", "(Zauthor:(pos=1) OR Zfoo:(pos=2))" },
    { "author::foo", "(author:(pos=1) PHRASE 2 foo:(pos=2))" },
    { "author:/ AND foo", "(Zauthor:(pos=1) AND Zfoo:(pos=2))" },
    { "author:: AND foo", "(Zauthor:(pos=1) AND Zfoo:(pos=2))" },
    { "foo AND author:/", "(Zfoo:(pos=1) AND Zauthor:(pos=2))" },
    { "foo AND author::", "(Zfoo:(pos=1) AND Zauthor:(pos=2))" },
    // Regression test for bug introduced into (and fixed) in SVN prior to 1.0.0.
    { "author:(title::case)", "(Atitle:(pos=1) PHRASE 2 Acase:(pos=2))" },
    // Regression test for bug fixed in 1.0.4 - the '+' would be ignored there
    // because the whitespace after the '"' wasn't noticed.
    { "\"hello world\" +python", "(Zpython:(pos=3) AND_MAYBE (hello:(pos=1) PHRASE 2 world:(pos=2)))" },
    // In 1.1.0, NON_SPACING_MARK was added as a word character.
    { "\xd8\xa7\xd9\x84\xd8\xb1\xd9\x91\xd8\xad\xd9\x85\xd9\x86", "Z\xd8\xa7\xd9\x84\xd8\xb1\xd9\x91\xd8\xad\xd9\x85\xd9\x86:(pos=1)" },
    // In 1.1.4, ENCLOSING_MARK and COMBINING_SPACING_MARK were added, and
    // code to ignore several zero-width space characters was added.
    { "\xe1\x80\x9d\xe1\x80\xae\xe2\x80\x8b\xe1\x80\x80\xe1\x80\xae\xe2\x80\x8b\xe1\x80\x95\xe1\x80\xad\xe2\x80\x8b\xe1\x80\x9e\xe1\x80\xaf\xe1\x80\xb6\xe1\x80\xb8\xe2\x80\x8b\xe1\x80\x85\xe1\x80\xbd\xe1\x80\xb2\xe2\x80\x8b\xe1\x80\x9e\xe1\x80\xb0\xe2\x80\x8b\xe1\x80\x99\xe1\x80\xbb\xe1\x80\xac\xe1\x80\xb8\xe1\x80\x80", "Z\xe1\x80\x9d\xe1\x80\xae\xe1\x80\x80\xe1\x80\xae\xe1\x80\x95\xe1\x80\xad\xe1\x80\x9e\xe1\x80\xaf\xe1\x80\xb6\xe1\x80\xb8\xe1\x80\x85\xe1\x80\xbd\xe1\x80\xb2\xe1\x80\x9e\xe1\x80\xb0\xe1\x80\x99\xe1\x80\xbb\xe1\x80\xac\xe1\x80\xb8\xe1\x80\x80:(pos=1)" },
    { "unmatched\"", "unmatched:(pos=1)" },
    { "unmatched \" \" ", "Zunmatch:(pos=1)" },
    { "hyphen-ated\" ", "(hyphen:(pos=1) PHRASE 2 ated:(pos=2))" },
    { "hyphen-ated\"  \"", "(hyphen:(pos=1) PHRASE 2 ated:(pos=2))" },
    { "\"1.4\"", "1.4:(pos=1)" },
    { "\"1.\"", "1:(pos=1)" },
    { "\"A#.B.\"", "(a#:(pos=1) PHRASE 2 b:(pos=2))" },
    { "\" Xapian QueryParser\" parses queries", "((xapian:(pos=1) PHRASE 2 queryparser:(pos=2)) OR Zpars:(pos=3) OR Zqueri:(pos=4))" },
    { "\" xapian queryParser\" parses queries", "((xapian:(pos=1) PHRASE 2 queryparser:(pos=2)) OR Zpars:(pos=3) OR Zqueri:(pos=4))" },
    { "h\xc3\xb6hle", "Zh\xc3\xb6hle:(pos=1)" },
    { "one +two three", "(Ztwo:(pos=2) AND_MAYBE (Zone:(pos=1) OR Zthree:(pos=3)))" },
    { "subject:test other", "(ZXTtest:(pos=1) OR Zother:(pos=2))" },
    { "subject:\"space flight\"", "(XTspace:(pos=1) PHRASE 2 XTflight:(pos=2))" },
    { "author:(twain OR poe) OR flight", "(ZAtwain:(pos=1) OR ZApoe:(pos=2) OR Zflight:(pos=3))" },
    { "author:(twain OR title:pit OR poe)", "(ZAtwain:(pos=1) OR ZXTpit:(pos=2) OR ZApoe:(pos=3))" },
    { "title:2001 title:space", "(XT2001:(pos=1) OR ZXTspace:(pos=2))" },
    { "(title:help)", "ZXThelp:(pos=1)" },
    { "beer NOT \"orange juice\"", "(Zbeer:(pos=1) AND_NOT (orange:(pos=2) PHRASE 2 juice:(pos=3)))" },
    { "beer AND NOT lager", "(Zbeer:(pos=1) AND_NOT Zlager:(pos=2))" },
    { "beer AND -lager", "(Zbeer:(pos=1) AND_NOT Zlager:(pos=2))" },
    { "beer AND +lager", "(Zbeer:(pos=1) AND Zlager:(pos=2))" },
    { "A OR B NOT C", "(a:(pos=1) OR (b:(pos=2) AND_NOT c:(pos=3)))" },
    { "A OR B AND NOT C", "(a:(pos=1) OR (b:(pos=2) AND_NOT c:(pos=3)))" },
    { "A OR B AND -C", "(a:(pos=1) OR (b:(pos=2) AND_NOT c:(pos=3)))" },
    { "A OR B AND +C", "(a:(pos=1) OR (b:(pos=2) AND c:(pos=3)))" },
    { "A OR B XOR C", "(a:(pos=1) OR (b:(pos=2) XOR c:(pos=3)))" },
    { "A XOR B NOT C", "(a:(pos=1) XOR (b:(pos=2) AND_NOT c:(pos=3)))" },
    { "one AND two", "(Zone:(pos=1) AND Ztwo:(pos=2))" },
    { "one A.N.D. two", "(Zone:(pos=1) OR and:(pos=2) OR Ztwo:(pos=3))" },
    { "one \xc3\x81ND two", "(Zone:(pos=1) OR \xc3\xa1nd:(pos=2) OR Ztwo:(pos=3))" },
    { "one author:AND two", "(Zone:(pos=1) OR Aand:(pos=2) OR Ztwo:(pos=3))" },
    { "author:hyphen-ated", "(Ahyphen:(pos=1) PHRASE 2 Aated:(pos=2))" },
    { "cvs site:xapian.org", "(Zcvs:(pos=1) FILTER Hxapian.org)" },
    { "cvs -site:xapian.org", "(Zcvs:(pos=1) AND_NOT Hxapian.org)" },
    { "foo -site:xapian.org bar", "((Zfoo:(pos=1) OR Zbar:(pos=2)) AND_NOT Hxapian.org)" },
    { "site:xapian.org mail", "(Zmail:(pos=1) FILTER Hxapian.org)" },
    { "-site:xapian.org mail", "(Zmail:(pos=1) AND_NOT Hxapian.org)" },
    { "mail AND -site:xapian.org", "(Zmail:(pos=1) AND_NOT 0 * Hxapian.org)" },
    { "-Wredundant-decls", "(wredundant:(pos=1) PHRASE 2 decls:(pos=2))" },
    { "site:xapian.org", "0 * Hxapian.org" },
    { "mug +site:xapian.org -site:cvs.xapian.org", "((Zmug:(pos=1) FILTER Hxapian.org) AND_NOT Hcvs.xapian.org)" },
    { "mug -site:cvs.xapian.org +site:xapian.org", "((Zmug:(pos=1) FILTER Hxapian.org) AND_NOT Hcvs.xapian.org)" },
    { "mug +site:xapian.org AND -site:cvs.xapian.org", "((Zmug:(pos=1) FILTER Hxapian.org) AND_NOT 0 * Hcvs.xapian.org)" },
    { "mug site:xapian.org AND -site:cvs.xapian.org", "((Zmug:(pos=1) FILTER Hxapian.org) AND_NOT 0 * Hcvs.xapian.org)" },
    { "mug site:xapian.org AND +site:cvs.xapian.org", "((Zmug:(pos=1) FILTER Hxapian.org) AND 0 * Hcvs.xapian.org)" },
    { "NOT windows", "Syntax: <expression> NOT <expression>" },
    { "a AND (NOT b)", "Syntax: <expression> NOT <expression>" },
    { "AND NOT windows", "Syntax: <expression> AND NOT <expression>" },
    { "AND -windows", "Syntax: <expression> AND <expression>" }, 
    { "gordian NOT", "Syntax: <expression> NOT <expression>" },
    { "gordian AND NOT", "Syntax: <expression> AND NOT <expression>" },
    { "gordian AND -", "Syntax: <expression> AND <expression>" },
    { "foo OR (something AND)", "Syntax: <expression> AND <expression>" },
    { "OR foo", "Syntax: <expression> OR <expression>" },
    { "XOR", "Syntax: <expression> XOR <expression>" },
    { "hard\xa0space", "(Zhard:(pos=1) OR Zspace:(pos=2))" },
    { " white\r\nspace\ttest ", "(Zwhite:(pos=1) OR Zspace:(pos=2) OR Ztest:(pos=3))" },
    { "one AND two three", "(Zone:(pos=1) AND (Ztwo:(pos=2) OR Zthree:(pos=3)))" },
    { "one two AND three", "((Zone:(pos=1) OR Ztwo:(pos=2)) AND Zthree:(pos=3))" },
    { "one AND two/three", "(Zone:(pos=1) AND (two:(pos=2) PHRASE 2 three:(pos=3)))" },
    { "one AND /two/three", "(Zone:(pos=1) AND (two:(pos=2) PHRASE 2 three:(pos=3)))" },
    { "one AND/two/three", "(Zone:(pos=1) AND (two:(pos=2) PHRASE 2 three:(pos=3)))" },
    { "one +/two/three", "((two:(pos=2) PHRASE 2 three:(pos=3)) AND_MAYBE Zone:(pos=1))" },
    { "one//two", "(one:(pos=1) PHRASE 2 two:(pos=2))" },
    { "\"missing quote", "(missing:(pos=1) PHRASE 2 quote:(pos=2))" },
    { "DVD+RW", "(dvd:(pos=1) OR rw:(pos=2))" }, // Would a phrase be better?
    { "+\"must have\" optional", "((must:(pos=1) PHRASE 2 have:(pos=2)) AND_MAYBE Zoption:(pos=3))" },
    { "one NEAR two NEAR three", "(one:(pos=1) NEAR 12 two:(pos=2) NEAR 12 three:(pos=3))" },
    { "something NEAR/3 else", "(something:(pos=1) NEAR 4 else:(pos=2))" },
    { "a NEAR/6 b NEAR c", "(a:(pos=1) NEAR 8 b:(pos=2) NEAR 8 c:(pos=3))" },
    { "something ADJ else", "(something:(pos=1) PHRASE 11 else:(pos=2))" },
    { "something ADJ/3 else", "(something:(pos=1) PHRASE 4 else:(pos=2))" },
    { "a ADJ/6 b ADJ c", "(a:(pos=1) PHRASE 8 b:(pos=2) PHRASE 8 c:(pos=3))" },
    // Regression test - Unicode character values were truncated to 8 bits
    // before testing C_isdigit(), so this rather artificial example parsed
    // to: (a:(pos=1) NEAR 262 b:(pos=2))
    { "a NEAR/\xc4\xb5 b", "(Za:(pos=1) OR (near:(pos=2) PHRASE 2 \xc4\xb5:(pos=3)) OR Zb:(pos=4))" },
    { "a ADJ/\xc4\xb5 b", "(Za:(pos=1) OR (adj:(pos=2) PHRASE 2 \xc4\xb5:(pos=3)) OR Zb:(pos=4))" },
    // Regression test - the first two cases were parsed as if the '/' were a
    // space, which was inconsistent with the second two.  Fixed in 1.2.5.
    { "a NEAR/b", "(Za:(pos=1) OR (near:(pos=2) PHRASE 2 b:(pos=3)))" },
    { "a ADJ/b", "(Za:(pos=1) OR (adj:(pos=2) PHRASE 2 b:(pos=3)))" },
    { "a NEAR/b c", "(Za:(pos=1) OR (near:(pos=2) PHRASE 2 b:(pos=3)) OR Zc:(pos=4))" },
    { "a ADJ/b c", "(Za:(pos=1) OR (adj:(pos=2) PHRASE 2 b:(pos=3)) OR Zc:(pos=4))" },
    // Regression tests - + and - didn't work on bracketed subexpressions prior
    // to 1.0.2.
    { "+(one two) three", "((Zone:(pos=1) OR Ztwo:(pos=2)) AND_MAYBE Zthree:(pos=3))" },
    { "zero -(one two)", "(Zzero:(pos=1) AND_NOT (Zone:(pos=2) OR Ztwo:(pos=3)))" },
    // Feature tests that ':' is inserted between prefix and term correctly:
    { "category:Foo", "0 * XCAT:Foo" },
    { "category:foo", "0 * XCATfoo" },
    { "category:\xc3\x96oo", "0 * XCAT\xc3\x96oo" },
    // Feature tests for quoted boolean terms:
    { "category:\"Hello world\"", "0 * XCAT:Hello world" },
    { "category:\"literal \"\"\"", "0 * XCATliteral \"" },
    { "category:\"(unterminated)", "0 * XCAT(unterminated)" },
    // Feature tests for implicitly closing brackets:
    { "(foo", "Zfoo:(pos=1)" },
    { "(foo XOR bar", "(Zfoo:(pos=1) XOR Zbar:(pos=2))" },
    { "(foo XOR (bar AND baz)", "(Zfoo:(pos=1) XOR (Zbar:(pos=2) AND Zbaz:(pos=3)))" },
    { "(foo XOR (bar AND baz", "(Zfoo:(pos=1) XOR (Zbar:(pos=2) AND Zbaz:(pos=3)))" },
    // Real world examples from tweakers.net:
    { "Call to undefined function: imagecreate()", "(call:(pos=1) OR Zto:(pos=2) OR Zundefin:(pos=3) OR Zfunction:(pos=4) OR imagecreate:(pos=5))" },
    { "mysql_fetch_row(): supplied argument is not a valid MySQL result resource", "(mysql_fetch_row:(pos=1) OR Zsuppli:(pos=2) OR Zargument:(pos=3) OR Zis:(pos=4) OR Znot:(pos=5) OR Za:(pos=6) OR Zvalid:(pos=7) OR mysql:(pos=8) OR Zresult:(pos=9) OR Zresourc:(pos=10))" },
    { "php date() nedelands", "(Zphp:(pos=1) OR date:(pos=2) OR Znedeland:(pos=3))" },
    { "wget domein --http-user", "(Zwget:(pos=1) OR Zdomein:(pos=2) OR (http:(pos=3) PHRASE 2 user:(pos=4)))" },
    { "@home problemen", "(Zhome:(pos=1) OR Zproblemen:(pos=2))" },
    { "'ipacsum'", "Zipacsum:(pos=1)" },
    { "canal + ", "Zcanal:(pos=1)" },
    { "/var/run/mysqld/mysqld.sock", "(var:(pos=1) PHRASE 5 run:(pos=2) PHRASE 5 mysqld:(pos=3) PHRASE 5 mysqld:(pos=4) PHRASE 5 sock:(pos=5))" },
    { "\"QSI-161 drivers\"", "(qsi:(pos=1) PHRASE 3 161:(pos=2) PHRASE 3 drivers:(pos=3))" },
    { "\"e-cube\" barebone", "((e:(pos=1) PHRASE 2 cube:(pos=2)) OR Zbarebon:(pos=3))" },
    { "\"./httpd: symbol not found: dlopen\"", "(httpd:(pos=1) PHRASE 5 symbol:(pos=2) PHRASE 5 not:(pos=3) PHRASE 5 found:(pos=4) PHRASE 5 dlopen:(pos=5))" },
    { "ERROR 2003: Can't connect to MySQL server on 'localhost' (10061)", "(error:(pos=1) OR 2003:(pos=2) OR can't:(pos=3) OR Zconnect:(pos=4) OR Zto:(pos=5) OR mysql:(pos=6) OR Zserver:(pos=7) OR Zon:(pos=8) OR Zlocalhost:(pos=9) OR 10061:(pos=10))" },
    { "location.href = \"\"", "(location:(pos=1) PHRASE 2 href:(pos=2))" },
    { "method=\"post\" action=\"\">", "(method:(pos=1) OR post:(pos=2) OR action:(pos=3))" },
    { "behuizing 19\" inch", "(Zbehuiz:(pos=1) OR 19:(pos=2) OR inch:(pos=3))" },
    { "19\" rack", "(19:(pos=1) OR rack:(pos=2))" },
    { "3,5\" mainboard", "(3,5:(pos=1) OR mainboard:(pos=2))" },
    { "553 sorry, that domain isn't in my list of allowed rcpthosts (#5.7.1)", "(553:(pos=1) OR Zsorri:(pos=2) OR Zthat:(pos=3) OR Zdomain:(pos=4) OR Zisn't:(pos=5) OR Zin:(pos=6) OR Zmy:(pos=7) OR Zlist:(pos=8) OR Zof:(pos=9) OR Zallow:(pos=10) OR Zrcpthost:(pos=11) OR 5.7.1:(pos=12))" },
    { "data error (clic redundancy check)", "(Zdata:(pos=1) OR Zerror:(pos=2) OR Zclic:(pos=3) OR Zredund:(pos=4) OR Zcheck:(pos=5))" },
    { "? mediaplayer 9\"", "(Zmediaplay:(pos=1) OR 9:(pos=2))" },
    { "date(\"w\")", "(date:(pos=1) OR w:(pos=2))" },
    { "Syntaxisfout (operator ontbreekt ASP", "(syntaxisfout:(pos=1) OR Zoper:(pos=2) OR Zontbreekt:(pos=3) OR asp:(pos=4))" },
    { "Request.ServerVariables(\"logon_user\")", "((request:(pos=1) PHRASE 2 servervariables:(pos=2)) OR logon_user:(pos=3))" },
    { "ASP \"request.form\" van \\\"enctype=\"MULTIPART/FORM-DATA\"\\\"", "(asp:(pos=1) OR (request:(pos=2) PHRASE 2 form:(pos=3)) OR Zvan:(pos=4) OR enctype:(pos=5) OR (multipart:(pos=6) PHRASE 3 form:(pos=7) PHRASE 3 data:(pos=8)))" },
    { "USER ftp (Login failed): Invalid shell: /sbin/nologin", "(user:(pos=1) OR Zftp:(pos=2) OR login:(pos=3) OR Zfail:(pos=4) OR invalid:(pos=5) OR Zshell:(pos=6) OR (sbin:(pos=7) PHRASE 2 nologin:(pos=8)))" },
    { "ip_masq_new(proto=TCP)", "(ip_masq_new:(pos=1) OR proto:(pos=2) OR tcp:(pos=3))" },
    { "\"document.write(\"", "(document:(pos=1) PHRASE 2 write:(pos=2))" },
    { "ERROR 1045: Access denied for user: 'root@localhost' (Using password: NO)", "(error:(pos=1) OR 1045:(pos=2) OR access:(pos=3) OR Zdeni:(pos=4) OR Zfor:(pos=5) OR Zuser:(pos=6) OR (root:(pos=7) PHRASE 2 localhost:(pos=8)) OR using:(pos=9) OR Zpassword:(pos=10) OR no:(pos=11))" },
    { "TIP !! subtitles op TV-out (via DVD max g400)", "(tip:(pos=1) OR Zsubtitl:(pos=2) OR Zop:(pos=3) OR (tv:(pos=4) PHRASE 2 out:(pos=5)) OR Zvia:(pos=6) OR dvd:(pos=7) OR Zmax:(pos=8) OR Zg400:(pos=9))" },
    { "Gigabyte 8PE667 (de Ultra versie) of Asus A7N8X Deluxe", "(gigabyte:(pos=1) OR 8pe667:(pos=2) OR Zde:(pos=3) OR ultra:(pos=4) OR Zversi:(pos=5) OR Zof:(pos=6) OR asus:(pos=7) OR a7n8x:(pos=8) OR deluxe:(pos=9))" },
    { "\"1) Ze testen 8x AF op de GFFX tegen \"", "(1:(pos=1) PHRASE 9 ze:(pos=2) PHRASE 9 testen:(pos=3) PHRASE 9 8x:(pos=4) PHRASE 9 af:(pos=5) PHRASE 9 op:(pos=6) PHRASE 9 de:(pos=7) PHRASE 9 gffx:(pos=8) PHRASE 9 tegen:(pos=9))" },
    { "\") Ze houden geen rekening met de kwaliteit van AF. Als ze dat gedaan hadden dan waren ze tot de conclusie gekomen dat Performance AF (dus Bilinear AF) op de 9700Pro goed te vergelijken is met Balanced AF op de GFFX. En dan hadden ze ook gezien dat de GFFX niet kan tippen aan de Quality AF van de 9700Pro.\"", "(ze:(pos=1) PHRASE 59 houden:(pos=2) PHRASE 59 geen:(pos=3) PHRASE 59 rekening:(pos=4) PHRASE 59 met:(pos=5) PHRASE 59 de:(pos=6) PHRASE 59 kwaliteit:(pos=7) PHRASE 59 van:(pos=8) PHRASE 59 af:(pos=9) PHRASE 59 als:(pos=10) PHRASE 59 ze:(pos=11) PHRASE 59 dat:(pos=12) PHRASE 59 gedaan:(pos=13) PHRASE 59 hadden:(pos=14) PHRASE 59 dan:(pos=15) PHRASE 59 waren:(pos=16) PHRASE 59 ze:(pos=17) PHRASE 59 tot:(pos=18) PHRASE 59 de:(pos=19) PHRASE 59 conclusie:(pos=20) PHRASE 59 gekomen:(pos=21) PHRASE 59 dat:(pos=22) PHRASE 59 performance:(pos=23) PHRASE 59 af:(pos=24) PHRASE 59 dus:(pos=25) PHRASE 59 bilinear:(pos=26) PHRASE 59 af:(pos=27) PHRASE 59 op:(pos=28) PHRASE 59 de:(pos=29) PHRASE 59 9700pro:(pos=30) PHRASE 59 goed:(pos=31) PHRASE 59 te:(pos=32) PHRASE 59 vergelijken:(pos=33) PHRASE 59 is:(pos=34) PHRASE 59 met:(pos=35) PHRASE 59 balanced:(pos=36) PHRASE 59 af:(pos=37) PHRASE 59 op:(pos=38) PHRASE 59 de:(pos=39) PHRASE 59 gffx:(pos=40) PHRASE 59 en:(pos=41) PHRASE 59 dan:(pos=42) PHRASE 59 hadden:(pos=43) PHRASE 59 ze:(pos=44) PHRASE 59 ook:(pos=45) PHRASE 59 gezien:(pos=46) PHRASE 59 dat:(pos=47) PHRASE 59 de:(pos=48) PHRASE 59 gffx:(pos=49) PHRASE 59 niet:(pos=50) PHRASE 59 kan:(pos=51) PHRASE 59 tippen:(pos=52) PHRASE 59 aan:(pos=53) PHRASE 59 de:(pos=54) PHRASE 59 quality:(pos=55) PHRASE 59 af:(pos=56) PHRASE 59 van:(pos=57) PHRASE 59 de:(pos=58) PHRASE 59 9700pro:(pos=59))" },
    { "\"Ze houden geen rekening met de kwaliteit van AF. Als ze dat gedaan hadden dan waren ze tot de conclusie gekomen dat Performance AF (dus Bilinear AF) op de 9700Pro goed te vergelijken is met Balanced AF op de GFFX. En dan hadden ze ook gezien dat de GFFX niet kan tippen aan de Quality AF van de 9700Pro.\"", "(ze:(pos=1) PHRASE 59 houden:(pos=2) PHRASE 59 geen:(pos=3) PHRASE 59 rekening:(pos=4) PHRASE 59 met:(pos=5) PHRASE 59 de:(pos=6) PHRASE 59 kwaliteit:(pos=7) PHRASE 59 van:(pos=8) PHRASE 59 af:(pos=9) PHRASE 59 als:(pos=10) PHRASE 59 ze:(pos=11) PHRASE 59 dat:(pos=12) PHRASE 59 gedaan:(pos=13) PHRASE 59 hadden:(pos=14) PHRASE 59 dan:(pos=15) PHRASE 59 waren:(pos=16) PHRASE 59 ze:(pos=17) PHRASE 59 tot:(pos=18) PHRASE 59 de:(pos=19) PHRASE 59 conclusie:(pos=20) PHRASE 59 gekomen:(pos=21) PHRASE 59 dat:(pos=22) PHRASE 59 performance:(pos=23) PHRASE 59 af:(pos=24) PHRASE 59 dus:(pos=25) PHRASE 59 bilinear:(pos=26) PHRASE 59 af:(pos=27) PHRASE 59 op:(pos=28) PHRASE 59 de:(pos=29) PHRASE 59 9700pro:(pos=30) PHRASE 59 goed:(pos=31) PHRASE 59 te:(pos=32) PHRASE 59 vergelijken:(pos=33) PHRASE 59 is:(pos=34) PHRASE 59 met:(pos=35) PHRASE 59 balanced:(pos=36) PHRASE 59 af:(pos=37) PHRASE 59 op:(pos=38) PHRASE 59 de:(pos=39) PHRASE 59 gffx:(pos=40) PHRASE 59 en:(pos=41) PHRASE 59 dan:(pos=42) PHRASE 59 hadden:(pos=43) PHRASE 59 ze:(pos=44) PHRASE 59 ook:(pos=45) PHRASE 59 gezien:(pos=46) PHRASE 59 dat:(pos=47) PHRASE 59 de:(pos=48) PHRASE 59 gffx:(pos=49) PHRASE 59 niet:(pos=50) PHRASE 59 kan:(pos=51) PHRASE 59 tippen:(pos=52) PHRASE 59 aan:(pos=53) PHRASE 59 de:(pos=54) PHRASE 59 quality:(pos=55) PHRASE 59 af:(pos=56) PHRASE 59 van:(pos=57) PHRASE 59 de:(pos=58) PHRASE 59 9700pro:(pos=59))" },
    { "$structure = imap_header($mbox, $tt);", "(Zstructur:(pos=1) OR imap_header:(pos=2) OR Zmbox:(pos=3) OR Ztt:(pos=4))" },
    { "\"ifup: Could not get a valid interface name: -> skipped\"", "(ifup:(pos=1) PHRASE 9 could:(pos=2) PHRASE 9 not:(pos=3) PHRASE 9 get:(pos=4) PHRASE 9 a:(pos=5) PHRASE 9 valid:(pos=6) PHRASE 9 interface:(pos=7) PHRASE 9 name:(pos=8) PHRASE 9 skipped:(pos=9))" },
    { "Er kan geen combinatie van filters worden gevonden om de gegevensstroom te genereren. (Error=80040218)", "(er:(pos=1) OR Zkan:(pos=2) OR Zgeen:(pos=3) OR Zcombinati:(pos=4) OR Zvan:(pos=5) OR Zfilter:(pos=6) OR Zworden:(pos=7) OR Zgevonden:(pos=8) OR Zom:(pos=9) OR Zde:(pos=10) OR Zgegevensstroom:(pos=11) OR Zte:(pos=12) OR Zgenereren:(pos=13) OR error:(pos=14) OR 80040218:(pos=15))" },
    { "ereg_replace(\"\\\\\",\"\\/\"", "ereg_replace:(pos=1)" },
    { "\\\\\"divx+geen+geluid\\\\\"", "(divx:(pos=1) PHRASE 3 geen:(pos=2) PHRASE 3 geluid:(pos=3))" },
    { "lcase(\"string\")", "(lcase:(pos=1) OR string:(pos=2))" },
    { "isEmpty( )  functie in visual basic", "(isempty:(pos=1) OR Zfuncti:(pos=2) OR Zin:(pos=3) OR Zvisual:(pos=4) OR Zbasic:(pos=5))" },
    { "*** stop: 0x0000001E (0xC0000005,0x00000000,0x00000000,0x00000000)", "(Zstop:(pos=1) OR 0x0000001e:(pos=2) OR 0xc0000005,0x00000000,0x00000000,0x00000000:(pos=3))" },
    { "\"ctrl+v+c+a fout\"", "(ctrl:(pos=1) PHRASE 5 v:(pos=2) PHRASE 5 c:(pos=3) PHRASE 5 a:(pos=4) PHRASE 5 fout:(pos=5))" },
    { "Server.CreateObject(\"ADODB.connection\")", "((server:(pos=1) PHRASE 2 createobject:(pos=2)) OR (adodb:(pos=3) PHRASE 2 connection:(pos=4)))" },
    { "Presario 6277EA-XP model P4/28 GHz-120GB-DVD-CDRW (512MBWXP) (470048-012)", "(presario:(pos=1) OR (6277ea:(pos=2) PHRASE 2 xp:(pos=3)) OR Zmodel:(pos=4) OR (p4:(pos=5) PHRASE 2 28:(pos=6)) OR (ghz:(pos=7) PHRASE 4 120gb:(pos=8) PHRASE 4 dvd:(pos=9) PHRASE 4 cdrw:(pos=10)) OR 512mbwxp:(pos=11) OR (470048:(pos=12) PHRASE 2 012:(pos=13)))" },
    { "Failed to connect agent. (AGENT=dbaxchg2, EC=UserId =NUll)", "(failed:(pos=1) OR Zto:(pos=2) OR Zconnect:(pos=3) OR Zagent:(pos=4) OR agent:(pos=5) OR Zdbaxchg2:(pos=6) OR ec:(pos=7) OR userid:(pos=8) OR null:(pos=9))" },
    { "delphi CreateOleObject(\"MSXML2.DomDocument\")", "(Zdelphi:(pos=1) OR createoleobject:(pos=2) OR (msxml2:(pos=3) PHRASE 2 domdocument:(pos=4)))" },
    { "Unhandled exeption in IEXPLORE.EXE (FTAPP.DLL)", "(unhandled:(pos=1) OR Zexept:(pos=2) OR Zin:(pos=3) OR (iexplore:(pos=4) PHRASE 2 exe:(pos=5)) OR (ftapp:(pos=6) PHRASE 2 dll:(pos=7)))" },
    { "IBM High Rate Wireless LAN PCI Adapter (Low Profile Enabled)", "(ibm:(pos=1) OR high:(pos=2) OR rate:(pos=3) OR wireless:(pos=4) OR lan:(pos=5) OR pci:(pos=6) OR adapter:(pos=7) OR low:(pos=8) OR profile:(pos=9) OR enabled:(pos=10))" },
    { "asp ' en \"", "(Zasp:(pos=1) OR Zen:(pos=2))" },
    { "Hercules 3D Prophet 8500 LE 64MB (OEM, Radeon 8500 LE)", "(hercules:(pos=1) OR 3d:(pos=2) OR prophet:(pos=3) OR 8500:(pos=4) OR le:(pos=5) OR 64mb:(pos=6) OR oem:(pos=7) OR radeon:(pos=8) OR 8500:(pos=9) OR le:(pos=10))" },
    { "session_set_cookie_params(echo \"hoi\")", "(session_set_cookie_params:(pos=1) OR Zecho:(pos=2) OR hoi:(pos=3))" },
    { "windows update werkt niet (windows se", "(Zwindow:(pos=1) OR Zupdat:(pos=2) OR Zwerkt:(pos=3) OR Zniet:(pos=4) OR Zwindow:(pos=5) OR Zse:(pos=6))" },
    { "De statuscode van de fout is ( 0 x 4 , 0 , 0 , 0 )", "(de:(pos=1) OR Zstatuscod:(pos=2) OR Zvan:(pos=3) OR Zde:(pos=4) OR Zfout:(pos=5) OR Zis:(pos=6) OR 0:(pos=7) OR Zx:(pos=8) OR 4:(pos=9) OR 0:(pos=10) OR 0:(pos=11) OR 0:(pos=12))" },
    { "sony +(u20 u-20)", "((Zu20:(pos=2) OR (u:(pos=3) PHRASE 2 20:(pos=4))) AND_MAYBE Zsoni:(pos=1))" },
    { "[crit] (17)File exists: unable to create scoreboard (name-based shared memory failure)", "(Zcrit:(pos=1) OR 17:(pos=2) OR file:(pos=3) OR Zexist:(pos=4) OR Zunabl:(pos=5) OR Zto:(pos=6) OR Zcreat:(pos=7) OR Zscoreboard:(pos=8) OR (name:(pos=9) PHRASE 2 based:(pos=10)) OR Zshare:(pos=11) OR Zmemori:(pos=12) OR Zfailur:(pos=13))" },
    { "directories lokaal php (uitlezen OR inladen)", "(Zdirectori:(pos=1) OR Zlokaal:(pos=2) OR Zphp:(pos=3) OR Zuitlezen:(pos=4) OR Zinladen:(pos=5))" },
    { "(multi pc modem)+ (line sync)", "(Zmulti:(pos=1) OR Zpc:(pos=2) OR Zmodem:(pos=3) OR Zline:(pos=4) OR Zsync:(pos=5))" },
    { "xp 5.1.2600.0 (xpclient.010817-1148)", "(Zxp:(pos=1) OR 5.1.2600.0:(pos=2) OR (xpclient:(pos=3) PHRASE 3 010817:(pos=4) PHRASE 3 1148:(pos=5)))" },
    { "DirectDraw test results: Failure at step 5 (User verification of rectangles): HRESULT = 0x00000000 (error code) Direct3D 7 test results: Failure at step 32 (User verification of Direct3D rendering): HRESULT = 0x00000000 (error code) Direct3D 8 test results: Failure at step 32 (User verification of Direct3D rendering): HRESULT = 0x00000000 (error code) Direct3D 9 test results: Failure at step 32 (User verification of Direct3D rendering): HRESULT = 0x00000000 (error code)", "(directdraw:(pos=1) OR Ztest:(pos=2) OR Zresult:(pos=3) OR failure:(pos=4) OR Zat:(pos=5) OR Zstep:(pos=6) OR 5:(pos=7) OR user:(pos=8) OR Zverif:(pos=9) OR Zof:(pos=10) OR Zrectangl:(pos=11) OR hresult:(pos=12) OR 0x00000000:(pos=13) OR Zerror:(pos=14) OR Zcode:(pos=15) OR direct3d:(pos=16) OR 7:(pos=17) OR Ztest:(pos=18) OR Zresult:(pos=19) OR failure:(pos=20) OR Zat:(pos=21) OR Zstep:(pos=22) OR 32:(pos=23) OR user:(pos=24) OR Zverif:(pos=25) OR Zof:(pos=26) OR direct3d:(pos=27) OR Zrender:(pos=28) OR hresult:(pos=29) OR 0x00000000:(pos=30) OR Zerror:(pos=31) OR Zcode:(pos=32) OR direct3d:(pos=33) OR 8:(pos=34) OR Ztest:(pos=35) OR Zresult:(pos=36) OR failure:(pos=37) OR Zat:(pos=38) OR Zstep:(pos=39) OR 32:(pos=40) OR user:(pos=41) OR Zverif:(pos=42) OR Zof:(pos=43) OR direct3d:(pos=44) OR Zrender:(pos=45) OR hresult:(pos=46) OR 0x00000000:(pos=47) OR Zerror:(pos=48) OR Zcode:(pos=49) OR direct3d:(pos=50) OR 9:(pos=51) OR Ztest:(pos=52) OR Zresult:(pos=53) OR failure:(pos=54) OR Zat:(pos=55) OR Zstep:(pos=56) OR 32:(pos=57) OR user:(pos=58) OR Zverif:(pos=59) OR Zof:(pos=60) OR direct3d:(pos=61) OR Zrender:(pos=62) OR hresult:(pos=63) OR 0x00000000:(pos=64) OR Zerror:(pos=65) OR Zcode:(pos=66))" },
    { "Thermaltake Aquarius II waterkoeling (kompleet voor P4 en XP)", "(thermaltake:(pos=1) OR aquarius:(pos=2) OR ii:(pos=3) OR Zwaterkoel:(pos=4) OR Zkompleet:(pos=5) OR Zvoor:(pos=6) OR p4:(pos=7) OR Zen:(pos=8) OR xp:(pos=9))" },
    { "E3501 unable to add job to database (EC=-2005)", "(e3501:(pos=1) OR Zunabl:(pos=2) OR Zto:(pos=3) OR Zadd:(pos=4) OR Zjob:(pos=5) OR Zto:(pos=6) OR Zdatabas:(pos=7) OR ec:(pos=8) OR 2005:(pos=9))" },
    { "\"arp -s\" ip veranderen", "((arp:(pos=1) PHRASE 2 s:(pos=2)) OR Zip:(pos=3) OR Zveranderen:(pos=4))" },
    { "header(\"content-type: application/octet-stream\");", "(header:(pos=1) OR (content:(pos=2) PHRASE 2 type:(pos=3)) OR (application:(pos=4) PHRASE 3 octet:(pos=5) PHRASE 3 stream:(pos=6)))" },
    { "$datum = date(\"d-m-Y\");", "(Zdatum:(pos=1) OR date:(pos=2) OR (d:(pos=3) PHRASE 3 m:(pos=4) PHRASE 3 y:(pos=5)))" },
    { "\"'\" +asp", "Zasp:(pos=1)" },
    { "+session +[", "Zsession:(pos=1)" },
    { "Dit apparaat kan niet starten. (Code 10)", "(dit:(pos=1) OR Zapparaat:(pos=2) OR Zkan:(pos=3) OR Zniet:(pos=4) OR Zstarten:(pos=5) OR code:(pos=6) OR 10:(pos=7))" },
    { "\"You cannot use the Administration program while the Domino Server is running. Either shut down the Domino Server (but keep the file server running) or choose the ican labeled 'Lotus Notes' instead.\"", "(you:(pos=1) PHRASE 32 cannot:(pos=2) PHRASE 32 use:(pos=3) PHRASE 32 the:(pos=4) PHRASE 32 administration:(pos=5) PHRASE 32 program:(pos=6) PHRASE 32 while:(pos=7) PHRASE 32 the:(pos=8) PHRASE 32 domino:(pos=9) PHRASE 32 server:(pos=10) PHRASE 32 is:(pos=11) PHRASE 32 running:(pos=12) PHRASE 32 either:(pos=13) PHRASE 32 shut:(pos=14) PHRASE 32 down:(pos=15) PHRASE 32 the:(pos=16) PHRASE 32 domino:(pos=17) PHRASE 32 server:(pos=18) PHRASE 32 but:(pos=19) PHRASE 32 keep:(pos=20) PHRASE 32 the:(pos=21) PHRASE 32 file:(pos=22) PHRASE 32 server:(pos=23) PHRASE 32 running:(pos=24) PHRASE 32 or:(pos=25) PHRASE 32 choose:(pos=26) PHRASE 32 the:(pos=27) PHRASE 32 ican:(pos=28) PHRASE 32 labeled:(pos=29) PHRASE 32 lotus:(pos=30) PHRASE 32 notes:(pos=31) PHRASE 32 instead:(pos=32))" },
    { "\"+irq +veranderen +xp\"", "(irq:(pos=1) PHRASE 3 veranderen:(pos=2) PHRASE 3 xp:(pos=3))" },
    { "\"is not a member of 'operator``global namespace''' + c++", "(is:(pos=1) PHRASE 9 not:(pos=2) PHRASE 9 a:(pos=3) PHRASE 9 member:(pos=4) PHRASE 9 of:(pos=5) PHRASE 9 operator:(pos=6) PHRASE 9 global:(pos=7) PHRASE 9 namespace:(pos=8) PHRASE 9 c++:(pos=9))" },
    { "mkdir() failed (File exists) php", "(mkdir:(pos=1) OR Zfail:(pos=2) OR file:(pos=3) OR Zexist:(pos=4) OR Zphp:(pos=5))" },
    { "laatsteIndex(int n)", "(laatsteindex:(pos=1) OR Zint:(pos=2) OR Zn:(pos=3))" },
    { "\"line+in\" OR \"c8783\"", "((line:(pos=1) PHRASE 2 in:(pos=2)) OR c8783:(pos=3))" },
    { "if ($_POST['Submit'])", "(Zif:(pos=1) OR _post:(pos=2) OR submit:(pos=3))" },
    { "NEC DVD+-RW ND-1300A", "(nec:(pos=1) OR (dvd+:(pos=2) PHRASE 2 rw:(pos=3)) OR (nd:(pos=4) PHRASE 2 1300a:(pos=5)))" },
    { "*String not found* (*String not found*.)", "(string:(pos=1) OR Znot:(pos=2) OR found:(pos=3) OR string:(pos=4) OR Znot:(pos=5) OR found:(pos=6))" },
    { "MSI G4Ti4200-TD 128MB (GeForce4 Ti4200)", "(msi:(pos=1) OR (g4ti4200:(pos=2) PHRASE 2 td:(pos=3)) OR 128mb:(pos=4) OR geforce4:(pos=5) OR ti4200:(pos=6))" },
    { "href=\"#\"", "href:(pos=1)" },
    { "Request.ServerVariables(\"REMOTE_USER\") javascript", "((request:(pos=1) PHRASE 2 servervariables:(pos=2)) OR remote_user:(pos=3) OR Zjavascript:(pos=4))" },
    { "XF86Config(-4) waar", "(xf86config:(pos=1) OR 4:(pos=2) OR Zwaar:(pos=3))" },
    { "Unknown (tag 2000)", "(unknown:(pos=1) OR Ztag:(pos=2) OR 2000:(pos=3))" },
    { "KT4V(MS-6712)", "(kt4v:(pos=1) OR (ms:(pos=2) PHRASE 2 6712:(pos=3)))" },
    { "scheduled+AND+nieuwsgroepen+AND+updaten", "(Zschedul:(pos=1) AND Znieuwsgroepen:(pos=2) AND Zupdaten:(pos=3))" },
    { "137(netbios-ns)", "(137:(pos=1) OR (netbios:(pos=2) PHRASE 2 ns:(pos=3)))" },
    { "HARWARE ERROR, TRACKING SERVO (4:0X09:0X01)", "(harware:(pos=1) OR error:(pos=2) OR tracking:(pos=3) OR servo:(pos=4) OR (4:(pos=5) PHRASE 3 0x09:(pos=6) PHRASE 3 0x01:(pos=7)))" },
    { "Chr(10) wat is code van \" teken", "(chr:(pos=1) OR 10:(pos=2) OR Zwat:(pos=3) OR Zis:(pos=4) OR Zcode:(pos=5) OR Zvan:(pos=6) OR Zteken:(pos=7))" },
    { "wat is code van \" teken", "(Zwat:(pos=1) OR Zis:(pos=2) OR Zcode:(pos=3) OR Zvan:(pos=4) OR teken:(pos=5))" },
    { "The Jet VBA file (VBAJET.dll for 16-bit version, VBAJET32.dll version", "(the:(pos=1) OR jet:(pos=2) OR vba:(pos=3) OR Zfile:(pos=4) OR (vbajet:(pos=5) PHRASE 2 dll:(pos=6)) OR Zfor:(pos=7) OR (16:(pos=8) PHRASE 2 bit:(pos=9)) OR Zversion:(pos=10) OR (vbajet32:(pos=11) PHRASE 2 dll:(pos=12)) OR Zversion:(pos=13))" },
    { "Permission denied (publickey,password,keyboard-interactive).", "(permission:(pos=1) OR Zdeni:(pos=2) OR Zpublickey:(pos=3) OR Zpassword:(pos=4) OR (keyboard:(pos=5) PHRASE 2 interactive:(pos=6)))" },
    { "De lees- of schrijfbewerking (\"written\") op het geheugen is mislukt", "(de:(pos=1) OR Zlee:(pos=2) OR Zof:(pos=3) OR Zschrijfbewerk:(pos=4) OR written:(pos=5) OR Zop:(pos=6) OR Zhet:(pos=7) OR Zgeheugen:(pos=8) OR Zis:(pos=9) OR Zmislukt:(pos=10))" },
    { "Primary IDE channel no 80 conductor cable installed\"", "(primary:(pos=1) OR ide:(pos=2) OR Zchannel:(pos=3) OR Zno:(pos=4) OR 80:(pos=5) OR Zconductor:(pos=6) OR Zcabl:(pos=7) OR installed:(pos=8))" },
    { "\"2020 NEAR zoom\"", "(2020:(pos=1) PHRASE 3 near:(pos=2) PHRASE 3 zoom:(pos=3))" },
    { "setcookie(\"naam\",\"$user\");", "(setcookie:(pos=1) OR naam:(pos=2) OR user:(pos=3))" },
    { "MSI 645 Ultra (MS-6547) Ver1", "(msi:(pos=1) OR 645:(pos=2) OR ultra:(pos=3) OR (ms:(pos=4) PHRASE 2 6547:(pos=5)) OR ver1:(pos=6))" },
    { "if ($HTTP", "(Zif:(pos=1) OR http:(pos=2))" },
    { "data error(cyclic redundancy check)", "(Zdata:(pos=1) OR error:(pos=2) OR Zcyclic:(pos=3) OR Zredund:(pos=4) OR Zcheck:(pos=5))" },
    { "UObject::StaticAllocateObject <- (NULL None) <- UObject::StaticConstructObject <- InitEngine", "((uobject:(pos=1) PHRASE 2 staticallocateobject:(pos=2)) OR null:(pos=3) OR none:(pos=4) OR (uobject:(pos=5) PHRASE 2 staticconstructobject:(pos=6)) OR initengine:(pos=7))" },
    { "Failure at step 8 (Creating 3D Device)", "(failure:(pos=1) OR Zat:(pos=2) OR Zstep:(pos=3) OR 8:(pos=4) OR creating:(pos=5) OR 3d:(pos=6) OR device:(pos=7))" },
    { "Call Shell(\"notepad.exe\",", "(call:(pos=1) OR shell:(pos=2) OR (notepad:(pos=3) PHRASE 2 exe:(pos=4)))" },
    { "2.5\" harddisk converter", "(2.5:(pos=1) OR (harddisk:(pos=2) PHRASE 2 converter:(pos=3)))" }, // FIXME better if " didn't generate a phrase here...
    { "creative labs \"dvd+rw\"", "(Zcreativ:(pos=1) OR Zlab:(pos=2) OR (dvd:(pos=3) PHRASE 2 rw:(pos=4)))" },
    { "\"het beleid van deze computer staat u niet toe interactief", "(het:(pos=1) PHRASE 10 beleid:(pos=2) PHRASE 10 van:(pos=3) PHRASE 10 deze:(pos=4) PHRASE 10 computer:(pos=5) PHRASE 10 staat:(pos=6) PHRASE 10 u:(pos=7) PHRASE 10 niet:(pos=8) PHRASE 10 toe:(pos=9) PHRASE 10 interactief:(pos=10))" },
    { "ati radeon \"driver cleaner", "(Zati:(pos=1) OR Zradeon:(pos=2) OR (driver:(pos=3) PHRASE 2 cleaner:(pos=4)))" },
    { "\"../\" path", "Zpath:(pos=1)" },
    { "(novell client) workstation only", "(Znovel:(pos=1) OR Zclient:(pos=2) OR Zworkstat:(pos=3) OR Zonli:(pos=4))" },
    { "Unable to find libgd.(a|so) anywhere", "(unable:(pos=1) OR Zto:(pos=2) OR Zfind:(pos=3) OR Zlibgd:(pos=4) OR Za:(pos=5) OR Zso:(pos=6) OR Zanywher:(pos=7))" },
    { "\"libstdc++-libc6.1-1.so.2\"", "(libstdc++:(pos=1) PHRASE 5 libc6.1:(pos=2) PHRASE 5 1:(pos=3) PHRASE 5 so:(pos=4) PHRASE 5 2:(pos=5))" },
    { "ipsec_setup (/etc/ipsec.conf, line 1) cannot open configuration file \"/etc/ipsec.conf\" -- `' aborted", "(Zipsec_setup:(pos=1) OR (etc:(pos=2) PHRASE 3 ipsec:(pos=3) PHRASE 3 conf:(pos=4)) OR Zline:(pos=5) OR 1:(pos=6) OR Zcannot:(pos=7) OR Zopen:(pos=8) OR Zconfigur:(pos=9) OR Zfile:(pos=10) OR (etc:(pos=11) PHRASE 3 ipsec:(pos=12) PHRASE 3 conf:(pos=13)) OR Zabort:(pos=14))" },
    { "Forwarden van domeinnaam (naar HTTP adres)", "(forwarden:(pos=1) OR Zvan:(pos=2) OR Zdomeinnaam:(pos=3) OR Znaar:(pos=4) OR http:(pos=5) OR Zadr:(pos=6))" },
    { "Compaq HP, 146.8 GB (MPN-286716-B22) Hard Drives", "(compaq:(pos=1) OR hp:(pos=2) OR 146.8:(pos=3) OR gb:(pos=4) OR (mpn:(pos=5) PHRASE 3 286716:(pos=6) PHRASE 3 b22:(pos=7)) OR hard:(pos=8) OR drives:(pos=9))" },
    { "httpd (no pid file) not running", "(Zhttpd:(pos=1) OR Zno:(pos=2) OR Zpid:(pos=3) OR Zfile:(pos=4) OR Znot:(pos=5) OR Zrun:(pos=6))" },
    { "apache httpd (pid file) not running", "(Zapach:(pos=1) OR Zhttpd:(pos=2) OR Zpid:(pos=3) OR Zfile:(pos=4) OR Znot:(pos=5) OR Zrun:(pos=6))" },
    { "Klasse is niet geregistreerd  (Fout=80040154).", "(klasse:(pos=1) OR Zis:(pos=2) OR Zniet:(pos=3) OR Zgeregistreerd:(pos=4) OR fout:(pos=5) OR 80040154:(pos=6))" },
    { "\"dvd+r\" \"dvd-r\"", "((dvd:(pos=1) PHRASE 2 r:(pos=2)) OR (dvd:(pos=3) PHRASE 2 r:(pos=4)))" },
    { "\"=\" tekens uit csvfile", "(Zteken:(pos=1) OR Zuit:(pos=2) OR Zcsvfile:(pos=3))" },
    { "libc.so.6(GLIBC_2.3)", "((libc:(pos=1) PHRASE 3 so:(pos=2) PHRASE 3 6:(pos=3)) OR glibc_2.3:(pos=4))" },
    { "Sitecom Broadband xDSL / Cable Router 4S (DC-202)", "(sitecom:(pos=1) OR broadband:(pos=2) OR Zxdsl:(pos=3) OR cable:(pos=4) OR router:(pos=5) OR 4s:(pos=6) OR (dc:(pos=7) PHRASE 2 202:(pos=8)))" },
    { "(t-mobile) bereik", "((t:(pos=1) PHRASE 2 mobile:(pos=2)) OR Zbereik:(pos=3))" },
    { "error LNK2001: unresolved external symbol \"public", "(Zerror:(pos=1) OR lnk2001:(pos=2) OR Zunresolv:(pos=3) OR Zextern:(pos=4) OR Zsymbol:(pos=5) OR public:(pos=6))" },
    { "patch linux exploit -p)", "(Zpatch:(pos=1) OR Zlinux:(pos=2) OR Zexploit:(pos=3) OR Zp:(pos=4))" },
    { "MYD not found (Errcode: 2)", "(myd:(pos=1) OR Znot:(pos=2) OR Zfound:(pos=3) OR errcode:(pos=4) OR 2:(pos=5))" },
    { "ob_start(\"ob_gzhandler\"); file download", "(ob_start:(pos=1) OR ob_gzhandler:(pos=2) OR Zfile:(pos=3) OR Zdownload:(pos=4))" },
    { "ECS Elitegroup K7VZA (VIA VT8363/VT8363A)", "(ecs:(pos=1) OR elitegroup:(pos=2) OR k7vza:(pos=3) OR via:(pos=4) OR (vt8363:(pos=5) PHRASE 2 vt8363a:(pos=6)))" },
    { "ASUS A7V8X (LAN + Serial-ATA + Firewire + Raid + Audio)", "(asus:(pos=1) OR a7v8x:(pos=2) OR lan:(pos=3) OR (serial:(pos=4) PHRASE 2 ata:(pos=5)) OR firewire:(pos=6) OR raid:(pos=7) OR audio:(pos=8))" },
    { "Javascript:history.go(-1)", "((javascript:(pos=1) PHRASE 3 history:(pos=2) PHRASE 3 go:(pos=3)) OR 1:(pos=4))" },
    { "java :) als icon", "(Zjava:(pos=1) OR Zal:(pos=2) OR Zicon:(pos=3))" },
    { "onmouseover=setPointer(this", "(onmouseover:(pos=1) OR setpointer:(pos=2) OR Zthis:(pos=3))" },
    { "\" in vbscript", "(in:(pos=1) PHRASE 2 vbscript:(pos=2))" },
    { "IRC (FAQ OR (hulp NEAR bij))", "(irc:(pos=1) OR faq:(pos=2) OR (hulp:(pos=3) NEAR 11 bij:(pos=4)))" },
    { "setProperty(\"McSquare\"+i, _xscale, _xscale++);", "(setproperty:(pos=1) OR mcsquare:(pos=2) OR Zi:(pos=3) OR _xscale:(pos=4) OR _xscale++:(pos=5))" },
    { "[warn] Apache does not support line-end comments. Consider using quotes around argument: \"#-1\"", "(Zwarn:(pos=1) OR apache:(pos=2) OR Zdoe:(pos=3) OR Znot:(pos=4) OR Zsupport:(pos=5) OR (line:(pos=6) PHRASE 2 end:(pos=7)) OR Zcomment:(pos=8) OR consider:(pos=9) OR Zuse:(pos=10) OR Zquot:(pos=11) OR Zaround:(pos=12) OR Zargument:(pos=13) OR 1:(pos=14))" },
    { "(php.ini) (memory_limit)", "((php:(pos=1) PHRASE 2 ini:(pos=2)) OR Zmemory_limit:(pos=3))" },
    { "line 8: syntax error near unexpected token `kernel_thread(f'", "(Zline:(pos=1) OR 8:(pos=2) OR Zsyntax:(pos=3) OR Zerror:(pos=4) OR Znear:(pos=5) OR Zunexpect:(pos=6) OR Ztoken:(pos=7) OR kernel_thread:(pos=8) OR Zf:(pos=9))" },
    { "VXD NAVEX()@)", "(vxd:(pos=1) OR navex:(pos=2))" },
    { "\"Iiyama AS4314UT 17\" \"", "(iiyama:(pos=1) PHRASE 3 as4314ut:(pos=2) PHRASE 3 17:(pos=3))" },
    { "include (\"$id.html\");", "(Zinclud:(pos=1) OR (id:(pos=2) PHRASE 2 html:(pos=3)))" },
    { "include id.Today's date is: <? print (date (\"M d, Y\")); ?>hp", "(Zinclud:(pos=1) OR (id:(pos=2) PHRASE 2 today's:(pos=3)) OR Zdate:(pos=4) OR Zis:(pos=5) OR Zprint:(pos=6) OR Zdate:(pos=7) OR (m:(pos=8) PHRASE 3 d:(pos=9) PHRASE 3 y:(pos=10)) OR Zhp:(pos=11))" },
    { "(program files\\common) opstarten", "(Zprogram:(pos=1) OR (files:(pos=2) PHRASE 2 common:(pos=3)) OR Zopstarten:(pos=4))" },
    { "java \" string", "(Zjava:(pos=1) OR string:(pos=2))" },
    { "+=", "" },
    { "php +=", "Zphp:(pos=1)" },
    { "[php] ereg_replace(\".\"", "(Zphp:(pos=1) OR ereg_replace:(pos=2))" },
    { "\"echo -e\" kleur", "((echo:(pos=1) PHRASE 2 e:(pos=2)) OR Zkleur:(pos=3))" },
    { "adobe premiere \"-1\"", "(Zadob:(pos=1) OR Zpremier:(pos=2) OR 1:(pos=3))" },
    { "DVD brander \"+\" en \"-\"", "(dvd:(pos=1) OR Zbrander:(pos=2) OR Zen:(pos=3))" },
    { "inspirion \"dvd+R\"", "(Zinspirion:(pos=1) OR (dvd:(pos=2) PHRASE 2 r:(pos=3)))" },
    { "asp 0x80040E14)", "(Zasp:(pos=1) OR 0x80040e14:(pos=2))" },
    { "\"e-tech motorola router", "(e:(pos=1) PHRASE 4 tech:(pos=2) PHRASE 4 motorola:(pos=3) PHRASE 4 router:(pos=4))" },
    { "bluetooth '1.3.2.19\"", "(Zbluetooth:(pos=1) OR 1.3.2.19:(pos=2))" },
    { "ms +-connect", "(Zms:(pos=1) OR Zconnect:(pos=2))" },
    { "php+print+\"", "(Zphp:(pos=1) OR print+:(pos=2))" },
    { "athlon 1400 :welke videokaart\"", "(Zathlon:(pos=1) OR 1400:(pos=2) OR Zwelk:(pos=3) OR videokaart:(pos=4))" },
    { "+-dvd", "Zdvd:(pos=1)" },
    { "glftpd \"-new-\"", "(Zglftpd:(pos=1) OR new:(pos=2))" },
    { "\"scandisk + dos5.0", "(scandisk:(pos=1) PHRASE 2 dos5.0:(pos=2))" },
    { "socket\\(\\)", "socket:(pos=1)" },
    { "msn (e-tech) router", "(Zmsn:(pos=1) OR (e:(pos=2) PHRASE 2 tech:(pos=3)) OR Zrouter:(pos=4))" },
    { "Het grote Epox 8k3a+ ervaring/prob topic\"", "(het:(pos=1) OR Zgrote:(pos=2) OR epox:(pos=3) OR 8k3a+:(pos=4) OR (ervaring:(pos=5) PHRASE 2 prob:(pos=6)) OR topic:(pos=7))" },
    { "\"CF+bluetooth\"", "(cf:(pos=1) PHRASE 2 bluetooth:(pos=2))" },
    { "kwaliteit (s-video) composite verschil tv out", "(Zkwaliteit:(pos=1) OR (s:(pos=2) PHRASE 2 video:(pos=3)) OR Zcomposit:(pos=4) OR Zverschil:(pos=5) OR Ztv:(pos=6) OR Zout:(pos=7))" },
    { "Wie kan deze oude hardware nog gebruiken\" Deel", "(wie:(pos=1) OR Zkan:(pos=2) OR Zdeze:(pos=3) OR Zoud:(pos=4) OR Zhardwar:(pos=5) OR Znog:(pos=6) OR gebruiken:(pos=7) OR deel:(pos=8))" },
    { "Public Declare Sub Sleep Lib \"kernel32\" (ByVal dwMilliseconds As Long)", "(public:(pos=1) OR declare:(pos=2) OR sub:(pos=3) OR sleep:(pos=4) OR lib:(pos=5) OR kernel32:(pos=6) OR byval:(pos=7) OR Zdwmillisecond:(pos=8) OR as:(pos=9) OR long:(pos=10))" },
    { "for inclusion (include_path='.:/usr/share/php')", "(Zfor:(pos=1) OR Zinclus:(pos=2) OR include_path:(pos=3) OR (usr:(pos=4) PHRASE 3 share:(pos=5) PHRASE 3 php:(pos=6)))" },
    { "\"muziek 2x zo snel\"\"", "(muziek:(pos=1) PHRASE 4 2x:(pos=2) PHRASE 4 zo:(pos=3) PHRASE 4 snel:(pos=4))" },
    { "execCommand('inserthorizontalrule'", "(execcommand:(pos=1) OR Zinserthorizontalrul:(pos=2))" },
    { "specs: IBM PS/2, Intel 8086 @ 25 mhz!!, 2 mb intern, 50 mb hd, 5.5\" floppy drive, toetsenbord en geen muis", "(Zspec:(pos=1) OR ibm:(pos=2) OR (ps:(pos=3) PHRASE 2 2:(pos=4)) OR intel:(pos=5) OR 8086:(pos=6) OR 25:(pos=7) OR Zmhz:(pos=8) OR 2:(pos=9) OR Zmb:(pos=10) OR Zintern:(pos=11) OR 50:(pos=12) OR Zmb:(pos=13) OR Zhd:(pos=14) OR 5.5:(pos=15) OR (floppy:(pos=16) PHRASE 6 drive:(pos=17) PHRASE 6 toetsenbord:(pos=18) PHRASE 6 en:(pos=19) PHRASE 6 geen:(pos=20) PHRASE 6 muis:(pos=21)))" },
    { "History: GetEventTool <- GetMusicManager <- GetMusicScript <- DMCallRoutine <- AMusicScriptEvent::execCallRoutine <- UObject::execClassContext <- (U2GameInfo M08A1.U2GameInfo0 @ Function U2.U2GameInfo.NotifyLevelChangeEnd : 0075 line 744) <- UObject::ProcessEvent <- (U2GameInfo M08A1.U2GameInfo0, Function U2.U2GameInfo.NotifyLevelChangeEnd) <- UGameEngine::LoadMap <- LocalMapURL <- UGameEngine::Browse <- ServerTravel <- UGameEngine::Tick <- UpdateWorld <- MainLoop", "(history:(pos=1) OR geteventtool:(pos=2) OR getmusicmanager:(pos=3) OR getmusicscript:(pos=4) OR dmcallroutine:(pos=5) OR (amusicscriptevent:(pos=6) PHRASE 2 execcallroutine:(pos=7)) OR (uobject:(pos=8) PHRASE 2 execclasscontext:(pos=9)) OR u2gameinfo:(pos=10) OR (m08a1:(pos=11) PHRASE 2 u2gameinfo0:(pos=12)) OR function:(pos=13) OR (u2:(pos=14) PHRASE 3 u2gameinfo:(pos=15) PHRASE 3 notifylevelchangeend:(pos=16)) OR 0075:(pos=17) OR Zline:(pos=18) OR 744:(pos=19) OR (uobject:(pos=20) PHRASE 2 processevent:(pos=21)) OR u2gameinfo:(pos=22) OR (m08a1:(pos=23) PHRASE 2 u2gameinfo0:(pos=24)) OR function:(pos=25) OR (u2:(pos=26) PHRASE 3 u2gameinfo:(pos=27) PHRASE 3 notifylevelchangeend:(pos=28)) OR (ugameengine:(pos=29) PHRASE 2 loadmap:(pos=30)) OR localmapurl:(pos=31) OR (ugameengine:(pos=32) PHRASE 2 browse:(pos=33)) OR servertravel:(pos=34) OR (ugameengine:(pos=35) PHRASE 2 tick:(pos=36)) OR updateworld:(pos=37) OR mainloop:(pos=38))" },
    { "Support AMD XP 2400+ & 2600+ (K7T Turbo2 only)", "(support:(pos=1) OR amd:(pos=2) OR xp:(pos=3) OR 2400+:(pos=4) OR 2600+:(pos=5) OR k7t:(pos=6) OR turbo2:(pos=7) OR Zonli:(pos=8))" },
    { "'\"><br>bla</br>", "(br:(pos=1) PHRASE 3 bla:(pos=2) PHRASE 3 br:(pos=3))" },
    { "The instruction at \"0x30053409\" referenced memory at \"0x06460504\". The memory could not be \"read'. Click OK to terminate the application.", "(the:(pos=1) OR Zinstruct:(pos=2) OR Zat:(pos=3) OR 0x30053409:(pos=4) OR Zreferenc:(pos=5) OR Zmemori:(pos=6) OR Zat:(pos=7) OR 0x06460504:(pos=8) OR the:(pos=9) OR Zmemori:(pos=10) OR Zcould:(pos=11) OR Znot:(pos=12) OR Zbe:(pos=13) OR (read:(pos=14) PHRASE 7 click:(pos=15) PHRASE 7 ok:(pos=16) PHRASE 7 to:(pos=17) PHRASE 7 terminate:(pos=18) PHRASE 7 the:(pos=19) PHRASE 7 application:(pos=20)))" },
    { "\"(P5A-b)\"", "(p5a:(pos=1) PHRASE 2 b:(pos=2))" },
    { "(13,5 > 13) == no-go!", "(13,5:(pos=1) OR 13:(pos=2) OR (no:(pos=3) PHRASE 2 go:(pos=4)))" },
    { "eth not found \"ifconfig -a\"", "(Zeth:(pos=1) OR Znot:(pos=2) OR Zfound:(pos=3) OR (ifconfig:(pos=4) PHRASE 2 a:(pos=5)))" },
    { "<META NAME=\"ROBOTS", "(meta:(pos=1) OR name:(pos=2) OR robots:(pos=3))" },
    { "lp0: using parport0 (interrupt-driven)", "(Zlp0:(pos=1) OR Zuse:(pos=2) OR Zparport0:(pos=3) OR (interrupt:(pos=4) PHRASE 2 driven:(pos=5)))" },
    { "ULTRA PC-TUNING, COOLING & MODDING (4,6)", "(ultra:(pos=1) OR (pc:(pos=2) PHRASE 2 tuning:(pos=3)) OR cooling:(pos=4) OR modding:(pos=5) OR 4,6:(pos=6))" },
    { "512MB PC2700 DDR SDRAM Rood (Dane-Elec)", "(512mb:(pos=1) OR pc2700:(pos=2) OR ddr:(pos=3) OR sdram:(pos=4) OR rood:(pos=5) OR (dane:(pos=6) PHRASE 2 elec:(pos=7)))" },
    { "header(\"Content Type: text/html\");", "(header:(pos=1) OR content:(pos=2) OR type:(pos=3) OR (text:(pos=4) PHRASE 2 html:(pos=5)))" },
    { "\"-RW\" \"+RW\"", "(rw:(pos=1) OR rw:(pos=2))" },
    { "\"cresta digital answering machine", "(cresta:(pos=1) PHRASE 4 digital:(pos=2) PHRASE 4 answering:(pos=3) PHRASE 4 machine:(pos=4))" },
    { "Arctic Super Silent PRO TC (Athlon/P3 - 2,3 GHz)", "(arctic:(pos=1) OR super:(pos=2) OR silent:(pos=3) OR pro:(pos=4) OR tc:(pos=5) OR (athlon:(pos=6) PHRASE 2 p3:(pos=7)) OR 2,3:(pos=8) OR ghz:(pos=9))" },
    { "c++ fopen \"r+t\"", "(Zc++:(pos=1) OR Zfopen:(pos=2) OR (r:(pos=3) PHRASE 2 t:(pos=4)))" },
    { "c++ fopen (r+t)", "(Zc++:(pos=1) OR Zfopen:(pos=2) OR Zr:(pos=3) OR Zt:(pos=4))" },
    { "\"DVD+R\"", "(dvd:(pos=1) PHRASE 2 r:(pos=2))" },
    { "Class.forName(\"jdbc.odbc.JdbcOdbcDriver\");", "((class:(pos=1) PHRASE 2 forname:(pos=2)) OR (jdbc:(pos=3) PHRASE 3 odbc:(pos=4) PHRASE 3 jdbcodbcdriver:(pos=5)))" },
    { "perl(find.pl)", "(perl:(pos=1) OR (find:(pos=2) PHRASE 2 pl:(pos=3)))" },
    { "\"-5v\" voeding", "(5v:(pos=1) OR Zvoed:(pos=2))" },
    { "\"-5v\" power supply", "(5v:(pos=1) OR Zpower:(pos=2) OR Zsuppli:(pos=3))" },
    { "An Error occurred whie attempting to initialize the Borland Database Engine (error $2108)", "(an:(pos=1) OR error:(pos=2) OR Zoccur:(pos=3) OR Zwhie:(pos=4) OR Zattempt:(pos=5) OR Zto:(pos=6) OR Ziniti:(pos=7) OR Zthe:(pos=8) OR borland:(pos=9) OR database:(pos=10) OR engine:(pos=11) OR Zerror:(pos=12) OR 2108:(pos=13))" },
    { "(error $2108) Borland", "(Zerror:(pos=1) OR 2108:(pos=2) OR borland:(pos=3))" },
    { "On Friday 04 April 2003 09:32, Edwin van Eersel wrote: > ik voel me eigenlijk wel behoorlijk kut :)", "(on:(pos=1) OR friday:(pos=2) OR 04:(pos=3) OR april:(pos=4) OR 2003:(pos=5) OR (09:(pos=6) PHRASE 2 32:(pos=7)) OR edwin:(pos=8) OR Zvan:(pos=9) OR eersel:(pos=10) OR Zwrote:(pos=11) OR Zik:(pos=12) OR Zvoel:(pos=13) OR Zme:(pos=14) OR Zeigenlijk:(pos=15) OR Zwel:(pos=16) OR Zbehoorlijk:(pos=17) OR Zkut:(pos=18))" },
    { "Elektrotechniek + \"hoe bevalt het?\"\"", "(elektrotechniek:(pos=1) OR (hoe:(pos=2) PHRASE 3 bevalt:(pos=3) PHRASE 3 het:(pos=4)))" },
    { "Shortcuts in menu (java", "(shortcuts:(pos=1) OR Zin:(pos=2) OR Zmenu:(pos=3) OR Zjava:(pos=4))" },
    { "detonator+settings\"", "(Zdeton:(pos=1) OR settings:(pos=2))" },
    { "(ez-bios) convert", "((ez:(pos=1) PHRASE 2 bios:(pos=2)) OR Zconvert:(pos=3))" },
    { "Sparkle 7100M4 64MB (GeForce4 MX440)", "(sparkle:(pos=1) OR 7100m4:(pos=2) OR 64mb:(pos=3) OR geforce4:(pos=4) OR mx440:(pos=5))" },
    { "freebsd \"boek OR newbie\"", "(Zfreebsd:(pos=1) OR (boek:(pos=2) PHRASE 3 or:(pos=3) PHRASE 3 newbie:(pos=4)))" },
    { "for (;;) c++", "(Zfor:(pos=1) OR Zc++:(pos=2))" },
    { "1700+-2100+", "(1700+:(pos=1) PHRASE 2 2100+:(pos=2))" },
    { "PHP Warning:  Invalid library (maybe not a PHP library) 'libmysqlclient.so'", "(php:(pos=1) OR warning:(pos=2) OR invalid:(pos=3) OR Zlibrari:(pos=4) OR Zmayb:(pos=5) OR Znot:(pos=6) OR Za:(pos=7) OR php:(pos=8) OR Zlibrari:(pos=9) OR (libmysqlclient:(pos=10) PHRASE 2 so:(pos=11)))" },
    { "NEC DV-5800B (Bul", "(nec:(pos=1) OR (dv:(pos=2) PHRASE 2 5800b:(pos=3)) OR bul:(pos=4))" },
    { "org.jdom.input.SAXBuilder.<init>(SAXBuilder.java)", "((org:(pos=1) PHRASE 4 jdom:(pos=2) PHRASE 4 input:(pos=3) PHRASE 4 saxbuilder:(pos=4)) OR init:(pos=5) OR (saxbuilder:(pos=6) PHRASE 2 java:(pos=7)))" },
    { "AMD Athlon XP 2500+ (1,83GHz, 512KB)", "(amd:(pos=1) OR athlon:(pos=2) OR xp:(pos=3) OR 2500+:(pos=4) OR 1,83ghz:(pos=5) OR 512kb:(pos=6))" },
    { "'q ben\"", "(Zq:(pos=1) OR ben:(pos=2))" },
    { "getsmbfilepwent: malformed password entry (uid not number)", "(Zgetsmbfilepw:(pos=1) OR Zmalform:(pos=2) OR Zpassword:(pos=3) OR Zentri:(pos=4) OR Zuid:(pos=5) OR Znot:(pos=6) OR Znumber:(pos=7))" },
    { "\xc3\xb6ude onderdelen\"", "(Z\xc3\xb6ude:(pos=1) OR onderdelen:(pos=2))" },
    { "Heeft iemand enig idee waarom de pioneer (zelf met originele firmware van pioneer) bij mij niet wil flashen ?" "?", "(heeft:(pos=1) OR Ziemand:(pos=2) OR Zenig:(pos=3) OR Zide:(pos=4) OR Zwaarom:(pos=5) OR Zde:(pos=6) OR Zpioneer:(pos=7) OR Zzelf:(pos=8) OR Zmet:(pos=9) OR Zoriginel:(pos=10) OR Zfirmwar:(pos=11) OR Zvan:(pos=12) OR Zpioneer:(pos=13) OR Zbij:(pos=14) OR Zmij:(pos=15) OR Zniet:(pos=16) OR Zwil:(pos=17) OR Zflashen:(pos=18))" }, // Split ? and ? to avoid trigram problems
    { "asus a7v266 bios nieuw -(a7v266-e)", "((Zasus:(pos=1) OR Za7v266:(pos=2) OR Zbio:(pos=3) OR Znieuw:(pos=4)) AND_NOT (a7v266:(pos=5) PHRASE 2 e:(pos=6)))" },
    { "cybercom \"dvd+r\"", "(Zcybercom:(pos=1) OR (dvd:(pos=2) PHRASE 2 r:(pos=3)))" },
    { "AMD PCNET Family Ethernet Adapter (PCI-ISA)", "(amd:(pos=1) OR pcnet:(pos=2) OR family:(pos=3) OR ethernet:(pos=4) OR adapter:(pos=5) OR (pci:(pos=6) PHRASE 2 isa:(pos=7)))" },
    { "relais +/-", "Zrelai:(pos=1)" },
    { "formules (slepen OR doortrekken) excel", "(Zformul:(pos=1) OR Zslepen:(pos=2) OR Zdoortrekken:(pos=3) OR Zexcel:(pos=4))" },
    { "\"%English", "english:(pos=1)" },
    { "select max( mysql", "(Zselect:(pos=1) OR max:(pos=2) OR Zmysql:(pos=3))" },
    { "leejow(saait", "(leejow:(pos=1) OR Zsaait:(pos=2))" },
    { "'Windows 2000 Advanced Server\" netwerkverbinding valt steeds weg", "(windows:(pos=1) OR 2000:(pos=2) OR advanced:(pos=3) OR server:(pos=4) OR (netwerkverbinding:(pos=5) PHRASE 4 valt:(pos=6) PHRASE 4 steeds:(pos=7) PHRASE 4 weg:(pos=8)))" },
    { "K7T Turbo 2  (MS-6330)", "(k7t:(pos=1) OR turbo:(pos=2) OR 2:(pos=3) OR (ms:(pos=4) PHRASE 2 6330:(pos=5)))" },
    { "failed to receive data from the client agent. (ec=1)", "(Zfail:(pos=1) OR Zto:(pos=2) OR Zreceiv:(pos=3) OR Zdata:(pos=4) OR Zfrom:(pos=5) OR Zthe:(pos=6) OR Zclient:(pos=7) OR Zagent:(pos=8) OR ec:(pos=9) OR 1:(pos=10))" },
    { "\"cannot find -lz\"", "(cannot:(pos=1) PHRASE 3 find:(pos=2) PHRASE 3 lz:(pos=3))" },
    { "undefined reference to `mysql_drop_db'\"", "(Zundefin:(pos=1) OR Zrefer:(pos=2) OR Zto:(pos=3) OR Zmysql_drop_db:(pos=4))" },
    { "search form asp \"%'", "(Zsearch:(pos=1) OR Zform:(pos=2) OR Zasp:(pos=3))" },
    { "(dvd+r) kwaliteit", "(Zdvd:(pos=1) OR Zr:(pos=2) OR Zkwaliteit:(pos=3))" },
    { "Fatal error: Allowed memory size of 8388608 bytes exhausted (tried to allocate 35 bytes)", "(fatal:(pos=1) OR Zerror:(pos=2) OR allowed:(pos=3) OR Zmemori:(pos=4) OR Zsize:(pos=5) OR Zof:(pos=6) OR 8388608:(pos=7) OR Zbyte:(pos=8) OR Zexhaust:(pos=9) OR Ztri:(pos=10) OR Zto:(pos=11) OR Zalloc:(pos=12) OR 35:(pos=13) OR Zbyte:(pos=14))" },
    { "geluid (schokt OR hapert)", "(Zgeluid:(pos=1) OR Zschokt:(pos=2) OR Zhapert:(pos=3))" },
    { "Het wordt pas echt leuk als het hard staat!! >:)", "(het:(pos=1) OR Zwordt:(pos=2) OR Zpas:(pos=3) OR Zecht:(pos=4) OR Zleuk:(pos=5) OR Zal:(pos=6) OR Zhet:(pos=7) OR Zhard:(pos=8) OR Zstaat:(pos=9))" },
    { "Uw configuratie bestand bevat instellingen (root zonder wachtwoord) die betrekking hebben tot de standaard MySQL account. Uw MySQL server draait met deze standaard waardes, en is open voor ongewilde toegang, het wordt dus aangeraden dit op te lossen", "(uw:(pos=1) OR Zconfigurati:(pos=2) OR Zbestand:(pos=3) OR Zbevat:(pos=4) OR Zinstellingen:(pos=5) OR Zroot:(pos=6) OR Zzonder:(pos=7) OR Zwachtwoord:(pos=8) OR Zdie:(pos=9) OR Zbetrekk:(pos=10) OR Zhebben:(pos=11) OR Ztot:(pos=12) OR Zde:(pos=13) OR Zstandaard:(pos=14) OR mysql:(pos=15) OR Zaccount:(pos=16) OR uw:(pos=17) OR mysql:(pos=18) OR Zserver:(pos=19) OR Zdraait:(pos=20) OR Zmet:(pos=21) OR Zdeze:(pos=22) OR Zstandaard:(pos=23) OR Zwaard:(pos=24) OR Zen:(pos=25) OR Zis:(pos=26) OR Zopen:(pos=27) OR Zvoor:(pos=28) OR Zongewild:(pos=29) OR Ztoegang:(pos=30) OR Zhet:(pos=31) OR Zwordt:(pos=32) OR Zdus:(pos=33) OR Zaangeraden:(pos=34) OR Zdit:(pos=35) OR Zop:(pos=36) OR Zte:(pos=37) OR Zlossen:(pos=38))" },
    { "(library qt-mt) not found", "(Zlibrari:(pos=1) OR (qt:(pos=2) PHRASE 2 mt:(pos=3)) OR Znot:(pos=4) OR Zfound:(pos=5))" },
    { "Qt (>= Qt 3.0.3) (library qt-mt) not found", "(qt:(pos=1) OR qt:(pos=2) OR 3.0.3:(pos=3) OR Zlibrari:(pos=4) OR (qt:(pos=5) PHRASE 2 mt:(pos=6)) OR Znot:(pos=7) OR Zfound:(pos=8))" },
    { "setup was unable to find (or could not read) the language specific setup resource dll, unable to continue. Please reboot and try again.", "(Zsetup:(pos=1) OR Zwas:(pos=2) OR Zunabl:(pos=3) OR Zto:(pos=4) OR Zfind:(pos=5) OR Zor:(pos=6) OR Zcould:(pos=7) OR Znot:(pos=8) OR Zread:(pos=9) OR Zthe:(pos=10) OR Zlanguag:(pos=11) OR Zspecif:(pos=12) OR Zsetup:(pos=13) OR Zresourc:(pos=14) OR Zdll:(pos=15) OR Zunabl:(pos=16) OR Zto:(pos=17) OR Zcontinu:(pos=18) OR please:(pos=19) OR Zreboot:(pos=20) OR Zand:(pos=21) OR Ztri:(pos=22) OR Zagain:(pos=23))" },
    { "Titan TTC-D5TB(4/CU35)", "(titan:(pos=1) OR (ttc:(pos=2) PHRASE 2 d5tb:(pos=3)) OR (4:(pos=4) PHRASE 2 cu35:(pos=5)))" },
    { "[php] date( min", "(Zphp:(pos=1) OR date:(pos=2) OR Zmin:(pos=3))" },
    { "EPOX EP-8RDA+ (nForce2 SPP+MCP-T) Rev. 1.1", "(epox:(pos=1) OR (ep:(pos=2) PHRASE 2 8rda+:(pos=3)) OR Znforce2:(pos=4) OR spp:(pos=5) OR (mcp:(pos=6) PHRASE 2 t:(pos=7)) OR rev:(pos=8) OR 1.1:(pos=9))" },
    { "554 5.4.6 Too many hops 53 (25 max)", "(554:(pos=1) OR 5.4.6:(pos=2) OR too:(pos=3) OR Zmani:(pos=4) OR Zhop:(pos=5) OR 53:(pos=6) OR 25:(pos=7) OR Zmax:(pos=8))" },
    { "ik had toch nog een vraagje: er zijn nu eigenlijk alleen maar schijfjes van 4.7GB alleen straks zullen er vast schijfjes van meer dan 4.7GB komen. Zal deze brander dit wel kunnen schijven?" "?(na bijvoorbeeld een firmware update?) ben erg benieuwd", "(Zik:(pos=1) OR Zhad:(pos=2) OR Ztoch:(pos=3) OR Znog:(pos=4) OR Zeen:(pos=5) OR Zvraagj:(pos=6) OR Zer:(pos=7) OR Zzijn:(pos=8) OR Znu:(pos=9) OR Zeigenlijk:(pos=10) OR Zalleen:(pos=11) OR Zmaar:(pos=12) OR Zschijfj:(pos=13) OR Zvan:(pos=14) OR 4.7gb:(pos=15) OR Zalleen:(pos=16) OR Zstrak:(pos=17) OR Zzullen:(pos=18) OR Zer:(pos=19) OR Zvast:(pos=20) OR Zschijfj:(pos=21) OR Zvan:(pos=22) OR Zmeer:(pos=23) OR Zdan:(pos=24) OR 4.7gb:(pos=25) OR Zkomen:(pos=26) OR zal:(pos=27) OR Zdeze:(pos=28) OR Zbrander:(pos=29) OR Zdit:(pos=30) OR Zwel:(pos=31) OR Zkunnen:(pos=32) OR Zschijven:(pos=33) OR Zna:(pos=34) OR Zbijvoorbeeld:(pos=35) OR Zeen:(pos=36) OR Zfirmwar:(pos=37) OR Zupdat:(pos=38) OR Zben:(pos=39) OR Zerg:(pos=40) OR Zbenieuwd:(pos=41))" }, // Split ? and ? to avoid trigram problems
    { "ati linux drivers (4.3.0)", "(Zati:(pos=1) OR Zlinux:(pos=2) OR Zdriver:(pos=3) OR 4.3.0:(pos=4))" },
    { "ENCAPSED_AND_WHITESPACE", "encapsed_and_whitespace:(pos=1)" },
    { "lpadmin: add-printer (set device) failed: client-error-not-possible", "(Zlpadmin:(pos=1) OR (add:(pos=2) PHRASE 2 printer:(pos=3)) OR Zset:(pos=4) OR Zdevic:(pos=5) OR Zfail:(pos=6) OR (client:(pos=7) PHRASE 4 error:(pos=8) PHRASE 4 not:(pos=9) PHRASE 4 possible:(pos=10)))" },
    { "welke dvd \"+r\" media", "(Zwelk:(pos=1) OR Zdvd:(pos=2) OR r:(pos=3) OR Zmedia:(pos=4))" },
    { "Warning: stat failed for fotos(errno=2 - No such file or directory)", "(warning:(pos=1) OR Zstat:(pos=2) OR Zfail:(pos=3) OR Zfor:(pos=4) OR fotos:(pos=5) OR errno:(pos=6) OR 2:(pos=7) OR no:(pos=8) OR Zsuch:(pos=9) OR Zfile:(pos=10) OR Zor:(pos=11) OR Zdirectori:(pos=12))" },
    { "dvd +/-", "Zdvd:(pos=1)" },
    { "7vaxp +voltage mod\"", "(Zvoltag:(pos=2) AND_MAYBE (7vaxp:(pos=1) OR mod:(pos=3)))" },
    { "lpt port (SPP/EPP) is enabled", "(Zlpt:(pos=1) OR Zport:(pos=2) OR (spp:(pos=3) PHRASE 2 epp:(pos=4)) OR Zis:(pos=5) OR Zenabl:(pos=6))" },
    { "getenv(\"HTTP_REFERER\")", "(getenv:(pos=1) OR http_referer:(pos=2))" },
    { "Error setting display mode: CreateDevice failed (D3DERR_DRIVERINTERNALERROR)", "(error:(pos=1) OR Zset:(pos=2) OR Zdisplay:(pos=3) OR Zmode:(pos=4) OR createdevice:(pos=5) OR Zfail:(pos=6) OR d3derr_driverinternalerror:(pos=7))" },
    { "Exception number: c0000005 (access violation)", "(exception:(pos=1) OR Znumber:(pos=2) OR Zc0000005:(pos=3) OR Zaccess:(pos=4) OR Zviolat:(pos=5))" },
    { "header(\"Content-type:application/octetstream\");", "(header:(pos=1) OR (content:(pos=2) PHRASE 4 type:(pos=3) PHRASE 4 application:(pos=4) PHRASE 4 octetstream:(pos=5)))" },
    { "java.security.AccessControlException: access denied (java.lang.RuntimePermission accessClassInPackage.sun.jdbc.odbc)", "((java:(pos=1) PHRASE 3 security:(pos=2) PHRASE 3 accesscontrolexception:(pos=3)) OR Zaccess:(pos=4) OR Zdeni:(pos=5) OR (java:(pos=6) PHRASE 3 lang:(pos=7) PHRASE 3 runtimepermission:(pos=8)) OR (accessclassinpackage:(pos=9) PHRASE 4 sun:(pos=10) PHRASE 4 jdbc:(pos=11) PHRASE 4 odbc:(pos=12)))" },
    { "(001.part.met", "(001:(pos=1) PHRASE 3 part:(pos=2) PHRASE 3 met:(pos=3))" },
    { "Warning: mail(): Use the -f option (5th param) to include valid reply-to address ! in /usr/home/vdb/www/mail.php on line 79", "(warning:(pos=1) OR mail:(pos=2) OR use:(pos=3) OR Zthe:(pos=4) OR Zf:(pos=5) OR Zoption:(pos=6) OR 5th:(pos=7) OR Zparam:(pos=8) OR Zto:(pos=9) OR Zinclud:(pos=10) OR Zvalid:(pos=11) OR (reply:(pos=12) PHRASE 2 to:(pos=13)) OR Zaddress:(pos=14) OR Zin:(pos=15) OR (usr:(pos=16) PHRASE 6 home:(pos=17) PHRASE 6 vdb:(pos=18) PHRASE 6 www:(pos=19) PHRASE 6 mail:(pos=20) PHRASE 6 php:(pos=21)) OR Zon:(pos=22) OR Zline:(pos=23) OR 79:(pos=24))" },
    { "PHP Use the -f option (5th param)", "((php:(pos=1) OR use:(pos=2) OR Zthe:(pos=3) OR Zoption:(pos=5) OR 5th:(pos=6) OR Zparam:(pos=7)) AND_NOT Zf:(pos=4))" },
    { "dvd \"+\" \"-\"", "Zdvd:(pos=1)" },
    { "bericht  ( %)", "Zbericht:(pos=1)" },
    { "2500+ of 2600+ (niett OC)", "(2500+:(pos=1) OR Zof:(pos=2) OR 2600+:(pos=3) OR Zniett:(pos=4) OR oc:(pos=5))" },
    { "maxtor windows xp werkt The drivers for this device are not installed. (Code 28)", "(Zmaxtor:(pos=1) OR Zwindow:(pos=2) OR Zxp:(pos=3) OR Zwerkt:(pos=4) OR the:(pos=5) OR Zdriver:(pos=6) OR Zfor:(pos=7) OR Zthis:(pos=8) OR Zdevic:(pos=9) OR Zare:(pos=10) OR Znot:(pos=11) OR Zinstal:(pos=12) OR code:(pos=13) OR 28:(pos=14))" },
    { "Warning: stat failed for /mnt/web/react/got/react/board/non-www/headlines/tnet-headlines.txt (errno=2 - No such file or directory) in /mnt/web/react/got/react/global/non-www/templates/got/functions.inc.php on line 303", "(warning:(pos=1) OR Zstat:(pos=2) OR Zfail:(pos=3) OR Zfor:(pos=4) OR (mnt:(pos=5) PHRASE 12 web:(pos=6) PHRASE 12 react:(pos=7) PHRASE 12 got:(pos=8) PHRASE 12 react:(pos=9) PHRASE 12 board:(pos=10) PHRASE 12 non:(pos=11) PHRASE 12 www:(pos=12) PHRASE 12 headlines:(pos=13) PHRASE 12 tnet:(pos=14) PHRASE 12 headlines:(pos=15) PHRASE 12 txt:(pos=16)) OR errno:(pos=17) OR 2:(pos=18) OR no:(pos=19) OR Zsuch:(pos=20) OR Zfile:(pos=21) OR Zor:(pos=22) OR Zdirectori:(pos=23) OR Zin:(pos=24) OR (mnt:(pos=25) PHRASE 13 web:(pos=26) PHRASE 13 react:(pos=27) PHRASE 13 got:(pos=28) PHRASE 13 react:(pos=29) PHRASE 13 global:(pos=30) PHRASE 13 non:(pos=31) PHRASE 13 www:(pos=32) PHRASE 13 templates:(pos=33) PHRASE 13 got:(pos=34) PHRASE 13 functions:(pos=35) PHRASE 13 inc:(pos=36) PHRASE 13 php:(pos=37)) OR Zon:(pos=38) OR Zline:(pos=39) OR 303:(pos=40))" },
    { "apm: BIOS version 1.2 Flags 0x03 (Driver version 1.16)", "(Zapm:(pos=1) OR bios:(pos=2) OR Zversion:(pos=3) OR 1.2:(pos=4) OR flags:(pos=5) OR 0x03:(pos=6) OR driver:(pos=7) OR Zversion:(pos=8) OR 1.16:(pos=9))" },
    { "GA-8IHXP(3.0)", "((ga:(pos=1) PHRASE 2 8ihxp:(pos=2)) OR 3.0:(pos=3))" },
    { "8IHXP(3.0)", "(8ihxp:(pos=1) OR 3.0:(pos=2))" },
    { "na\xc2\xb7si (de ~ (m.))", "(Zna\xc2\xb7si:(pos=1) OR Zde:(pos=2) OR Zm:(pos=3))" },
    { "header(\"Content-Disposition: attachment;", "(header:(pos=1) OR (content:(pos=2) PHRASE 3 disposition:(pos=3) PHRASE 3 attachment:(pos=4)))" },
    { "\"header(\"Content-Disposition: attachment;\"", "(header:(pos=1) OR (content:(pos=2) PHRASE 2 disposition:(pos=3)) OR Zattach:(pos=4))" },
    { "\"Beep -f\"", "(beep:(pos=1) PHRASE 2 f:(pos=2))" },
    { "kraan NEAR (Elektrisch OR Electrisch)", "(Zkraan:(pos=1) OR near:(pos=2) OR elektrisch:(pos=3) OR or:(pos=4) OR electrisch:(pos=5))" },
    { "checking for Qt... configure: error: Qt (>= Qt 3.0.2) (headers and libraries) not found. Please check your installation!", "(Zcheck:(pos=1) OR Zfor:(pos=2) OR qt:(pos=3) OR Zconfigur:(pos=4) OR Zerror:(pos=5) OR qt:(pos=6) OR qt:(pos=7) OR 3.0.2:(pos=8) OR Zheader:(pos=9) OR Zand:(pos=10) OR Zlibrari:(pos=11) OR Znot:(pos=12) OR Zfound:(pos=13) OR please:(pos=14) OR Zcheck:(pos=15) OR Zyour:(pos=16) OR Zinstal:(pos=17))" },
    { "parse error, unexpected '\\\"', expecting T_STRING or T_VARIABLE or T_NUM_STRING", "(Zpars:(pos=1) OR Zerror:(pos=2) OR Zunexpect:(pos=3) OR (expecting:(pos=4) PHRASE 6 t_string:(pos=5) PHRASE 6 or:(pos=6) PHRASE 6 t_variable:(pos=7) PHRASE 6 or:(pos=8) PHRASE 6 t_num_string:(pos=9)))" },
    { "ac3 (0x2000) \"Dolby Laboratories,", "(Zac3:(pos=1) OR 0x2000:(pos=2) OR (dolby:(pos=3) PHRASE 2 laboratories:(pos=4)))" },
    { "Movie.FileName=(\"../../../~animations/\"+lesson1.recordset.fields('column3')+\"Intro.avi\")", "((movie:(pos=1) PHRASE 2 filename:(pos=2)) OR animations:(pos=3) OR (lesson1:(pos=4) PHRASE 3 recordset:(pos=5) PHRASE 3 fields:(pos=6)) OR Zcolumn3:(pos=7) OR (intro:(pos=8) PHRASE 2 avi:(pos=9)))" },
    { "502 Permission Denied - Permission Denied - news.chello.nl -- http://www.chello.nl/ (Typhoon v1.2.3)", "(502:(pos=1) OR permission:(pos=2) OR denied:(pos=3) OR permission:(pos=4) OR denied:(pos=5) OR (news:(pos=6) PHRASE 3 chello:(pos=7) PHRASE 3 nl:(pos=8)) OR (http:(pos=9) PHRASE 4 www:(pos=10) PHRASE 4 chello:(pos=11) PHRASE 4 nl:(pos=12)) OR typhoon:(pos=13) OR Zv1.2.3:(pos=14))" },
    { "Motion JPEG (MJPEG codec)", "(motion:(pos=1) OR jpeg:(pos=2) OR mjpeg:(pos=3) OR Zcodec:(pos=4))" },
    { ": zoomtext\"", "zoomtext:(pos=1)" },
    { "Your SORT command does not seem to support the \"-r -n -k 7\"", "(your:(pos=1) OR sort:(pos=2) OR Zcommand:(pos=3) OR Zdoe:(pos=4) OR Znot:(pos=5) OR Zseem:(pos=6) OR Zto:(pos=7) OR Zsupport:(pos=8) OR Zthe:(pos=9) OR (r:(pos=10) PHRASE 4 n:(pos=11) PHRASE 4 k:(pos=12) PHRASE 4 7:(pos=13)))" },
    { "Geef de naam van de MSDOS prompt op C:\\\\WINDOWS.COM\\\"", "(geef:(pos=1) OR Zde:(pos=2) OR Znaam:(pos=3) OR Zvan:(pos=4) OR Zde:(pos=5) OR msdos:(pos=6) OR Zprompt:(pos=7) OR Zop:(pos=8) OR (c:(pos=9) PHRASE 3 windows:(pos=10) PHRASE 3 com:(pos=11)))" },
    { "\"\"wa is fase\"", "(Zwa:(pos=1) OR Zis:(pos=2) OR fase:(pos=3))" },
    { "<v:imagedata src=\"", "((v:(pos=1) PHRASE 2 imagedata:(pos=2)) OR src:(pos=3))" },
    { "system(play ringin.wav); ?>", "(system:(pos=1) OR Zplay:(pos=2) OR (ringin:(pos=3) PHRASE 2 wav:(pos=4)))" },
    { "\"perfect NEAR systems\"", "(perfect:(pos=1) PHRASE 3 near:(pos=2) PHRASE 3 systems:(pos=3))" },
    { "LoadLibrary(\"mainta/gamex86.dll\") failed", "(loadlibrary:(pos=1) OR (mainta:(pos=2) PHRASE 3 gamex86:(pos=3) PHRASE 3 dll:(pos=4)) OR Zfail:(pos=5))" },
    { "DATE_FORMAT('1997-10-04 22:23:00', '%W %M %Y');", "(date_format:(pos=1) OR (1997:(pos=2) PHRASE 3 10:(pos=3) PHRASE 3 04:(pos=4)) OR (22:(pos=5) PHRASE 3 23:(pos=6) PHRASE 3 00:(pos=7)) OR w:(pos=8) OR m:(pos=9) OR y:(pos=10))" },
    { "secundaire IDE-controller (dubbele fifo)", "(Zsecundair:(pos=1) OR (ide:(pos=2) PHRASE 2 controller:(pos=3)) OR Zdubbel:(pos=4) OR Zfifo:(pos=5))" },
    { "\"Postal2+Explorer.exe\"", "(postal2:(pos=1) PHRASE 3 explorer:(pos=2) PHRASE 3 exe:(pos=3))" },
    { "COUNT(*)", "count:(pos=1)" },
    { "Nuttige Windows progs   (1/11)", "(nuttige:(pos=1) OR windows:(pos=2) OR Zprog:(pos=3) OR (1:(pos=4) PHRASE 2 11:(pos=5)))" },
    { "if(usercode==passcode==)", "(if:(pos=1) OR usercode:(pos=2) OR passcode:(pos=3))" },
    { "lg 8160b (dvd+r)", "(Zlg:(pos=1) OR 8160b:(pos=2) OR Zdvd:(pos=3) OR Zr:(pos=4))" },
    { "iPAQ Pocket PC 2002 End User Update (EUU - Service Pack)", "(Zipaq:(pos=1) OR pocket:(pos=2) OR pc:(pos=3) OR 2002:(pos=4) OR end:(pos=5) OR user:(pos=6) OR update:(pos=7) OR euu:(pos=8) OR service:(pos=9) OR pack:(pos=10))" },
    { "'ipod pakt tags niet\"", "(Zipod:(pos=1) OR Zpakt:(pos=2) OR Ztag:(pos=3) OR niet:(pos=4))" },
    { "\"DVD+/-R\"", "(dvd+:(pos=1) PHRASE 2 r:(pos=2))" },
    { "\"DVD+R DVD-R\"", "(dvd:(pos=1) PHRASE 4 r:(pos=2) PHRASE 4 dvd:(pos=3) PHRASE 4 r:(pos=4))" },
    { "php ;)  in een array zetten", "(Zphp:(pos=1) OR Zin:(pos=2) OR Zeen:(pos=3) OR Zarray:(pos=4) OR Zzetten:(pos=5))" },
    { "De inhoud van uw advertentie is niet geschikt voor plaatsing op marktplaats! (001", "(de:(pos=1) OR Zinhoud:(pos=2) OR Zvan:(pos=3) OR Zuw:(pos=4) OR Zadvertenti:(pos=5) OR Zis:(pos=6) OR Zniet:(pos=7) OR Zgeschikt:(pos=8) OR Zvoor:(pos=9) OR Zplaats:(pos=10) OR Zop:(pos=11) OR Zmarktplaat:(pos=12) OR 001:(pos=13))" },
    { "creative (soundblaster OR sb) 128", "(Zcreativ:(pos=1) OR Zsoundblast:(pos=2) OR Zsb:(pos=3) OR 128:(pos=4))" },
    { "Can't open file: (errno: 145)", "(can't:(pos=1) OR Zopen:(pos=2) OR Zfile:(pos=3) OR Zerrno:(pos=4) OR 145:(pos=5))" },
    { "Formateren lukt niet(98,XP)", "(formateren:(pos=1) OR Zlukt:(pos=2) OR niet:(pos=3) OR 98:(pos=4) OR xp:(pos=5))" },
    { "access denied (java.io.", "(Zaccess:(pos=1) OR Zdeni:(pos=2) OR (java:(pos=3) PHRASE 2 io:(pos=4)))" },
    { "(access denied (java.io.)", "(Zaccess:(pos=1) OR Zdeni:(pos=2) OR (java:(pos=3) PHRASE 2 io:(pos=4)))" },
    { "wil niet installeren ( crc fouten)", "(Zwil:(pos=1) OR Zniet:(pos=2) OR Zinstalleren:(pos=3) OR Zcrc:(pos=4) OR Zfouten:(pos=5))" },
    { "(DVD+RW) brandsoftware meerdere", "(dvd:(pos=1) OR rw:(pos=2) OR Zbrandsoftwar:(pos=3) OR Zmeerder:(pos=4))" },
    { "(database OF databases) EN geheugen", "(Zdatabas:(pos=1) OR of:(pos=2) OR Zdatabas:(pos=3) OR en:(pos=4) OR Zgeheugen:(pos=5))" },
    { "(server 2003) winroute", "(Zserver:(pos=1) OR 2003:(pos=2) OR Zwinrout:(pos=3))" },
    { "54MHz (kanaal 2 VHF) tot tenminste 806 MHz (kanaal 69 UHF)", "(54mhz:(pos=1) OR Zkanaal:(pos=2) OR 2:(pos=3) OR vhf:(pos=4) OR Ztot:(pos=5) OR Ztenminst:(pos=6) OR 806:(pos=7) OR mhz:(pos=8) OR Zkanaal:(pos=9) OR 69:(pos=10) OR uhf:(pos=11))" },
    { "(draadloos OR wireless) netwerk", "(Zdraadloo:(pos=1) OR Zwireless:(pos=2) OR Znetwerk:(pos=3))" },
    { "localtime(time(NULL));", "(localtime:(pos=1) OR time:(pos=2) OR null:(pos=3))" },
    { "ob_start(\"ob_gzhandler\");", "(ob_start:(pos=1) OR ob_gzhandler:(pos=2))" },
    { "PPP Closed : LCP Time-out (VPN-0)", "(ppp:(pos=1) OR closed:(pos=2) OR lcp:(pos=3) OR (time:(pos=4) PHRASE 2 out:(pos=5)) OR (vpn:(pos=6) PHRASE 2 0:(pos=7)))" },
    { "COM+-gebeurtenissysteem", "(com+:(pos=1) PHRASE 2 gebeurtenissysteem:(pos=2))" },
    { "rcpthosts (#5.7.1)", "(Zrcpthost:(pos=1) OR 5.7.1:(pos=2))" },
    { "Dit apparaat werkt niet goed omdat Windows de voor dit apparaat vereiste stuurprogramma's niet kan laden.  (Code 31)", "(dit:(pos=1) OR Zapparaat:(pos=2) OR Zwerkt:(pos=3) OR Zniet:(pos=4) OR Zgo:(pos=5) OR Zomdat:(pos=6) OR windows:(pos=7) OR Zde:(pos=8) OR Zvoor:(pos=9) OR Zdit:(pos=10) OR Zapparaat:(pos=11) OR Zvereist:(pos=12) OR Zstuurprogramma:(pos=13) OR Zniet:(pos=14) OR Zkan:(pos=15) OR Zladen:(pos=16) OR code:(pos=17) OR 31:(pos=18))" },
    { "window.open( scrollbar", "((window:(pos=1) PHRASE 2 open:(pos=2)) OR Zscrollbar:(pos=3))" },
    { "T68i truc ->", "(t68i:(pos=1) OR Ztruc:(pos=2))" },
    { "T68i ->", "t68i:(pos=1)" },
    { "\"de lijn is bezet\"\"", "(de:(pos=1) PHRASE 4 lijn:(pos=2) PHRASE 4 is:(pos=3) PHRASE 4 bezet:(pos=4))" },
    { "if (eregi(\"", "(Zif:(pos=1) OR eregi:(pos=2))" },
    { "This device is not working properly because Windows cannot load the drivers required for this device. (Code 31)", "(this:(pos=1) OR Zdevic:(pos=2) OR Zis:(pos=3) OR Znot:(pos=4) OR Zwork:(pos=5) OR Zproper:(pos=6) OR Zbecaus:(pos=7) OR windows:(pos=8) OR Zcannot:(pos=9) OR Zload:(pos=10) OR Zthe:(pos=11) OR Zdriver:(pos=12) OR Zrequir:(pos=13) OR Zfor:(pos=14) OR Zthis:(pos=15) OR Zdevic:(pos=16) OR code:(pos=17) OR 31:(pos=18))" },
    { "execCommand(\"Paste\");", "(execcommand:(pos=1) OR paste:(pos=2))" },
    { "\"-1 unread\"", "(1:(pos=1) PHRASE 2 unread:(pos=2))" },
    { "\"www.historical-fire-engines", "(www:(pos=1) PHRASE 4 historical:(pos=2) PHRASE 4 fire:(pos=3) PHRASE 4 engines:(pos=4))" },
    { "\"DVD+RW\" erase", "((dvd:(pos=1) PHRASE 2 rw:(pos=2)) OR Zeras:(pos=3))" },
    { "[showjekamer)", "Zshowjekam:(pos=1)" },
    { "The description for Event ID  1  in Source  True Vector Engine ) cannot be found. The local computer may not have the necessary registry information or message DLL files to display messages from a remote computer. You may be able to use the /AUXSOURC", "(the:(pos=1) OR Zdescript:(pos=2) OR Zfor:(pos=3) OR event:(pos=4) OR id:(pos=5) OR 1:(pos=6) OR Zin:(pos=7) OR source:(pos=8) OR true:(pos=9) OR vector:(pos=10) OR engine:(pos=11) OR Zcannot:(pos=12) OR Zbe:(pos=13) OR Zfound:(pos=14) OR the:(pos=15) OR Zlocal:(pos=16) OR Zcomput:(pos=17) OR Zmay:(pos=18) OR Znot:(pos=19) OR Zhave:(pos=20) OR Zthe:(pos=21) OR Znecessari:(pos=22) OR Zregistri:(pos=23) OR Zinform:(pos=24) OR Zor:(pos=25) OR Zmessag:(pos=26) OR dll:(pos=27) OR Zfile:(pos=28) OR Zto:(pos=29) OR Zdisplay:(pos=30) OR Zmessag:(pos=31) OR Zfrom:(pos=32) OR Za:(pos=33) OR Zremot:(pos=34) OR Zcomput:(pos=35) OR you:(pos=36) OR Zmay:(pos=37) OR Zbe:(pos=38) OR Zabl:(pos=39) OR Zto:(pos=40) OR Zuse:(pos=41) OR Zthe:(pos=42) OR auxsourc:(pos=43))" },
    { "org.apache.jasper.JasperException: This absolute uri (http://java.sun.com/jstl/core) cannot be resolved in either web.xml or the jar files deployed with this application", "((org:(pos=1) PHRASE 4 apache:(pos=2) PHRASE 4 jasper:(pos=3) PHRASE 4 jasperexception:(pos=4)) OR this:(pos=5) OR Zabsolut:(pos=6) OR Zuri:(pos=7) OR (http:(pos=8) PHRASE 6 java:(pos=9) PHRASE 6 sun:(pos=10) PHRASE 6 com:(pos=11) PHRASE 6 jstl:(pos=12) PHRASE 6 core:(pos=13)) OR Zcannot:(pos=14) OR Zbe:(pos=15) OR Zresolv:(pos=16) OR Zin:(pos=17) OR Zeither:(pos=18) OR (web:(pos=19) PHRASE 2 xml:(pos=20)) OR Zor:(pos=21) OR Zthe:(pos=22) OR Zjar:(pos=23) OR Zfile:(pos=24) OR Zdeploy:(pos=25) OR Zwith:(pos=26) OR Zthis:(pos=27) OR Zapplic:(pos=28))" },
    { "This absolute uri (http://java.sun.com/jstl/core) cannot be resolved in either web.xml or the jar files deployed with this application", "(this:(pos=1) OR Zabsolut:(pos=2) OR Zuri:(pos=3) OR (http:(pos=4) PHRASE 6 java:(pos=5) PHRASE 6 sun:(pos=6) PHRASE 6 com:(pos=7) PHRASE 6 jstl:(pos=8) PHRASE 6 core:(pos=9)) OR Zcannot:(pos=10) OR Zbe:(pos=11) OR Zresolv:(pos=12) OR Zin:(pos=13) OR Zeither:(pos=14) OR (web:(pos=15) PHRASE 2 xml:(pos=16)) OR Zor:(pos=17) OR Zthe:(pos=18) OR Zjar:(pos=19) OR Zfile:(pos=20) OR Zdeploy:(pos=21) OR Zwith:(pos=22) OR Zthis:(pos=23) OR Zapplic:(pos=24))" },
    { "vervangen # \"/", "Zvervangen:(pos=1)" },
    { "vervangen # /\"", "Zvervangen:(pos=1)" },
    { "while(list($key, $val) = each($HTTP_POST_VARS))", "(while:(pos=1) OR list:(pos=2) OR Zkey:(pos=3) OR Zval:(pos=4) OR each:(pos=5) OR http_post_vars:(pos=6))" },
    { "PowerDVD does not support the current display mode. (DDraw Overlay mode is recommended)", "(powerdvd:(pos=1) OR Zdoe:(pos=2) OR Znot:(pos=3) OR Zsupport:(pos=4) OR Zthe:(pos=5) OR Zcurrent:(pos=6) OR Zdisplay:(pos=7) OR Zmode:(pos=8) OR ddraw:(pos=9) OR overlay:(pos=10) OR Zmode:(pos=11) OR Zis:(pos=12) OR Zrecommend:(pos=13))" },
    { "Warning:  Unexpected character in input:  '' (ASCII=92) state=1  highlight", "(warning:(pos=1) OR unexpected:(pos=2) OR Zcharact:(pos=3) OR Zin:(pos=4) OR Zinput:(pos=5) OR ascii:(pos=6) OR 92:(pos=7) OR state:(pos=8) OR 1:(pos=9) OR Zhighlight:(pos=10))" },
    { "error: Qt-1.4 (headers and libraries) not found. Please check your installation!", "(Zerror:(pos=1) OR (qt:(pos=2) PHRASE 2 1.4:(pos=3)) OR Zheader:(pos=4) OR Zand:(pos=5) OR Zlibrari:(pos=6) OR Znot:(pos=7) OR Zfound:(pos=8) OR please:(pos=9) OR Zcheck:(pos=10) OR Zyour:(pos=11) OR Zinstal:(pos=12))" },
    { "Error while initializing the sound driver: device /dev/dsp can't be opened (No such device) The sound server will continue, using the null output device.", "(error:(pos=1) OR Zwhile:(pos=2) OR Ziniti:(pos=3) OR Zthe:(pos=4) OR Zsound:(pos=5) OR Zdriver:(pos=6) OR Zdevic:(pos=7) OR (dev:(pos=8) PHRASE 2 dsp:(pos=9)) OR Zcan't:(pos=10) OR Zbe:(pos=11) OR Zopen:(pos=12) OR no:(pos=13) OR Zsuch:(pos=14) OR Zdevic:(pos=15) OR the:(pos=16) OR Zsound:(pos=17) OR Zserver:(pos=18) OR Zwill:(pos=19) OR Zcontinu:(pos=20) OR Zuse:(pos=21) OR Zthe:(pos=22) OR Znull:(pos=23) OR Zoutput:(pos=24) OR Zdevic:(pos=25))" },
    { "mag mijn waarschuwing nu weg ? ;)", "(Zmag:(pos=1) OR Zmijn:(pos=2) OR Zwaarschuw:(pos=3) OR Znu:(pos=4) OR Zweg:(pos=5))" },
    { "Abit NF7-S (nForce 2 Chipset) Rev 2.0", "(abit:(pos=1) OR (nf7:(pos=2) PHRASE 2 s:(pos=3)) OR Znforc:(pos=4) OR 2:(pos=5) OR chipset:(pos=6) OR rev:(pos=7) OR 2.0:(pos=8))" },
    { "Setup Could Not Verify the Integrity of the File\" Error Message Occurs When You Try to Install Windows XP Service Pack 1", "(setup:(pos=1) OR could:(pos=2) OR not:(pos=3) OR verify:(pos=4) OR Zthe:(pos=5) OR integrity:(pos=6) OR Zof:(pos=7) OR Zthe:(pos=8) OR file:(pos=9) OR (error:(pos=10) PHRASE 13 message:(pos=11) PHRASE 13 occurs:(pos=12) PHRASE 13 when:(pos=13) PHRASE 13 you:(pos=14) PHRASE 13 try:(pos=15) PHRASE 13 to:(pos=16) PHRASE 13 install:(pos=17) PHRASE 13 windows:(pos=18) PHRASE 13 xp:(pos=19) PHRASE 13 service:(pos=20) PHRASE 13 pack:(pos=21) PHRASE 13 1:(pos=22)))" },
    { "(browser 19) citrix", "(Zbrowser:(pos=1) OR 19:(pos=2) OR Zcitrix:(pos=3))" },
    { "preg_replace (.*?)", "Zpreg_replac:(pos=1)" },
    { "formule excel #naam\"?\"", "(Zformul:(pos=1) OR Zexcel:(pos=2) OR naam:(pos=3))" },
    { "->", "" },
    { "De instructie op 0x77f436f7 verwijst naar geheugen op 0x007f4778. De lees-of schrijfbewerking (\"written\") op het geheugen is mislukt", "(de:(pos=1) OR Zinstructi:(pos=2) OR Zop:(pos=3) OR 0x77f436f7:(pos=4) OR Zverwijst:(pos=5) OR Znaar:(pos=6) OR Zgeheugen:(pos=7) OR Zop:(pos=8) OR 0x007f4778:(pos=9) OR de:(pos=10) OR (lees:(pos=11) PHRASE 2 of:(pos=12)) OR Zschrijfbewerk:(pos=13) OR written:(pos=14) OR Zop:(pos=15) OR Zhet:(pos=16) OR Zgeheugen:(pos=17) OR Zis:(pos=18) OR Zmislukt:(pos=19))" },
    { "<iframe src=\"www.tweakers.net></iframe>", "(Zifram:(pos=1) OR src:(pos=2) OR (www:(pos=3) PHRASE 4 tweakers:(pos=4) PHRASE 4 net:(pos=5) PHRASE 4 iframe:(pos=6)))" },
    { "\"rpm -e httpd\"", "(rpm:(pos=1) PHRASE 3 e:(pos=2) PHRASE 3 httpd:(pos=3))" },
    { "automatisch op All Flis (*.*)", "(Zautomatisch:(pos=1) OR Zop:(pos=2) OR all:(pos=3) OR flis:(pos=4))" },
    { "(Windows; U; Windows NT 5.1; en-US; rv:1.3b) Gecko/20030210", "(windows:(pos=1) OR u:(pos=2) OR windows:(pos=3) OR nt:(pos=4) OR 5.1:(pos=5) OR (en:(pos=6) PHRASE 2 us:(pos=7)) OR (rv:(pos=8) PHRASE 2 1.3b:(pos=9)) OR (gecko:(pos=10) PHRASE 2 20030210:(pos=11)))" },
    { "en-US; rv:1.3b) Gecko/20030210", "((en:(pos=1) PHRASE 2 us:(pos=2)) OR (rv:(pos=3) PHRASE 2 1.3b:(pos=4)) OR (gecko:(pos=5) PHRASE 2 20030210:(pos=6)))" },
    { "\"en-US; rv:1.3b) Gecko/20030210\"", "(en:(pos=1) PHRASE 6 us:(pos=2) PHRASE 6 rv:(pos=3) PHRASE 6 1.3b:(pos=4) PHRASE 6 gecko:(pos=5) PHRASE 6 20030210:(pos=6))" },
    { "(./) chmod.sh", "(chmod:(pos=1) PHRASE 2 sh:(pos=2))" },
    { "document.write(ssg(\" html", "((document:(pos=1) PHRASE 2 write:(pos=2)) OR ssg:(pos=3) OR html:(pos=4))" },
    { "superstack \"mac+adressen\"", "(Zsuperstack:(pos=1) OR (mac:(pos=2) PHRASE 2 adressen:(pos=3)))" },
    { "IIS getenv(REMOTE_HOST)_", "(iis:(pos=1) OR getenv:(pos=2) OR remote_host:(pos=3) OR _:(pos=4))" },
    { "IIS en getenv(REMOTE_HOST)", "(iis:(pos=1) OR Zen:(pos=2) OR getenv:(pos=3) OR remote_host:(pos=4))" },
    { "php getenv(\"HTTP_REFERER\")", "(Zphp:(pos=1) OR getenv:(pos=2) OR http_referer:(pos=3))" },
    { "nec+-1300", "(nec+:(pos=1) PHRASE 2 1300:(pos=2))" },
    { "smbpasswd script \"-s\"", "(Zsmbpasswd:(pos=1) OR Zscript:(pos=2) OR s:(pos=3))" },
    { "leestekens \" \xc3\xb6 \xc3\xab", "(Zleesteken:(pos=1) OR (\xc3\xb6:(pos=2) PHRASE 2 \xc3\xab:(pos=3)))" },
    { "freesco and (all seeing eye)", "(Zfreesco:(pos=1) OR Zand:(pos=2) OR Zall:(pos=3) OR Zsee:(pos=4) OR Zeye:(pos=5))" },
    { "('all seeing eye') and freesco", "(Zall:(pos=1) OR Zsee:(pos=2) OR Zeye:(pos=3) OR Zand:(pos=4) OR Zfreesco:(pos=5))" },
    { "\"[......\"", "" },
    { "Error = 11004 (500 No Data (Winsock error #11004))", "(error:(pos=1) OR 11004:(pos=2) OR 500:(pos=3) OR no:(pos=4) OR data:(pos=5) OR winsock:(pos=6) OR Zerror:(pos=7) OR 11004:(pos=8))" },
    { "gegevensfout (cyclishe redundantiecontrole)", "(Zgegevensfout:(pos=1) OR Zcyclish:(pos=2) OR Zredundantiecontrol:(pos=3))" },
    { "firmware versie waar NEC\"", "(Zfirmwar:(pos=1) OR Zversi:(pos=2) OR Zwaar:(pos=3) OR nec:(pos=4))" },
    { "nu.nl \"-1\"", "((nu:(pos=1) PHRASE 2 nl:(pos=2)) OR 1:(pos=3))" },
    { "provider+-webspace", "(provider+:(pos=1) PHRASE 2 webspace:(pos=2))" },
    { "verschil \"dvd+rw\" \"dvd-rw\"", "(Zverschil:(pos=1) OR (dvd:(pos=2) PHRASE 2 rw:(pos=3)) OR (dvd:(pos=4) PHRASE 2 rw:(pos=5)))" },
    { "(dhcp client) + hangt", "(Zdhcp:(pos=1) OR Zclient:(pos=2) OR Zhangt:(pos=3))" },
    { "MSI 875P Neo-FIS2R (Intel 875P)", "(msi:(pos=1) OR 875p:(pos=2) OR (neo:(pos=3) PHRASE 2 fis2r:(pos=4)) OR intel:(pos=5) OR 875p:(pos=6))" },
    { "voeding passief gekoeld\"", "(Zvoed:(pos=1) OR Zpassief:(pos=2) OR gekoeld:(pos=3))" },
    { "if (mysql_num_rows($resultaat)==1)", "(Zif:(pos=1) OR mysql_num_rows:(pos=2) OR Zresultaat:(pos=3) OR 1:(pos=4))" },
    { "Server.CreateObject(\"Persits.Upload.1\")", "((server:(pos=1) PHRASE 2 createobject:(pos=2)) OR (persits:(pos=3) PHRASE 3 upload:(pos=4) PHRASE 3 1:(pos=5)))" },
    { "if(cod>9999999)cod=parseInt(cod/64)", "(if:(pos=1) OR cod:(pos=2) OR 9999999:(pos=3) OR cod:(pos=4) OR parseint:(pos=5) OR (cod:(pos=6) PHRASE 2 64:(pos=7)))" },
    { "if (cod>9999999", "(Zif:(pos=1) OR cod:(pos=2) OR 9999999:(pos=3))" },
    { "\"rm -rf /bin/laden\"", "(rm:(pos=1) PHRASE 4 rf:(pos=2) PHRASE 4 bin:(pos=3) PHRASE 4 laden:(pos=4))" },
    { "\">>> 0) & 0xFF\"", "(0:(pos=1) PHRASE 2 0xff:(pos=2))" },
    { "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01//EN\"> document.body.scrollHeight", "(doctype:(pos=1) OR html:(pos=2) OR public:(pos=3) OR (w3c:(pos=4) PHRASE 5 dtd:(pos=5) PHRASE 5 html:(pos=6) PHRASE 5 4.01:(pos=7) PHRASE 5 en:(pos=8)) OR (document:(pos=9) PHRASE 3 body:(pos=10) PHRASE 3 scrollheight:(pos=11)))" },
    { "<BR>window.resizeBy(offsetX,offsetY)<P>kweet", "(br:(pos=1) OR (window:(pos=2) PHRASE 2 resizeby:(pos=3)) OR Zoffsetx:(pos=4) OR Zoffseti:(pos=5) OR p:(pos=6) OR Zkweet:(pos=7))" },
    { "linux humor :)", "(Zlinux:(pos=1) OR Zhumor:(pos=2))" },
    { "ClassFactory kan aangevraagde klasse niet leveren  (Fout=80040111)", "(classfactory:(pos=1) OR Zkan:(pos=2) OR Zaangevraagd:(pos=3) OR Zklass:(pos=4) OR Zniet:(pos=5) OR Zleveren:(pos=6) OR fout:(pos=7) OR 80040111:(pos=8))" },
    { "remote_smtp defer (-44)", "(Zremote_smtp:(pos=1) OR Zdefer:(pos=2) OR 44:(pos=3))" },
    { "txtlogin.getText().trim().toUpperCase().intern() == inuser[2 * (i - 1) + 2].trim().toUpperCase().intern() && txtpass.getText().trim().toUpperCase().intern() == inuser[2 * (i - 1) + 3].trim().toUpperCase().intern())", "((txtlogin:(pos=1) PHRASE 2 gettext:(pos=2)) OR trim:(pos=3) OR touppercase:(pos=4) OR intern:(pos=5) OR inuser:(pos=6) OR 2:(pos=7) OR Zi:(pos=8) OR 1:(pos=9) OR 2:(pos=10) OR trim:(pos=11) OR touppercase:(pos=12) OR intern:(pos=13) OR (txtpass:(pos=14) PHRASE 2 gettext:(pos=15)) OR trim:(pos=16) OR touppercase:(pos=17) OR intern:(pos=18) OR inuser:(pos=19) OR 2:(pos=20) OR Zi:(pos=21) OR 1:(pos=22) OR 3:(pos=23) OR trim:(pos=24) OR touppercase:(pos=25) OR intern:(pos=26))" },
    { "Koper + amoniak (NH2", "(koper:(pos=1) OR Zamoniak:(pos=2) OR nh2:(pos=3))" },
    { "nec dvd -/+r", "((Znec:(pos=1) OR Zdvd:(pos=2)) AND_NOT Zr:(pos=3))" }, // Not ideal at all - "-" shouldn't fire here...
    { "er is een gereserveerde fout (-1104) opgetreden", "(Zer:(pos=1) OR Zis:(pos=2) OR Zeen:(pos=3) OR Zgereserveerd:(pos=4) OR Zfout:(pos=5) OR 1104:(pos=6) OR Zopgetreden:(pos=7))" },
    { "Cor \\(CCN\\)'\" <cor.kloet@ccn.controlec.nl>", "(cor:(pos=1) OR ccn:(pos=2) OR (cor:(pos=3) PHRASE 5 kloet:(pos=4) PHRASE 5 ccn:(pos=5) PHRASE 5 controlec:(pos=6) PHRASE 5 nl:(pos=7)))" },
    { "Warning: Failed opening for inclusion (include_path='') in Unknown on line 0", "(warning:(pos=1) OR failed:(pos=2) OR Zopen:(pos=3) OR Zfor:(pos=4) OR Zinclus:(pos=5) OR include_path:(pos=6) OR Zin:(pos=7) OR unknown:(pos=8) OR Zon:(pos=9) OR Zline:(pos=10) OR 0:(pos=11))" },
    { "\"~\" + \"c:\\\"", "Zc:(pos=1)" },
    { "mysql count(*)", "(Zmysql:(pos=1) OR count:(pos=2))" },
    { "for %f in (*.*) do", "(Zfor:(pos=1) OR Zf:(pos=2) OR Zin:(pos=3) OR Zdo:(pos=4))" },
    { "raar \"~\" bestand", "(Zraar:(pos=1) OR Zbestand:(pos=2))" },
    { "NEC DVD +-R/RW 1300", "(nec:(pos=1) OR dvd:(pos=2) OR (r:(pos=3) PHRASE 2 rw:(pos=4)) OR 1300:(pos=5))" },
    { "approved (ref: 38446-263)", "(Zapprov:(pos=1) OR Zref:(pos=2) OR (38446:(pos=3) PHRASE 2 263:(pos=4)))" },
    { "GA-7VRXP(2.0)", "((ga:(pos=1) PHRASE 2 7vrxp:(pos=2)) OR 2.0:(pos=3))" },
    { "~ Could not retrieve directory listing for \"/\"", "(could:(pos=1) OR Znot:(pos=2) OR Zretriev:(pos=3) OR Zdirectori:(pos=4) OR Zlist:(pos=5) OR Zfor:(pos=6))" },
    { "asp CreateObject(\"Word.Document\")", "(Zasp:(pos=1) OR createobject:(pos=2) OR (word:(pos=3) PHRASE 2 document:(pos=4)))" },
    { "De lees- of schrijfbewerking (\"written\") op het geheugen is mislukt.", "(de:(pos=1) OR Zlee:(pos=2) OR Zof:(pos=3) OR Zschrijfbewerk:(pos=4) OR written:(pos=5) OR Zop:(pos=6) OR Zhet:(pos=7) OR Zgeheugen:(pos=8) OR Zis:(pos=9) OR Zmislukt:(pos=10))" },
    { "putStr (map (\\x -> chr (round (21/2 * x^3 - 92 * x^2 + 503/2 * x - 105))) [1..4])", "(Zputstr:(pos=1) OR Zmap:(pos=2) OR ((Zx:(pos=3) OR Zround:(pos=5) OR (21:(pos=6) PHRASE 2 2:(pos=7)) OR Zx:(pos=8) OR 3:(pos=9) OR 92:(pos=10) OR Zx:(pos=11) OR 2:(pos=12) OR (503:(pos=13) PHRASE 2 2:(pos=14)) OR Zx:(pos=15) OR 105:(pos=16)) AND_NOT Zchr:(pos=4)) OR (1:(pos=17) PHRASE 2 4:(pos=18)))" },
    { "parent.document.getElementById(\\\"leftmenu\\\").cols", "((parent:(pos=1) PHRASE 3 document:(pos=2) PHRASE 3 getelementbyid:(pos=3)) OR leftmenu:(pos=4) OR Zcol:(pos=5))" },
    { "<% if not isEmpty(Request.QueryString) then", "(Zif:(pos=1) OR Znot:(pos=2) OR isempty:(pos=3) OR (request:(pos=4) PHRASE 2 querystring:(pos=5)) OR Zthen:(pos=6))" },
    { "Active Desktop (Hier issie)", "(active:(pos=1) OR desktop:(pos=2) OR hier:(pos=3) OR Zissi:(pos=4))" },
    { "Asus A7V8X (LAN + Sound)", "(asus:(pos=1) OR a7v8x:(pos=2) OR lan:(pos=3) OR sound:(pos=4))" },
    { "Novell This pentium class machine (or greater) lacks some required CPU feature(s", "(novell:(pos=1) OR this:(pos=2) OR Zpentium:(pos=3) OR Zclass:(pos=4) OR Zmachin:(pos=5) OR Zor:(pos=6) OR Zgreater:(pos=7) OR Zlack:(pos=8) OR Zsome:(pos=9) OR Zrequir:(pos=10) OR cpu:(pos=11) OR feature:(pos=12) OR Zs:(pos=13))" },
    { "sql server install fails error code (-1)", "(Zsql:(pos=1) OR Zserver:(pos=2) OR Zinstal:(pos=3) OR Zfail:(pos=4) OR Zerror:(pos=5) OR Zcode:(pos=6) OR 1:(pos=7))" },
    { "session_register(\"login\");", "(session_register:(pos=1) OR login:(pos=2))" },
    { "\"kylix+ndmb\"", "(kylix:(pos=1) PHRASE 2 ndmb:(pos=2))" },
    { "Cannot find imap library (libc-client.a).", "(cannot:(pos=1) OR Zfind:(pos=2) OR Zimap:(pos=3) OR Zlibrari:(pos=4) OR (libc:(pos=5) PHRASE 3 client:(pos=6) PHRASE 3 a:(pos=7)))" },
    { "If ($_SESSION[\"Login\"] == 1)", "(if:(pos=1) OR _session:(pos=2) OR login:(pos=3) OR 1:(pos=4))" },
    { "You have an error in your SQL syntax near '1')' at line 1", "(you:(pos=1) OR Zhave:(pos=2) OR Zan:(pos=3) OR Zerror:(pos=4) OR Zin:(pos=5) OR Zyour:(pos=6) OR sql:(pos=7) OR Zsyntax:(pos=8) OR Znear:(pos=9) OR 1:(pos=10) OR Zat:(pos=11) OR Zline:(pos=12) OR 1:(pos=13))" },
    { "ASRock K7VT2 (incl. LAN)", "(asrock:(pos=1) OR k7vt2:(pos=2) OR Zincl:(pos=3) OR lan:(pos=4))" },
    { "+windows98 +(geen communicatie) +ie5", "(Zwindows98:(pos=1) AND (Zgeen:(pos=2) OR Zcommunicati:(pos=3)) AND Zie5:(pos=4))" },
    { "\"xterm -fn\"", "(xterm:(pos=1) PHRASE 2 fn:(pos=2))" },
    { "IRQL_NOT_LESS_OR_EQUAL", "irql_not_less_or_equal:(pos=1)" },
    { "access query \"NOT IN\"", "(Zaccess:(pos=1) OR Zqueri:(pos=2) OR (not:(pos=3) PHRASE 2 in:(pos=4)))" },
    { "\"phrase one \"phrase two\"", "((phrase:(pos=1) PHRASE 2 one:(pos=2)) OR Zphrase:(pos=3) OR two:(pos=4))" }, // FIXME: 2 phrases better?
    { "NEAR 207 46 249 27", "(near:(pos=1) OR 207:(pos=2) OR 46:(pos=3) OR 249:(pos=4) OR 27:(pos=5))" },
    { "- NEAR 12V voeding", "(near:(pos=1) OR 12v:(pos=2) OR Zvoed:(pos=3))" },
    { "waarom \"~\" in directorynaam", "(Zwaarom:(pos=1) OR Zin:(pos=2) OR Zdirectorynaam:(pos=3))" },
    { "cd'r NEAR toebehoren", "(cd'r:(pos=1) NEAR 11 toebehoren:(pos=2))" },
    { "site:1 site:2", "0 * (H1 OR H2)" },
    { "site:1 site2:2", "0 * (H1 AND J2)" },
    { "site:1 site:2 site2:2", "0 * ((H1 OR H2) AND J2)" },
    { "site:1 OR site:2", "(0 * H1 OR 0 * H2)" },
    { "site:1 AND site:2", "(0 * H1 AND 0 * H2)" },
    { "foo AND site:2", "(Zfoo:(pos=1) AND 0 * H2)" },
    // Non-exclusive boolean prefixes feature tests (ticket#402):
    { "category:1 category:2", "0 * (XCAT1 AND XCAT2)" },
    { "category:1 site2:2", "0 * (J2 AND XCAT1)" },
    { "category:1 category:2 site2:2", "0 * (J2 AND XCAT1 AND XCAT2)" },
    { "category:1 OR category:2", "(0 * XCAT1 OR 0 * XCAT2)" },
    { "category:1 AND category:2", "(0 * XCAT1 AND 0 * XCAT2)" },
    { "foo AND category:2", "(Zfoo:(pos=1) AND 0 * XCAT2)" },
#if 0
    { "A site:1 site:2", "(a FILTER (H1 OR H2))" },
    { "A (site:1 OR site:2)", "(a FILTER (H1 OR H2))" },
    { "A (site:1 OR site:2)", "(a FILTER (H1 OR H2))" },
    { "A site:1 site2:2", "(a FILTER (H1 AND J2))" },
    { "A site:1 site:2 site2:2", "(a FILTER ((H1 OR H2) AND J2))" },
    { "A site:1 OR site:2", "(a FILTER (H1 OR H2))" },
    { "A site:1 AND site:2", "(a FILTER (H1 AND H2))" },
#endif
    { "site:xapian.org OR site:www.xapian.org", "(0 * Hxapian.org OR 0 * Hwww.xapian.org)" },
    { "site:xapian.org site:www.xapian.org", "0 * (Hxapian.org OR Hwww.xapian.org)" },
    { "site:xapian.org AND site:www.xapian.org", "(0 * Hxapian.org AND 0 * Hwww.xapian.org)" },
    { "Xapian site:xapian.org site:www.xapian.org", "(xapian:(pos=1) FILTER (Hxapian.org OR Hwww.xapian.org))" },
    { "author:richard author:olly writer:charlie", "(ZArichard:(pos=1) OR ZAolli:(pos=2) OR ZAcharli:(pos=3))"},
    { "author:richard NEAR title:book", "(Arichard:(pos=1) NEAR 11 XTbook:(pos=2))"},
    { "authortitle:richard NEAR title:book", "((Arichard:(pos=1) NEAR 11 XTbook:(pos=2)) OR (XTrichard:(pos=1) NEAR 11 XTbook:(pos=2)))"},
    { "multisite:xapian.org", "0 * (Hxapian.org OR Jxapian.org)"},
    { "authortitle:richard", "(ZArichard:(pos=1) OR ZXTrichard:(pos=1))"},
    { "multisite:xapian.org site:www.xapian.org author:richard authortitle:richard", "((ZArichard:(pos=1) OR ZArichard:(pos=2) OR ZXTrichard:(pos=2)) FILTER (Hwww.xapian.org AND (Hxapian.org OR Jxapian.org)))"},
    { "authortitle:richard-boulton", "((Arichard:(pos=1) PHRASE 2 Aboulton:(pos=2)) OR (XTrichard:(pos=1) PHRASE 2 XTboulton:(pos=2)))"},
    { "authortitle:\"richard boulton\"", "((Arichard:(pos=1) PHRASE 2 Aboulton:(pos=2)) OR (XTrichard:(pos=1) PHRASE 2 XTboulton:(pos=2)))"},
    // Some CJK tests.
    { "久有归天愿", "(久:(pos=1) AND 久有:(pos=1) AND 有:(pos=1) AND 有归:(pos=1) AND 归:(pos=1) AND 归天:(pos=1) AND 天:(pos=1) AND 天愿:(pos=1) AND 愿:(pos=1))" },
    { "久有 归天愿", "((久:(pos=1) AND 久有:(pos=1) AND 有:(pos=1)) OR (归:(pos=2) AND 归天:(pos=2) AND 天:(pos=2) AND 天愿:(pos=2) AND 愿:(pos=2)))" },
    { "久有！归天愿", "((久:(pos=1) AND 久有:(pos=1) AND 有:(pos=1)) OR (归:(pos=2) AND 归天:(pos=2) AND 天:(pos=2) AND 天愿:(pos=2) AND 愿:(pos=2)))" },
    { "title:久有 归 天愿", "((XT久:(pos=1) AND XT久有:(pos=1) AND XT有:(pos=1)) OR 归:(pos=2) OR (天:(pos=3) AND 天愿:(pos=3) AND 愿:(pos=3)))" },
    { "h众ello万众", "(Zh:(pos=1) OR 众:(pos=2) OR Zello:(pos=3) OR (万:(pos=4) AND 万众:(pos=4) AND 众:(pos=4)))" },
    { "世(の中)TEST_tm", "(世:(pos=1) OR (の:(pos=2) AND の中:(pos=2) AND 中:(pos=2)) OR test_tm:(pos=3))" },
    { "다녀 AND 와야", "(다:(pos=1) AND 다녀:(pos=1) AND 녀:(pos=1) AND 와:(pos=2) AND 와야:(pos=2) AND 야:(pos=2))" },
    { "authortitle:학술 OR 연구를", "((A학:(pos=1) AND XT학:(pos=1) AND A학술:(pos=1) AND XT학술:(pos=1) AND A술:(pos=1) AND XT술:(pos=1)) OR (연:(pos=2) AND 연구:(pos=2) AND 구:(pos=2) AND 구를:(pos=2) AND 를:(pos=2)))" },
    // FIXME: These should really filter by bigrams to accelerate:
    { "\"久有归\"", "(久:(pos=1) PHRASE 3 有:(pos=1) PHRASE 3 归:(pos=1))" },
    { "\"久有test归\"", "(久:(pos=1) PHRASE 4 有:(pos=1) PHRASE 4 test:(pos=2) PHRASE 4 归:(pos=3))" },
    // FIXME: this should work: { "久 NEAR 有", "(久:(pos=1) NEAR 11 有:(pos=2))" },
    { NULL, NULL }
};

static bool test_queryparser1()
{
    Xapian::QueryParser queryparser;
    queryparser.set_stemmer(Xapian::Stem("english"));
    queryparser.set_stemming_strategy(Xapian::QueryParser::STEM_SOME);
    queryparser.add_prefix("author", "A");
    queryparser.add_prefix("writer", "A");
    queryparser.add_prefix("title", "XT");
    queryparser.add_prefix("subject", "XT");
    queryparser.add_prefix("authortitle", "A");
    queryparser.add_prefix("authortitle", "XT");
    queryparser.add_boolean_prefix("site", "H");
    queryparser.add_boolean_prefix("site2", "J");
    queryparser.add_boolean_prefix("multisite", "H");
    queryparser.add_boolean_prefix("multisite", "J");
    queryparser.add_boolean_prefix("category", "XCAT", false);
    TEST_EXCEPTION(Xapian::InvalidOperationError,
	queryparser.add_boolean_prefix("authortitle", "B");
    );
    TEST_EXCEPTION(Xapian::InvalidOperationError,
	queryparser.add_prefix("multisite", "B");
    );
    for (const test *p = test_or_queries; p->query; ++p) {
	string expect, parsed;
	if (p->expect)
	    expect = p->expect;
	else
	    expect = "parse error";
	try {
	    Xapian::Query qobj = queryparser.parse_query(p->query);
	    parsed = qobj.get_description();
	    expect = string("Xapian::Query(") + expect + ')';
	} catch (const Xapian::QueryParserError &e) {
	    parsed = e.get_msg();
	} catch (const Xapian::Error &e) {
	    parsed = e.get_description();
	} catch (...) {
	    parsed = "Unknown exception!";
	}
	tout << "Query: " << p->query << '\n';
	TEST_STRINGS_EQUAL(parsed, expect);
    }
    return true;
}

static const test test_and_queries[] = {
    { "internet explorer title:(http www)", "(Zinternet:(pos=1) AND Zexplor:(pos=2) AND ZXThttp:(pos=3) AND ZXTwww:(pos=4))" },
    // Regression test for bug in 0.9.2 and earlier - this would give
    // (two:(pos=2) AND_MAYBE (one:(pos=1) AND three:(pos=3)))
    { "one +two three", "(Zone:(pos=1) AND Ztwo:(pos=2) AND Zthree:(pos=3))" },
    { "hello -title:\"hello world\"", "(Zhello:(pos=1) AND_NOT (XThello:(pos=2) PHRASE 2 XTworld:(pos=3)))" },
    // Regression test for bug fixed in 1.0.4 - the '-' would be ignored there
    // because the whitespace after the '"' wasn't noticed.
    { "\"hello world\" -C++", "((hello:(pos=1) PHRASE 2 world:(pos=2)) AND_NOT c++:(pos=3))" },
    // Regression tests for bug fixed in 1.0.4 - queries with only boolean
    // filter and HATE terms weren't accepted.
    { "-cup site:world", "(0 * Hworld AND_NOT Zcup:(pos=1))" },
    { "site:world -cup", "(0 * Hworld AND_NOT Zcup:(pos=1))" },
    // Regression test for bug fixed in 1.0.4 - the KET token for ')' was lost.
    { "(site:world) -cup", "(0 * Hworld AND_NOT Zcup:(pos=1))" },
    // Regression test for bug fixed in 1.0.4 - a boolean filter term between
    // probabilistic terms caused a parse error (probably broken during the
    // addition of synonym support in 1.0.2).
    { "foo site:xapian.org bar", "((Zfoo:(pos=1) AND Zbar:(pos=2)) FILTER Hxapian.org)" },
    // Add coverage for other cases similar to the above.
    { "a b site:xapian.org", "((Za:(pos=1) AND Zb:(pos=2)) FILTER Hxapian.org)" },
    { "site:xapian.org a b", "((Za:(pos=1) AND Zb:(pos=2)) FILTER Hxapian.org)" },
    // Some CJK tests.
    { "author:험가 OR subject:万众 hello world!", "((A험:(pos=1) AND A험가:(pos=1) AND A가:(pos=1)) OR (XT万:(pos=2) AND XT万众:(pos=2) AND XT众:(pos=2) AND Zhello:(pos=3) AND Zworld:(pos=4)))" },
    { "洛伊one儿差点two脸three", "(洛:(pos=1) AND 洛伊:(pos=1) AND 伊:(pos=1) AND Zone:(pos=2) AND 儿:(pos=3) AND 儿差:(pos=3) AND 差:(pos=3) AND 差点:(pos=3) AND 点:(pos=3) AND Ztwo:(pos=4) AND 脸:(pos=5) AND Zthree:(pos=6))" },
    { NULL, NULL }
};

// With default_op = OP_AND.
static bool test_qp_default_op1()
{
    Xapian::QueryParser queryparser;
    queryparser.set_stemmer(Xapian::Stem("english"));
    queryparser.set_stemming_strategy(Xapian::QueryParser::STEM_SOME);
    queryparser.add_prefix("author", "A");
    queryparser.add_prefix("title", "XT");
    queryparser.add_prefix("subject", "XT");
    queryparser.add_boolean_prefix("site", "H");
    queryparser.set_default_op(Xapian::Query::OP_AND);
    for (const test *p = test_and_queries; p->query; ++p) {
	string expect, parsed;
	if (p->expect)
	    expect = p->expect;
	else
	    expect = "parse error";
	try {
	    Xapian::Query qobj = queryparser.parse_query(p->query);
	    parsed = qobj.get_description();
	    expect = string("Xapian::Query(") + expect + ')';
	} catch (const Xapian::QueryParserError &e) {
	    parsed = e.get_msg();
	} catch (const Xapian::Error &e) {
	    parsed = e.get_description();
	} catch (...) {
	    parsed = "Unknown exception!";
	}
	tout << "Query: " << p->query << '\n';
	TEST_STRINGS_EQUAL(parsed, expect);
    }
    return true;
}

// Feature test for specify the default prefix (new in Xapian 1.0.0).
static bool test_qp_default_prefix1()
{
    Xapian::QueryParser qp;
    qp.set_stemmer(Xapian::Stem("english"));
    qp.set_stemming_strategy(Xapian::QueryParser::STEM_SOME);
    qp.add_prefix("title", "XT");

    Xapian::Query qobj;
    qobj = qp.parse_query("hello world", 0, "A");
    TEST_STRINGS_EQUAL(qobj.get_description(), "Xapian::Query((ZAhello:(pos=1) OR ZAworld:(pos=2)))");
    qobj = qp.parse_query("me title:stuff", 0, "A");
    TEST_STRINGS_EQUAL(qobj.get_description(), "Xapian::Query((ZAme:(pos=1) OR ZXTstuff:(pos=2)))");
    qobj = qp.parse_query("title:(stuff) me", Xapian::QueryParser::FLAG_BOOLEAN, "A");
    TEST_STRINGS_EQUAL(qobj.get_description(), "Xapian::Query((ZXTstuff:(pos=1) OR ZAme:(pos=2)))");
    qobj = qp.parse_query("英国 title:文森hello", 0, "A");
    TEST_STRINGS_EQUAL(qobj.get_description(), "Xapian::Query(((A英:(pos=1) AND A英国:(pos=1) AND A国:(pos=1)) OR (XT文:(pos=2) AND XT文森:(pos=2) AND XT森:(pos=2)) OR ZAhello:(pos=3)))");
    return true;
}

// Feature test for setting the default prefix with add_prefix()
// (new in Xapian 1.0.3).
static bool test_qp_default_prefix2()
{
    Xapian::QueryParser qp;
    qp.set_stemmer(Xapian::Stem("english"));
    qp.set_stemming_strategy(Xapian::QueryParser::STEM_SOME);

    // test that default prefixes can only be set with add_prefix().
    TEST_EXCEPTION(Xapian::UnimplementedError,
	qp.add_boolean_prefix("", "B");
    );

    qp.add_prefix("title", "XT");
    qp.add_prefix("", "A");

    Xapian::Query qobj;
    qobj = qp.parse_query("hello world", 0);
    TEST_STRINGS_EQUAL(qobj.get_description(), "Xapian::Query((ZAhello:(pos=1) OR ZAworld:(pos=2)))");
    qobj = qp.parse_query("me title:stuff", 0);
    TEST_STRINGS_EQUAL(qobj.get_description(), "Xapian::Query((ZAme:(pos=1) OR ZXTstuff:(pos=2)))");
    qobj = qp.parse_query("title:(stuff) me", Xapian::QueryParser::FLAG_BOOLEAN);
    TEST_STRINGS_EQUAL(qobj.get_description(), "Xapian::Query((ZXTstuff:(pos=1) OR ZAme:(pos=2)))");

    qobj = qp.parse_query("hello world", 0, "B");
    TEST_STRINGS_EQUAL(qobj.get_description(), "Xapian::Query((ZBhello:(pos=1) OR ZBworld:(pos=2)))");
    qobj = qp.parse_query("me title:stuff", 0, "B");
    TEST_STRINGS_EQUAL(qobj.get_description(), "Xapian::Query((ZBme:(pos=1) OR ZXTstuff:(pos=2)))");
    qobj = qp.parse_query("title:(stuff) me", Xapian::QueryParser::FLAG_BOOLEAN, "B");
    TEST_STRINGS_EQUAL(qobj.get_description(), "Xapian::Query((ZXTstuff:(pos=1) OR ZBme:(pos=2)))");

    qp.add_prefix("", "B");
    qobj = qp.parse_query("me-us title:(stuff) me", Xapian::QueryParser::FLAG_BOOLEAN);
    TEST_STRINGS_EQUAL(qobj.get_description(), "Xapian::Query(((Ame:(pos=1) PHRASE 2 Aus:(pos=2)) OR (Bme:(pos=1) PHRASE 2 Bus:(pos=2)) OR ZXTstuff:(pos=3) OR ZAme:(pos=4) OR ZBme:(pos=4)))");
    qobj = qp.parse_query("me-us title:(stuff) me", Xapian::QueryParser::FLAG_BOOLEAN, "C");
    TEST_STRINGS_EQUAL(qobj.get_description(), "Xapian::Query(((Cme:(pos=1) PHRASE 2 Cus:(pos=2)) OR ZXTstuff:(pos=3) OR ZCme:(pos=4)))");

    qobj = qp.parse_query("me-us title:\"not-me\"", Xapian::QueryParser::FLAG_PHRASE);
    TEST_STRINGS_EQUAL(qobj.get_description(), "Xapian::Query(((Ame:(pos=1) PHRASE 2 Aus:(pos=2)) OR (Bme:(pos=1) PHRASE 2 Bus:(pos=2)) OR (XTnot:(pos=3) PHRASE 2 XTme:(pos=4))))");
    return true;
}

// Test query with odd characters in.
static bool test_qp_odd_chars1()
{
    Xapian::QueryParser qp;
    string query("\x01weird\x00stuff\x7f", 13);
    Xapian::Query qobj = qp.parse_query(query);
    tout << "Query:  " << query << '\n';
    TEST_STRINGS_EQUAL(qobj.get_description(), "Xapian::Query((weird:(pos=1) OR stuff:(pos=2)))"); // FIXME: should these be stemmed?
    return true;
}

// Test right truncation.
static bool test_qp_flag_wildcard1()
{
#ifndef XAPIAN_HAS_INMEMORY_BACKEND
    SKIP_TEST("Testcase requires the InMemory backend which is disabled");
#else
    Xapian::WritableDatabase db(Xapian::InMemory::open());
    Xapian::Document doc;
    doc.add_term("abc");
    doc.add_term("main");
    doc.add_term("muscat");
    doc.add_term("muscle");
    doc.add_term("musclebound");
    doc.add_term("muscular");
    doc.add_term("mutton");
    db.add_document(doc);
    Xapian::QueryParser qp;
    qp.set_database(db);
    Xapian::Query qobj = qp.parse_query("ab*", Xapian::QueryParser::FLAG_WILDCARD);
    TEST_STRINGS_EQUAL(qobj.get_description(), "Xapian::Query(abc:(pos=1))");
    qobj = qp.parse_query("muscle*", Xapian::QueryParser::FLAG_WILDCARD);
    TEST_STRINGS_EQUAL(qobj.get_description(), "Xapian::Query((muscle:(pos=1) SYNONYM musclebound:(pos=1)))");
    qobj = qp.parse_query("meat*", Xapian::QueryParser::FLAG_WILDCARD);
    TEST_STRINGS_EQUAL(qobj.get_description(), "Xapian::Query()");
    qobj = qp.parse_query("musc*", Xapian::QueryParser::FLAG_WILDCARD);
    TEST_STRINGS_EQUAL(qobj.get_description(), "Xapian::Query((muscat:(pos=1) SYNONYM muscle:(pos=1) SYNONYM musclebound:(pos=1) SYNONYM muscular:(pos=1)))");
    qobj = qp.parse_query("mutt*", Xapian::QueryParser::FLAG_WILDCARD);
    TEST_STRINGS_EQUAL(qobj.get_description(), "Xapian::Query(mutton:(pos=1))");
    // Regression test (we weren't lowercasing terms before checking if they
    // were in the database or not):
    qobj = qp.parse_query("mUTTON++");
    TEST_STRINGS_EQUAL(qobj.get_description(), "Xapian::Query(mutton:(pos=1))");
    // Regression test: check that wildcards work with +terms.
    unsigned flags = Xapian::QueryParser::FLAG_WILDCARD |
		     Xapian::QueryParser::FLAG_LOVEHATE;
    qobj = qp.parse_query("+mai* main", flags);
    TEST_STRINGS_EQUAL(qobj.get_description(), "Xapian::Query((main:(pos=1) AND_MAYBE main:(pos=2)))");
    // Regression test (if we had a +term which was a wildcard and wasn't
    // present, the query could still match documents).
    qobj = qp.parse_query("foo* main", flags);
    TEST_STRINGS_EQUAL(qobj.get_description(), "Xapian::Query(main:(pos=2))");
    qobj = qp.parse_query("main foo*", flags);
    TEST_STRINGS_EQUAL(qobj.get_description(), "Xapian::Query(main:(pos=1))");
    qobj = qp.parse_query("+foo* main", flags);
    TEST_STRINGS_EQUAL(qobj.get_description(), "Xapian::Query()");
    qobj = qp.parse_query("main +foo*", flags);
    TEST_STRINGS_EQUAL(qobj.get_description(), "Xapian::Query()");
    qobj = qp.parse_query("foo* +main", flags);
    TEST_STRINGS_EQUAL(qobj.get_description(), "Xapian::Query(main:(pos=2))");
    qobj = qp.parse_query("+main foo*", flags);
    TEST_STRINGS_EQUAL(qobj.get_description(), "Xapian::Query(main:(pos=1))");
    qobj = qp.parse_query("+foo* +main", flags);
    TEST_STRINGS_EQUAL(qobj.get_description(), "Xapian::Query()");
    qobj = qp.parse_query("+main +foo*", flags);
    TEST_STRINGS_EQUAL(qobj.get_description(), "Xapian::Query()");
    qobj = qp.parse_query("foo* mai", flags);
    TEST_STRINGS_EQUAL(qobj.get_description(), "Xapian::Query(mai:(pos=2))");
    qobj = qp.parse_query("mai foo*", flags);
    TEST_STRINGS_EQUAL(qobj.get_description(), "Xapian::Query(mai:(pos=1))");
    qobj = qp.parse_query("+foo* mai", flags);
    TEST_STRINGS_EQUAL(qobj.get_description(), "Xapian::Query()");
    qobj = qp.parse_query("mai +foo*", flags);
    TEST_STRINGS_EQUAL(qobj.get_description(), "Xapian::Query()");
    qobj = qp.parse_query("foo* +mai", flags);
    TEST_STRINGS_EQUAL(qobj.get_description(), "Xapian::Query(mai:(pos=2))");
    qobj = qp.parse_query("+mai foo*", flags);
    TEST_STRINGS_EQUAL(qobj.get_description(), "Xapian::Query(mai:(pos=1))");
    qobj = qp.parse_query("+foo* +mai", flags);
    TEST_STRINGS_EQUAL(qobj.get_description(), "Xapian::Query()");
    qobj = qp.parse_query("+mai +foo*", flags);
    TEST_STRINGS_EQUAL(qobj.get_description(), "Xapian::Query()");
    qobj = qp.parse_query("-foo* main", flags);
    TEST_STRINGS_EQUAL(qobj.get_description(), "Xapian::Query(main:(pos=2))");
    qobj = qp.parse_query("main -foo*", flags);
    TEST_STRINGS_EQUAL(qobj.get_description(), "Xapian::Query(main:(pos=1))");
    qobj = qp.parse_query("main -foo* -bar", flags);
    TEST_STRINGS_EQUAL(qobj.get_description(), "Xapian::Query((main:(pos=1) AND_NOT bar:(pos=3)))");
    qobj = qp.parse_query("main -bar -foo*", flags);
    TEST_STRINGS_EQUAL(qobj.get_description(), "Xapian::Query((main:(pos=1) AND_NOT bar:(pos=2)))");
    // Check with OP_AND too.
    qp.set_default_op(Xapian::Query::OP_AND);
    qobj = qp.parse_query("foo* main", flags);
    TEST_STRINGS_EQUAL(qobj.get_description(), "Xapian::Query()");
    qobj = qp.parse_query("main foo*", flags);
    TEST_STRINGS_EQUAL(qobj.get_description(), "Xapian::Query()");
    qp.set_default_op(Xapian::Query::OP_AND);
    qobj = qp.parse_query("+foo* main", flags);
    TEST_STRINGS_EQUAL(qobj.get_description(), "Xapian::Query()");
    qobj = qp.parse_query("main +foo*", flags);
    TEST_STRINGS_EQUAL(qobj.get_description(), "Xapian::Query()");
    qobj = qp.parse_query("-foo* main", flags);
    TEST_STRINGS_EQUAL(qobj.get_description(), "Xapian::Query(main:(pos=2))");
    qobj = qp.parse_query("main -foo*", flags);
    TEST_STRINGS_EQUAL(qobj.get_description(), "Xapian::Query(main:(pos=1))");
    // Check empty wildcard followed by negation.
    qobj = qp.parse_query("foo* -main", Xapian::QueryParser::FLAG_LOVEHATE|Xapian::QueryParser::FLAG_WILDCARD);
    TEST_STRINGS_EQUAL(qobj.get_description(), "Xapian::Query()");
    // Regression test for bug#484 fixed in 1.2.1 and 1.0.21.
    qobj = qp.parse_query("abc muscl* main", flags);
    TEST_STRINGS_EQUAL(qobj.get_description(), "Xapian::Query((abc:(pos=1) AND (muscle:(pos=2) SYNONYM musclebound:(pos=2)) AND main:(pos=3)))");
    return true;
#endif
}

// Test right truncation with prefixes.
static bool test_qp_flag_wildcard2()
{
#ifndef XAPIAN_HAS_INMEMORY_BACKEND
    SKIP_TEST("Testcase requires the InMemory backend which is disabled");
#else
    Xapian::WritableDatabase db(Xapian::InMemory::open());
    Xapian::Document doc;
    doc.add_term("Aheinlein");
    doc.add_term("Ahuxley");
    doc.add_term("hello");
    db.add_document(doc);
    Xapian::QueryParser qp;
    qp.set_database(db);
    qp.add_prefix("author", "A");
    Xapian::Query qobj;
    qobj = qp.parse_query("author:h*", Xapian::QueryParser::FLAG_WILDCARD);
    TEST_STRINGS_EQUAL(qobj.get_description(), "Xapian::Query((Aheinlein:(pos=1) SYNONYM Ahuxley:(pos=1)))");
    qobj = qp.parse_query("author:h* test", Xapian::QueryParser::FLAG_WILDCARD);
    TEST_STRINGS_EQUAL(qobj.get_description(), "Xapian::Query(((Aheinlein:(pos=1) SYNONYM Ahuxley:(pos=1)) OR test:(pos=2)))");
    return true;
#endif
}

#ifdef XAPIAN_HAS_INMEMORY_BACKEND
static void
test_qp_flag_wildcard1_helper(const Xapian::Database &db,
			      Xapian::termcount max_expansion,
			      const string & query_string)
{
    Xapian::QueryParser qp;
    qp.set_database(db);
    qp.set_max_wildcard_expansion(max_expansion);
    Xapian::Enquire e(db);
    e.set_query(qp.parse_query(query_string, Xapian::QueryParser::FLAG_WILDCARD));
    // The exception for expanding too much may happen at parse time or later
    // so we need to calculate the MSet too.
    e.get_mset(0, 10);
}
#endif

// Test right truncation with a limit on expansion.
static bool test_qp_flag_wildcard3()
{
#ifndef XAPIAN_HAS_INMEMORY_BACKEND
    SKIP_TEST("Testcase requires the InMemory backend which is disabled");
#else
    Xapian::WritableDatabase db(Xapian::InMemory::open());
    Xapian::Document doc;
    doc.add_term("abc");
    doc.add_term("main");
    doc.add_term("muscat");
    doc.add_term("muscle");
    doc.add_term("musclebound");
    doc.add_term("muscular");
    doc.add_term("mutton");
    db.add_document(doc);

    // Test that a max of 0 doesn't set a limit.
    test_qp_flag_wildcard1_helper(db, 0, "z*");
    test_qp_flag_wildcard1_helper(db, 0, "m*");

    // These cases should expand to the limit given.
    test_qp_flag_wildcard1_helper(db, 1, "z*");
    test_qp_flag_wildcard1_helper(db, 1, "ab*");
    test_qp_flag_wildcard1_helper(db, 2, "muscle*");
    test_qp_flag_wildcard1_helper(db, 4, "musc*");
    test_qp_flag_wildcard1_helper(db, 4, "mus*");
    test_qp_flag_wildcard1_helper(db, 5, "mu*");
    test_qp_flag_wildcard1_helper(db, 6, "m*");

    // These cases should expand to one more than the limit.
    TEST_EXCEPTION(Xapian::QueryParserError,
	test_qp_flag_wildcard1_helper(db, 1, "muscle*"));
    TEST_EXCEPTION(Xapian::QueryParserError,
	test_qp_flag_wildcard1_helper(db, 3, "musc*"));
    TEST_EXCEPTION(Xapian::QueryParserError,
	test_qp_flag_wildcard1_helper(db, 3, "mus*"));
    TEST_EXCEPTION(Xapian::QueryParserError,
	test_qp_flag_wildcard1_helper(db, 4, "mu*"));
    TEST_EXCEPTION(Xapian::QueryParserError,
	test_qp_flag_wildcard1_helper(db, 5, "m*"));

    return true;
#endif
}

// Test partial queries.
static bool test_qp_flag_partial1()
{
#ifndef XAPIAN_HAS_INMEMORY_BACKEND
    SKIP_TEST("Testcase requires the InMemory backend which is disabled");
#else
    Xapian::WritableDatabase db(Xapian::InMemory::open());
    Xapian::Document doc;
    Xapian::Stem stemmer("english");
    doc.add_term("abc");
    doc.add_term("main");
    doc.add_term("muscat");
    doc.add_term("muscle");
    doc.add_term("musclebound");
    doc.add_term("muscular");
    doc.add_term("mutton");
    doc.add_term("Z" + stemmer("outside"));
    doc.add_term("Z" + stemmer("out"));
    doc.add_term("outside");
    doc.add_term("out");
    doc.add_term("XTcove");
    doc.add_term("XTcows");
    doc.add_term("XTcowl");
    doc.add_term("XTcox");
    doc.add_term("ZXTcow");
    doc.add_term("XONEpartial");
    doc.add_term("XONEpartial2");
    doc.add_term("XTWOpartial3");
    doc.add_term("XTWOpartial4");
    db.add_document(doc);
    Xapian::QueryParser qp;
    qp.set_database(db);
    qp.set_stemmer(stemmer);
    qp.set_stemming_strategy(Xapian::QueryParser::STEM_SOME);
    qp.add_prefix("title", "XT");
    qp.add_prefix("double", "XONE");
    qp.add_prefix("double", "XTWO");

    // Check behaviour with unstemmed terms
    Xapian::Query qobj = qp.parse_query("a", Xapian::QueryParser::FLAG_PARTIAL);
    TEST_STRINGS_EQUAL(qobj.get_description(), "Xapian::Query((abc:(pos=1) OR Za:(pos=1)))");
    qobj = qp.parse_query("ab", Xapian::QueryParser::FLAG_PARTIAL);
    TEST_STRINGS_EQUAL(qobj.get_description(), "Xapian::Query((abc:(pos=1) OR Zab:(pos=1)))");
    qobj = qp.parse_query("muscle", Xapian::QueryParser::FLAG_PARTIAL);
    TEST_STRINGS_EQUAL(qobj.get_description(), "Xapian::Query(((muscle:(pos=1) SYNONYM musclebound:(pos=1)) OR Zmuscl:(pos=1)))");
    qobj = qp.parse_query("meat", Xapian::QueryParser::FLAG_PARTIAL);
    TEST_STRINGS_EQUAL(qobj.get_description(), "Xapian::Query(Zmeat:(pos=1))");
    qobj = qp.parse_query("musc", Xapian::QueryParser::FLAG_PARTIAL);
    TEST_STRINGS_EQUAL(qobj.get_description(), "Xapian::Query(((muscat:(pos=1) SYNONYM muscle:(pos=1) SYNONYM musclebound:(pos=1) SYNONYM muscular:(pos=1)) OR Zmusc:(pos=1)))");
    qobj = qp.parse_query("mutt", Xapian::QueryParser::FLAG_PARTIAL);
    TEST_STRINGS_EQUAL(qobj.get_description(), "Xapian::Query((mutton:(pos=1) OR Zmutt:(pos=1)))");
    qobj = qp.parse_query("abc musc", Xapian::QueryParser::FLAG_PARTIAL);
    TEST_STRINGS_EQUAL(qobj.get_description(), "Xapian::Query((Zabc:(pos=1) OR (muscat:(pos=2) SYNONYM muscle:(pos=2) SYNONYM musclebound:(pos=2) SYNONYM muscular:(pos=2)) OR Zmusc:(pos=2)))");
    qobj = qp.parse_query("a* mutt", Xapian::QueryParser::FLAG_PARTIAL | Xapian::QueryParser::FLAG_WILDCARD);
    TEST_STRINGS_EQUAL(qobj.get_description(), "Xapian::Query((abc:(pos=1) OR mutton:(pos=2) OR Zmutt:(pos=2)))");

    // Check behaviour with stemmed terms, and stem strategy STEM_SOME.
    qobj = qp.parse_query("o", Xapian::QueryParser::FLAG_PARTIAL);
    TEST_STRINGS_EQUAL(qobj.get_description(), "Xapian::Query(((out:(pos=1) SYNONYM outside:(pos=1)) OR Zo:(pos=1)))");
    qobj = qp.parse_query("ou", Xapian::QueryParser::FLAG_PARTIAL);
    TEST_STRINGS_EQUAL(qobj.get_description(), "Xapian::Query(((out:(pos=1) SYNONYM outside:(pos=1)) OR Zou:(pos=1)))");
    qobj = qp.parse_query("out", Xapian::QueryParser::FLAG_PARTIAL);
    TEST_STRINGS_EQUAL(qobj.get_description(), "Xapian::Query(((out:(pos=1) SYNONYM outside:(pos=1)) OR Zout:(pos=1)))");
    qobj = qp.parse_query("outs", Xapian::QueryParser::FLAG_PARTIAL);
    TEST_STRINGS_EQUAL(qobj.get_description(), "Xapian::Query((outside:(pos=1) OR Zout:(pos=1)))");
    qobj = qp.parse_query("outsi", Xapian::QueryParser::FLAG_PARTIAL);
    TEST_STRINGS_EQUAL(qobj.get_description(), "Xapian::Query((outside:(pos=1) OR Zoutsi:(pos=1)))");
    qobj = qp.parse_query("outsid", Xapian::QueryParser::FLAG_PARTIAL);
    TEST_STRINGS_EQUAL(qobj.get_description(), "Xapian::Query((outside:(pos=1) OR Zoutsid:(pos=1)))");
    qobj = qp.parse_query("outside", Xapian::QueryParser::FLAG_PARTIAL);
    TEST_STRINGS_EQUAL(qobj.get_description(), "Xapian::Query((outside:(pos=1) OR Zoutsid:(pos=1)))");

    // Check behaviour with capitalised terms, and stem strategy STEM_SOME.
    qobj = qp.parse_query("Out", Xapian::QueryParser::FLAG_PARTIAL);
    TEST_STRINGS_EQUAL(qobj.get_description(), "Xapian::Query(((out:(pos=1) SYNONYM outside:(pos=1)) OR out:(pos=1)))");
    qobj = qp.parse_query("Outs", Xapian::QueryParser::FLAG_PARTIAL);
    TEST_STRINGS_EQUAL(qobj.get_description(), "Xapian::Query((outside:(pos=1) OR outs:(pos=1)))");
    qobj = qp.parse_query("Outside", Xapian::QueryParser::FLAG_PARTIAL);
    TEST_STRINGS_EQUAL(qobj.get_description(), "Xapian::Query(outside:(pos=1,wqf=2))");

    // And now with stemming strategy STEM_ALL.
    qp.set_stemming_strategy(Xapian::QueryParser::STEM_ALL);
    qobj = qp.parse_query("Out", Xapian::QueryParser::FLAG_PARTIAL);
    TEST_STRINGS_EQUAL(qobj.get_description(), "Xapian::Query(((out:(pos=1) SYNONYM outside:(pos=1)) OR out:(pos=1)))");
    qobj = qp.parse_query("Outs", Xapian::QueryParser::FLAG_PARTIAL);
    TEST_STRINGS_EQUAL(qobj.get_description(), "Xapian::Query((outside:(pos=1) OR out:(pos=1)))");
    qobj = qp.parse_query("Outside", Xapian::QueryParser::FLAG_PARTIAL);
    TEST_STRINGS_EQUAL(qobj.get_description(), "Xapian::Query((outside:(pos=1) OR outsid:(pos=1)))");

    // And now with stemming strategy STEM_ALL_Z.
    qp.set_stemming_strategy(Xapian::QueryParser::STEM_ALL_Z);
    qobj = qp.parse_query("Out", Xapian::QueryParser::FLAG_PARTIAL);
    TEST_STRINGS_EQUAL(qobj.get_description(), "Xapian::Query(((out:(pos=1) SYNONYM outside:(pos=1)) OR Zout:(pos=1)))");
    qobj = qp.parse_query("Outs", Xapian::QueryParser::FLAG_PARTIAL);
    TEST_STRINGS_EQUAL(qobj.get_description(), "Xapian::Query((outside:(pos=1) OR Zout:(pos=1)))");
    qobj = qp.parse_query("Outside", Xapian::QueryParser::FLAG_PARTIAL);
    TEST_STRINGS_EQUAL(qobj.get_description(), "Xapian::Query((outside:(pos=1) OR Zoutsid:(pos=1)))");

    // Check handling of a case with a prefix.
    qp.set_stemming_strategy(Xapian::QueryParser::STEM_SOME);
    qobj = qp.parse_query("title:cow", Xapian::QueryParser::FLAG_PARTIAL);
    TEST_STRINGS_EQUAL(qobj.get_description(), "Xapian::Query(((XTcowl:(pos=1) SYNONYM XTcows:(pos=1)) OR ZXTcow:(pos=1)))");
    qobj = qp.parse_query("title:cows", Xapian::QueryParser::FLAG_PARTIAL);
    TEST_STRINGS_EQUAL(qobj.get_description(), "Xapian::Query((XTcows:(pos=1) OR ZXTcow:(pos=1)))");
    qobj = qp.parse_query("title:Cow", Xapian::QueryParser::FLAG_PARTIAL);
    TEST_STRINGS_EQUAL(qobj.get_description(), "Xapian::Query(((XTcowl:(pos=1) SYNONYM XTcows:(pos=1)) OR XTcow:(pos=1)))");
    qobj = qp.parse_query("title:Cows", Xapian::QueryParser::FLAG_PARTIAL);
    TEST_STRINGS_EQUAL(qobj.get_description(), "Xapian::Query(XTcows:(pos=1,wqf=2))");

    // Regression test - the initial version of the multi-prefix code would
    // inflate the wqf of the "parsed as normal" version of a partial term
    // by multiplying it by the number of prefixes mapped to.
    qobj = qp.parse_query("double:vision", Xapian::QueryParser::FLAG_PARTIAL);
    TEST_STRINGS_EQUAL(qobj.get_description(), "Xapian::Query((ZXONEvision:(pos=1) SYNONYM ZXTWOvision:(pos=1)))");

    // Test handling of FLAG_PARTIAL when there's more than one prefix.
    qobj = qp.parse_query("double:part", Xapian::QueryParser::FLAG_PARTIAL);
    TEST_STRINGS_EQUAL(qobj.get_description(), "Xapian::Query(((XONEpartial:(pos=1) SYNONYM XONEpartial2:(pos=1) SYNONYM XTWOpartial3:(pos=1) SYNONYM XTWOpartial4:(pos=1)) OR (ZXONEpart:(pos=1) SYNONYM ZXTWOpart:(pos=1))))");

    // Test handling of FLAG_PARTIAL when there's more than one prefix, without
    // stemming.
    qp.set_stemming_strategy(Xapian::QueryParser::STEM_NONE);
    qobj = qp.parse_query("double:part", Xapian::QueryParser::FLAG_PARTIAL);
    TEST_STRINGS_EQUAL(qobj.get_description(), "Xapian::Query(((XONEpartial:(pos=1) SYNONYM XONEpartial2:(pos=1) SYNONYM XTWOpartial3:(pos=1) SYNONYM XTWOpartial4:(pos=1)) OR (XONEpart:(pos=1) SYNONYM XTWOpart:(pos=1))))");
    qobj = qp.parse_query("double:partial", Xapian::QueryParser::FLAG_PARTIAL);
    TEST_STRINGS_EQUAL(qobj.get_description(), "Xapian::Query(((XONEpartial:(pos=1) SYNONYM XONEpartial2:(pos=1) SYNONYM XTWOpartial3:(pos=1) SYNONYM XTWOpartial4:(pos=1)) OR (XONEpartial:(pos=1) SYNONYM XTWOpartial:(pos=1))))");

    return true;
#endif
}

static bool test_qp_flag_bool_any_case1()
{
    using Xapian::QueryParser;
    Xapian::QueryParser qp;
    Xapian::Query qobj;
    qobj = qp.parse_query("to and fro", QueryParser::FLAG_BOOLEAN | QueryParser::FLAG_BOOLEAN_ANY_CASE);
    TEST_STRINGS_EQUAL(qobj.get_description(), "Xapian::Query((to:(pos=1) AND fro:(pos=2)))");
    qobj = qp.parse_query("to and fro", QueryParser::FLAG_BOOLEAN);
    TEST_STRINGS_EQUAL(qobj.get_description(), "Xapian::Query((to:(pos=1) OR and:(pos=2) OR fro:(pos=3)))");
    // Regression test for bug in 0.9.4 and earlier.
    qobj = qp.parse_query("to And fro", QueryParser::FLAG_BOOLEAN | QueryParser::FLAG_BOOLEAN_ANY_CASE);
    TEST_STRINGS_EQUAL(qobj.get_description(), "Xapian::Query((to:(pos=1) AND fro:(pos=2)))");
    qobj = qp.parse_query("to And fro", QueryParser::FLAG_BOOLEAN);
    TEST_STRINGS_EQUAL(qobj.get_description(), "Xapian::Query((to:(pos=1) OR and:(pos=2) OR fro:(pos=3)))");
    return true;
}

static const test test_stop_queries[] = {
    { "test the queryparser", "(test:(pos=1) AND queryparser:(pos=3))" },
    // Regression test for bug in 0.9.6 and earlier.  This would fail to
    // parse.
    { "test AND the AND queryparser", "(test:(pos=1) AND the:(pos=2) AND queryparser:(pos=3))" },
    // 0.9.6 and earlier ignored a stopword even if it was the only term.
    // More recent versions don't ever treat a single term as a stopword.
    { "the", "the:(pos=1)" },
    // 1.2.2 and earlier ignored an all-stopword query with multiple terms,
    // which prevents 'to be or not to be' for being searchable unless the
    // user made it into a phrase query or prefixed all terms with '+'
    // (ticket#245).
    { "an the a", "(an:(pos=1) AND the:(pos=2) AND a:(pos=3))" },
    // Regression test for bug in initial version of the patch for the
    // "all-stopword" case.
    { "the AND a an", "(the:(pos=1) AND a:(pos=2) AND an:(pos=3))" },
    { NULL, NULL }
};

static bool test_qp_stopper1()
{
    Xapian::QueryParser qp;
    const char * stopwords[] = { "a", "an", "the" };
    Xapian::SimpleStopper stop(stopwords, stopwords + 3);
    qp.set_stopper(&stop);
    qp.set_default_op(Xapian::Query::OP_AND);
    for (const test *p = test_stop_queries; p->query; ++p) {
	string expect, parsed;
	if (p->expect)
	    expect = p->expect;
	else
	    expect = "parse error";
	try {
	    Xapian::Query qobj = qp.parse_query(p->query);
	    parsed = qobj.get_description();
	    expect = string("Xapian::Query(") + expect + ')';
	} catch (const Xapian::QueryParserError &e) {
	    parsed = e.get_msg();
	} catch (const Xapian::Error &e) {
	    parsed = e.get_description();
	} catch (...) {
	    parsed = "Unknown exception!";
	}
	tout << "Query: " << p->query << '\n';
	TEST_STRINGS_EQUAL(parsed, expect);
    }
    return true;
}

static const test test_pure_not_queries[] = {
    { "NOT windows", "(<alldocuments> AND_NOT Zwindow:(pos=1))" },
    { "a AND (NOT b)", "(Za:(pos=1) AND (<alldocuments> AND_NOT Zb:(pos=2)))" },
    { "AND NOT windows", "Syntax: <expression> AND NOT <expression>" },
    { "gordian NOT", "Syntax: <expression> NOT <expression>" },
    { "gordian AND NOT", "Syntax: <expression> AND NOT <expression>" },
    { NULL, NULL }
};

static bool test_qp_flag_pure_not1()
{
    using Xapian::QueryParser;
    Xapian::QueryParser qp;
    qp.set_stemmer(Xapian::Stem("english"));
    qp.set_stemming_strategy(QueryParser::STEM_SOME);
    for (const test *p = test_pure_not_queries; p->query; ++p) {
	string expect, parsed;
	if (p->expect)
	    expect = p->expect;
	else
	    expect = "parse error";
	try {
	    Xapian::Query qobj = qp.parse_query(p->query,
						QueryParser::FLAG_BOOLEAN |
						QueryParser::FLAG_PURE_NOT);
	    parsed = qobj.get_description();
	    expect = string("Xapian::Query(") + expect + ')';
	} catch (const Xapian::QueryParserError &e) {
	    parsed = e.get_msg();
	} catch (const Xapian::Error &e) {
	    parsed = e.get_description();
	} catch (...) {
	    parsed = "Unknown exception!";
	}
	tout << "Query: " << p->query << '\n';
	TEST_STRINGS_EQUAL(parsed, expect);
    }
    return true;
}

// Debatable if this is a regression test or a feature test, as it's not
// obvious is this was a bug fix or a new feature.  Either way, it first
// appeared in Xapian 1.0.0.
static bool test_qp_unstem_boolean_prefix()
{
    Xapian::QueryParser qp;
    qp.add_boolean_prefix("test", "XTEST");
    Xapian::Query q = qp.parse_query("hello test:foo");
    TEST_STRINGS_EQUAL(q.get_description(), "Xapian::Query((hello:(pos=1) FILTER XTESTfoo))");
    Xapian::TermIterator u = qp.unstem_begin("XTESTfoo");
    TEST(u != qp.unstem_end("XTESTfoo"));
    TEST_EQUAL(*u, "test:foo");
    ++u;
    TEST(u == qp.unstem_end("XTESTfoo"));
    return true;
}

static const test test_value_range1_queries[] = {
    { "a..b", "VALUE_RANGE 1 a b" },
    { "$50..100", "VALUE_RANGE 1 $50 100" },
    { "$50..$100", "VALUE_RANGE 1 $50 $100" },
    { "02/03/1979..10/12/1980", "VALUE_RANGE 1 02/03/1979 10/12/1980" },
    { "a..b hello", "(hello:(pos=1) FILTER VALUE_RANGE 1 a b)" },
    { "hello a..b", "(hello:(pos=1) FILTER VALUE_RANGE 1 a b)" },
    { "hello a..b world", "((hello:(pos=1) OR world:(pos=2)) FILTER VALUE_RANGE 1 a b)" },
    { "hello a..b test:foo", "(hello:(pos=1) FILTER (VALUE_RANGE 1 a b AND XTESTfoo))" },
    { "hello a..b test:foo test:bar", "(hello:(pos=1) FILTER (VALUE_RANGE 1 a b AND (XTESTfoo OR XTESTbar)))" },
    { "hello a..b c..d test:foo", "(hello:(pos=1) FILTER ((VALUE_RANGE 1 a b OR VALUE_RANGE 1 c d) AND XTESTfoo))" },
    { "hello a..b c..d test:foo test:bar", "(hello:(pos=1) FILTER ((VALUE_RANGE 1 a b OR VALUE_RANGE 1 c d) AND (XTESTfoo OR XTESTbar)))" },
    { "-5..7", "VALUE_RANGE 1 -5 7" },
    { "hello -5..7", "(hello:(pos=1) FILTER VALUE_RANGE 1 -5 7)" },
    { "-5..7 hello", "(hello:(pos=1) FILTER VALUE_RANGE 1 -5 7)" },
    { "\"time flies\" 09:00..12:30", "((time:(pos=1) PHRASE 2 flies:(pos=2)) FILTER VALUE_RANGE 1 09:00 12:30)" },
    // Feature test for single-ended ranges (ticket#480):
    { "..b", "VALUE_RANGE 1  b" },
    { "a..", "VALUE_GE 1 a" },
    { NULL, NULL }
};

// Simple test of ValueRangeProcessor class.
static bool test_qp_value_range1()
{
    Xapian::QueryParser qp;
    qp.add_boolean_prefix("test", "XTEST");
    Xapian::StringValueRangeProcessor vrp(1);
    qp.add_valuerangeprocessor(&vrp);
    for (const test *p = test_value_range1_queries; p->query; ++p) {
	string expect, parsed;
	if (p->expect)
	    expect = p->expect;
	else
	    expect = "parse error";
	try {
	    Xapian::Query qobj = qp.parse_query(p->query);
	    parsed = qobj.get_description();
	    expect = string("Xapian::Query(") + expect + ')';
	} catch (const Xapian::QueryParserError &e) {
	    parsed = e.get_msg();
	} catch (const Xapian::Error &e) {
	    parsed = e.get_description();
	} catch (...) {
	    parsed = "Unknown exception!";
	}
	tout << "Query: " << p->query << '\n';
	TEST_STRINGS_EQUAL(parsed, expect);
    }
    return true;
}

static const test test_value_range2_queries[] = {
    { "a..b", "VALUE_RANGE 3 a b" },
    { "1..12", "VALUE_RANGE 2 \240 \256" },
    { "20070201..20070228", "VALUE_RANGE 1 20070201 20070228" },
    { "$10..20", "VALUE_RANGE 4 \255 \261" },
    { "$10..$20", "VALUE_RANGE 4 \255 \261" },
    // Feature test for single-ended ranges (ticket#480):
    { "$..20", "VALUE_RANGE 4  \261" },
    { "$10..", "VALUE_GE 4 \255" },
    { "12..42kg", "VALUE_RANGE 5 \256 \265@" },
    { "12kg..42kg", "VALUE_RANGE 5 \256 \265@" },
    { "12kg..42", "VALUE_RANGE 3 12kg 42" },
    { "10..$20", "VALUE_RANGE 3 10 $20" },
    { "1999-03-12..2020-12-30", "VALUE_RANGE 1 19990312 20201230" },
    { "1999/03/12..2020/12/30", "VALUE_RANGE 1 19990312 20201230" },
    { "1999.03.12..2020.12.30", "VALUE_RANGE 1 19990312 20201230" },
    // Feature test for single-ended ranges (ticket#480):
    { "..2020.12.30", "VALUE_RANGE 1  20201230" },
    { "1999.03.12..", "VALUE_GE 1 19990312" },
    { "12/03/99..12/04/01", "VALUE_RANGE 1 19990312 20010412" },
    { "03-12-99..04-14-01", "VALUE_RANGE 1 19990312 20010414" },
    { "(test:a..test:b hello)", "(hello:(pos=1) FILTER VALUE_RANGE 3 test:a test:b)" },
    { "12..42kg 5..6kg 1..12", "0 * (VALUE_RANGE 2 \240 \256 AND (VALUE_RANGE 5 \256 \265@ OR VALUE_RANGE 5 \251 \252))" },
    // Check that a VRP which fails to match doesn't remove a prefix or suffix.
    // 1.0.13/1.1.1 and earlier got this wrong in some cases.
    { "$12a..13", "VALUE_RANGE 3 $12a 13" },
    { "$12..13b", "VALUE_RANGE 3 $12 13b" },
    { "$12..12kg", "VALUE_RANGE 3 $12 12kg" },
    { "12..b12kg", "VALUE_RANGE 3 12 b12kg" },
    { NULL, NULL }
};

// Test chaining of ValueRangeProcessor classes.
static bool test_qp_value_range2()
{
    Xapian::QueryParser qp;
    qp.add_boolean_prefix("test", "XTEST");
    Xapian::DateValueRangeProcessor vrp_date(1);
    Xapian::NumberValueRangeProcessor vrp_num(2);
    Xapian::StringValueRangeProcessor vrp_str(3);
    Xapian::NumberValueRangeProcessor vrp_cash(4, "$");
    Xapian::NumberValueRangeProcessor vrp_weight(5, "kg", false);
    qp.add_valuerangeprocessor(&vrp_date);
    qp.add_valuerangeprocessor(&vrp_num);
    qp.add_valuerangeprocessor(&vrp_cash);
    qp.add_valuerangeprocessor(&vrp_weight);
    qp.add_valuerangeprocessor(&vrp_str);
    for (const test *p = test_value_range2_queries; p->query; ++p) {
	string expect, parsed;
	if (p->expect)
	    expect = p->expect;
	else
	    expect = "parse error";
	try {
	    Xapian::Query qobj = qp.parse_query(p->query);
	    parsed = qobj.get_description();
	    expect = string("Xapian::Query(") + expect + ')';
	} catch (const Xapian::QueryParserError &e) {
	    parsed = e.get_msg();
	} catch (const Xapian::Error &e) {
	    parsed = e.get_description();
	} catch (...) {
	    parsed = "Unknown exception!";
	}
	tout << "Query: " << p->query << '\n';
	TEST_STRINGS_EQUAL(parsed, expect);
    }
    return true;
}

// Test NumberValueRangeProcessors with actual data.
static bool test_qp_value_range3()
{
#ifndef XAPIAN_HAS_INMEMORY_BACKEND
    SKIP_TEST("Testcase requires the InMemory backend which is disabled");
#else
    Xapian::WritableDatabase db(Xapian::InMemory::open());
    double low = -10;
    int steps = 60;
    double step = 0.5;

    for (int i = 0; i <= steps; ++i) {
	double v = low + i * step;
	Xapian::Document doc;
	doc.add_value(1, Xapian::sortable_serialise(v));
	db.add_document(doc);
    }

    Xapian::NumberValueRangeProcessor vrp_num(1);
    Xapian::QueryParser qp;
    qp.add_valuerangeprocessor(&vrp_num);

    for (int j = 0; j <= steps; ++j) {
	double start = low + j * step;
	for (int k = 0; k <= steps; ++k) {
	    double end = low + k * step;
	    string query = str(start) + ".." + str(end);
	    tout << "Query: " << query << '\n';
	    Xapian::Query qobj = qp.parse_query(query);
	    Xapian::Enquire enq(db);
	    enq.set_query(qobj);
	    Xapian::MSet mset = enq.get_mset(0, steps + 1);
	    if (end < start) {
		TEST_EQUAL(mset.size(), 0);
	    } else {
		TEST_EQUAL(mset.size(), 1u + (k - j));
		for (unsigned int m = 0; m != mset.size(); ++m) {
		    double v = start + m * step;
		    TEST_EQUAL(mset[m].get_document().get_value(1),
			       Xapian::sortable_serialise(v));
		}
	    }
	}
    }
    return true;
#endif
}

static const double test_value_range_numbers[] = {
#ifdef INFINITY
    -INFINITY,
#endif
    -HUGE_VAL,
    -DBL_MAX,
    -pow(2.0, 1022),
    -1024.5,
    -3.14159265358979323846,
    -3,
    -2,
    -1.8,
    -1.1,
    -1,
    -0.5,
    -0.2,
    -0.1,
    -0.000005,
    -0.000002,
    -0.000001,
    -pow(2.0, -1023),
    -pow(2.0, -1024),
    -pow(2.0, -1074),
    -DBL_MIN,
    0,
    DBL_MIN,
    pow(2.0, -1074),
    pow(2.0, -1024),
    pow(2.0, -1023),
    0.000001,
    0.000002,
    0.000005,
    0.1,
    0.2,
    0.5,
    1,
    1.1,
    1.8,
    2,
    3,
    3.14159265358979323846,
    1024.5,
    pow(2.0, 1022),
    DBL_MAX,
    HUGE_VAL,
#ifdef INFINITY
    INFINITY,
#endif

    64 // Magic number which we stop at.
};

static const test test_value_range4_queries[] = {
    { "id:19254@foo..example.com", "0 * Q19254@foo..example.com" },
    { "hello:world", "0 * XHELLOworld" },
    { "hello:mum..world", "VALUE_RANGE 1 mum world" },
    { NULL, NULL }
};

/** Test a boolean filter which happens to contain "..".
 *
 *  Regression test for bug fixed in 1.2.3.
 *
 *  Also test that the same prefix can be set for a valuerange and filter.
 */
static bool test_qp_value_range4()
{
    Xapian::QueryParser qp;
    qp.add_boolean_prefix("id", "Q");
    qp.add_boolean_prefix("hello", "XHELLO");
    Xapian::StringValueRangeProcessor vrp_str(1, "hello:");
    qp.add_valuerangeprocessor(&vrp_str);
    for (const test *p = test_value_range4_queries; p->query; ++p) {
	string expect, parsed;
	if (p->expect)
	    expect = p->expect;
	else
	    expect = "parse error";
	try {
	    Xapian::Query qobj = qp.parse_query(p->query);
	    parsed = qobj.get_description();
	    expect = string("Xapian::Query(") + expect + ')';
	} catch (const Xapian::QueryParserError &e) {
	    parsed = e.get_msg();
	} catch (const Xapian::Error &e) {
	    parsed = e.get_description();
	} catch (...) {
	    parsed = "Unknown exception!";
	}
	tout << "Query: " << p->query << '\n';
	TEST_STRINGS_EQUAL(parsed, expect);
    }
    return true;
}


// Test serialisation and unserialisation of various numbers.
static bool test_value_range_serialise1()
{
    double prevnum = 0;
    string prevstr;
    bool started = false;
    for (const double *p = test_value_range_numbers; *p != 64; ++p) {
	double num = *p;
	tout << "Number: " << num << '\n';
	string str = Xapian::sortable_serialise(num);
	tout << "String: " << str << '\n';
	TEST_EQUAL(Xapian::sortable_unserialise(str), num);

	if (started) {
	    int num_cmp = 0;
	    if (prevnum < num) {
		num_cmp = -1;
	    } else if (prevnum > num) {
		num_cmp = 1;
	    }
	    int str_cmp = 0;
	    if (prevstr < str) {
		str_cmp = -1;
	    } else if (prevstr > str) {
		str_cmp = 1;
	    }

	    TEST_AND_EXPLAIN(num_cmp == str_cmp,
			     "Numbers " << prevnum << " and " << num <<
			     " don't sort the same way as their string "
			     "counterparts");
	}

	prevnum = num;
	prevstr = str;
	started = true;
    }
    return true;
}

static const test test_value_daterange1_queries[] = {
    { "12/03/99..12/04/01", "VALUE_RANGE 1 19991203 20011204" },
    { "03-12-99..04-14-01", "VALUE_RANGE 1 19990312 20010414" },
    { "01/30/60..02/02/59", "VALUE_RANGE 1 19600130 20590202" },
    { "1999-03-12..2001-04-14", "VALUE_RANGE 1 19990312 20010414" },
    { "12/03/99..02", "Unknown range operation" },
    { "1999-03-12..2001", "Unknown range operation" },
    { NULL, NULL }
};

// Test DateValueRangeProcessor
static bool test_qp_value_daterange1()
{
    Xapian::QueryParser qp;
    Xapian::DateValueRangeProcessor vrp_date(1, true, 1960);
    qp.add_valuerangeprocessor(&vrp_date);
    for (const test *p = test_value_daterange1_queries; p->query; ++p) {
	string expect, parsed;
	if (p->expect)
	    expect = p->expect;
	else
	    expect = "parse error";
	try {
	    Xapian::Query qobj = qp.parse_query(p->query);
	    parsed = qobj.get_description();
	    expect = string("Xapian::Query(") + expect + ')';
	} catch (const Xapian::QueryParserError &e) {
	    parsed = e.get_msg();
	} catch (const Xapian::Error &e) {
	    parsed = e.get_description();
	} catch (...) {
	    parsed = "Unknown exception!";
	}
	tout << "Query: " << p->query << '\n';
	TEST_STRINGS_EQUAL(parsed, expect);
    }
    return true;
}

static const test test_value_daterange2_queries[] = {
    { "created:12/03/99..12/04/01", "VALUE_RANGE 1 19991203 20011204" },
    { "modified:03-12-99..04-14-01", "VALUE_RANGE 2 19990312 20010414" },
    { "accessed:01/30/70..02/02/69", "VALUE_RANGE 3 19700130 20690202" },
    // In <=1.2.12, and in 1.3.0, this gave "Unknown range operation":
    { "deleted:12/03/99..12/04/01", "VALUE_RANGE 4 19990312 20010412" },
    { "1999-03-12..2001-04-14", "Unknown range operation" },
    { "12/03/99..created:12/04/01", "Unknown range operation" },
    { "12/03/99created:..12/04/01", "Unknown range operation" },
    { "12/03/99..12/04/01created:", "Unknown range operation" },
    { "12/03/99..02", "Unknown range operation" },
    { "1999-03-12..2001", "Unknown range operation" },
    { NULL, NULL }
};

// Feature test DateValueRangeProcessor with prefixes (added in 1.1.2).
static bool test_qp_value_daterange2()
{
    Xapian::QueryParser qp;
    Xapian::DateValueRangeProcessor vrp_cdate(1, "created:", true, true, 1970);
    Xapian::DateValueRangeProcessor vrp_mdate(2, "modified:", true, true, 1970);
    Xapian::DateValueRangeProcessor vrp_adate(3, "accessed:", true, true, 1970);
    // Regression test - here a const char * was taken as a bool rather than a
    // std::string when resolving the overloaded forms.  Fixed in 1.2.13 and
    // 1.3.1.
    Xapian::DateValueRangeProcessor vrp_ddate(4, "deleted:");
    qp.add_valuerangeprocessor(&vrp_cdate);
    qp.add_valuerangeprocessor(&vrp_mdate);
    qp.add_valuerangeprocessor(&vrp_adate);
    qp.add_valuerangeprocessor(&vrp_ddate);
    for (const test *p = test_value_daterange2_queries; p->query; ++p) {
	string expect, parsed;
	if (p->expect)
	    expect = p->expect;
	else
	    expect = "parse error";
	try {
	    Xapian::Query qobj = qp.parse_query(p->query);
	    parsed = qobj.get_description();
	    expect = string("Xapian::Query(") + expect + ')';
	} catch (const Xapian::QueryParserError &e) {
	    parsed = e.get_msg();
	} catch (const Xapian::Error &e) {
	    parsed = e.get_description();
	} catch (...) {
	    parsed = "Unknown exception!";
	}
	tout << "Query: " << p->query << '\n';
	TEST_STRINGS_EQUAL(parsed, expect);
    }
    return true;
}

static const test test_value_stringrange1_queries[] = {
    { "tag:bar..foo", "VALUE_RANGE 1 bar foo" },
    { "bar..foo", "VALUE_RANGE 0 bar foo" },
    { NULL, NULL }
};

// Feature test StringValueRangeProcessor with prefixes (added in 1.1.2).
static bool test_qp_value_stringrange1()
{
    Xapian::QueryParser qp;
    Xapian::StringValueRangeProcessor vrp_default(0);
    Xapian::StringValueRangeProcessor vrp_tag(1, "tag:", true);
    qp.add_valuerangeprocessor(&vrp_tag);
    qp.add_valuerangeprocessor(&vrp_default);
    for (const test *p = test_value_stringrange1_queries; p->query; ++p) {
	string expect, parsed;
	if (p->expect)
	    expect = p->expect;
	else
	    expect = "parse error";
	try {
	    Xapian::Query qobj = qp.parse_query(p->query);
	    parsed = qobj.get_description();
	    expect = string("Xapian::Query(") + expect + ')';
	} catch (const Xapian::QueryParserError &e) {
	    parsed = e.get_msg();
	} catch (const Xapian::Error &e) {
	    parsed = e.get_description();
	} catch (...) {
	    parsed = "Unknown exception!";
	}
	tout << "Query: " << p->query << '\n';
	TEST_STRINGS_EQUAL(parsed, expect);
    }
    return true;
}

struct AuthorValueRangeProcessor : public Xapian::ValueRangeProcessor {
    AuthorValueRangeProcessor() {}

    Xapian::valueno operator()(std::string &begin, std::string &end) {
	if (!startswith(begin, "author:"))
	    return Xapian::BAD_VALUENO;
	begin.erase(0, 7);
	begin = Xapian::Unicode::tolower(begin);
	end = Xapian::Unicode::tolower(end);
	return 4;
    }
};

static const test test_value_customrange1_queries[] = {
    { "mars author:Asimov..Bradbury", "(mars:(pos=1) FILTER VALUE_RANGE 4 asimov bradbury)" },
    { NULL, NULL }
};

// Test custom ValueRangeProcessor subclass.
static bool test_qp_value_customrange1()
{
    Xapian::QueryParser qp;
    AuthorValueRangeProcessor vrp_author;
    qp.add_valuerangeprocessor(&vrp_author);
    for (const test *p = test_value_customrange1_queries; p->query; ++p) {
	string expect, parsed;
	if (p->expect)
	    expect = p->expect;
	else
	    expect = "parse error";
	try {
	    Xapian::Query qobj = qp.parse_query(p->query);
	    parsed = qobj.get_description();
	    expect = string("Xapian::Query(") + expect + ')';
	} catch (const Xapian::QueryParserError &e) {
	    parsed = e.get_msg();
	} catch (const Xapian::Error &e) {
	    parsed = e.get_description();
	} catch (...) {
	    parsed = "Unknown exception!";
	}
	tout << "Query: " << p->query << '\n';
	TEST_STRINGS_EQUAL(parsed, expect);
    }
    return true;
}

static bool test_qp_stoplist1()
{
    Xapian::QueryParser qp;
    const char * stopwords[] = { "a", "an", "the" };
    Xapian::SimpleStopper stop(stopwords, stopwords + 3);
    qp.set_stopper(&stop);

    Xapian::TermIterator i;

    Xapian::Query query1 = qp.parse_query("some mice");
    i = qp.stoplist_begin();
    TEST(i == qp.stoplist_end());

    Xapian::Query query2 = qp.parse_query("the cat");
    i = qp.stoplist_begin();
    TEST(i != qp.stoplist_end());
    TEST_EQUAL(*i, "the");
    ++i;
    TEST(i == qp.stoplist_end());

    // Regression test - prior to Xapian 1.0.0 the stoplist wasn't being cleared
    // when a new query was parsed.
    Xapian::Query query3 = qp.parse_query("an aardvark");
    i = qp.stoplist_begin();
    TEST(i != qp.stoplist_end());
    TEST_EQUAL(*i, "an");
    ++i;
    TEST(i == qp.stoplist_end());

    return true;
}

static const test test_mispelled_queries[] = {
    { "doucment search", "document search" },
    { "doucment seeacrh", "document search" },
    { "docment seeacrh test", "document search test" },
    { "\"paragahp pineapple\"", "\"paragraph pineapple\"" },
    { "\"paragahp pineapple\"", "\"paragraph pineapple\"" },
    { "test S.E.A.R.C.", "" },
    { "this AND that", "" },
    { "documento", "document" },
    { "documento-documento", "document-document" },
    { "documento-searcho", "document-search" },
    { "test saerch", "test search" },
    { "paragraf search", "paragraph search" },
    { NULL, NULL }
};

// Test spelling correction in the QueryParser.
static bool test_qp_spell1()
{
    mkdir(".flint", 0755);
    string dbdir = ".flint/qp_spell1";
    Xapian::WritableDatabase db(dbdir, Xapian::DB_CREATE_OR_OVERWRITE);

    Xapian::Document doc;
    doc.add_term("document", 6);
    doc.add_term("search", 7);
    doc.add_term("saerch", 1);
    doc.add_term("paragraph", 8);
    doc.add_term("paragraf", 2);
    db.add_document(doc);

    db.add_spelling("document");
    db.add_spelling("search");
    db.add_spelling("paragraph");
    db.add_spelling("band");

    Xapian::QueryParser qp;
    qp.set_stemming_strategy(Xapian::QueryParser::STEM_SOME);
    qp.set_database(db);

    for (const test *p = test_mispelled_queries; p->query; ++p) {
	Xapian::Query q;
	q = qp.parse_query(p->query,
			   Xapian::QueryParser::FLAG_SPELLING_CORRECTION |
			   Xapian::QueryParser::FLAG_BOOLEAN );
	tout << "Query: " << p->query << endl;
	TEST_STRINGS_EQUAL(qp.get_corrected_query_string(), p->expect);
    }

    return true;
}

// Test spelling correction in the QueryParser with multiple databases.
static bool test_qp_spell2()
{
    mkdir(".flint", 0755);
    string dbdir = ".flint/qp_spell2";
    Xapian::WritableDatabase db1(dbdir, Xapian::DB_CREATE_OR_OVERWRITE);

    db1.add_spelling("document");
    db1.add_spelling("search");
    db1.commit();

    dbdir = ".flint/qp_spell2b";
    Xapian::WritableDatabase db2(dbdir, Xapian::DB_CREATE_OR_OVERWRITE);

    db2.add_spelling("document");
    db2.add_spelling("paragraph");
    db2.add_spelling("band");

    Xapian::Database db;
    db.add_database(db1);
    db.add_database(db2);

    Xapian::QueryParser qp;
    qp.set_stemming_strategy(Xapian::QueryParser::STEM_SOME);
    qp.set_database(db);

    for (const test *p = test_mispelled_queries; p->query; ++p) {
	Xapian::Query q;
	q = qp.parse_query(p->query,
			   Xapian::QueryParser::FLAG_SPELLING_CORRECTION |
			   Xapian::QueryParser::FLAG_BOOLEAN );
	tout << "Query: " << p->query << endl;
	TEST_STRINGS_EQUAL(qp.get_corrected_query_string(), p->expect);
    }

    return true;
}

static const test test_mispelled_wildcard_queries[] = {
    { "doucment", "document" },
    { "doucment*", "" },
    { "doucment* seearch", "doucment* search" },
    { "doucment* search", "" },
    { NULL, NULL }
};

// Test spelling correction in the QueryParser with wildcards.
// Regression test for bug fixed in 1.1.3 and 1.0.17.
static bool test_qp_spellwild1()
{
    mkdir(".flint", 0755);
    string dbdir = ".flint/qp_spellwild1";
    Xapian::WritableDatabase db(dbdir, Xapian::DB_CREATE_OR_OVERWRITE);

    db.add_spelling("document");
    db.add_spelling("search");
    db.add_spelling("paragraph");
    db.add_spelling("band");

    Xapian::QueryParser qp;
    qp.set_database(db);

    const test *p;
    for (p = test_mispelled_queries; p->query; ++p) {
	Xapian::Query q;
	q = qp.parse_query(p->query,
			   Xapian::QueryParser::FLAG_SPELLING_CORRECTION |
			   Xapian::QueryParser::FLAG_BOOLEAN |
			   Xapian::QueryParser::FLAG_WILDCARD);
	tout << "Query: " << p->query << endl;
	TEST_STRINGS_EQUAL(qp.get_corrected_query_string(), p->expect);
    }
    for (p = test_mispelled_wildcard_queries; p->query; ++p) {
	Xapian::Query q;
	q = qp.parse_query(p->query,
			   Xapian::QueryParser::FLAG_SPELLING_CORRECTION |
			   Xapian::QueryParser::FLAG_BOOLEAN |
			   Xapian::QueryParser::FLAG_WILDCARD);
	tout << "Query: " << p->query << endl;
	TEST_STRINGS_EQUAL(qp.get_corrected_query_string(), p->expect);
    }

    return true;
}

static const test test_mispelled_partial_queries[] = {
    { "doucment", "" },
    { "doucment ", "document " },
    { "documen", "" },
    { "documen ", "document " },
    { "seearch documen", "search documen" },
    { "search documen", "" },
    { NULL, NULL }
};

// Test spelling correction in the QueryParser with FLAG_PARTIAL.
// Regression test for bug fixed in 1.1.3 and 1.0.17.
static bool test_qp_spellpartial1()
{
    mkdir(".flint", 0755);
    string dbdir = ".flint/qp_spellpartial1";
    Xapian::WritableDatabase db(dbdir, Xapian::DB_CREATE_OR_OVERWRITE);

    db.add_spelling("document");
    db.add_spelling("search");
    db.add_spelling("paragraph");
    db.add_spelling("band");

    Xapian::QueryParser qp;
    qp.set_database(db);

    for (const test *p = test_mispelled_partial_queries; p->query; ++p) {
	Xapian::Query q;
	q = qp.parse_query(p->query,
			   Xapian::QueryParser::FLAG_SPELLING_CORRECTION |
			   Xapian::QueryParser::FLAG_PARTIAL);
	tout << "Query: " << p->query << endl;
	TEST_STRINGS_EQUAL(qp.get_corrected_query_string(), p->expect);
    }

    return true;
}

static const test test_synonym_queries[] = {
    { "searching", "(Zsearch:(pos=1) SYNONYM Zfind:(pos=1) SYNONYM Zlocate:(pos=1))" },
    { "search", "(Zsearch:(pos=1) SYNONYM find:(pos=1))" },
    { "Search", "(search:(pos=1) SYNONYM find:(pos=1))" },
    { "Searching", "searching:(pos=1)" },
    { "searching OR terms", "((Zsearch:(pos=1) SYNONYM Zfind:(pos=1) SYNONYM Zlocate:(pos=1)) OR Zterm:(pos=2))" },
    { "search OR terms", "((Zsearch:(pos=1) SYNONYM find:(pos=1)) OR Zterm:(pos=2))" },
    { "search +terms", "(Zterm:(pos=2) AND_MAYBE (Zsearch:(pos=1) SYNONYM find:(pos=1)))" },
    { "search -terms", "((Zsearch:(pos=1) SYNONYM find:(pos=1)) AND_NOT Zterm:(pos=2))" },
    { "+search terms", "((Zsearch:(pos=1) SYNONYM find:(pos=1)) AND_MAYBE Zterm:(pos=2))" },
    { "-search terms", "(Zterm:(pos=2) AND_NOT (Zsearch:(pos=1) SYNONYM find:(pos=1)))" },
    { "search terms", "((Zsearch:(pos=1) SYNONYM find:(pos=1)) OR Zterm:(pos=2))" },
    // Shouldn't trigger synonyms:
    { "\"search terms\"", "(search:(pos=1) PHRASE 2 terms:(pos=2))" },
    // Check that setting FLAG_AUTO_SYNONYMS doesn't enable multi-word
    // synonyms.  Regression test for bug fixed in 1.3.0 and 1.2.9.
    { "regression test", "(Zregress:(pos=1) OR Ztest:(pos=2))" },
    { NULL, NULL }
};

// Test single term synonyms in the QueryParser.
static bool test_qp_synonym1()
{
    mkdir(".flint", 0755);
    string dbdir = ".flint/qp_synonym1";
    Xapian::WritableDatabase db(dbdir, Xapian::DB_CREATE_OR_OVERWRITE);

    db.add_synonym("Zsearch", "Zfind");
    db.add_synonym("Zsearch", "Zlocate");
    db.add_synonym("search", "find");
    db.add_synonym("Zseek", "Zsearch");
    db.add_synonym("regression test", "magic");

    db.commit();

    Xapian::QueryParser qp;
    qp.set_stemmer(Xapian::Stem("english"));
    qp.set_stemming_strategy(Xapian::QueryParser::STEM_SOME);
    qp.set_database(db);

    for (const test *p = test_synonym_queries; p->query; ++p) {
	string expect = "Xapian::Query(";
	expect += p->expect;
	expect += ')';
	Xapian::Query q;
	q = qp.parse_query(p->query, qp.FLAG_AUTO_SYNONYMS|qp.FLAG_DEFAULT);
	tout << "Query: " << p->query << endl;
	TEST_STRINGS_EQUAL(q.get_description(), expect);
    }

    return true;
}

static const test test_multi_synonym_queries[] = {
    { "sun OR tan OR cream", "(Zsun:(pos=1) OR Ztan:(pos=2) OR Zcream:(pos=3))" },
    { "sun tan", "((Zsun:(pos=1) OR Ztan:(pos=2)) SYNONYM bathe:(pos=1))" },
    { "sun tan cream", "((Zsun:(pos=1) OR Ztan:(pos=2) OR Zcream:(pos=3)) SYNONYM lotion:(pos=1))" },
    { "beach sun tan holiday", "(Zbeach:(pos=1) OR ((Zsun:(pos=2) OR Ztan:(pos=3)) SYNONYM bathe:(pos=2)) OR Zholiday:(pos=4))" },
    { "sun tan sun tan cream", "(((Zsun:(pos=1) OR Ztan:(pos=2)) SYNONYM bathe:(pos=1)) OR ((Zsun:(pos=3) OR Ztan:(pos=4) OR Zcream:(pos=5)) SYNONYM lotion:(pos=3)))" },
    { "single", "(Zsingl:(pos=1) SYNONYM record:(pos=1))" },
    { NULL, NULL }
};

// Test multi term synonyms in the QueryParser.
static bool test_qp_synonym2()
{
    mkdir(".flint", 0755);
    string dbdir = ".flint/qp_synonym2";
    Xapian::WritableDatabase db(dbdir, Xapian::DB_CREATE_OR_OVERWRITE);

    db.add_synonym("sun tan cream", "lotion");
    db.add_synonym("sun tan", "bathe");
    db.add_synonym("single", "record");

    db.commit();

    Xapian::QueryParser qp;
    qp.set_stemmer(Xapian::Stem("english"));
    qp.set_stemming_strategy(Xapian::QueryParser::STEM_SOME);
    qp.set_database(db);

    for (const test *p = test_multi_synonym_queries; p->query; ++p) {
	string expect = "Xapian::Query(";
	expect += p->expect;
	expect += ')';
	Xapian::Query q;
	q = qp.parse_query(p->query,
			   Xapian::QueryParser::FLAG_AUTO_MULTIWORD_SYNONYMS |
			   Xapian::QueryParser::FLAG_DEFAULT);
	tout << "Query: " << p->query << endl;
	TEST_STRINGS_EQUAL(q.get_description(), expect);
    }

    return true;
}

static const test test_synonym_op_queries[] = {
    { "searching", "Zsearch:(pos=1)" },
    { "~searching", "(Zsearch:(pos=1) SYNONYM Zfind:(pos=1) SYNONYM Zlocate:(pos=1))" },
    { "~search", "(Zsearch:(pos=1) SYNONYM find:(pos=1))" },
    { "~Search", "(search:(pos=1) SYNONYM find:(pos=1))" },
    { "~Searching", "searching:(pos=1)" },
    { "~searching OR terms", "((Zsearch:(pos=1) SYNONYM Zfind:(pos=1) SYNONYM Zlocate:(pos=1)) OR Zterm:(pos=2))" },
    { "~search OR terms", "((Zsearch:(pos=1) SYNONYM find:(pos=1)) OR Zterm:(pos=2))" },
    { "~search +terms", "(Zterm:(pos=2) AND_MAYBE (Zsearch:(pos=1) SYNONYM find:(pos=1)))" },
    { "~search -terms", "((Zsearch:(pos=1) SYNONYM find:(pos=1)) AND_NOT Zterm:(pos=2))" },
    { "+~search terms", "((Zsearch:(pos=1) SYNONYM find:(pos=1)) AND_MAYBE Zterm:(pos=2))" },
    { "-~search terms", "(Zterm:(pos=2) AND_NOT (Zsearch:(pos=1) SYNONYM find:(pos=1)))" },
    { "~search terms", "((Zsearch:(pos=1) SYNONYM find:(pos=1)) OR Zterm:(pos=2))" },
    { "~foo:search", "(ZXFOOsearch:(pos=1) SYNONYM prefixated:(pos=1))" },
    // FIXME: should look for multi-term synonym...
    { "~\"search terms\"", "(search:(pos=1) PHRASE 2 terms:(pos=2))" },
    { NULL, NULL }
};

// Test the synonym operator in the QueryParser.
static bool test_qp_synonym3()
{
    mkdir(".flint", 0755);
    string dbdir = ".flint/qp_synonym3";
    Xapian::WritableDatabase db(dbdir, Xapian::DB_CREATE_OR_OVERWRITE);

    db.add_synonym("Zsearch", "Zfind");
    db.add_synonym("Zsearch", "Zlocate");
    db.add_synonym("search", "find");
    db.add_synonym("Zseek", "Zsearch");
    db.add_synonym("ZXFOOsearch", "prefixated");

    db.commit();

    Xapian::QueryParser qp;
    qp.set_stemmer(Xapian::Stem("english"));
    qp.set_stemming_strategy(Xapian::QueryParser::STEM_SOME);
    qp.set_database(db);
    qp.add_prefix("foo", "XFOO");

    for (const test *p = test_synonym_op_queries; p->query; ++p) {
	string expect = "Xapian::Query(";
	expect += p->expect;
	expect += ')';
	Xapian::Query q;
	q = qp.parse_query(p->query,
			   Xapian::QueryParser::FLAG_SYNONYM |
			   Xapian::QueryParser::FLAG_BOOLEAN |
			   Xapian::QueryParser::FLAG_LOVEHATE |
			   Xapian::QueryParser::FLAG_PHRASE );
	tout << "Query: " << p->query << endl;
	TEST_STRINGS_EQUAL(q.get_description(), expect);
    }

    return true;
}

static const test test_stem_all_queries[] = {
    { "\"chemical engineers\"", "(chemic:(pos=1) PHRASE 2 engin:(pos=2))" },
    { "chemical NEAR engineers", "(chemic:(pos=1) NEAR 11 engin:(pos=2))" },
    { "chemical engineers", "(chemic:(pos=1) OR engin:(pos=2))" },
    { "title:(chemical engineers)", "(XTchemic:(pos=1) OR XTengin:(pos=2))" },
    { NULL, NULL }
};

static bool test_qp_stem_all1()
{
    Xapian::QueryParser qp;
    qp.set_stemmer(Xapian::Stem("english"));
    qp.set_stemming_strategy(qp.STEM_ALL);
    qp.add_prefix("title", "XT");
    for (const test *p = test_stem_all_queries; p->query; ++p) {
	string expect, parsed;
	if (p->expect)
	    expect = p->expect;
	else
	    expect = "parse error";
	try {
	    Xapian::Query qobj = qp.parse_query(p->query);
	    parsed = qobj.get_description();
	    expect = string("Xapian::Query(") + expect + ')';
	} catch (const Xapian::QueryParserError &e) {
	    parsed = e.get_msg();
	} catch (const Xapian::Error &e) {
	    parsed = e.get_description();
	} catch (...) {
	    parsed = "Unknown exception!";
	}
	tout << "Query: " << p->query << '\n';
	TEST_STRINGS_EQUAL(parsed, expect);
    }
    return true;
}

static const test test_stem_all_z_queries[] = {
    { "\"chemical engineers\"", "(Zchemic:(pos=1) PHRASE 2 Zengin:(pos=2))" },
    { "chemical NEAR engineers", "(Zchemic:(pos=1) NEAR 11 Zengin:(pos=2))" },
    { "chemical engineers", "(Zchemic:(pos=1) OR Zengin:(pos=2))" },
    { "title:(chemical engineers)", "(ZXTchemic:(pos=1) OR ZXTengin:(pos=2))" },
    { NULL, NULL }
};

static bool test_qp_stem_all_z1()
{
    Xapian::QueryParser qp;
    qp.set_stemmer(Xapian::Stem("english"));
    qp.set_stemming_strategy(qp.STEM_ALL_Z);
    qp.add_prefix("title", "XT");
    for (const test *p = test_stem_all_z_queries; p->query; ++p) {
	string expect, parsed;
	if (p->expect)
	    expect = p->expect;
	else
	    expect = "parse error";
	try {
	    Xapian::Query qobj = qp.parse_query(p->query);
	    parsed = qobj.get_description();
	    expect = string("Xapian::Query(") + expect + ')';
	} catch (const Xapian::QueryParserError &e) {
	    parsed = e.get_msg();
	} catch (const Xapian::Error &e) {
	    parsed = e.get_description();
	} catch (...) {
	    parsed = "Unknown exception!";
	}
	tout << "Query: " << p->query << '\n';
	TEST_STRINGS_EQUAL(parsed, expect);
    }
    return true;
}

static double
time_query_parse(const Xapian::Database & db, const string & q,
		 int repetitions, unsigned flags)
{
    Xapian::QueryParser qp;
    qp.set_database(db);
    CPUTimer timer;
    std::vector<Xapian::Query> qs;
    qs.reserve(repetitions);
    for (int i = 0; i != repetitions; ++i) {
	qs.push_back(qp.parse_query(q, flags));
    }
    if (repetitions > 1) {
	Xapian::Query qc(Xapian::Query::OP_OR, qs.begin(), qs.end());
    }
    return timer.get_time();
}

static void
qp_scale1_helper(const Xapian::Database &db, const string & q, unsigned n,
		 unsigned flags)
{
    double time1;
    while (true) {
	time1 = time_query_parse(db, q, n, flags);
	if (time1 != 0.0) break;

	// The first test completed before the timer ticked at all, so increase
	// the number of repetitions and retry.
	unsigned n_new = n * 10;
	if (n_new < n)
	    SKIP_TEST("Can't count enough repetitions to be able to time test");
	n = n_new;
    }

    n /= 5;

    string q_n;
    q_n.reserve(q.size() * n);
    for (unsigned i = n; i != 0; --i) {
	q_n += q;
    }

    // Time 5 repetitions so we average random variations a bit.
    double time2 = time_query_parse(db, q_n, 5, flags);
    tout << "small=" << time1 << "s, large=" << time2 << "s\n";

    // Allow a factor of 2.15 difference, to cover random variation and a
    // native time interval which isn't an exact multiple of 1/CLK_TCK.
    TEST_REL(time2,<,time1 * 2.15);
}

// Regression test: check that query parser doesn't scale very badly with the
// size of the query.
static bool test_qp_scale1()
{
    mkdir(".flint", 0755);
    string dbdir = ".flint/qp_scale1";
    Xapian::WritableDatabase db(dbdir, Xapian::DB_CREATE_OR_OVERWRITE);

    db.add_synonym("foo", "bar");
    db.commit();

    string q1("foo ");
    string q1b("baz ");
    const unsigned repetitions = 5000;

    // A long multiword synonym.
    string syn;
    for (int j = 60; j != 0; --j) {
	syn += q1;
    }
    syn.resize(syn.size() - 1);

    unsigned synflags = Xapian::QueryParser::FLAG_DEFAULT |
	    Xapian::QueryParser::FLAG_SYNONYM |
	    Xapian::QueryParser::FLAG_AUTO_MULTIWORD_SYNONYMS;

    // First, we test a simple query.
    qp_scale1_helper(db, q1, repetitions, Xapian::QueryParser::FLAG_DEFAULT);

    // If synonyms are enabled, a different code-path is followed.
    // Test a query which has no synonyms.
    qp_scale1_helper(db, q1b, repetitions, synflags);

    // Test a query which has short synonyms.
    qp_scale1_helper(db, q1, repetitions, synflags);

    // Add a synonym for the whole query, to test that code path.
    db.add_synonym(syn, "bar");
    db.commit();

    qp_scale1_helper(db, q1, repetitions, synflags);

    return true;
}

static const test test_near_queries[] = {
    { "simple-example", "(simple:(pos=1) PHRASE 2 example:(pos=2))" },
    { "stock -cooking", "(Zstock:(pos=1) AND_NOT Zcook:(pos=2))" },
// FIXME: these give NEAR 2
//    { "foo -baz bar", "((foo:(pos=1) NEAR 11 bar:(pos=3)) AND_NOT Zbaz:(pos=2))" },
//    { "one +two three", "(Ztwo:(pos=2) AND_MAYBE (one:(pos=1) NEAR 11 three:(pos=3)))" },
    { "foo bar", "(foo:(pos=1) NEAR 11 bar:(pos=2))" },
    { "foo bar baz", "(foo:(pos=1) NEAR 12 bar:(pos=2) NEAR 12 baz:(pos=3))" },
    { "gtk+ -gnome", "(Zgtk+:(pos=1) AND_NOT Zgnome:(pos=2))" },
    { "c++ -d--", "(Zc++:(pos=1) AND_NOT Zd:(pos=2))" },
    { "\"c++ library\"", "(c++:(pos=1) PHRASE 2 library:(pos=2))" },
    { "author:orwell animal farm", "(Aorwell:(pos=1) NEAR 12 animal:(pos=2) NEAR 12 farm:(pos=3))" },
    { "author:Orwell Animal Farm", "(Aorwell:(pos=1) NEAR 12 animal:(pos=2) NEAR 12 farm:(pos=3))" },
    { "beer NOT \"orange juice\"", "(Zbeer:(pos=1) AND_NOT (orange:(pos=2) PHRASE 2 juice:(pos=3)))" },
    { "beer AND NOT lager", "(Zbeer:(pos=1) AND_NOT Zlager:(pos=2))" },
    { "A OR B NOT C", "(a:(pos=1) OR (b:(pos=2) AND_NOT c:(pos=3)))" },
    { "A OR B AND NOT C", "(a:(pos=1) OR (b:(pos=2) AND_NOT c:(pos=3)))" },
    { "A OR B XOR C", "(a:(pos=1) OR (b:(pos=2) XOR c:(pos=3)))" },
    { "A XOR B NOT C", "(a:(pos=1) XOR (b:(pos=2) AND_NOT c:(pos=3)))" },
    { "one AND two", "(Zone:(pos=1) AND Ztwo:(pos=2))" },
    { "NOT windows", "Syntax: <expression> NOT <expression>" },
    { "a AND (NOT b)", "Syntax: <expression> NOT <expression>" },
    { "AND NOT windows", "Syntax: <expression> AND NOT <expression>" },
    { "gordian NOT", "Syntax: <expression> NOT <expression>" },
    { "gordian AND NOT", "Syntax: <expression> AND NOT <expression>" },
    { "foo OR (something AND)", "Syntax: <expression> AND <expression>" },
    { "OR foo", "Syntax: <expression> OR <expression>" },
    { "XOR", "Syntax: <expression> XOR <expression>" },
    { "hard\xa0space", "(hard:(pos=1) NEAR 11 space:(pos=2))" },
    { NULL, NULL }
};

static bool test_qp_near1()
{
    Xapian::QueryParser queryparser;
    queryparser.set_stemmer(Xapian::Stem("english"));
    queryparser.set_stemming_strategy(Xapian::QueryParser::STEM_SOME);
    queryparser.add_prefix("author", "A");
    queryparser.add_prefix("writer", "A");
    queryparser.add_prefix("title", "XT");
    queryparser.add_prefix("subject", "XT");
    queryparser.add_prefix("authortitle", "A");
    queryparser.add_prefix("authortitle", "XT");
    queryparser.add_boolean_prefix("site", "H");
    queryparser.add_boolean_prefix("site2", "J");
    queryparser.add_boolean_prefix("multisite", "H");
    queryparser.add_boolean_prefix("multisite", "J");
    queryparser.add_boolean_prefix("category", "XCAT", false);
    queryparser.set_default_op(Xapian::Query::OP_NEAR);
    for (const test *p = test_near_queries; p->query; ++p) {
	string expect, parsed;
	if (p->expect)
	    expect = p->expect;
	else
	    expect = "parse error";
	try {
	    Xapian::Query qobj = queryparser.parse_query(p->query);
	    parsed = qobj.get_description();
	    expect = string("Xapian::Query(") + expect + ')';
	} catch (const Xapian::QueryParserError &e) {
	    parsed = e.get_msg();
	} catch (const Xapian::Error &e) {
	    parsed = e.get_description();
	} catch (...) {
	    parsed = "Unknown exception!";
	}
	tout << "Query: " << p->query << '\n';
	TEST_STRINGS_EQUAL(parsed, expect);
    }
    return true;
}

static const test test_phrase_queries[] = {
    { "simple-example", "(simple:(pos=1) PHRASE 2 example:(pos=2))" },
    { "stock -cooking", "(Zstock:(pos=1) AND_NOT Zcook:(pos=2))" },
// FIXME: these give PHRASE 2
//    { "foo -baz bar", "((foo:(pos=1) PHRASE 11 bar:(pos=3)) AND_NOT Zbaz:(pos=2))" },
//    { "one +two three", "(Ztwo:(pos=2) AND_MAYBE (one:(pos=1) PHRASE 11 three:(pos=3)))" },
    { "foo bar", "(foo:(pos=1) PHRASE 11 bar:(pos=2))" },
    { "foo bar baz", "(foo:(pos=1) PHRASE 12 bar:(pos=2) PHRASE 12 baz:(pos=3))" },
    { "gtk+ -gnome", "(Zgtk+:(pos=1) AND_NOT Zgnome:(pos=2))" },
    { "c++ -d--", "(Zc++:(pos=1) AND_NOT Zd:(pos=2))" },
    { "\"c++ library\"", "(c++:(pos=1) PHRASE 2 library:(pos=2))" },
    { "author:orwell animal farm", "(Aorwell:(pos=1) PHRASE 12 animal:(pos=2) PHRASE 12 farm:(pos=3))" },
    { "author:Orwell Animal Farm", "(Aorwell:(pos=1) PHRASE 12 animal:(pos=2) PHRASE 12 farm:(pos=3))" },
    { "beer NOT \"orange juice\"", "(Zbeer:(pos=1) AND_NOT (orange:(pos=2) PHRASE 2 juice:(pos=3)))" },
    { "beer AND NOT lager", "(Zbeer:(pos=1) AND_NOT Zlager:(pos=2))" },
    { "A OR B NOT C", "(a:(pos=1) OR (b:(pos=2) AND_NOT c:(pos=3)))" },
    { "A OR B AND NOT C", "(a:(pos=1) OR (b:(pos=2) AND_NOT c:(pos=3)))" },
    { "A OR B XOR C", "(a:(pos=1) OR (b:(pos=2) XOR c:(pos=3)))" },
    { "A XOR B NOT C", "(a:(pos=1) XOR (b:(pos=2) AND_NOT c:(pos=3)))" },
    { "one AND two", "(Zone:(pos=1) AND Ztwo:(pos=2))" },
    { "NOT windows", "Syntax: <expression> NOT <expression>" },
    { "a AND (NOT b)", "Syntax: <expression> NOT <expression>" },
    { "AND NOT windows", "Syntax: <expression> AND NOT <expression>" },
    { "gordian NOT", "Syntax: <expression> NOT <expression>" },
    { "gordian AND NOT", "Syntax: <expression> AND NOT <expression>" },
    { "foo OR (something AND)", "Syntax: <expression> AND <expression>" },
    { "OR foo", "Syntax: <expression> OR <expression>" },
    { "XOR", "Syntax: <expression> XOR <expression>" },
    { "hard\xa0space", "(hard:(pos=1) PHRASE 11 space:(pos=2))" },
    // FIXME: this isn't what we want, but fixing phrase to work with
    // subqueries first might be the best approach.
    { "(one AND two) three", "((Zone:(pos=1) PHRASE 11 Zthree:(pos=3)) AND (Ztwo:(pos=2) PHRASE 11 Zthree:(pos=3)))" },
    { NULL, NULL }
};

static bool test_qp_phrase1()
{
    Xapian::QueryParser queryparser;
    queryparser.set_stemmer(Xapian::Stem("english"));
    queryparser.set_stemming_strategy(Xapian::QueryParser::STEM_SOME);
    queryparser.add_prefix("author", "A");
    queryparser.add_prefix("writer", "A");
    queryparser.add_prefix("title", "XT");
    queryparser.add_prefix("subject", "XT");
    queryparser.add_prefix("authortitle", "A");
    queryparser.add_prefix("authortitle", "XT");
    queryparser.add_boolean_prefix("site", "H");
    queryparser.add_boolean_prefix("site2", "J");
    queryparser.add_boolean_prefix("multisite", "H");
    queryparser.add_boolean_prefix("multisite", "J");
    queryparser.add_boolean_prefix("category", "XCAT", false);
    queryparser.set_default_op(Xapian::Query::OP_PHRASE);
    for (const test *p = test_phrase_queries; p->query; ++p) {
	string expect, parsed;
	if (p->expect)
	    expect = p->expect;
	else
	    expect = "parse error";
	try {
	    Xapian::Query qobj = queryparser.parse_query(p->query);
	    parsed = qobj.get_description();
	    expect = string("Xapian::Query(") + expect + ')';
	} catch (const Xapian::QueryParserError &e) {
	    parsed = e.get_msg();
	} catch (const Xapian::Error &e) {
	    parsed = e.get_description();
	} catch (...) {
	    parsed = "Unknown exception!";
	}
	tout << "Query: " << p->query << '\n';
	TEST_STRINGS_EQUAL(parsed, expect);
    }
    return true;
}

static const test test_stopword_group_or_queries[] = {
    { "this is a test", "test:(pos=4)" },
    { "test*", "(test:(pos=1) SYNONYM testable:(pos=1) SYNONYM tester:(pos=1))" },
    { "a test*", "(test:(pos=2) SYNONYM testable:(pos=2) SYNONYM tester:(pos=2))" },
    { "is a test*", "(test:(pos=3) SYNONYM testable:(pos=3) SYNONYM tester:(pos=3))" },
    { "this is a test*", "(test:(pos=4) SYNONYM testable:(pos=4) SYNONYM tester:(pos=4))" },
    { "this is a us* test*", "(user:(pos=4) OR (test:(pos=5) SYNONYM testable:(pos=5) SYNONYM tester:(pos=5)))" },
    { "this is a user test*", "(user:(pos=4) OR (test:(pos=5) SYNONYM testable:(pos=5) SYNONYM tester:(pos=5)))" },
    { NULL, NULL }
};

static const test test_stopword_group_and_queries[] = {
    { "this is a test", "test:(pos=4)" },
    { "test*", "(test:(pos=1) SYNONYM testable:(pos=1) SYNONYM tester:(pos=1))" },
    { "a test*", "(test:(pos=2) SYNONYM testable:(pos=2) SYNONYM tester:(pos=2))" },
    // Two stopwords + one wildcard failed in 1.0.16
    { "is a test*", "(test:(pos=3) SYNONYM testable:(pos=3) SYNONYM tester:(pos=3))" },
    // Three stopwords + one wildcard failed in 1.0.16
    { "this is a test*", "(test:(pos=4) SYNONYM testable:(pos=4) SYNONYM tester:(pos=4))" },
    // Three stopwords + two wildcards failed in 1.0.16
    { "this is a us* test*", "(user:(pos=4) AND (test:(pos=5) SYNONYM testable:(pos=5) SYNONYM tester:(pos=5)))" },
    { "this is a user test*", "(user:(pos=4) AND (test:(pos=5) SYNONYM testable:(pos=5) SYNONYM tester:(pos=5)))" },
    { NULL, NULL }
};

// Regression test for bug fixed in 1.0.17 and 1.1.3.
static bool test_qp_stopword_group1()
{
#ifndef XAPIAN_HAS_INMEMORY_BACKEND
    SKIP_TEST("Testcase requires the InMemory backend which is disabled");
#else
    Xapian::WritableDatabase db(Xapian::InMemory::open());
    Xapian::Document doc;
    doc.add_term("test");
    doc.add_term("tester");
    doc.add_term("testable");
    doc.add_term("user");
    db.add_document(doc);

    Xapian::SimpleStopper stopper;
    stopper.add("this");
    stopper.add("is");
    stopper.add("a");

    Xapian::QueryParser qp;
    qp.set_stopper(&stopper);
    qp.set_database(db);

    // Process test cases with OP_OR first, then with OP_AND.
    qp.set_default_op(Xapian::Query::OP_OR);
    const test *p = test_stopword_group_or_queries;
    for (int i = 1; i <= 2; ++i) {
	for ( ; p->query; ++p) {
	    string expect, parsed;
	    if (p->expect)
		expect = p->expect;
	    else
		expect = "parse error";
	    try {
		Xapian::Query qobj = qp.parse_query(p->query, qp.FLAG_WILDCARD);
		parsed = qobj.get_description();
		expect = string("Xapian::Query(") + expect + ')';
	    } catch (const Xapian::QueryParserError &e) {
		parsed = e.get_msg();
	    } catch (const Xapian::Error &e) {
		parsed = e.get_description();
	    } catch (...) {
		parsed = "Unknown exception!";
	    }
	    tout << "Query: " << p->query << '\n';
	    TEST_STRINGS_EQUAL(parsed, expect);
	}

	qp.set_default_op(Xapian::Query::OP_AND);
	p = test_stopword_group_and_queries;
    }

    return true;
#endif
}

/// Regression test for bug with default_op set such that we get an exception.
// Fixed in 1.0.23 and 1.2.4.
static bool test_qp_default_op2()
{
    Xapian::QueryParser qp;
    static const Xapian::Query::op ops[] = {
	Xapian::Query::OP_AND_NOT,
	Xapian::Query::OP_AND_MAYBE,
	Xapian::Query::OP_FILTER,
	Xapian::Query::OP_VALUE_RANGE,
	Xapian::Query::OP_SCALE_WEIGHT,
	Xapian::Query::OP_VALUE_GE,
	Xapian::Query::OP_VALUE_LE
    };
    const Xapian::Query::op * p;
    for (p = ops; p - ops != sizeof(ops) / sizeof(*ops); ++p) {
	tout << *p << endl;
	qp.set_default_op(*p);
	// Before the fix, we tried to free an object twice when parsing the
	// following query with default_op set such that we get an exception.
	TEST_EXCEPTION(Xapian::InvalidArgumentError,
		       qp.parse_query("a-b NEAR c NEAR d"));
    }
    return true;
}

/// Test cases for the QueryParser.
static const test_desc tests[] = {
    TESTCASE(queryparser1),
    TESTCASE(qp_default_op1),
    TESTCASE(qp_odd_chars1),
    TESTCASE(qp_flag_wildcard1),
    TESTCASE(qp_flag_wildcard2),
    TESTCASE(qp_flag_wildcard3),
    TESTCASE(qp_flag_partial1),
    TESTCASE(qp_flag_bool_any_case1),
    TESTCASE(qp_stopper1),
    TESTCASE(qp_flag_pure_not1),
    TESTCASE(qp_unstem_boolean_prefix),
    TESTCASE(qp_default_prefix1),
    TESTCASE(qp_default_prefix2),
    TESTCASE(value_range_serialise1),
    TESTCASE(qp_value_range1),
    TESTCASE(qp_value_range2),
    TESTCASE(qp_value_range3),
    TESTCASE(qp_value_range4),
    TESTCASE(qp_value_daterange1),
    TESTCASE(qp_value_daterange2),
    TESTCASE(qp_value_stringrange1),
    TESTCASE(qp_value_customrange1),
    TESTCASE(qp_stoplist1),
    TESTCASE(qp_spell1),
    TESTCASE(qp_spell2),
    TESTCASE(qp_spellwild1),
    TESTCASE(qp_spellpartial1),
    TESTCASE(qp_synonym1),
    TESTCASE(qp_synonym2),
    TESTCASE(qp_synonym3),
    TESTCASE(qp_stem_all1),
    TESTCASE(qp_stem_all_z1),
    TESTCASE(qp_scale1),
    TESTCASE(qp_near1),
    TESTCASE(qp_phrase1),
    TESTCASE(qp_stopword_group1),
    TESTCASE(qp_default_op2),
    END_OF_TESTCASES
};

int main(int argc, char **argv)
try {
    // FIXME: It would be better to test with and without XAPIAN_CJK_NGRAM set.
#ifdef HAVE__PUTENV_S
    _putenv_s("XAPIAN_CJK_NGRAM", "1");
#elif defined HAVE_SETENV
    setenv("XAPIAN_CJK_NGRAM", "1", 1);
#else
    putenv(const_cast<char*>("XAPIAN_CJK_NGRAM=1"));
#endif
    test_driver::parse_command_line(argc, argv);
    return test_driver::run(tests);
} catch (const char * e) {
    cout << e << endl;
    return 1;
}
