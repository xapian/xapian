/** @file api_queryparser.cc
 * @brief Tests of Xapian::QueryParser
 */
/* Copyright (C) 2002,2003,2004,2005,2006,2007,2008,2009,2010,2011,2012,2013,2015,2016 Olly Betts
 * Copyright (C) 2006,2007,2009 Lemur Consulting Ltd
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

#include "api_queryparser.h"

#define XAPIAN_DEPRECATED(D) D
#include <xapian.h>

#include "apitest.h"
#include "cputimer.h"
#include "str.h"
#include "stringutils.h"

#include <cmath>
#include <string>
#include <vector>

using namespace std;

#include "testsuite.h"
#include "testutils.h"

struct test {
    const char *query;
    const char *expect;
};

static const test test_or_queries[] = {
    { "simple-example", "(simple@1 PHRASE 2 example@2)" },
    { "time_t", "Ztime_t@1" },
    { "stock -cooking", "(Zstock@1 AND_NOT Zcook@2)" },
    { "foo -baz bar", "((Zfoo@1 OR Zbar@3) AND_NOT Zbaz@2)" },
    { "d- school report", "(Zd@1 OR (Zschool@2 OR Zreport@3))" },
    { "gtk+ -gnome", "(Zgtk+@1 AND_NOT Zgnome@2)" },
    { "c++ -d--", "(Zc++@1 AND_NOT Zd@2)" },
    { "Mg2+ Cl-", "(mg2+@1 OR cl@2)" },
    { "\"c++ library\"", "(c++@1 PHRASE 2 library@2)" },
    { "A&L A&RMCO AD&D", "(a&l@1 OR a&rmco@2 OR ad&d@3)" },
    { "C# vs C++", "(c#@1 OR Zvs@2 OR c++@3)" },
    { "j##", "Zj##@1" },
    { "a#b", "(Za@1 OR Zb@2)" },
    { "O.K. U.N.C.L.E XY.Z.", "((ok@1 OR uncle@2) OR (xy@3 PHRASE 2 z@4))" },
    { "author:orwell animal farm", "(ZAorwel@1 OR Zanim@2 OR Zfarm@3)" },
    { "author:Orwell Animal Farm", "(Aorwell@1 OR animal@2 OR farm@3)" },
    // Regression test for bug reported in 0.9.6.
    { "author:\"orwell\" title:\"animal\"", "(Aorwell@1 OR XTanimal@2)" },
    // Regression test for bug related to one reported in 0.9.6.
    { "author:(orwell) title:(animal)", "(ZAorwel@1 OR ZXTanim@2)" },
    // Regression test for bug caused by fix for previous bug.
    { "author:\"milne, a.a.\"", "(Amilne@1 PHRASE 3 Aa@2 PHRASE 3 Aa@3)" },
    { "author:\"milne a.a.\"", "(Amilne@1 PHRASE 3 Aa@2 PHRASE 3 Aa@3)" },
    // Regression test for bug reported in 0.9.7.
    { "site:/path/name", "0 * H/path/name" },
    // Regression test for bug introduced (and fixed) in SVN prior to 1.0.0.
    { "author:/path/name", "(Apath@1 PHRASE 2 Aname@2)" },
    // Feature tests for change to allow phrase generators after prefix in 1.2.4.
    { "author:/path", "ZApath@1" },
    { "author:-Foo", "Afoo@1" },
    { "author:/", "Zauthor@1" },
    { "author::", "Zauthor@1" },
    { "author:/ foo", "(Zauthor@1 OR Zfoo@2)" },
    { "author:: foo", "(Zauthor@1 OR Zfoo@2)" },
    { "author::foo", "(author@1 PHRASE 2 foo@2)" },
    { "author:/ AND foo", "(Zauthor@1 AND Zfoo@2)" },
    { "author:: AND foo", "(Zauthor@1 AND Zfoo@2)" },
    { "foo AND author:/", "(Zfoo@1 AND Zauthor@2)" },
    { "foo AND author::", "(Zfoo@1 AND Zauthor@2)" },
    // Regression test for bug introduced into (and fixed) in SVN prior to 1.0.0.
    { "author:(title::case)", "(Atitle@1 PHRASE 2 Acase@2)" },
    // Regression test for bug fixed in 1.0.4 - the '+' would be ignored there
    // because the whitespace after the '"' wasn't noticed.
    { "\"hello world\" +python", "(Zpython@3 AND_MAYBE (hello@1 PHRASE 2 world@2))" },
    // In 1.1.0, NON_SPACING_MARK was added as a word character.
    { "\xd8\xa7\xd9\x84\xd8\xb1\xd9\x91\xd8\xad\xd9\x85\xd9\x86", "Z\xd8\xa7\xd9\x84\xd8\xb1\xd9\x91\xd8\xad\xd9\x85\xd9\x86@1" },
    // In 1.1.4, ENCLOSING_MARK and COMBINING_SPACING_MARK were added, and
    // code to ignore several zero-width space characters was added.
    { "\xe1\x80\x9d\xe1\x80\xae\xe2\x80\x8b\xe1\x80\x80\xe1\x80\xae\xe2\x80\x8b\xe1\x80\x95\xe1\x80\xad\xe2\x80\x8b\xe1\x80\x9e\xe1\x80\xaf\xe1\x80\xb6\xe1\x80\xb8\xe2\x80\x8b\xe1\x80\x85\xe1\x80\xbd\xe1\x80\xb2\xe2\x80\x8b\xe1\x80\x9e\xe1\x80\xb0\xe2\x80\x8b\xe1\x80\x99\xe1\x80\xbb\xe1\x80\xac\xe1\x80\xb8\xe1\x80\x80", "Z\xe1\x80\x9d\xe1\x80\xae\xe1\x80\x80\xe1\x80\xae\xe1\x80\x95\xe1\x80\xad\xe1\x80\x9e\xe1\x80\xaf\xe1\x80\xb6\xe1\x80\xb8\xe1\x80\x85\xe1\x80\xbd\xe1\x80\xb2\xe1\x80\x9e\xe1\x80\xb0\xe1\x80\x99\xe1\x80\xbb\xe1\x80\xac\xe1\x80\xb8\xe1\x80\x80@1" },
    { "unmatched\"", "unmatched@1" },
    { "unmatched \" \" ", "Zunmatch@1" },
    { "hyphen-ated\" ", "(hyphen@1 PHRASE 2 ated@2)" },
    { "hyphen-ated\"  \"", "(hyphen@1 PHRASE 2 ated@2)" },
    { "\"1.4\"", "1.4@1" },
    { "\"1.\"", "1@1" },
    { "\"A#.B.\"", "(a#@1 PHRASE 2 b@2)" },
    { "\" Xapian QueryParser\" parses queries", "((xapian@1 PHRASE 2 queryparser@2) OR (Zpars@3 OR Zqueri@4))" },
    { "\" xapian queryParser\" parses queries", "((xapian@1 PHRASE 2 queryparser@2) OR (Zpars@3 OR Zqueri@4))" },
    { "h\xc3\xb6hle", "Zh\xc3\xb6hle@1" },
    { "one +two three", "(Ztwo@2 AND_MAYBE (Zone@1 OR Zthree@3))" },
    { "subject:test other", "(ZXTtest@1 OR Zother@2)" },
    { "subject:\"space flight\"", "(XTspace@1 PHRASE 2 XTflight@2)" },
    { "author:(twain OR poe) OR flight", "((ZAtwain@1 OR ZApoe@2) OR Zflight@3)" },
    { "author:(twain OR title:pit OR poe)", "((ZAtwain@1 OR ZXTpit@2) OR ZApoe@3)" },
    { "title:2001 title:space", "(XT2001@1 OR ZXTspace@2)" },
    { "(title:help)", "ZXThelp@1" },
    { "beer NOT \"orange juice\"", "(Zbeer@1 AND_NOT (orange@2 PHRASE 2 juice@3))" },
    { "beer AND NOT lager", "(Zbeer@1 AND_NOT Zlager@2)" },
    { "beer AND -lager", "(Zbeer@1 AND_NOT Zlager@2)" },
    { "beer AND +lager", "(Zbeer@1 AND Zlager@2)" },
    { "A OR B NOT C", "(a@1 OR (b@2 AND_NOT c@3))" },
    { "A OR B AND NOT C", "(a@1 OR (b@2 AND_NOT c@3))" },
    { "A OR B AND -C", "(a@1 OR (b@2 AND_NOT c@3))" },
    { "A OR B AND +C", "(a@1 OR (b@2 AND c@3))" },
    { "A OR B XOR C", "(a@1 OR (b@2 XOR c@3))" },
    { "A XOR B NOT C", "(a@1 XOR (b@2 AND_NOT c@3))" },
    { "one AND two", "(Zone@1 AND Ztwo@2)" },
    { "one A.N.D. two", "(Zone@1 OR and@2 OR Ztwo@3)" },
    { "one \xc3\x81ND two", "(Zone@1 OR \xc3\xa1nd@2 OR Ztwo@3)" },
    { "one author:AND two", "(Zone@1 OR Aand@2 OR Ztwo@3)" },
    { "author:hyphen-ated", "(Ahyphen@1 PHRASE 2 Aated@2)" },
    { "cvs site:xapian.org", "(Zcvs@1 FILTER Hxapian.org)" },
    { "cvs -site:xapian.org", "(Zcvs@1 AND_NOT Hxapian.org)" },
    { "foo -site:xapian.org bar", "((Zfoo@1 OR Zbar@2) AND_NOT Hxapian.org)" },
    { "site:xapian.org mail", "(Zmail@1 FILTER Hxapian.org)" },
    { "-site:xapian.org mail", "(Zmail@1 AND_NOT Hxapian.org)" },
    { "mail AND -site:xapian.org", "(Zmail@1 AND_NOT Hxapian.org)" },
    { "-Wredundant-decls", "(wredundant@1 PHRASE 2 decls@2)" },
    { "site:xapian.org", "0 * Hxapian.org" },
    { "mug +site:xapian.org -site:cvs.xapian.org", "((Zmug@1 FILTER Hxapian.org) AND_NOT Hcvs.xapian.org)" },
    { "mug -site:cvs.xapian.org +site:xapian.org", "((Zmug@1 FILTER Hxapian.org) AND_NOT Hcvs.xapian.org)" },
    { "mug +site:xapian.org AND -site:cvs.xapian.org", "((Zmug@1 FILTER Hxapian.org) AND_NOT Hcvs.xapian.org)" },
    { "mug site:xapian.org AND -site:cvs.xapian.org", "((Zmug@1 FILTER Hxapian.org) AND_NOT Hcvs.xapian.org)" },
    { "mug site:xapian.org AND +site:cvs.xapian.org", "((Zmug@1 FILTER Hxapian.org) AND 0 * Hcvs.xapian.org)" },
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
    { "hard\xa0space", "(Zhard@1 OR Zspace@2)" },
    { " white\r\nspace\ttest ", "(Zwhite@1 OR Zspace@2 OR Ztest@3)" },
    { "one AND two three", "(Zone@1 AND (Ztwo@2 OR Zthree@3))" },
    { "one two AND three", "((Zone@1 OR Ztwo@2) AND Zthree@3)" },
    { "one AND two/three", "(Zone@1 AND (two@2 PHRASE 2 three@3))" },
    { "one AND /two/three", "(Zone@1 AND (two@2 PHRASE 2 three@3))" },
    { "one AND/two/three", "(Zone@1 AND (two@2 PHRASE 2 three@3))" },
    { "one +/two/three", "((two@2 PHRASE 2 three@3) AND_MAYBE Zone@1)" },
    { "one//two", "(one@1 PHRASE 2 two@2)" },
    { "\"missing quote", "(missing@1 PHRASE 2 quote@2)" },
    { "DVD+RW", "(dvd@1 OR rw@2)" }, // Would a phrase be better?
    { "+\"must have\" optional", "((must@1 PHRASE 2 have@2) AND_MAYBE Zoption@3)" },
    { "one NEAR two NEAR three", "(one@1 NEAR 12 two@2 NEAR 12 three@3)" },
    { "something NEAR/3 else", "(something@1 NEAR 4 else@2)" },
    { "a NEAR/6 b NEAR c", "(a@1 NEAR 8 b@2 NEAR 8 c@3)" },
    { "something ADJ else", "(something@1 PHRASE 11 else@2)" },
    { "something ADJ/3 else", "(something@1 PHRASE 4 else@2)" },
    { "a ADJ/6 b ADJ c", "(a@1 PHRASE 8 b@2 PHRASE 8 c@3)" },
    // Regression test - Unicode character values were truncated to 8 bits
    // before testing C_isdigit(), so this rather artificial example parsed
    // to: (a@1 NEAR 262 b@2)
    { "a NEAR/\xc4\xb5 b", "((Za@1 OR (near@2 PHRASE 2 \xc4\xb5@3)) OR Zb@4)" },
    { "a ADJ/\xc4\xb5 b", "((Za@1 OR (adj@2 PHRASE 2 \xc4\xb5@3)) OR Zb@4)" },
    // Regression test - the first two cases were parsed as if the '/' were a
    // space, which was inconsistent with the second two.  Fixed in 1.2.5.
    { "a NEAR/b", "(Za@1 OR (near@2 PHRASE 2 b@3))" },
    { "a ADJ/b", "(Za@1 OR (adj@2 PHRASE 2 b@3))" },
    { "a NEAR/b c", "((Za@1 OR (near@2 PHRASE 2 b@3)) OR Zc@4)" },
    { "a ADJ/b c", "((Za@1 OR (adj@2 PHRASE 2 b@3)) OR Zc@4)" },
    // Regression tests - + and - didn't work on bracketed subexpressions prior
    // to 1.0.2.
    { "+(one two) three", "((Zone@1 OR Ztwo@2) AND_MAYBE Zthree@3)" },
    { "zero -(one two)", "(Zzero@1 AND_NOT (Zone@2 OR Ztwo@3))" },
    // Feature tests that ':' is inserted between prefix and term correctly:
    { "category:Foo", "0 * XCAT:Foo" },
    { "category:foo", "0 * XCATfoo" },
    { "category:\xc3\x96oo", "0 * XCAT\xc3\x96oo" },
    // Feature tests for quoted boolean terms:
    { "category:\"Hello world\"", "0 * XCAT:Hello world" },
    { "category:\"literal \"\"\"", "0 * XCATliteral \"" },
    { "category:\" \"", "0 * XCAT " },
    { "category:\"\"", "0 * XCAT" },
    { "category:\"(unterminated)", "0 * XCAT(unterminated)" },
    // Feature tests for curly double quotes:
    { "“curly quotes”", "(curly@1 PHRASE 2 quotes@2)" },
    // Feature tests for implicitly closing brackets:
    { "(foo", "Zfoo@1" },
    { "(foo XOR bar", "(Zfoo@1 XOR Zbar@2)" },
    { "(foo XOR (bar AND baz)", "(Zfoo@1 XOR (Zbar@2 AND Zbaz@3))" },
    { "(foo XOR (bar AND baz", "(Zfoo@1 XOR (Zbar@2 AND Zbaz@3))" },
    // Slightly arbitrarily we accept mismatched quotes.
    { "\"curly quotes”", "(curly@1 PHRASE 2 quotes@2)" },
    { "“curly quotes\"", "(curly@1 PHRASE 2 quotes@2)" },
    { "“curly quotes“", "(curly@1 PHRASE 2 quotes@2)" },
    { "”curly quotes”", "(curly@1 PHRASE 2 quotes@2)" },
    { "author:“orwell” title:“animal\"", "(Aorwell@1 OR XTanimal@2)" },
    { "author:\"orwell” title:“animal”", "(Aorwell@1 OR XTanimal@2)" },
    { "author:“milne, a.a.”", "(Amilne@1 PHRASE 3 Aa@2 PHRASE 3 Aa@3)" },
    { "author:“milne, a.a.\"", "(Amilne@1 PHRASE 3 Aa@2 PHRASE 3 Aa@3)" },
    { "author:\"milne a.a.”", "(Amilne@1 PHRASE 3 Aa@2 PHRASE 3 Aa@3)" },
    { "“hello world” +python", "(Zpython@3 AND_MAYBE (hello@1 PHRASE 2 world@2))" },
    { "unmatched“", "Zunmatch@1" },
    { "unmatched”", "Zunmatch@1" },
    { "unmatched “ ” ", "Zunmatch@1" },
    { "unmatched \" ” ", "Zunmatch@1" },
    { "unmatched “ \" ", "Zunmatch@1" },
    { "hyphen-ated“ ", "(hyphen@1 PHRASE 2 ated@2)" },
    { "hyphen-ated” ", "(hyphen@1 PHRASE 2 ated@2)" },
    { "hyphen-ated“  ”", "(hyphen@1 PHRASE 2 ated@2)" },
    { "hyphen-ated“  \"", "(hyphen@1 PHRASE 2 ated@2)" },
    { "hyphen-ated\"  ”", "(hyphen@1 PHRASE 2 ated@2)" },
    { "“1.4”", "1.4@1" },
    { "“1.\"", "1@1" },
    { "\"A#.B.”", "(a#@1 PHRASE 2 b@2)" },
    { "“ Xapian QueryParser” parses queries", "((xapian@1 PHRASE 2 queryparser@2) OR (Zpars@3 OR Zqueri@4))" },
    { "“ xapian queryParser\" parses queries", "((xapian@1 PHRASE 2 queryparser@2) OR (Zpars@3 OR Zqueri@4))" },
    { "beer NOT “orange juice”", "(Zbeer@1 AND_NOT (orange@2 PHRASE 2 juice@3))" },
    { "“missing quote", "(missing@1 PHRASE 2 quote@2)" },
    { "+“must have” optional", "((must@1 PHRASE 2 have@2) AND_MAYBE Zoption@3)" },
    { "category:“Hello world”", "0 * XCAT:Hello world" },
    { "category:“literal \"\"”", "0 * XCATliteral \"" },
    { "category:“ ”", "0 * XCAT " },
    { "category:\" ”\"", "0 * XCAT ”" },
    { "category:\" ”", "0 * XCAT ”" },
    { "category:“ \"", "0 * XCAT " },
    { "category:“”", "0 * XCAT" },
    { "category:\"”\"", "0 * XCAT”" },
    { "category:\"”", "0 * XCAT”" },
    { "category:“\"", "0 * XCAT" },
    { "category:“(unterminated)", "0 * XCAT(unterminated)" },
    // Real world examples from tweakers.net:
    { "Call to undefined function: imagecreate()", "(call@1 OR Zto@2 OR Zundefin@3 OR Zfunction@4 OR imagecreate@5)" },
    { "mysql_fetch_row(): supplied argument is not a valid MySQL result resource", "(mysql_fetch_row@1 OR (Zsuppli@2 OR Zargument@3 OR Zis@4 OR Znot@5 OR Za@6 OR Zvalid@7 OR mysql@8 OR Zresult@9 OR Zresourc@10))" },
    { "php date() nedelands", "((Zphp@1 OR date@2) OR Znedeland@3)" },
    { "wget domein --http-user", "((Zwget@1 OR Zdomein@2) OR (http@3 PHRASE 2 user@4))" },
    { "@home problemen", "(Zhome@1 OR Zproblemen@2)" },
    { "'ipacsum'", "Zipacsum@1" },
    { "canal + ", "Zcanal@1" },
    { "/var/run/mysqld/mysqld.sock", "(var@1 PHRASE 5 run@2 PHRASE 5 mysqld@3 PHRASE 5 mysqld@4 PHRASE 5 sock@5)" },
    { "\"QSI-161 drivers\"", "(qsi@1 PHRASE 3 161@2 PHRASE 3 drivers@3)" },
    { "\"e-cube\" barebone", "((e@1 PHRASE 2 cube@2) OR Zbarebon@3)" },
    { "\"./httpd: symbol not found: dlopen\"", "(httpd@1 PHRASE 5 symbol@2 PHRASE 5 not@3 PHRASE 5 found@4 PHRASE 5 dlopen@5)" },
    { "ERROR 2003: Can't connect to MySQL server on 'localhost' (10061)", "(((error@1 OR 2003@2 OR can't@3 OR Zconnect@4 OR Zto@5 OR mysql@6 OR Zserver@7 OR Zon@8) OR Zlocalhost@9) OR 10061@10)" },
    { "location.href = \"\"", "(location@1 PHRASE 2 href@2)" },
    { "method=\"post\" action=\"\">", "((method@1 OR post@2) OR action@3)" },
    { "behuizing 19\" inch", "((Zbehuiz@1 OR 19@2) OR inch@3)" },
    { "19\" rack", "(19@1 OR rack@2)" },
    { "3,5\" mainboard", "(3,5@1 OR mainboard@2)" },
    { "553 sorry, that domain isn't in my list of allowed rcpthosts (#5.7.1)", "(((553@1 OR Zsorri@2) OR (Zthat@3 OR Zdomain@4 OR Zisn't@5 OR Zin@6 OR Zmy@7 OR Zlist@8 OR Zof@9 OR Zallow@10 OR Zrcpthost@11)) OR 5.7.1@12)" },
    { "data error (clic redundancy check)", "((Zdata@1 OR Zerror@2) OR (Zclic@3 OR Zredund@4 OR Zcheck@5))" },
    { "? mediaplayer 9\"", "(Zmediaplay@1 OR 9@2)" },
    { "date(\"w\")", "(date@1 OR w@2)" },
    { "Syntaxisfout (operator ontbreekt ASP", "(syntaxisfout@1 OR (Zoper@2 OR Zontbreekt@3 OR asp@4))" },
    { "Request.ServerVariables(\"logon_user\")", "((request@1 PHRASE 2 servervariables@2) OR logon_user@3)" },
    { "ASP \"request.form\" van \\\"enctype=\"MULTIPART/FORM-DATA\"\\\"", "((((asp@1 OR (request@2 PHRASE 2 form@3)) OR Zvan@4) OR enctype@5) OR (multipart@6 PHRASE 3 form@7 PHRASE 3 data@8))" },
    { "USER ftp (Login failed): Invalid shell: /sbin/nologin", "((((user@1 OR Zftp@2) OR (login@3 OR Zfail@4)) OR (invalid@5 OR Zshell@6)) OR (sbin@7 PHRASE 2 nologin@8))" },
    { "ip_masq_new(proto=TCP)", "((ip_masq_new@1 OR proto@2) OR tcp@3)" },
    { "\"document.write(\"", "(document@1 PHRASE 2 write@2)" },
    { "ERROR 1045: Access denied for user: 'root@localhost' (Using password: NO)", "(((error@1 OR 1045@2 OR access@3 OR Zdeni@4 OR Zfor@5 OR Zuser@6) OR (root@7 PHRASE 2 localhost@8)) OR (using@9 OR Zpassword@10 OR no@11))" },
    { "TIP !! subtitles op TV-out (via DVD max g400)", "(((tip@1 OR (Zsubtitl@2 OR Zop@3)) OR (tv@4 PHRASE 2 out@5)) OR (Zvia@6 OR dvd@7 OR Zmax@8 OR Zg400@9))" },
    { "Gigabyte 8PE667 (de Ultra versie) of Asus A7N8X Deluxe", "(((gigabyte@1 OR 8pe667@2) OR (Zde@3 OR ultra@4 OR Zversi@5)) OR (Zof@6 OR asus@7 OR a7n8x@8 OR deluxe@9))" },
    { "\"1) Ze testen 8x AF op de GFFX tegen \"", "(1@1 PHRASE 9 ze@2 PHRASE 9 testen@3 PHRASE 9 8x@4 PHRASE 9 af@5 PHRASE 9 op@6 PHRASE 9 de@7 PHRASE 9 gffx@8 PHRASE 9 tegen@9)" },
    { "\") Ze houden geen rekening met de kwaliteit van AF. Als ze dat gedaan hadden dan waren ze tot de conclusie gekomen dat Performance AF (dus Bilinear AF) op de 9700Pro goed te vergelijken is met Balanced AF op de GFFX. En dan hadden ze ook gezien dat de GFFX niet kan tippen aan de Quality AF van de 9700Pro.\"", "(ze@1 PHRASE 59 houden@2 PHRASE 59 geen@3 PHRASE 59 rekening@4 PHRASE 59 met@5 PHRASE 59 de@6 PHRASE 59 kwaliteit@7 PHRASE 59 van@8 PHRASE 59 af@9 PHRASE 59 als@10 PHRASE 59 ze@11 PHRASE 59 dat@12 PHRASE 59 gedaan@13 PHRASE 59 hadden@14 PHRASE 59 dan@15 PHRASE 59 waren@16 PHRASE 59 ze@17 PHRASE 59 tot@18 PHRASE 59 de@19 PHRASE 59 conclusie@20 PHRASE 59 gekomen@21 PHRASE 59 dat@22 PHRASE 59 performance@23 PHRASE 59 af@24 PHRASE 59 dus@25 PHRASE 59 bilinear@26 PHRASE 59 af@27 PHRASE 59 op@28 PHRASE 59 de@29 PHRASE 59 9700pro@30 PHRASE 59 goed@31 PHRASE 59 te@32 PHRASE 59 vergelijken@33 PHRASE 59 is@34 PHRASE 59 met@35 PHRASE 59 balanced@36 PHRASE 59 af@37 PHRASE 59 op@38 PHRASE 59 de@39 PHRASE 59 gffx@40 PHRASE 59 en@41 PHRASE 59 dan@42 PHRASE 59 hadden@43 PHRASE 59 ze@44 PHRASE 59 ook@45 PHRASE 59 gezien@46 PHRASE 59 dat@47 PHRASE 59 de@48 PHRASE 59 gffx@49 PHRASE 59 niet@50 PHRASE 59 kan@51 PHRASE 59 tippen@52 PHRASE 59 aan@53 PHRASE 59 de@54 PHRASE 59 quality@55 PHRASE 59 af@56 PHRASE 59 van@57 PHRASE 59 de@58 PHRASE 59 9700pro@59)" },
    { "\"Ze houden geen rekening met de kwaliteit van AF. Als ze dat gedaan hadden dan waren ze tot de conclusie gekomen dat Performance AF (dus Bilinear AF) op de 9700Pro goed te vergelijken is met Balanced AF op de GFFX. En dan hadden ze ook gezien dat de GFFX niet kan tippen aan de Quality AF van de 9700Pro.\"", "(ze@1 PHRASE 59 houden@2 PHRASE 59 geen@3 PHRASE 59 rekening@4 PHRASE 59 met@5 PHRASE 59 de@6 PHRASE 59 kwaliteit@7 PHRASE 59 van@8 PHRASE 59 af@9 PHRASE 59 als@10 PHRASE 59 ze@11 PHRASE 59 dat@12 PHRASE 59 gedaan@13 PHRASE 59 hadden@14 PHRASE 59 dan@15 PHRASE 59 waren@16 PHRASE 59 ze@17 PHRASE 59 tot@18 PHRASE 59 de@19 PHRASE 59 conclusie@20 PHRASE 59 gekomen@21 PHRASE 59 dat@22 PHRASE 59 performance@23 PHRASE 59 af@24 PHRASE 59 dus@25 PHRASE 59 bilinear@26 PHRASE 59 af@27 PHRASE 59 op@28 PHRASE 59 de@29 PHRASE 59 9700pro@30 PHRASE 59 goed@31 PHRASE 59 te@32 PHRASE 59 vergelijken@33 PHRASE 59 is@34 PHRASE 59 met@35 PHRASE 59 balanced@36 PHRASE 59 af@37 PHRASE 59 op@38 PHRASE 59 de@39 PHRASE 59 gffx@40 PHRASE 59 en@41 PHRASE 59 dan@42 PHRASE 59 hadden@43 PHRASE 59 ze@44 PHRASE 59 ook@45 PHRASE 59 gezien@46 PHRASE 59 dat@47 PHRASE 59 de@48 PHRASE 59 gffx@49 PHRASE 59 niet@50 PHRASE 59 kan@51 PHRASE 59 tippen@52 PHRASE 59 aan@53 PHRASE 59 de@54 PHRASE 59 quality@55 PHRASE 59 af@56 PHRASE 59 van@57 PHRASE 59 de@58 PHRASE 59 9700pro@59)" },
    { "$structure = imap_header($mbox, $tt);", "(((Zstructur@1 OR imap_header@2) OR Zmbox@3) OR Ztt@4)" },
    { "\"ifup: Could not get a valid interface name: -> skipped\"", "(ifup@1 PHRASE 9 could@2 PHRASE 9 not@3 PHRASE 9 get@4 PHRASE 9 a@5 PHRASE 9 valid@6 PHRASE 9 interface@7 PHRASE 9 name@8 PHRASE 9 skipped@9)" },
    { "Er kan geen combinatie van filters worden gevonden om de gegevensstroom te genereren. (Error=80040218)", "((er@1 OR Zkan@2 OR Zgeen@3 OR Zcombinati@4 OR Zvan@5 OR Zfilter@6 OR Zworden@7 OR Zgevonden@8 OR Zom@9 OR Zde@10 OR Zgegevensstroom@11 OR Zte@12 OR Zgenereren@13) OR (error@14 OR 80040218@15))" },
    { "ereg_replace(\"\\\\\",\"\\/\"", "ereg_replace@1" },
    { "\\\\\"divx+geen+geluid\\\\\"", "(divx@1 PHRASE 3 geen@2 PHRASE 3 geluid@3)" },
    { "lcase(\"string\")", "(lcase@1 OR string@2)" },
    { "isEmpty( )  functie in visual basic", "(isempty@1 OR (Zfuncti@2 OR Zin@3 OR Zvisual@4 OR Zbasic@5))" },
    { "*** stop: 0x0000001E (0xC0000005,0x00000000,0x00000000,0x00000000)", "((Zstop@1 OR 0x0000001e@2) OR 0xc0000005,0x00000000,0x00000000,0x00000000@3)" },
    { "\"ctrl+v+c+a fout\"", "(ctrl@1 PHRASE 5 v@2 PHRASE 5 c@3 PHRASE 5 a@4 PHRASE 5 fout@5)" },
    { "Server.CreateObject(\"ADODB.connection\")", "((server@1 PHRASE 2 createobject@2) OR (adodb@3 PHRASE 2 connection@4))" },
    { "Presario 6277EA-XP model P4/28 GHz-120GB-DVD-CDRW (512MBWXP) (470048-012)", "((((((presario@1 OR (6277ea@2 PHRASE 2 xp@3)) OR Zmodel@4) OR (p4@5 PHRASE 2 28@6)) OR (ghz@7 PHRASE 4 120gb@8 PHRASE 4 dvd@9 PHRASE 4 cdrw@10)) OR 512mbwxp@11) OR (470048@12 PHRASE 2 012@13))" },
    { "Failed to connect agent. (AGENT=dbaxchg2, EC=UserId =NUll)", "((failed@1 OR Zto@2 OR Zconnect@3 OR Zagent@4) OR ((((agent@5 OR Zdbaxchg2@6) OR ec@7) OR userid@8) OR null@9))" },
    { "delphi CreateOleObject(\"MSXML2.DomDocument\")", "((Zdelphi@1 OR createoleobject@2) OR (msxml2@3 PHRASE 2 domdocument@4))" },
    { "Unhandled exeption in IEXPLORE.EXE (FTAPP.DLL)", "(((unhandled@1 OR Zexept@2 OR Zin@3) OR (iexplore@4 PHRASE 2 exe@5)) OR (ftapp@6 PHRASE 2 dll@7))" },
    { "IBM High Rate Wireless LAN PCI Adapter (Low Profile Enabled)", "((ibm@1 OR high@2 OR rate@3 OR wireless@4 OR lan@5 OR pci@6 OR adapter@7) OR (low@8 OR profile@9 OR enabled@10))" },
    { "asp ' en \"", "(Zasp@1 OR Zen@2)" },
    { "Hercules 3D Prophet 8500 LE 64MB (OEM, Radeon 8500 LE)", "((hercules@1 OR 3d@2 OR prophet@3 OR 8500@4 OR le@5 OR 64mb@6) OR (oem@7 OR (radeon@8 OR 8500@9 OR le@10)))" },
    { "session_set_cookie_params(echo \"hoi\")", "((session_set_cookie_params@1 OR Zecho@2) OR hoi@3)" },
    { "windows update werkt niet (windows se", "((Zwindow@1 OR Zupdat@2 OR Zwerkt@3 OR Zniet@4) OR (Zwindow@5 OR Zse@6))" },
    { "De statuscode van de fout is ( 0 x 4 , 0 , 0 , 0 )", "((de@1 OR Zstatuscod@2 OR Zvan@3 OR Zde@4 OR Zfout@5 OR Zis@6) OR ((((0@7 OR Zx@8 OR 4@9) OR 0@10) OR 0@11) OR 0@12))" },
    { "sony +(u20 u-20)", "((Zu20@2 OR (u@3 PHRASE 2 20@4)) AND_MAYBE Zsoni@1)" },
    { "[crit] (17)File exists: unable to create scoreboard (name-based shared memory failure)", "(((Zcrit@1 OR 17@2) OR (file@3 OR Zexist@4 OR Zunabl@5 OR Zto@6 OR Zcreat@7 OR Zscoreboard@8)) OR ((name@9 PHRASE 2 based@10) OR (Zshare@11 OR Zmemori@12 OR Zfailur@13)))" },
    { "directories lokaal php (uitlezen OR inladen)", "((Zdirectori@1 OR Zlokaal@2 OR Zphp@3) OR (Zuitlezen@4 OR Zinladen@5))" },
    { "(multi pc modem)+ (line sync)", "((Zmulti@1 OR Zpc@2 OR Zmodem@3) OR (Zline@4 OR Zsync@5))" },
    { "xp 5.1.2600.0 (xpclient.010817-1148)", "((Zxp@1 OR 5.1.2600.0@2) OR (xpclient@3 PHRASE 3 010817@4 PHRASE 3 1148@5))" },
    { "DirectDraw test results: Failure at step 5 (User verification of rectangles): HRESULT = 0x00000000 (error code) Direct3D 7 test results: Failure at step 32 (User verification of Direct3D rendering): HRESULT = 0x00000000 (error code) Direct3D 8 test results: Failure at step 32 (User verification of Direct3D rendering): HRESULT = 0x00000000 (error code) Direct3D 9 test results: Failure at step 32 (User verification of Direct3D rendering): HRESULT = 0x00000000 (error code)", "((((((((((((((((((((directdraw@1 OR Ztest@2 OR Zresult@3 OR failure@4 OR Zat@5 OR Zstep@6 OR 5@7) OR (user@8 OR Zverif@9 OR Zof@10 OR Zrectangl@11)) OR hresult@12) OR 0x00000000@13) OR (Zerror@14 OR Zcode@15)) OR (direct3d@16 OR 7@17 OR Ztest@18 OR Zresult@19 OR failure@20 OR Zat@21 OR Zstep@22 OR 32@23)) OR (user@24 OR Zverif@25 OR Zof@26 OR direct3d@27 OR Zrender@28)) OR hresult@29) OR 0x00000000@30) OR (Zerror@31 OR Zcode@32)) OR (direct3d@33 OR 8@34 OR Ztest@35 OR Zresult@36 OR failure@37 OR Zat@38 OR Zstep@39 OR 32@40)) OR (user@41 OR Zverif@42 OR Zof@43 OR direct3d@44 OR Zrender@45)) OR hresult@46) OR 0x00000000@47) OR (Zerror@48 OR Zcode@49)) OR (direct3d@50 OR 9@51 OR Ztest@52 OR Zresult@53 OR failure@54 OR Zat@55 OR Zstep@56 OR 32@57)) OR (user@58 OR Zverif@59 OR Zof@60 OR direct3d@61 OR Zrender@62)) OR hresult@63) OR 0x00000000@64) OR (Zerror@65 OR Zcode@66))" },
    { "Thermaltake Aquarius II waterkoeling (kompleet voor P4 en XP)", "((thermaltake@1 OR aquarius@2 OR ii@3 OR Zwaterkoel@4) OR (Zkompleet@5 OR Zvoor@6 OR p4@7 OR Zen@8 OR xp@9))" },
    { "E3501 unable to add job to database (EC=-2005)", "((e3501@1 OR Zunabl@2 OR Zto@3 OR Zadd@4 OR Zjob@5 OR Zto@6 OR Zdatabas@7) OR (ec@8 OR 2005@9))" },
    { "\"arp -s\" ip veranderen", "((arp@1 PHRASE 2 s@2) OR (Zip@3 OR Zveranderen@4))" },
    { "header(\"content-type: application/octet-stream\");", "((header@1 OR (content@2 PHRASE 2 type@3)) OR (application@4 PHRASE 3 octet@5 PHRASE 3 stream@6))" },
    { "$datum = date(\"d-m-Y\");", "((Zdatum@1 OR date@2) OR (d@3 PHRASE 3 m@4 PHRASE 3 y@5))" },
    { "\"'\" +asp", "Zasp@1" },
    { "+session +[", "Zsession@1" },
    { "Dit apparaat kan niet starten. (Code 10)", "((dit@1 OR Zapparaat@2 OR Zkan@3 OR Zniet@4 OR Zstarten@5) OR (code@6 OR 10@7))" },
    { "\"You cannot use the Administration program while the Domino Server is running. Either shut down the Domino Server (but keep the file server running) or choose the ican labeled 'Lotus Notes' instead.\"", "(you@1 PHRASE 32 cannot@2 PHRASE 32 use@3 PHRASE 32 the@4 PHRASE 32 administration@5 PHRASE 32 program@6 PHRASE 32 while@7 PHRASE 32 the@8 PHRASE 32 domino@9 PHRASE 32 server@10 PHRASE 32 is@11 PHRASE 32 running@12 PHRASE 32 either@13 PHRASE 32 shut@14 PHRASE 32 down@15 PHRASE 32 the@16 PHRASE 32 domino@17 PHRASE 32 server@18 PHRASE 32 but@19 PHRASE 32 keep@20 PHRASE 32 the@21 PHRASE 32 file@22 PHRASE 32 server@23 PHRASE 32 running@24 PHRASE 32 or@25 PHRASE 32 choose@26 PHRASE 32 the@27 PHRASE 32 ican@28 PHRASE 32 labeled@29 PHRASE 32 lotus@30 PHRASE 32 notes@31 PHRASE 32 instead@32)" },
    { "\"+irq +veranderen +xp\"", "(irq@1 PHRASE 3 veranderen@2 PHRASE 3 xp@3)" },
    { "\"is not a member of 'operator``global namespace''' + c++", "(is@1 PHRASE 9 not@2 PHRASE 9 a@3 PHRASE 9 member@4 PHRASE 9 of@5 PHRASE 9 operator@6 PHRASE 9 global@7 PHRASE 9 namespace@8 PHRASE 9 c++@9)" },
    { "mkdir() failed (File exists) php", "(((mkdir@1 OR Zfail@2) OR (file@3 OR Zexist@4)) OR Zphp@5)" },
    { "laatsteIndex(int n)", "(laatsteindex@1 OR (Zint@2 OR Zn@3))" },
    { "\"line+in\" OR \"c8783\"", "((line@1 PHRASE 2 in@2) OR c8783@3)" },
    { "if ($_POST['Submit'])", "(Zif@1 OR (_post@2 OR submit@3))" },
    { "NEC DVD+-RW ND-1300A", "((nec@1 OR (dvd+@2 PHRASE 2 rw@3)) OR (nd@4 PHRASE 2 1300a@5))" },
    { "*String not found* (*String not found*.)", "((string@1 OR Znot@2 OR found@3) OR (string@4 OR Znot@5 OR found@6))" },
    { "MSI G4Ti4200-TD 128MB (GeForce4 Ti4200)", "(((msi@1 OR (g4ti4200@2 PHRASE 2 td@3)) OR 128mb@4) OR (geforce4@5 OR ti4200@6))" },
    { "href=\"#\"", "href@1" },
    { "Request.ServerVariables(\"REMOTE_USER\") javascript", "(((request@1 PHRASE 2 servervariables@2) OR remote_user@3) OR Zjavascript@4)" },
    { "XF86Config(-4) waar", "((xf86config@1 OR 4@2) OR Zwaar@3)" },
    { "Unknown (tag 2000)", "(unknown@1 OR (Ztag@2 OR 2000@3))" },
    { "KT4V(MS-6712)", "(kt4v@1 OR (ms@2 PHRASE 2 6712@3))" },
    { "scheduled+AND+nieuwsgroepen+AND+updaten", "((Zschedul@1 AND Znieuwsgroepen@2) AND Zupdaten@3)" },
    { "137(netbios-ns)", "(137@1 OR (netbios@2 PHRASE 2 ns@3))" },
    { "HARWARE ERROR, TRACKING SERVO (4:0X09:0X01)", "(((harware@1 OR error@2) OR (tracking@3 OR servo@4)) OR (4@5 PHRASE 3 0x09@6 PHRASE 3 0x01@7))" },
    { "Chr(10) wat is code van \" teken", "(((chr@1 OR 10@2) OR (Zwat@3 OR Zis@4 OR Zcode@5 OR Zvan@6)) OR Zteken@7)" },
    { "wat is code van \" teken", "((Zwat@1 OR Zis@2 OR Zcode@3 OR Zvan@4) OR teken@5)" },
    { "The Jet VBA file (VBAJET.dll for 16-bit version, VBAJET32.dll version", "((the@1 OR jet@2 OR vba@3 OR Zfile@4) OR ((((((vbajet@5 PHRASE 2 dll@6) OR Zfor@7) OR (16@8 PHRASE 2 bit@9)) OR Zversion@10) OR (vbajet32@11 PHRASE 2 dll@12)) OR Zversion@13))" },
    { "Permission denied (publickey,password,keyboard-interactive).", "((permission@1 OR Zdeni@2) OR ((Zpublickey@3 OR Zpassword@4) OR (keyboard@5 PHRASE 2 interactive@6)))" },
    { "De lees- of schrijfbewerking (\"written\") op het geheugen is mislukt", "(((de@1 OR Zlee@2 OR Zof@3 OR Zschrijfbewerk@4) OR written@5) OR (Zop@6 OR Zhet@7 OR Zgeheugen@8 OR Zis@9 OR Zmislukt@10))" },
    { "Primary IDE channel no 80 conductor cable installed\"", "(primary@1 OR ide@2 OR Zchannel@3 OR Zno@4 OR 80@5 OR Zconductor@6 OR Zcabl@7 OR installed@8)" },
    { "\"2020 NEAR zoom\"", "(2020@1 PHRASE 3 near@2 PHRASE 3 zoom@3)" },
    { "setcookie(\"naam\",\"$user\");", "((setcookie@1 OR naam@2) OR user@3)" },
    { "MSI 645 Ultra (MS-6547) Ver1", "(((msi@1 OR 645@2 OR ultra@3) OR (ms@4 PHRASE 2 6547@5)) OR ver1@6)" },
    { "if ($HTTP", "(Zif@1 OR http@2)" },
    { "data error(cyclic redundancy check)", "((Zdata@1 OR error@2) OR (Zcyclic@3 OR Zredund@4 OR Zcheck@5))" },
    { "UObject::StaticAllocateObject <- (NULL None) <- UObject::StaticConstructObject <- InitEngine", "((((uobject@1 PHRASE 2 staticallocateobject@2) OR (null@3 OR none@4)) OR (uobject@5 PHRASE 2 staticconstructobject@6)) OR initengine@7)" },
    { "Failure at step 8 (Creating 3D Device)", "((failure@1 OR Zat@2 OR Zstep@3 OR 8@4) OR (creating@5 OR 3d@6 OR device@7))" },
    { "Call Shell(\"notepad.exe\",", "((call@1 OR shell@2) OR (notepad@3 PHRASE 2 exe@4))" },
    { "2.5\" harddisk converter", "(2.5@1 OR (harddisk@2 PHRASE 2 converter@3))" },
    { "creative labs \"dvd+rw\"", "((Zcreativ@1 OR Zlab@2) OR (dvd@3 PHRASE 2 rw@4))" },
    { "\"het beleid van deze computer staat u niet toe interactief", "(het@1 PHRASE 10 beleid@2 PHRASE 10 van@3 PHRASE 10 deze@4 PHRASE 10 computer@5 PHRASE 10 staat@6 PHRASE 10 u@7 PHRASE 10 niet@8 PHRASE 10 toe@9 PHRASE 10 interactief@10)" },
    { "ati radeon \"driver cleaner", "((Zati@1 OR Zradeon@2) OR (driver@3 PHRASE 2 cleaner@4))" },
    { "\"../\" path", "Zpath@1" },
    { "(novell client) workstation only", "((Znovel@1 OR Zclient@2) OR (Zworkstat@3 OR Zonli@4))" },
    { "Unable to find libgd.(a|so) anywhere", "((((unable@1 OR Zto@2 OR Zfind@3 OR Zlibgd@4) OR Za@5) OR Zso@6) OR Zanywher@7)" },
    { "\"libstdc++-libc6.1-1.so.2\"", "(libstdc++@1 PHRASE 5 libc6.1@2 PHRASE 5 1@3 PHRASE 5 so@4 PHRASE 5 2@5)" },
    { "ipsec_setup (/etc/ipsec.conf, line 1) cannot open configuration file \"/etc/ipsec.conf\" -- `' aborted", "((((Zipsec_setup@1 OR ((etc@2 PHRASE 3 ipsec@3 PHRASE 3 conf@4) OR (Zline@5 OR 1@6))) OR (Zcannot@7 OR Zopen@8 OR Zconfigur@9 OR Zfile@10)) OR (etc@11 PHRASE 3 ipsec@12 PHRASE 3 conf@13)) OR Zabort@14)" },
    { "Forwarden van domeinnaam (naar HTTP adres)", "((forwarden@1 OR Zvan@2 OR Zdomeinnaam@3) OR (Znaar@4 OR http@5 OR Zadr@6))" },
    { "Compaq HP, 146.8 GB (MPN-286716-B22) Hard Drives", "((((compaq@1 OR hp@2) OR (146.8@3 OR gb@4)) OR (mpn@5 PHRASE 3 286716@6 PHRASE 3 b22@7)) OR (hard@8 OR drives@9))" },
    { "httpd (no pid file) not running", "((Zhttpd@1 OR (Zno@2 OR Zpid@3 OR Zfile@4)) OR (Znot@5 OR Zrun@6))" },
    { "apache httpd (pid file) not running", "(((Zapach@1 OR Zhttpd@2) OR (Zpid@3 OR Zfile@4)) OR (Znot@5 OR Zrun@6))" },
    { "Klasse is niet geregistreerd  (Fout=80040154).", "((klasse@1 OR Zis@2 OR Zniet@3 OR Zgeregistreerd@4) OR (fout@5 OR 80040154@6))" },
    { "\"dvd+r\" \"dvd-r\"", "((dvd@1 PHRASE 2 r@2) OR (dvd@3 PHRASE 2 r@4))" },
    { "\"=\" tekens uit csvfile", "(Zteken@1 OR Zuit@2 OR Zcsvfile@3)" },
    { "libc.so.6(GLIBC_2.3)", "((libc@1 PHRASE 3 so@2 PHRASE 3 6@3) OR glibc_2.3@4)" },
    { "Sitecom Broadband xDSL / Cable Router 4S (DC-202)", "(((sitecom@1 OR broadband@2 OR Zxdsl@3) OR (cable@4 OR router@5 OR 4s@6)) OR (dc@7 PHRASE 2 202@8))" },
    { "(t-mobile) bereik", "((t@1 PHRASE 2 mobile@2) OR Zbereik@3)" },
    { "error LNK2001: unresolved external symbol \"public", "((Zerror@1 OR lnk2001@2 OR Zunresolv@3 OR Zextern@4 OR Zsymbol@5) OR public@6)" },
    { "patch linux exploit -p)", "((Zpatch@1 OR Zlinux@2 OR Zexploit@3) OR Zp@4)" },
    { "MYD not found (Errcode: 2)", "((myd@1 OR Znot@2 OR Zfound@3) OR (errcode@4 OR 2@5))" },
    { "ob_start(\"ob_gzhandler\"); file download", "((ob_start@1 OR ob_gzhandler@2) OR (Zfile@3 OR Zdownload@4))" },
    { "ECS Elitegroup K7VZA (VIA VT8363/VT8363A)", "((ecs@1 OR elitegroup@2 OR k7vza@3) OR (via@4 OR (vt8363@5 PHRASE 2 vt8363a@6)))" },
    { "ASUS A7V8X (LAN + Serial-ATA + Firewire + Raid + Audio)", "((asus@1 OR a7v8x@2) OR ((((lan@3 OR (serial@4 PHRASE 2 ata@5)) OR firewire@6) OR raid@7) OR audio@8))" },
    { "Javascript:history.go(-1)", "((javascript@1 PHRASE 3 history@2 PHRASE 3 go@3) OR 1@4)" },
    { "java :) als icon", "(Zjava@1 OR (Zal@2 OR Zicon@3))" },
    { "onmouseover=setPointer(this", "((onmouseover@1 OR setpointer@2) OR Zthis@3)" },
    { "\" in vbscript", "(in@1 PHRASE 2 vbscript@2)" },
    { "IRC (FAQ OR (hulp NEAR bij))", "(irc@1 OR (faq@2 OR (hulp@3 NEAR 11 bij@4)))" },
    { "setProperty(\"McSquare\"+i, _xscale, _xscale++);", "((((setproperty@1 OR mcsquare@2) OR Zi@3) OR _xscale@4) OR _xscale++@5)" },
    { "[warn] Apache does not support line-end comments. Consider using quotes around argument: \"#-1\"", "(((((Zwarn@1 OR (apache@2 OR Zdoe@3 OR Znot@4 OR Zsupport@5)) OR (line@6 PHRASE 2 end@7)) OR Zcomment@8) OR (consider@9 OR Zuse@10 OR Zquot@11 OR Zaround@12 OR Zargument@13)) OR 1@14)" },
    { "(php.ini) (memory_limit)", "((php@1 PHRASE 2 ini@2) OR Zmemory_limit@3)" },
    { "line 8: syntax error near unexpected token `kernel_thread(f'", "(((Zline@1 OR 8@2 OR Zsyntax@3 OR Zerror@4 OR Znear@5 OR Zunexpect@6 OR Ztoken@7) OR kernel_thread@8) OR Zf@9)" },
    { "VXD NAVEX()@)", "(vxd@1 OR navex@2)" },
    { "\"Iiyama AS4314UT 17\" \"", "(iiyama@1 PHRASE 3 as4314ut@2 PHRASE 3 17@3)" },
    { "include (\"$id.html\");", "(Zinclud@1 OR (id@2 PHRASE 2 html@3))" },
    { "include id.Today's date is: <? print (date (\"M d, Y\")); ?>hp", "(((((Zinclud@1 OR (id@2 PHRASE 2 today's@3)) OR (Zdate@4 OR Zis@5)) OR Zprint@6) OR (Zdate@7 OR (m@8 PHRASE 3 d@9 PHRASE 3 y@10))) OR Zhp@11)" },
    { "(program files\\common) opstarten", "((Zprogram@1 OR (files@2 PHRASE 2 common@3)) OR Zopstarten@4)" },
    { "java \" string", "(Zjava@1 OR string@2)" },
    { "+=", "" },
    { "php +=", "Zphp@1" },
    { "[php] ereg_replace(\".\"", "(Zphp@1 OR ereg_replace@2)" },
    { "\"echo -e\" kleur", "((echo@1 PHRASE 2 e@2) OR Zkleur@3)" },
    { "adobe premiere \"-1\"", "((Zadob@1 OR Zpremier@2) OR 1@3)" },
    { "DVD brander \"+\" en \"-\"", "((dvd@1 OR Zbrander@2) OR Zen@3)" },
    { "inspirion \"dvd+R\"", "(Zinspirion@1 OR (dvd@2 PHRASE 2 r@3))" },
    { "asp 0x80040E14)", "(Zasp@1 OR 0x80040e14@2)" },
    { "\"e-tech motorola router", "(e@1 PHRASE 4 tech@2 PHRASE 4 motorola@3 PHRASE 4 router@4)" },
    { "bluetooth '1.3.2.19\"", "(Zbluetooth@1 OR 1.3.2.19@2)" },
    { "ms +-connect", "(Zms@1 OR Zconnect@2)" },
    { "php+print+\"", "(Zphp@1 OR print+@2)" },
    { "athlon 1400 :welke videokaart\"", "((Zathlon@1 OR 1400@2) OR (Zwelk@3 OR videokaart@4))" },
    { "+-dvd", "Zdvd@1" },
    { "glftpd \"-new-\"", "(Zglftpd@1 OR new@2)" },
    { "\"scandisk + dos5.0", "(scandisk@1 PHRASE 2 dos5.0@2)" },
    { "socket\\(\\)", "socket@1" },
    { "msn (e-tech) router", "((Zmsn@1 OR (e@2 PHRASE 2 tech@3)) OR Zrouter@4)" },
    { "Het grote Epox 8k3a+ ervaring/prob topic\"", "(((het@1 OR Zgrote@2 OR epox@3 OR 8k3a+@4) OR (ervaring@5 PHRASE 2 prob@6)) OR topic@7)" },
    { "\"CF+bluetooth\"", "(cf@1 PHRASE 2 bluetooth@2)" },
    { "kwaliteit (s-video) composite verschil tv out", "((Zkwaliteit@1 OR (s@2 PHRASE 2 video@3)) OR (Zcomposit@4 OR Zverschil@5 OR Ztv@6 OR Zout@7))" },
    { "Wie kan deze oude hardware nog gebruiken\" Deel", "((wie@1 OR Zkan@2 OR Zdeze@3 OR Zoud@4 OR Zhardwar@5 OR Znog@6 OR gebruiken@7) OR deel@8)" },
    { "Public Declare Sub Sleep Lib \"kernel32\" (ByVal dwMilliseconds As Long)", "(((public@1 OR declare@2 OR sub@3 OR sleep@4 OR lib@5) OR kernel32@6) OR (byval@7 OR Zdwmillisecond@8 OR as@9 OR long@10))" },
    { "for inclusion (include_path='.:/usr/share/php')", "((Zfor@1 OR Zinclus@2) OR (include_path@3 OR (usr@4 PHRASE 3 share@5 PHRASE 3 php@6)))" },
    { "\"muziek 2x zo snel\"\"", "(muziek@1 PHRASE 4 2x@2 PHRASE 4 zo@3 PHRASE 4 snel@4)" },
    { "execCommand('inserthorizontalrule'", "(execcommand@1 OR Zinserthorizontalrul@2)" },
    { "specs: IBM PS/2, Intel 8086 @ 25 mhz!!, 2 mb intern, 50 mb hd, 5.5\" floppy drive, toetsenbord en geen muis", "((((((((Zspec@1 OR ibm@2) OR (ps@3 PHRASE 2 2@4)) OR (intel@5 OR 8086@6)) OR (25@7 OR Zmhz@8)) OR (2@9 OR Zmb@10 OR Zintern@11)) OR (50@12 OR Zmb@13 OR Zhd@14)) OR 5.5@15) OR (floppy@16 PHRASE 6 drive@17 PHRASE 6 toetsenbord@18 PHRASE 6 en@19 PHRASE 6 geen@20 PHRASE 6 muis@21))" },
    { "History: GetEventTool <- GetMusicManager <- GetMusicScript <- DMCallRoutine <- AMusicScriptEvent::execCallRoutine <- UObject::execClassContext <- (U2GameInfo M08A1.U2GameInfo0 @ Function U2.U2GameInfo.NotifyLevelChangeEnd : 0075 line 744) <- UObject::ProcessEvent <- (U2GameInfo M08A1.U2GameInfo0, Function U2.U2GameInfo.NotifyLevelChangeEnd) <- UGameEngine::LoadMap <- LocalMapURL <- UGameEngine::Browse <- ServerTravel <- UGameEngine::Tick <- UpdateWorld <- MainLoop", "((((((((((((((((history@1 OR geteventtool@2) OR getmusicmanager@3) OR getmusicscript@4) OR dmcallroutine@5) OR (amusicscriptevent@6 PHRASE 2 execcallroutine@7)) OR (uobject@8 PHRASE 2 execclasscontext@9)) OR ((((u2gameinfo@10 OR (m08a1@11 PHRASE 2 u2gameinfo0@12)) OR function@13) OR (u2@14 PHRASE 3 u2gameinfo@15 PHRASE 3 notifylevelchangeend@16)) OR (0075@17 OR Zline@18 OR 744@19))) OR (uobject@20 PHRASE 2 processevent@21)) OR (((u2gameinfo@22 OR (m08a1@23 PHRASE 2 u2gameinfo0@24)) OR function@25) OR (u2@26 PHRASE 3 u2gameinfo@27 PHRASE 3 notifylevelchangeend@28))) OR (ugameengine@29 PHRASE 2 loadmap@30)) OR localmapurl@31) OR (ugameengine@32 PHRASE 2 browse@33)) OR servertravel@34) OR (ugameengine@35 PHRASE 2 tick@36)) OR updateworld@37) OR mainloop@38)" },
    { "Support AMD XP 2400+ & 2600+ (K7T Turbo2 only)", "(((support@1 OR amd@2 OR xp@3 OR 2400+@4) OR 2600+@5) OR (k7t@6 OR turbo2@7 OR Zonli@8))" },
    { "'\"><br>bla</br>", "(br@1 PHRASE 3 bla@2 PHRASE 3 br@3)" },
    { "The instruction at \"0x30053409\" referenced memory at \"0x06460504\". The memory could not be \"read'. Click OK to terminate the application.", "((((((the@1 OR Zinstruct@2 OR Zat@3) OR 0x30053409@4) OR (Zreferenc@5 OR Zmemori@6 OR Zat@7)) OR 0x06460504@8) OR (the@9 OR Zmemori@10 OR Zcould@11 OR Znot@12 OR Zbe@13)) OR (read@14 PHRASE 7 click@15 PHRASE 7 ok@16 PHRASE 7 to@17 PHRASE 7 terminate@18 PHRASE 7 the@19 PHRASE 7 application@20))" },
    { "\"(P5A-b)\"", "(p5a@1 PHRASE 2 b@2)" },
    { "(13,5 > 13) == no-go!", "((13,5@1 OR 13@2) OR (no@3 PHRASE 2 go@4))" },
    { "eth not found \"ifconfig -a\"", "((Zeth@1 OR Znot@2 OR Zfound@3) OR (ifconfig@4 PHRASE 2 a@5))" },
    { "<META NAME=\"ROBOTS", "((meta@1 OR name@2) OR robots@3)" },
    { "lp0: using parport0 (interrupt-driven)", "((Zlp0@1 OR (Zuse@2 OR Zparport0@3)) OR (interrupt@4 PHRASE 2 driven@5))" },
    { "ULTRA PC-TUNING, COOLING & MODDING (4,6)", "((((ultra@1 OR (pc@2 PHRASE 2 tuning@3)) OR cooling@4) OR modding@5) OR 4,6@6)" },
    { "512MB PC2700 DDR SDRAM Rood (Dane-Elec)", "((512mb@1 OR pc2700@2 OR ddr@3 OR sdram@4 OR rood@5) OR (dane@6 PHRASE 2 elec@7))" },
    { "header(\"Content Type: text/html\");", "((header@1 OR (content@2 OR type@3)) OR (text@4 PHRASE 2 html@5))" },
    { "\"-RW\" \"+RW\"", "(rw@1 OR rw@2)" },
    { "\"cresta digital answering machine", "(cresta@1 PHRASE 4 digital@2 PHRASE 4 answering@3 PHRASE 4 machine@4)" },
    { "Arctic Super Silent PRO TC (Athlon/P3 - 2,3 GHz)", "((arctic@1 OR super@2 OR silent@3 OR pro@4 OR tc@5) OR ((athlon@6 PHRASE 2 p3@7) OR (2,3@8 OR ghz@9)))" },
    { "c++ fopen \"r+t\"", "((Zc++@1 OR Zfopen@2) OR (r@3 PHRASE 2 t@4))" },
    { "c++ fopen (r+t)", "((Zc++@1 OR Zfopen@2) OR (Zr@3 OR Zt@4))" },
    { "\"DVD+R\"", "(dvd@1 PHRASE 2 r@2)" },
    { "Class.forName(\"jdbc.odbc.JdbcOdbcDriver\");", "((class@1 PHRASE 2 forname@2) OR (jdbc@3 PHRASE 3 odbc@4 PHRASE 3 jdbcodbcdriver@5))" },
    { "perl(find.pl)", "(perl@1 OR (find@2 PHRASE 2 pl@3))" },
    { "\"-5v\" voeding", "(5v@1 OR Zvoed@2)" },
    { "\"-5v\" power supply", "(5v@1 OR (Zpower@2 OR Zsuppli@3))" },
    { "An Error occurred whie attempting to initialize the Borland Database Engine (error $2108)", "((an@1 OR error@2 OR Zoccur@3 OR Zwhie@4 OR Zattempt@5 OR Zto@6 OR Ziniti@7 OR Zthe@8 OR borland@9 OR database@10 OR engine@11) OR (Zerror@12 OR 2108@13))" },
    { "(error $2108) Borland", "((Zerror@1 OR 2108@2) OR borland@3)" },
    { "On Friday 04 April 2003 09:32, Edwin van Eersel wrote: > ik voel me eigenlijk wel behoorlijk kut :)", "((((on@1 OR friday@2 OR 04@3 OR april@4 OR 2003@5) OR (09@6 PHRASE 2 32@7)) OR (edwin@8 OR Zvan@9 OR eersel@10 OR Zwrote@11)) OR (Zik@12 OR Zvoel@13 OR Zme@14 OR Zeigenlijk@15 OR Zwel@16 OR Zbehoorlijk@17 OR Zkut@18))" },
    { "Elektrotechniek + \"hoe bevalt het?\"\"", "(elektrotechniek@1 OR (hoe@2 PHRASE 3 bevalt@3 PHRASE 3 het@4))" },
    { "Shortcuts in menu (java", "((shortcuts@1 OR Zin@2 OR Zmenu@3) OR Zjava@4)" },
    { "detonator+settings\"", "(Zdeton@1 OR settings@2)" },
    { "(ez-bios) convert", "((ez@1 PHRASE 2 bios@2) OR Zconvert@3)" },
    { "Sparkle 7100M4 64MB (GeForce4 MX440)", "((sparkle@1 OR 7100m4@2 OR 64mb@3) OR (geforce4@4 OR mx440@5))" },
    { "freebsd \"boek OR newbie\"", "(Zfreebsd@1 OR (boek@2 PHRASE 3 or@3 PHRASE 3 newbie@4))" },
    { "for (;;) c++", "(Zfor@1 OR Zc++@2)" },
    { "1700+-2100+", "(1700+@1 PHRASE 2 2100+@2)" },
    { "PHP Warning:  Invalid library (maybe not a PHP library) 'libmysqlclient.so'", "(((php@1 OR warning@2 OR invalid@3 OR Zlibrari@4) OR (Zmayb@5 OR Znot@6 OR Za@7 OR php@8 OR Zlibrari@9)) OR (libmysqlclient@10 PHRASE 2 so@11))" },
    { "NEC DV-5800B (Bul", "((nec@1 OR (dv@2 PHRASE 2 5800b@3)) OR bul@4)" },
    { "org.jdom.input.SAXBuilder.<init>(SAXBuilder.java)", "(((org@1 PHRASE 4 jdom@2 PHRASE 4 input@3 PHRASE 4 saxbuilder@4) OR init@5) OR (saxbuilder@6 PHRASE 2 java@7))" },
    { "AMD Athlon XP 2500+ (1,83GHz, 512KB)", "((amd@1 OR athlon@2 OR xp@3 OR 2500+@4) OR (1,83ghz@5 OR 512kb@6))" },
    { "'q ben\"", "(Zq@1 OR ben@2)" },
    { "getsmbfilepwent: malformed password entry (uid not number)", "((Zgetsmbfilepw@1 OR (Zmalform@2 OR Zpassword@3 OR Zentri@4)) OR (Zuid@5 OR Znot@6 OR Znumber@7))" },
    { "\xc3\xb6ude onderdelen\"", "(Z\xc3\xb6ude@1 OR onderdelen@2)" },
    { "Heeft iemand enig idee waarom de pioneer (zelf met originele firmware van pioneer) bij mij niet wil flashen ??", "(((heeft@1 OR Ziemand@2 OR Zenig@3 OR Zide@4 OR Zwaarom@5 OR Zde@6 OR Zpioneer@7) OR (Zzelf@8 OR Zmet@9 OR Zoriginel@10 OR Zfirmwar@11 OR Zvan@12 OR Zpioneer@13)) OR (Zbij@14 OR Zmij@15 OR Zniet@16 OR Zwil@17 OR Zflashen@18))" },
    { "asus a7v266 bios nieuw -(a7v266-e)", "((Zasus@1 OR Za7v266@2 OR Zbio@3 OR Znieuw@4) AND_NOT (a7v266@5 PHRASE 2 e@6))" },
    { "cybercom \"dvd+r\"", "(Zcybercom@1 OR (dvd@2 PHRASE 2 r@3))" },
    { "AMD PCNET Family Ethernet Adapter (PCI-ISA)", "((amd@1 OR pcnet@2 OR family@3 OR ethernet@4 OR adapter@5) OR (pci@6 PHRASE 2 isa@7))" },
    { "relais +/-", "Zrelai@1" },
    { "formules (slepen OR doortrekken) excel", "((Zformul@1 OR (Zslepen@2 OR Zdoortrekken@3)) OR Zexcel@4)" },
    { "\"%English", "english@1" },
    { "select max( mysql", "((Zselect@1 OR max@2) OR Zmysql@3)" },
    { "leejow(saait", "(leejow@1 OR Zsaait@2)" },
    { "'Windows 2000 Advanced Server\" netwerkverbinding valt steeds weg", "((windows@1 OR 2000@2 OR advanced@3 OR server@4) OR (netwerkverbinding@5 PHRASE 4 valt@6 PHRASE 4 steeds@7 PHRASE 4 weg@8))" },
    { "K7T Turbo 2  (MS-6330)", "((k7t@1 OR turbo@2 OR 2@3) OR (ms@4 PHRASE 2 6330@5))" },
    { "failed to receive data from the client agent. (ec=1)", "((Zfail@1 OR Zto@2 OR Zreceiv@3 OR Zdata@4 OR Zfrom@5 OR Zthe@6 OR Zclient@7 OR Zagent@8) OR (ec@9 OR 1@10))" },
    { "\"cannot find -lz\"", "(cannot@1 PHRASE 3 find@2 PHRASE 3 lz@3)" },
    { "undefined reference to `mysql_drop_db'\"", "((Zundefin@1 OR Zrefer@2 OR Zto@3) OR Zmysql_drop_db@4)" },
    { "search form asp \"%'", "(Zsearch@1 OR Zform@2 OR Zasp@3)" },
    { "(dvd+r) kwaliteit", "((Zdvd@1 OR Zr@2) OR Zkwaliteit@3)" },
    { "Fatal error: Allowed memory size of 8388608 bytes exhausted (tried to allocate 35 bytes)", "((fatal@1 OR Zerror@2 OR allowed@3 OR Zmemori@4 OR Zsize@5 OR Zof@6 OR 8388608@7 OR Zbyte@8 OR Zexhaust@9) OR (Ztri@10 OR Zto@11 OR Zalloc@12 OR 35@13 OR Zbyte@14))" },
    { "geluid (schokt OR hapert)", "(Zgeluid@1 OR (Zschokt@2 OR Zhapert@3))" },
    { "Het wordt pas echt leuk als het hard staat!! >:)", "(het@1 OR Zwordt@2 OR Zpas@3 OR Zecht@4 OR Zleuk@5 OR Zal@6 OR Zhet@7 OR Zhard@8 OR Zstaat@9)" },
    { "Uw configuratie bestand bevat instellingen (root zonder wachtwoord) die betrekking hebben tot de standaard MySQL account. Uw MySQL server draait met deze standaard waardes, en is open voor ongewilde toegang, het wordt dus aangeraden dit op te lossen", "(((((uw@1 OR Zconfigurati@2 OR Zbestand@3 OR Zbevat@4 OR Zinstellingen@5) OR (Zroot@6 OR Zzonder@7 OR Zwachtwoord@8)) OR (Zdie@9 OR Zbetrekk@10 OR Zhebben@11 OR Ztot@12 OR Zde@13 OR Zstandaard@14 OR mysql@15 OR Zaccount@16 OR uw@17 OR mysql@18 OR Zserver@19 OR Zdraait@20 OR Zmet@21 OR Zdeze@22 OR Zstandaard@23 OR Zwaard@24)) OR (Zen@25 OR Zis@26 OR Zopen@27 OR Zvoor@28 OR Zongewild@29 OR Ztoegang@30)) OR (Zhet@31 OR Zwordt@32 OR Zdus@33 OR Zaangeraden@34 OR Zdit@35 OR Zop@36 OR Zte@37 OR Zlossen@38))" },
    { "(library qt-mt) not found", "((Zlibrari@1 OR (qt@2 PHRASE 2 mt@3)) OR (Znot@4 OR Zfound@5))" },
    { "Qt (>= Qt 3.0.3) (library qt-mt) not found", "(((qt@1 OR (qt@2 OR 3.0.3@3)) OR (Zlibrari@4 OR (qt@5 PHRASE 2 mt@6))) OR (Znot@7 OR Zfound@8))" },
    { "setup was unable to find (or could not read) the language specific setup resource dll, unable to continue. Please reboot and try again.", "((((Zsetup@1 OR Zwas@2 OR Zunabl@3 OR Zto@4 OR Zfind@5) OR (Zor@6 OR Zcould@7 OR Znot@8 OR Zread@9)) OR (Zthe@10 OR Zlanguag@11 OR Zspecif@12 OR Zsetup@13 OR Zresourc@14 OR Zdll@15)) OR (Zunabl@16 OR Zto@17 OR Zcontinu@18 OR please@19 OR Zreboot@20 OR Zand@21 OR Ztri@22 OR Zagain@23))" },
    { "Titan TTC-D5TB(4/CU35)", "((titan@1 OR (ttc@2 PHRASE 2 d5tb@3)) OR (4@4 PHRASE 2 cu35@5))" },
    { "[php] date( min", "((Zphp@1 OR date@2) OR Zmin@3)" },
    { "EPOX EP-8RDA+ (nForce2 SPP+MCP-T) Rev. 1.1", "((((epox@1 OR (ep@2 PHRASE 2 8rda+@3)) OR ((Znforce2@4 OR spp@5) OR (mcp@6 PHRASE 2 t@7))) OR rev@8) OR 1.1@9)" },
    { "554 5.4.6 Too many hops 53 (25 max)", "((554@1 OR 5.4.6@2 OR too@3 OR Zmani@4 OR Zhop@5 OR 53@6) OR (25@7 OR Zmax@8))" },
    { "ik had toch nog een vraagje: er zijn nu eigenlijk alleen maar schijfjes van 4.7GB alleen straks zullen er vast schijfjes van meer dan 4.7GB komen. Zal deze brander dit wel kunnen schijven?""?(na bijvoorbeeld een firmware update?) ben erg benieuwd", "(((Zik@1 OR Zhad@2 OR Ztoch@3 OR Znog@4 OR Zeen@5 OR Zvraagj@6 OR Zer@7 OR Zzijn@8 OR Znu@9 OR Zeigenlijk@10 OR Zalleen@11 OR Zmaar@12 OR Zschijfj@13 OR Zvan@14 OR 4.7gb@15 OR Zalleen@16 OR Zstrak@17 OR Zzullen@18 OR Zer@19 OR Zvast@20 OR Zschijfj@21 OR Zvan@22 OR Zmeer@23 OR Zdan@24 OR 4.7gb@25 OR Zkomen@26 OR zal@27 OR Zdeze@28 OR Zbrander@29 OR Zdit@30 OR Zwel@31 OR Zkunnen@32 OR Zschijven@33) OR (Zna@34 OR Zbijvoorbeeld@35 OR Zeen@36 OR Zfirmwar@37 OR Zupdat@38)) OR (Zben@39 OR Zerg@40 OR Zbenieuwd@41))" },
    { "ati linux drivers (4.3.0)", "((Zati@1 OR Zlinux@2 OR Zdriver@3) OR 4.3.0@4)" },
    { "ENCAPSED_AND_WHITESPACE", "encapsed_and_whitespace@1" },
    { "lpadmin: add-printer (set device) failed: client-error-not-possible", "((((Zlpadmin@1 OR (add@2 PHRASE 2 printer@3)) OR (Zset@4 OR Zdevic@5)) OR Zfail@6) OR (client@7 PHRASE 4 error@8 PHRASE 4 not@9 PHRASE 4 possible@10))" },
    { "welke dvd \"+r\" media", "(((Zwelk@1 OR Zdvd@2) OR r@3) OR Zmedia@4)" },
    { "Warning: stat failed for fotos(errno=2 - No such file or directory)", "((((warning@1 OR (Zstat@2 OR Zfail@3 OR Zfor@4 OR fotos@5)) OR errno@6) OR 2@7) OR (no@8 OR Zsuch@9 OR Zfile@10 OR Zor@11 OR Zdirectori@12))" },
    { "dvd +/-", "Zdvd@1" },
    { "7vaxp +voltage mod\"", "(Zvoltag@2 AND_MAYBE (7vaxp@1 OR mod@3))" },
    { "lpt port (SPP/EPP) is enabled", "(((Zlpt@1 OR Zport@2) OR (spp@3 PHRASE 2 epp@4)) OR (Zis@5 OR Zenabl@6))" },
    { "getenv(\"HTTP_REFERER\")", "(getenv@1 OR http_referer@2)" },
    { "Error setting display mode: CreateDevice failed (D3DERR_DRIVERINTERNALERROR)", "((error@1 OR Zset@2 OR Zdisplay@3 OR Zmode@4 OR createdevice@5 OR Zfail@6) OR d3derr_driverinternalerror@7)" },
    { "Exception number: c0000005 (access violation)", "((exception@1 OR Znumber@2 OR Zc0000005@3) OR (Zaccess@4 OR Zviolat@5))" },
    { "header(\"Content-type:application/octetstream\");", "(header@1 OR (content@2 PHRASE 4 type@3 PHRASE 4 application@4 PHRASE 4 octetstream@5))" },
    { "java.security.AccessControlException: access denied (java.lang.RuntimePermission accessClassInPackage.sun.jdbc.odbc)", "(((java@1 PHRASE 3 security@2 PHRASE 3 accesscontrolexception@3) OR (Zaccess@4 OR Zdeni@5)) OR ((java@6 PHRASE 3 lang@7 PHRASE 3 runtimepermission@8) OR (accessclassinpackage@9 PHRASE 4 sun@10 PHRASE 4 jdbc@11 PHRASE 4 odbc@12)))" },
    { "(001.part.met", "(001@1 PHRASE 3 part@2 PHRASE 3 met@3)" },
    { "Warning: mail(): Use the -f option (5th param) to include valid reply-to address ! in /usr/home/vdb/www/mail.php on line 79", "((((((((((warning@1 OR mail@2) OR (use@3 OR Zthe@4)) OR (Zf@5 OR Zoption@6)) OR (5th@7 OR Zparam@8)) OR (Zto@9 OR Zinclud@10 OR Zvalid@11)) OR (reply@12 PHRASE 2 to@13)) OR Zaddress@14) OR Zin@15) OR (usr@16 PHRASE 6 home@17 PHRASE 6 vdb@18 PHRASE 6 www@19 PHRASE 6 mail@20 PHRASE 6 php@21)) OR (Zon@22 OR Zline@23 OR 79@24))" },
    { "PHP Use the -f option (5th param)", "((((php@1 OR use@2 OR Zthe@3) OR Zoption@5) OR (5th@6 OR Zparam@7)) AND_NOT Zf@4)" },
    { "dvd \"+\" \"-\"", "Zdvd@1" },
    { "bericht  ( %)", "Zbericht@1" },
    { "2500+ of 2600+ (niett OC)", "((2500+@1 OR Zof@2 OR 2600+@3) OR (Zniett@4 OR oc@5))" },
    { "maxtor windows xp werkt The drivers for this device are not installed. (Code 28)", "((Zmaxtor@1 OR Zwindow@2 OR Zxp@3 OR Zwerkt@4 OR the@5 OR Zdriver@6 OR Zfor@7 OR Zthis@8 OR Zdevic@9 OR Zare@10 OR Znot@11 OR Zinstal@12) OR (code@13 OR 28@14))" },
    { "Warning: stat failed for /mnt/web/react/got/react/board/non-www/headlines/tnet-headlines.txt (errno=2 - No such file or directory) in /mnt/web/react/got/react/global/non-www/templates/got/functions.inc.php on line 303", "((((((warning@1 OR (Zstat@2 OR Zfail@3 OR Zfor@4)) OR (mnt@5 PHRASE 12 web@6 PHRASE 12 react@7 PHRASE 12 got@8 PHRASE 12 react@9 PHRASE 12 board@10 PHRASE 12 non@11 PHRASE 12 www@12 PHRASE 12 headlines@13 PHRASE 12 tnet@14 PHRASE 12 headlines@15 PHRASE 12 txt@16)) OR ((errno@17 OR 2@18) OR (no@19 OR Zsuch@20 OR Zfile@21 OR Zor@22 OR Zdirectori@23))) OR Zin@24) OR (mnt@25 PHRASE 13 web@26 PHRASE 13 react@27 PHRASE 13 got@28 PHRASE 13 react@29 PHRASE 13 global@30 PHRASE 13 non@31 PHRASE 13 www@32 PHRASE 13 templates@33 PHRASE 13 got@34 PHRASE 13 functions@35 PHRASE 13 inc@36 PHRASE 13 php@37)) OR (Zon@38 OR Zline@39 OR 303@40))" },
    { "apm: BIOS version 1.2 Flags 0x03 (Driver version 1.16)", "((Zapm@1 OR (bios@2 OR Zversion@3 OR 1.2@4 OR flags@5 OR 0x03@6)) OR (driver@7 OR Zversion@8 OR 1.16@9))" },
    { "GA-8IHXP(3.0)", "((ga@1 PHRASE 2 8ihxp@2) OR 3.0@3)" },
    { "8IHXP(3.0)", "(8ihxp@1 OR 3.0@2)" },
    { "na\xc2\xb7si (de ~ (m.))", "(Zna\xc2\xb7si@1 OR (Zde@2 OR Zm@3))" },
    { "header(\"Content-Disposition: attachment;", "(header@1 OR (content@2 PHRASE 3 disposition@3 PHRASE 3 attachment@4))" },
    { "\"header(\"Content-Disposition: attachment;\"", "((header@1 OR (content@2 PHRASE 2 disposition@3)) OR Zattach@4)" },
    { "\"Beep -f\"", "(beep@1 PHRASE 2 f@2)" },
    { "kraan NEAR (Elektrisch OR Electrisch)", "((Zkraan@1 OR near@2) OR (elektrisch@3 OR or@4 OR electrisch@5))" },
    { "checking for Qt... configure: error: Qt (>= Qt 3.0.2) (headers and libraries) not found. Please check your installation!", "((((Zcheck@1 OR Zfor@2 OR qt@3 OR Zconfigur@4 OR Zerror@5 OR qt@6) OR (qt@7 OR 3.0.2@8)) OR (Zheader@9 OR Zand@10 OR Zlibrari@11)) OR (Znot@12 OR Zfound@13 OR please@14 OR Zcheck@15 OR Zyour@16 OR Zinstal@17))" },
    { "parse error, unexpected '\\\"', expecting T_STRING or T_VARIABLE or T_NUM_STRING", "(((Zpars@1 OR Zerror@2) OR Zunexpect@3) OR (expecting@4 PHRASE 6 t_string@5 PHRASE 6 or@6 PHRASE 6 t_variable@7 PHRASE 6 or@8 PHRASE 6 t_num_string@9))" },
    { "ac3 (0x2000) \"Dolby Laboratories,", "((Zac3@1 OR 0x2000@2) OR (dolby@3 PHRASE 2 laboratories@4))" },
    { "Movie.FileName=(\"../../../~animations/\"+lesson1.recordset.fields('column3')+\"Intro.avi\")", "(((((movie@1 PHRASE 2 filename@2) OR animations@3) OR (lesson1@4 PHRASE 3 recordset@5 PHRASE 3 fields@6)) OR Zcolumn3@7) OR (intro@8 PHRASE 2 avi@9))" },
    { "502 Permission Denied - Permission Denied - news.chello.nl -- http://www.chello.nl/ (Typhoon v1.2.3)", "(((((502@1 OR permission@2 OR denied@3) OR (permission@4 OR denied@5)) OR (news@6 PHRASE 3 chello@7 PHRASE 3 nl@8)) OR (http@9 PHRASE 4 www@10 PHRASE 4 chello@11 PHRASE 4 nl@12)) OR (typhoon@13 OR Zv1.2.3@14))" },
    { "Motion JPEG (MJPEG codec)", "((motion@1 OR jpeg@2) OR (mjpeg@3 OR Zcodec@4))" },
    { ": zoomtext\"", "zoomtext@1" },
    { "Your SORT command does not seem to support the \"-r -n -k 7\"", "((your@1 OR sort@2 OR Zcommand@3 OR Zdoe@4 OR Znot@5 OR Zseem@6 OR Zto@7 OR Zsupport@8 OR Zthe@9) OR (r@10 PHRASE 4 n@11 PHRASE 4 k@12 PHRASE 4 7@13))" },
    { "Geef de naam van de MSDOS prompt op C:\\\\WINDOWS.COM\\\"", "((geef@1 OR Zde@2 OR Znaam@3 OR Zvan@4 OR Zde@5 OR msdos@6 OR Zprompt@7 OR Zop@8) OR (c@9 PHRASE 3 windows@10 PHRASE 3 com@11))" },
    { "\"\"wa is fase\"", "(Zwa@1 OR Zis@2 OR fase@3)" },
    { "<v:imagedata src=\"", "((v@1 PHRASE 2 imagedata@2) OR src@3)" },
    { "system(play ringin.wav); ?>", "((system@1 OR Zplay@2) OR (ringin@3 PHRASE 2 wav@4))" },
    { "\"perfect NEAR systems\"", "(perfect@1 PHRASE 3 near@2 PHRASE 3 systems@3)" },
    { "LoadLibrary(\"mainta/gamex86.dll\") failed", "((loadlibrary@1 OR (mainta@2 PHRASE 3 gamex86@3 PHRASE 3 dll@4)) OR Zfail@5)" },
    { "DATE_FORMAT('1997-10-04 22:23:00', '%W %M %Y');", "(((((date_format@1 OR (1997@2 PHRASE 3 10@3 PHRASE 3 04@4)) OR (22@5 PHRASE 3 23@6 PHRASE 3 00@7)) OR w@8) OR m@9) OR y@10)" },
    { "secundaire IDE-controller (dubbele fifo)", "((Zsecundair@1 OR (ide@2 PHRASE 2 controller@3)) OR (Zdubbel@4 OR Zfifo@5))" },
    { "\"Postal2+Explorer.exe\"", "(postal2@1 PHRASE 3 explorer@2 PHRASE 3 exe@3)" },
    { "COUNT(*)", "count@1" },
    { "Nuttige Windows progs   (1/11)", "((nuttige@1 OR windows@2 OR Zprog@3) OR (1@4 PHRASE 2 11@5))" },
    { "if(usercode==passcode==)", "((if@1 OR usercode@2) OR passcode@3)" },
    { "lg 8160b (dvd+r)", "((Zlg@1 OR 8160b@2) OR (Zdvd@3 OR Zr@4))" },
    { "iPAQ Pocket PC 2002 End User Update (EUU - Service Pack)", "((Zipaq@1 OR pocket@2 OR pc@3 OR 2002@4 OR end@5 OR user@6 OR update@7) OR (euu@8 OR (service@9 OR pack@10)))" },
    { "'ipod pakt tags niet\"", "(Zipod@1 OR Zpakt@2 OR Ztag@3 OR niet@4)" },
    { "\"DVD+/-R\"", "(dvd+@1 PHRASE 2 r@2)" },
    { "\"DVD+R DVD-R\"", "(dvd@1 PHRASE 4 r@2 PHRASE 4 dvd@3 PHRASE 4 r@4)" },
    { "php ;)  in een array zetten", "(Zphp@1 OR (Zin@2 OR Zeen@3 OR Zarray@4 OR Zzetten@5))" },
    { "De inhoud van uw advertentie is niet geschikt voor plaatsing op marktplaats! (001", "((de@1 OR Zinhoud@2 OR Zvan@3 OR Zuw@4 OR Zadvertenti@5 OR Zis@6 OR Zniet@7 OR Zgeschikt@8 OR Zvoor@9 OR Zplaats@10 OR Zop@11 OR Zmarktplaat@12) OR 001@13)" },
    { "creative (soundblaster OR sb) 128", "((Zcreativ@1 OR (Zsoundblast@2 OR Zsb@3)) OR 128@4)" },
    { "Can't open file: (errno: 145)", "((can't@1 OR Zopen@2 OR Zfile@3) OR (Zerrno@4 OR 145@5))" },
    { "Formateren lukt niet(98,XP)", "(((formateren@1 OR Zlukt@2 OR niet@3) OR 98@4) OR xp@5)" },
    { "access denied (java.io.", "((Zaccess@1 OR Zdeni@2) OR (java@3 PHRASE 2 io@4))" },
    { "(access denied (java.io.)", "((Zaccess@1 OR Zdeni@2) OR (java@3 PHRASE 2 io@4))" },
    { "wil niet installeren ( crc fouten)", "((Zwil@1 OR Zniet@2 OR Zinstalleren@3) OR (Zcrc@4 OR Zfouten@5))" },
    { "(DVD+RW) brandsoftware meerdere", "((dvd@1 OR rw@2) OR (Zbrandsoftwar@3 OR Zmeerder@4))" },
    { "(database OF databases) EN geheugen", "((Zdatabas@1 OR of@2 OR Zdatabas@3) OR (en@4 OR Zgeheugen@5))" },
    { "(server 2003) winroute", "((Zserver@1 OR 2003@2) OR Zwinrout@3)" },
    { "54MHz (kanaal 2 VHF) tot tenminste 806 MHz (kanaal 69 UHF)", "(((54mhz@1 OR (Zkanaal@2 OR 2@3 OR vhf@4)) OR (Ztot@5 OR Ztenminst@6 OR 806@7 OR mhz@8)) OR (Zkanaal@9 OR 69@10 OR uhf@11))" },
    { "(draadloos OR wireless) netwerk", "((Zdraadloo@1 OR Zwireless@2) OR Znetwerk@3)" },
    { "localtime(time(NULL));", "((localtime@1 OR time@2) OR null@3)" },
    { "ob_start(\"ob_gzhandler\");", "(ob_start@1 OR ob_gzhandler@2)" },
    { "PPP Closed : LCP Time-out (VPN-0)", "((((ppp@1 OR closed@2) OR lcp@3) OR (time@4 PHRASE 2 out@5)) OR (vpn@6 PHRASE 2 0@7))" },
    { "COM+-gebeurtenissysteem", "(com+@1 PHRASE 2 gebeurtenissysteem@2)" },
    { "rcpthosts (#5.7.1)", "(Zrcpthost@1 OR 5.7.1@2)" },
    { "Dit apparaat werkt niet goed omdat Windows de voor dit apparaat vereiste stuurprogramma's niet kan laden.  (Code 31)", "((dit@1 OR Zapparaat@2 OR Zwerkt@3 OR Zniet@4 OR Zgo@5 OR Zomdat@6 OR windows@7 OR Zde@8 OR Zvoor@9 OR Zdit@10 OR Zapparaat@11 OR Zvereist@12 OR Zstuurprogramma@13 OR Zniet@14 OR Zkan@15 OR Zladen@16) OR (code@17 OR 31@18))" },
    { "window.open( scrollbar", "((window@1 PHRASE 2 open@2) OR Zscrollbar@3)" },
    { "T68i truc ->", "(t68i@1 OR Ztruc@2)" },
    { "T68i ->", "t68i@1" },
    { "\"de lijn is bezet\"\"", "(de@1 PHRASE 4 lijn@2 PHRASE 4 is@3 PHRASE 4 bezet@4)" },
    { "if (eregi(\"", "(Zif@1 OR eregi@2)" },
    { "This device is not working properly because Windows cannot load the drivers required for this device. (Code 31)", "((this@1 OR Zdevic@2 OR Zis@3 OR Znot@4 OR Zwork@5 OR Zproper@6 OR Zbecaus@7 OR windows@8 OR Zcannot@9 OR Zload@10 OR Zthe@11 OR Zdriver@12 OR Zrequir@13 OR Zfor@14 OR Zthis@15 OR Zdevic@16) OR (code@17 OR 31@18))" },
    { "execCommand(\"Paste\");", "(execcommand@1 OR paste@2)" },
    { "\"-1 unread\"", "(1@1 PHRASE 2 unread@2)" },
    { "\"www.historical-fire-engines", "(www@1 PHRASE 4 historical@2 PHRASE 4 fire@3 PHRASE 4 engines@4)" },
    { "\"DVD+RW\" erase", "((dvd@1 PHRASE 2 rw@2) OR Zeras@3)" },
    { "[showjekamer)", "Zshowjekam@1" },
    { "The description for Event ID  1  in Source  True Vector Engine ) cannot be found. The local computer may not have the necessary registry information or message DLL files to display messages from a remote computer. You may be able to use the /AUXSOURC", "(((the@1 OR Zdescript@2 OR Zfor@3 OR event@4 OR id@5 OR 1@6 OR Zin@7 OR source@8 OR true@9 OR vector@10 OR engine@11) OR (Zcannot@12 OR Zbe@13 OR Zfound@14 OR the@15 OR Zlocal@16 OR Zcomput@17 OR Zmay@18 OR Znot@19 OR Zhave@20 OR Zthe@21 OR Znecessari@22 OR Zregistri@23 OR Zinform@24 OR Zor@25 OR Zmessag@26 OR dll@27 OR Zfile@28 OR Zto@29 OR Zdisplay@30 OR Zmessag@31 OR Zfrom@32 OR Za@33 OR Zremot@34 OR Zcomput@35 OR you@36 OR Zmay@37 OR Zbe@38 OR Zabl@39 OR Zto@40 OR Zuse@41 OR Zthe@42)) OR auxsourc@43)" },
    { "org.apache.jasper.JasperException: This absolute uri (http://java.sun.com/jstl/core) cannot be resolved in either web.xml or the jar files deployed with this application", "((((((org@1 PHRASE 4 apache@2 PHRASE 4 jasper@3 PHRASE 4 jasperexception@4) OR (this@5 OR Zabsolut@6 OR Zuri@7)) OR (http@8 PHRASE 6 java@9 PHRASE 6 sun@10 PHRASE 6 com@11 PHRASE 6 jstl@12 PHRASE 6 core@13)) OR (Zcannot@14 OR Zbe@15 OR Zresolv@16 OR Zin@17 OR Zeither@18)) OR (web@19 PHRASE 2 xml@20)) OR (Zor@21 OR Zthe@22 OR Zjar@23 OR Zfile@24 OR Zdeploy@25 OR Zwith@26 OR Zthis@27 OR Zapplic@28))" },
    { "This absolute uri (http://java.sun.com/jstl/core) cannot be resolved in either web.xml or the jar files deployed with this application", "(((((this@1 OR Zabsolut@2 OR Zuri@3) OR (http@4 PHRASE 6 java@5 PHRASE 6 sun@6 PHRASE 6 com@7 PHRASE 6 jstl@8 PHRASE 6 core@9)) OR (Zcannot@10 OR Zbe@11 OR Zresolv@12 OR Zin@13 OR Zeither@14)) OR (web@15 PHRASE 2 xml@16)) OR (Zor@17 OR Zthe@18 OR Zjar@19 OR Zfile@20 OR Zdeploy@21 OR Zwith@22 OR Zthis@23 OR Zapplic@24))" },
    { "vervangen # \"/", "Zvervangen@1" },
    { "vervangen # /\"", "Zvervangen@1" },
    { "while(list($key, $val) = each($HTTP_POST_VARS))", "(((((while@1 OR list@2) OR Zkey@3) OR Zval@4) OR each@5) OR http_post_vars@6)" },
    { "PowerDVD does not support the current display mode. (DDraw Overlay mode is recommended)", "((powerdvd@1 OR Zdoe@2 OR Znot@3 OR Zsupport@4 OR Zthe@5 OR Zcurrent@6 OR Zdisplay@7 OR Zmode@8) OR (ddraw@9 OR overlay@10 OR Zmode@11 OR Zis@12 OR Zrecommend@13))" },
    { "Warning:  Unexpected character in input:  '' (ASCII=92) state=1  highlight", "((((warning@1 OR (unexpected@2 OR Zcharact@3 OR Zin@4 OR Zinput@5)) OR (ascii@6 OR 92@7)) OR state@8) OR (1@9 OR Zhighlight@10))" },
    { "error: Qt-1.4 (headers and libraries) not found. Please check your installation!", "(((Zerror@1 OR (qt@2 PHRASE 2 1.4@3)) OR (Zheader@4 OR Zand@5 OR Zlibrari@6)) OR (Znot@7 OR Zfound@8 OR please@9 OR Zcheck@10 OR Zyour@11 OR Zinstal@12))" },
    { "Error while initializing the sound driver: device /dev/dsp can't be opened (No such device) The sound server will continue, using the null output device.", "((((((error@1 OR Zwhile@2 OR Ziniti@3 OR Zthe@4 OR Zsound@5 OR Zdriver@6 OR Zdevic@7) OR (dev@8 PHRASE 2 dsp@9)) OR (Zcan't@10 OR Zbe@11 OR Zopen@12)) OR (no@13 OR Zsuch@14 OR Zdevic@15)) OR (the@16 OR Zsound@17 OR Zserver@18 OR Zwill@19 OR Zcontinu@20)) OR (Zuse@21 OR Zthe@22 OR Znull@23 OR Zoutput@24 OR Zdevic@25))" },
    { "mag mijn waarschuwing nu weg ? ;)", "(Zmag@1 OR Zmijn@2 OR Zwaarschuw@3 OR Znu@4 OR Zweg@5)" },
    { "Abit NF7-S (nForce 2 Chipset) Rev 2.0", "(((abit@1 OR (nf7@2 PHRASE 2 s@3)) OR (Znforc@4 OR 2@5 OR chipset@6)) OR (rev@7 OR 2.0@8))" },
    { "Setup Could Not Verify the Integrity of the File\" Error Message Occurs When You Try to Install Windows XP Service Pack 1", "((setup@1 OR could@2 OR not@3 OR verify@4 OR Zthe@5 OR integrity@6 OR Zof@7 OR Zthe@8 OR file@9) OR (error@10 PHRASE 13 message@11 PHRASE 13 occurs@12 PHRASE 13 when@13 PHRASE 13 you@14 PHRASE 13 try@15 PHRASE 13 to@16 PHRASE 13 install@17 PHRASE 13 windows@18 PHRASE 13 xp@19 PHRASE 13 service@20 PHRASE 13 pack@21 PHRASE 13 1@22))" },
    { "(browser 19) citrix", "((Zbrowser@1 OR 19@2) OR Zcitrix@3)" },
    { "preg_replace (.*?)", "Zpreg_replac@1" },
    { "formule excel #naam\"?\"", "((Zformul@1 OR Zexcel@2) OR naam@3)" },
    { "->", "" },
    { "De instructie op 0x77f436f7 verwijst naar geheugen op 0x007f4778. De lees-of schrijfbewerking (\"written\") op het geheugen is mislukt", "(((((de@1 OR Zinstructi@2 OR Zop@3 OR 0x77f436f7@4 OR Zverwijst@5 OR Znaar@6 OR Zgeheugen@7 OR Zop@8 OR 0x007f4778@9 OR de@10) OR (lees@11 PHRASE 2 of@12)) OR Zschrijfbewerk@13) OR written@14) OR (Zop@15 OR Zhet@16 OR Zgeheugen@17 OR Zis@18 OR Zmislukt@19))" },
    { "<iframe src=\"www.tweakers.net></iframe>", "((Zifram@1 OR src@2) OR (www@3 PHRASE 4 tweakers@4 PHRASE 4 net@5 PHRASE 4 iframe@6))" },
    { "\"rpm -e httpd\"", "(rpm@1 PHRASE 3 e@2 PHRASE 3 httpd@3)" },
    { "automatisch op All Flis (*.*)", "(Zautomatisch@1 OR Zop@2 OR all@3 OR flis@4)" },
    { "(Windows; U; Windows NT 5.1; en-US; rv:1.3b) Gecko/20030210", "(((((windows@1 OR u@2) OR (windows@3 OR nt@4 OR 5.1@5)) OR (en@6 PHRASE 2 us@7)) OR (rv@8 PHRASE 2 1.3b@9)) OR (gecko@10 PHRASE 2 20030210@11))" },
    { "en-US; rv:1.3b) Gecko/20030210", "(((en@1 PHRASE 2 us@2) OR (rv@3 PHRASE 2 1.3b@4)) OR (gecko@5 PHRASE 2 20030210@6))" },
    { "\"en-US; rv:1.3b) Gecko/20030210\"", "(en@1 PHRASE 6 us@2 PHRASE 6 rv@3 PHRASE 6 1.3b@4 PHRASE 6 gecko@5 PHRASE 6 20030210@6)" },
    { "(./) chmod.sh", "(chmod@1 PHRASE 2 sh@2)" },
    { "document.write(ssg(\" html", "(((document@1 PHRASE 2 write@2) OR ssg@3) OR html@4)" },
    { "superstack \"mac+adressen\"", "(Zsuperstack@1 OR (mac@2 PHRASE 2 adressen@3))" },
    { "IIS getenv(REMOTE_HOST)_", "(((iis@1 OR getenv@2) OR remote_host@3) OR _@4)" },
    { "IIS en getenv(REMOTE_HOST)", "((iis@1 OR Zen@2 OR getenv@3) OR remote_host@4)" },
    { "php getenv(\"HTTP_REFERER\")", "((Zphp@1 OR getenv@2) OR http_referer@3)" },
    { "nec+-1300", "(nec+@1 PHRASE 2 1300@2)" },
    { "smbpasswd script \"-s\"", "((Zsmbpasswd@1 OR Zscript@2) OR s@3)" },
    { "leestekens \" \xc3\xb6 \xc3\xab", "(Zleesteken@1 OR (\xc3\xb6@2 PHRASE 2 \xc3\xab@3))" },
    { "freesco and (all seeing eye)", "((Zfreesco@1 OR Zand@2) OR (Zall@3 OR Zsee@4 OR Zeye@5))" },
    { "('all seeing eye') and freesco", "((Zall@1 OR Zsee@2 OR Zeye@3) OR (Zand@4 OR Zfreesco@5))" },
    { "\"[......\"", "" },
    { "Error = 11004 (500 No Data (Winsock error #11004))", "((error@1 OR 11004@2) OR ((500@3 OR no@4 OR data@5) OR ((winsock@6 OR Zerror@7) OR 11004@8)))" },
    { "gegevensfout (cyclishe redundantiecontrole)", "(Zgegevensfout@1 OR (Zcyclish@2 OR Zredundantiecontrol@3))" },
    { "firmware versie waar NEC\"", "(Zfirmwar@1 OR Zversi@2 OR Zwaar@3 OR nec@4)" },
    { "nu.nl \"-1\"", "((nu@1 PHRASE 2 nl@2) OR 1@3)" },
    { "provider+-webspace", "(provider+@1 PHRASE 2 webspace@2)" },
    { "verschil \"dvd+rw\" \"dvd-rw\"", "((Zverschil@1 OR (dvd@2 PHRASE 2 rw@3)) OR (dvd@4 PHRASE 2 rw@5))" },
    { "(dhcp client) + hangt", "((Zdhcp@1 OR Zclient@2) OR Zhangt@3)" },
    { "MSI 875P Neo-FIS2R (Intel 875P)", "(((msi@1 OR 875p@2) OR (neo@3 PHRASE 2 fis2r@4)) OR (intel@5 OR 875p@6))" },
    { "voeding passief gekoeld\"", "(Zvoed@1 OR Zpassief@2 OR gekoeld@3)" },
    { "if (mysql_num_rows($resultaat)==1)", "(((Zif@1 OR mysql_num_rows@2) OR Zresultaat@3) OR 1@4)" },
    { "Server.CreateObject(\"Persits.Upload.1\")", "((server@1 PHRASE 2 createobject@2) OR (persits@3 PHRASE 3 upload@4 PHRASE 3 1@5))" },
    { "if(cod>9999999)cod=parseInt(cod/64)", "(((((if@1 OR cod@2) OR 9999999@3) OR cod@4) OR parseint@5) OR (cod@6 PHRASE 2 64@7))" },
    { "if (cod>9999999", "(Zif@1 OR (cod@2 OR 9999999@3))" },
    { "\"rm -rf /bin/laden\"", "(rm@1 PHRASE 4 rf@2 PHRASE 4 bin@3 PHRASE 4 laden@4)" },
    { "\">>> 0) & 0xFF\"", "(0@1 PHRASE 2 0xff@2)" },
    { "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01//EN\"> document.body.scrollHeight", "(((doctype@1 OR html@2 OR public@3) OR (w3c@4 PHRASE 5 dtd@5 PHRASE 5 html@6 PHRASE 5 4.01@7 PHRASE 5 en@8)) OR (document@9 PHRASE 3 body@10 PHRASE 3 scrollheight@11))" },
    { "<BR>window.resizeBy(offsetX,offsetY)<P>kweet", "(((((br@1 OR (window@2 PHRASE 2 resizeby@3)) OR Zoffsetx@4) OR Zoffseti@5) OR p@6) OR Zkweet@7)" },
    { "linux humor :)", "(Zlinux@1 OR Zhumor@2)" },
    { "ClassFactory kan aangevraagde klasse niet leveren  (Fout=80040111)", "((classfactory@1 OR Zkan@2 OR Zaangevraagd@3 OR Zklass@4 OR Zniet@5 OR Zleveren@6) OR (fout@7 OR 80040111@8))" },
    { "remote_smtp defer (-44)", "((Zremote_smtp@1 OR Zdefer@2) OR 44@3)" },
    { "txtlogin.getText().trim().toUpperCase().intern() == inuser[2 * (i - 1) + 2].trim().toUpperCase().intern() && txtpass.getText().trim().toUpperCase().intern() == inuser[2 * (i - 1) + 3].trim().toUpperCase().intern())", "((((((((((((((((((((((((txtlogin@1 PHRASE 2 gettext@2) OR trim@3) OR touppercase@4) OR intern@5) OR inuser@6) OR 2@7) OR Zi@8) OR 1@9) OR 2@10) OR trim@11) OR touppercase@12) OR intern@13) OR (txtpass@14 PHRASE 2 gettext@15)) OR trim@16) OR touppercase@17) OR intern@18) OR inuser@19) OR 2@20) OR Zi@21) OR 1@22) OR 3@23) OR trim@24) OR touppercase@25) OR intern@26)" },
    { "Koper + amoniak (NH2", "((koper@1 OR Zamoniak@2) OR nh2@3)" },
    { "nec dvd -/+r", "((Znec@1 OR Zdvd@2) AND_NOT Zr@3)" },
    { "er is een gereserveerde fout (-1104) opgetreden", "(((Zer@1 OR Zis@2 OR Zeen@3 OR Zgereserveerd@4 OR Zfout@5) OR 1104@6) OR Zopgetreden@7)" },
    { "Cor \\(CCN\\)'\" <cor.kloet@ccn.controlec.nl>", "((cor@1 OR ccn@2) OR (cor@3 PHRASE 5 kloet@4 PHRASE 5 ccn@5 PHRASE 5 controlec@6 PHRASE 5 nl@7))" },
    { "Warning: Failed opening for inclusion (include_path='') in Unknown on line 0", "(((warning@1 OR (failed@2 OR Zopen@3 OR Zfor@4 OR Zinclus@5)) OR include_path@6) OR (Zin@7 OR unknown@8 OR Zon@9 OR Zline@10 OR 0@11))" },
    { "\"~\" + \"c:\\\"", "Zc@1" },
    { "mysql count(*)", "(Zmysql@1 OR count@2)" },
    { "for %f in (*.*) do", "((Zfor@1 OR (Zf@2 OR Zin@3)) OR Zdo@4)" },
    { "raar \"~\" bestand", "(Zraar@1 OR Zbestand@2)" },
    { "NEC DVD +-R/RW 1300", "(((nec@1 OR dvd@2) OR (r@3 PHRASE 2 rw@4)) OR 1300@5)" },
    { "approved (ref: 38446-263)", "(Zapprov@1 OR (Zref@2 OR (38446@3 PHRASE 2 263@4)))" },
    { "GA-7VRXP(2.0)", "((ga@1 PHRASE 2 7vrxp@2) OR 2.0@3)" },
    { "~ Could not retrieve directory listing for \"/\"", "(could@1 OR Znot@2 OR Zretriev@3 OR Zdirectori@4 OR Zlist@5 OR Zfor@6)" },
    { "asp CreateObject(\"Word.Document\")", "((Zasp@1 OR createobject@2) OR (word@3 PHRASE 2 document@4))" },
    { "De lees- of schrijfbewerking (\"written\") op het geheugen is mislukt.", "(((de@1 OR Zlee@2 OR Zof@3 OR Zschrijfbewerk@4) OR written@5) OR (Zop@6 OR Zhet@7 OR Zgeheugen@8 OR Zis@9 OR Zmislukt@10))" },
    { "putStr (map (\\x -> chr (round (21/2 * x^3 - 92 * x^2 + 503/2 * x - 105))) [1..4])", "(Zputstr@1 OR ((Zmap@2 OR ((Zx@3 OR (Zround@5 OR (((((((((21@6 PHRASE 2 2@7) OR Zx@8) OR 3@9) OR 92@10) OR Zx@11) OR 2@12) OR (503@13 PHRASE 2 2@14)) OR Zx@15) OR 105@16))) AND_NOT Zchr@4)) OR (1@17 PHRASE 2 4@18)))" },
    { "parent.document.getElementById(\\\"leftmenu\\\").cols", "(((parent@1 PHRASE 3 document@2 PHRASE 3 getelementbyid@3) OR leftmenu@4) OR Zcol@5)" },
    { "<% if not isEmpty(Request.QueryString) then", "(((Zif@1 OR Znot@2 OR isempty@3) OR (request@4 PHRASE 2 querystring@5)) OR Zthen@6)" },
    { "Active Desktop (Hier issie)", "((active@1 OR desktop@2) OR (hier@3 OR Zissi@4))" },
    { "Asus A7V8X (LAN + Sound)", "((asus@1 OR a7v8x@2) OR (lan@3 OR sound@4))" },
    { "Novell This pentium class machine (or greater) lacks some required CPU feature(s", "((((novell@1 OR this@2 OR Zpentium@3 OR Zclass@4 OR Zmachin@5) OR (Zor@6 OR Zgreater@7)) OR (Zlack@8 OR Zsome@9 OR Zrequir@10 OR cpu@11 OR feature@12)) OR Zs@13)" },
    { "sql server install fails error code (-1)", "((Zsql@1 OR Zserver@2 OR Zinstal@3 OR Zfail@4 OR Zerror@5 OR Zcode@6) OR 1@7)" },
    { "session_register(\"login\");", "(session_register@1 OR login@2)" },
    { "\"kylix+ndmb\"", "(kylix@1 PHRASE 2 ndmb@2)" },
    { "Cannot find imap library (libc-client.a).", "((cannot@1 OR Zfind@2 OR Zimap@3 OR Zlibrari@4) OR (libc@5 PHRASE 3 client@6 PHRASE 3 a@7))" },
    { "If ($_SESSION[\"Login\"] == 1)", "(if@1 OR ((_session@2 OR login@3) OR 1@4))" },
    { "You have an error in your SQL syntax near '1')' at line 1", "(((you@1 OR Zhave@2 OR Zan@3 OR Zerror@4 OR Zin@5 OR Zyour@6 OR sql@7 OR Zsyntax@8 OR Znear@9) OR 1@10) OR (Zat@11 OR Zline@12 OR 1@13))" },
    { "ASRock K7VT2 (incl. LAN)", "((asrock@1 OR k7vt2@2) OR (Zincl@3 OR lan@4))" },
    { "+windows98 +(geen communicatie) +ie5", "((Zwindows98@1 AND (Zgeen@2 OR Zcommunicati@3)) AND Zie5@4)" },
    { "\"xterm -fn\"", "(xterm@1 PHRASE 2 fn@2)" },
    { "IRQL_NOT_LESS_OR_EQUAL", "irql_not_less_or_equal@1" },
    { "access query \"NOT IN\"", "((Zaccess@1 OR Zqueri@2) OR (not@3 PHRASE 2 in@4))" },
    { "\"phrase one \"phrase two\"", "((phrase@1 PHRASE 2 one@2) OR (Zphrase@3 OR two@4))" },
    { "NEAR 207 46 249 27", "(near@1 OR 207@2 OR 46@3 OR 249@4 OR 27@5)" },
    { "- NEAR 12V voeding", "(near@1 OR 12v@2 OR Zvoed@3)" },
    { "waarom \"~\" in directorynaam", "(Zwaarom@1 OR (Zin@2 OR Zdirectorynaam@3))" },
    { "cd'r NEAR toebehoren", "(cd'r@1 NEAR 11 toebehoren@2)" },
    { "site:1 site:2", "0 * (H1 OR H2)" },
    { "site:1 site2:2", "0 * (H1 AND J2)" },
    { "site:1 site:2 site2:2", "0 * ((H1 OR H2) AND J2)" },
    { "site:1 OR site:2", "(0 * H1 OR 0 * H2)" },
    { "site:1 AND site:2", "(0 * H1 AND 0 * H2)" },
    { "foo AND site:2", "(Zfoo@1 AND 0 * H2)" },
    // Non-exclusive boolean prefixes feature tests (ticket#402):
    { "category:1 category:2", "0 * (XCAT1 AND XCAT2)" },
    { "category:1 site2:2", "0 * (XCAT1 AND J2)" },
    { "category:1 category:2 site2:2", "0 * ((XCAT1 AND XCAT2) AND J2)" },
    { "category:1 OR category:2", "(0 * XCAT1 OR 0 * XCAT2)" },
    { "category:1 AND category:2", "(0 * XCAT1 AND 0 * XCAT2)" },
    { "foo AND category:2", "(Zfoo@1 AND 0 * XCAT2)" },
    // Regression test for combining multiple non-exclusive prefixes, fixed in
    // 1.2.22 and 1.3.4.
    { "category:1 dogegory:2", "0 * (XCAT1 AND XDOG2)" },
    { "A site:1 site:2", "(a@1 FILTER (H1 OR H2))" },
#if 0
    { "A (site:1 OR site:2)", "(a@1 FILTER (H1 OR H2))" },
#endif
    { "A site:1 site2:2", "(a@1 FILTER (H1 AND J2))" },
    { "A site:1 site:2 site2:2", "(a@1 FILTER ((H1 OR H2) AND J2))" },
#if 0
    { "A site:1 OR site:2", "(a@1 FILTER (H1 OR H2))" },
#endif
    { "A site:1 AND site:2", "((a@1 FILTER H1) AND 0 * H2)" },
    { "site:xapian.org OR site:www.xapian.org", "(0 * Hxapian.org OR 0 * Hwww.xapian.org)" },
    { "site:xapian.org site:www.xapian.org", "0 * (Hxapian.org OR Hwww.xapian.org)" },
    { "site:xapian.org AND site:www.xapian.org", "(0 * Hxapian.org AND 0 * Hwww.xapian.org)" },
    { "Xapian site:xapian.org site:www.xapian.org", "(xapian@1 FILTER (Hxapian.org OR Hwww.xapian.org))" },
    { "author:richard author:olly writer:charlie", "(ZArichard@1 OR ZAolli@2 OR ZAcharli@3)"},
    { "author:richard NEAR title:book", "(Arichard@1 NEAR 11 XTbook@2)"},
// FIXME: This throws an exception as of 1.3.6, but once implemented we
// should re-enable it.
//    { "authortitle:richard NEAR title:book", "((Arichard@1 OR XTrichard@1) NEAR 11 XTbook@2)" },
    { "multisite:xapian.org", "0 * (Hxapian.org OR Jxapian.org)"},
    { "authortitle:richard", "(ZArichard@1 OR ZXTrichard@1)"},
    { "multisite:xapian.org site:www.xapian.org author:richard authortitle:richard", "((ZArichard@1 OR (ZArichard@2 OR ZXTrichard@2)) FILTER ((Hxapian.org OR Jxapian.org) AND Hwww.xapian.org))" },
    { "authortitle:richard-boulton", "((Arichard@1 PHRASE 2 Aboulton@2) OR (XTrichard@1 PHRASE 2 XTboulton@2))"},
    { "authortitle:\"richard boulton\"", "((Arichard@1 PHRASE 2 Aboulton@2) OR (XTrichard@1 PHRASE 2 XTboulton@2))"},
    // Test FLAG_CJK_NGRAM isn't on by default:
    { "久有归天愿", "Z久有归天愿@1" },
    { NULL, "CJK" }, // Enable FLAG_CJK_NGRAM
    // Test non-CJK queries still parse the same:
    { "gtk+ -gnome", "(Zgtk+@1 AND_NOT Zgnome@2)" },
    { "“curly quotes”", "(curly@1 PHRASE 2 quotes@2)" },
    // Test n-gram generation:
    { "久有归天愿", "(久@1 AND 久有@1 AND 有@1 AND 有归@1 AND 归@1 AND 归天@1 AND 天@1 AND 天愿@1 AND 愿@1)" },
    { "久有 归天愿", "((久@1 AND 久有@1 AND 有@1) OR (归@2 AND 归天@2 AND 天@2 AND 天愿@2 AND 愿@2))" },
    { "久有！归天愿", "((久@1 AND 久有@1 AND 有@1) OR (归@2 AND 归天@2 AND 天@2 AND 天愿@2 AND 愿@2))" },
    { "title:久有 归 天愿", "(((XT久@1 AND XT久有@1 AND XT有@1) OR 归@2) OR (天@3 AND 天愿@3 AND 愿@3))" },
    { "h众ello万众", "(((Zh@1 OR 众@2) OR Zello@3) OR (万@4 AND 万众@4 AND 众@4))" },
    { "世(の中)TEST_tm", "((世@1 OR (の@2 AND の中@2 AND 中@2)) OR test_tm@3)" },
    { "다녀 AND 와야", "((다@1 AND 다녀@1 AND 녀@1) AND (와@2 AND 와야@2 AND 야@2))" },
    { "authortitle:학술 OR 연구를", "(((A학@1 AND A학술@1 AND A술@1) OR (XT학@1 AND XT학술@1 AND XT술@1)) OR (연@2 AND 연구@2 AND 구@2 AND 구를@2 AND 를@2))" },
    // FIXME: These should really filter by bigrams to accelerate:
    { "\"久有归\"", "(久@1 PHRASE 3 有@1 PHRASE 3 归@1)" },
    { "\"久有test归\"", "(久@1 PHRASE 4 有@1 PHRASE 4 test@2 PHRASE 4 归@3)" },
    // FIXME: this should work: { "久 NEAR 有", "(久@1 NEAR 11 有@2)" },
    { NULL, NULL }
};

DEFINE_TESTCASE(queryparser1, !backend) {
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
    queryparser.add_boolean_prefix("dogegory", "XDOG", false);
    TEST_EXCEPTION(Xapian::InvalidOperationError,
	queryparser.add_boolean_prefix("authortitle", "B");
    );
    TEST_EXCEPTION(Xapian::InvalidOperationError,
	queryparser.add_prefix("multisite", "B");
    );
    unsigned flags = queryparser.FLAG_DEFAULT;
    for (const test *p = test_or_queries; ; ++p) {
	if (!p->query) {
	    if (!p->expect) break;
	    if (strcmp(p->expect, "CJK") == 0) {
		flags = queryparser.FLAG_DEFAULT|queryparser.FLAG_CJK_NGRAM;
		continue;
	    }
	    FAIL_TEST("Unknown flag code: " << p->expect);
	}
	string expect, parsed;
	if (p->expect)
	    expect = p->expect;
	else
	    expect = "parse error";
	try {
	    Xapian::Query qobj = queryparser.parse_query(p->query, flags);
	    parsed = qobj.get_description();
	    expect = string("Query(") + expect + ')';
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
    { "internet explorer title:(http www)", "((Zinternet@1 AND Zexplor@2) AND (ZXThttp@3 AND ZXTwww@4))" },
    // Regression test for bug in 0.9.2 and earlier - this would give
    // (two@2 AND_MAYBE (one@1 AND three@3))
    { "one +two three", "((Zone@1 AND Ztwo@2) AND Zthree@3)" },
    { "hello -title:\"hello world\"", "(Zhello@1 AND_NOT (XThello@2 PHRASE 2 XTworld@3))" },
    // Regression test for bug fixed in 1.0.4 - the '-' would be ignored there
    // because the whitespace after the '"' wasn't noticed.
    { "\"hello world\" -C++", "((hello@1 PHRASE 2 world@2) AND_NOT c++@3)" },
    // Regression tests for bug fixed in 1.0.4 - queries with only boolean
    // filter and HATE terms weren't accepted.
    { "-cup site:world", "(0 * Hworld AND_NOT Zcup@1)" },
    { "site:world -cup", "(0 * Hworld AND_NOT Zcup@1)" },
    // Regression test for bug fixed in 1.0.4 - the KET token for ')' was lost.
    { "(site:world) -cup", "(0 * Hworld AND_NOT Zcup@1)" },
    // Regression test for bug fixed in 1.0.4 - a boolean filter term between
    // probabilistic terms caused a parse error (probably broken during the
    // addition of synonym support in 1.0.2).
    { "foo site:xapian.org bar", "((Zfoo@1 AND Zbar@2) FILTER Hxapian.org)" },
    // Add coverage for other cases similar to the above.
    { "a b site:xapian.org", "((Za@1 AND Zb@2) FILTER Hxapian.org)" },
    { "site:xapian.org a b", "((Za@1 AND Zb@2) FILTER Hxapian.org)" },
    { NULL, "CJK" }, // Enable FLAG_CJK_NGRAM
    // Test n-gram generation:
    { "author:험가 OR subject:万众 hello world!", "((A험@1 AND A험가@1 AND A가@1) OR ((XT万@2 AND XT万众@2 AND XT众@2) AND (Zhello@3 AND Zworld@4)))" },
    { "洛伊one儿差点two脸three", "((((((洛@1 AND 洛伊@1 AND 伊@1) AND Zone@2) AND (儿@3 AND 儿差@3 AND 差@3 AND 差点@3 AND 点@3)) AND Ztwo@4) AND 脸@5) AND Zthree@6)" },
    { NULL, NULL }
};

// With default_op = OP_AND.
DEFINE_TESTCASE(qp_default_op1, !backend) {
    Xapian::QueryParser queryparser;
    queryparser.set_stemmer(Xapian::Stem("english"));
    queryparser.set_stemming_strategy(Xapian::QueryParser::STEM_SOME);
    queryparser.add_prefix("author", "A");
    queryparser.add_prefix("title", "XT");
    queryparser.add_prefix("subject", "XT");
    queryparser.add_boolean_prefix("site", "H");
    queryparser.set_default_op(Xapian::Query::OP_AND);
    unsigned flags = queryparser.FLAG_DEFAULT;
    for (const test *p = test_and_queries; ; ++p) {
	if (!p->query) {
	    if (!p->expect) break;
	    if (strcmp(p->expect, "CJK") == 0) {
		flags = queryparser.FLAG_DEFAULT|queryparser.FLAG_CJK_NGRAM;
		continue;
	    }
	    FAIL_TEST("Unknown flag code: " << p->expect);
	}
	string expect, parsed;
	if (p->expect)
	    expect = p->expect;
	else
	    expect = "parse error";
	try {
	    Xapian::Query qobj = queryparser.parse_query(p->query, flags);
	    parsed = qobj.get_description();
	    expect = string("Query(") + expect + ')';
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
DEFINE_TESTCASE(qp_default_prefix1, !backend) {
    Xapian::QueryParser qp;
    qp.set_stemmer(Xapian::Stem("english"));
    qp.set_stemming_strategy(Xapian::QueryParser::STEM_SOME);
    qp.add_prefix("title", "XT");

    Xapian::Query qobj;
    qobj = qp.parse_query("hello world", 0, "A");
    TEST_STRINGS_EQUAL(qobj.get_description(), "Query((ZAhello@1 OR ZAworld@2))");
    qobj = qp.parse_query("me title:stuff", 0, "A");
    TEST_STRINGS_EQUAL(qobj.get_description(), "Query((ZAme@1 OR ZXTstuff@2))");
    qobj = qp.parse_query("title:(stuff) me", Xapian::QueryParser::FLAG_BOOLEAN, "A");
    TEST_STRINGS_EQUAL(qobj.get_description(), "Query((ZXTstuff@1 OR ZAme@2))");
    qobj = qp.parse_query("英国 title:文森hello", qp.FLAG_CJK_NGRAM, "A");
    TEST_STRINGS_EQUAL(qobj.get_description(), "Query((((A英@1 AND A英国@1 AND A国@1) OR (XT文@2 AND XT文森@2 AND XT森@2)) OR ZAhello@3))");
    return true;
}

// Feature test for setting the default prefix with add_prefix()
// (new in Xapian 1.0.3).
DEFINE_TESTCASE(qp_default_prefix2, !backend) {
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
    TEST_STRINGS_EQUAL(qobj.get_description(), "Query((ZAhello@1 OR ZAworld@2))");
    qobj = qp.parse_query("me title:stuff", 0);
    TEST_STRINGS_EQUAL(qobj.get_description(), "Query((ZAme@1 OR ZXTstuff@2))");
    qobj = qp.parse_query("title:(stuff) me", Xapian::QueryParser::FLAG_BOOLEAN);
    TEST_STRINGS_EQUAL(qobj.get_description(), "Query((ZXTstuff@1 OR ZAme@2))");

    qobj = qp.parse_query("hello world", 0, "B");
    TEST_STRINGS_EQUAL(qobj.get_description(), "Query((ZBhello@1 OR ZBworld@2))");
    qobj = qp.parse_query("me title:stuff", 0, "B");
    TEST_STRINGS_EQUAL(qobj.get_description(), "Query((ZBme@1 OR ZXTstuff@2))");
    qobj = qp.parse_query("title:(stuff) me", Xapian::QueryParser::FLAG_BOOLEAN, "B");
    TEST_STRINGS_EQUAL(qobj.get_description(), "Query((ZXTstuff@1 OR ZBme@2))");

    qp.add_prefix("", "B");
    qobj = qp.parse_query("me-us title:(stuff) me", Xapian::QueryParser::FLAG_BOOLEAN);
    TEST_STRINGS_EQUAL(qobj.get_description(), "Query(((((Ame@1 PHRASE 2 Aus@2) OR (Bme@1 PHRASE 2 Bus@2)) OR ZXTstuff@3) OR (ZAme@4 OR ZBme@4)))");
    qobj = qp.parse_query("me-us title:(stuff) me", Xapian::QueryParser::FLAG_BOOLEAN, "C");
    TEST_STRINGS_EQUAL(qobj.get_description(), "Query((((Cme@1 PHRASE 2 Cus@2) OR ZXTstuff@3) OR ZCme@4))");

    qobj = qp.parse_query("me-us title:\"not-me\"", Xapian::QueryParser::FLAG_PHRASE);
    TEST_STRINGS_EQUAL(qobj.get_description(), "Query((((Ame@1 PHRASE 2 Aus@2) OR (Bme@1 PHRASE 2 Bus@2)) OR (XTnot@3 PHRASE 2 XTme@4)))");
    return true;
}

// Test query with odd characters in.
DEFINE_TESTCASE(qp_odd_chars1, !backend) {
    Xapian::QueryParser qp;
    string query("\x01weird\x00stuff\x7f", 13);
    Xapian::Query qobj = qp.parse_query(query);
    tout << "Query:  " << query << '\n';
    TEST_STRINGS_EQUAL(qobj.get_description(), "Query((weird@1 OR stuff@2))"); // FIXME: should these be stemmed?
    return true;
}

// Test right truncation.
DEFINE_TESTCASE(qp_flag_wildcard1, writable) {
    Xapian::WritableDatabase db = get_writable_database();
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
    TEST_STRINGS_EQUAL(qobj.get_description(), "Query((SYNONYM WILDCARD OR ab))");
    qobj = qp.parse_query("muscle*", Xapian::QueryParser::FLAG_WILDCARD);
    TEST_STRINGS_EQUAL(qobj.get_description(), "Query((SYNONYM WILDCARD OR muscle))");
    qobj = qp.parse_query("meat*", Xapian::QueryParser::FLAG_WILDCARD);
    TEST_STRINGS_EQUAL(qobj.get_description(), "Query((SYNONYM WILDCARD OR meat))");
    qobj = qp.parse_query("musc*", Xapian::QueryParser::FLAG_WILDCARD);
    TEST_STRINGS_EQUAL(qobj.get_description(), "Query((SYNONYM WILDCARD OR musc))");
    qobj = qp.parse_query("mutt*", Xapian::QueryParser::FLAG_WILDCARD);
    TEST_STRINGS_EQUAL(qobj.get_description(), "Query((SYNONYM WILDCARD OR mutt))");
    // Regression test (we weren't lowercasing terms before checking if they
    // were in the database or not):
    qobj = qp.parse_query("mUTTON++");
    TEST_STRINGS_EQUAL(qobj.get_description(), "Query(mutton@1)");
    // Regression test: check that wildcards work with +terms.
    unsigned flags = Xapian::QueryParser::FLAG_WILDCARD |
		     Xapian::QueryParser::FLAG_LOVEHATE;
    qobj = qp.parse_query("+mai* main", flags);
    TEST_STRINGS_EQUAL(qobj.get_description(), "Query(((SYNONYM WILDCARD OR mai) AND_MAYBE main@2))");
    // Regression test (if we had a +term which was a wildcard and wasn't
    // present, the query could still match documents).
    qobj = qp.parse_query("foo* main", flags);
    TEST_STRINGS_EQUAL(qobj.get_description(), "Query(((SYNONYM WILDCARD OR foo) OR main@2))");
    qobj = qp.parse_query("main foo*", flags);
    TEST_STRINGS_EQUAL(qobj.get_description(), "Query((main@1 OR (SYNONYM WILDCARD OR foo)))");
    qobj = qp.parse_query("+foo* main", flags);
    TEST_STRINGS_EQUAL(qobj.get_description(), "Query(((SYNONYM WILDCARD OR foo) AND_MAYBE main@2))");
    qobj = qp.parse_query("main +foo*", flags);
    TEST_STRINGS_EQUAL(qobj.get_description(), "Query(((SYNONYM WILDCARD OR foo) AND_MAYBE main@1))");
    qobj = qp.parse_query("foo* +main", flags);
    TEST_STRINGS_EQUAL(qobj.get_description(), "Query((main@2 AND_MAYBE (SYNONYM WILDCARD OR foo)))");
    qobj = qp.parse_query("+main foo*", flags);
    TEST_STRINGS_EQUAL(qobj.get_description(), "Query((main@1 AND_MAYBE (SYNONYM WILDCARD OR foo)))");
    qobj = qp.parse_query("+foo* +main", flags);
    TEST_STRINGS_EQUAL(qobj.get_description(), "Query(((SYNONYM WILDCARD OR foo) AND main@2))");
    qobj = qp.parse_query("+main +foo*", flags);
    TEST_STRINGS_EQUAL(qobj.get_description(), "Query((main@1 AND (SYNONYM WILDCARD OR foo)))");
    qobj = qp.parse_query("foo* mai", flags);
    TEST_STRINGS_EQUAL(qobj.get_description(), "Query(((SYNONYM WILDCARD OR foo) OR mai@2))");
    qobj = qp.parse_query("mai foo*", flags);
    TEST_STRINGS_EQUAL(qobj.get_description(), "Query((mai@1 OR (SYNONYM WILDCARD OR foo)))");
    qobj = qp.parse_query("+foo* mai", flags);
    TEST_STRINGS_EQUAL(qobj.get_description(), "Query(((SYNONYM WILDCARD OR foo) AND_MAYBE mai@2))");
    qobj = qp.parse_query("mai +foo*", flags);
    TEST_STRINGS_EQUAL(qobj.get_description(), "Query(((SYNONYM WILDCARD OR foo) AND_MAYBE mai@1))");
    qobj = qp.parse_query("foo* +mai", flags);
    TEST_STRINGS_EQUAL(qobj.get_description(), "Query((mai@2 AND_MAYBE (SYNONYM WILDCARD OR foo)))");
    qobj = qp.parse_query("+mai foo*", flags);
    TEST_STRINGS_EQUAL(qobj.get_description(), "Query((mai@1 AND_MAYBE (SYNONYM WILDCARD OR foo)))");
    qobj = qp.parse_query("+foo* +mai", flags);
    TEST_STRINGS_EQUAL(qobj.get_description(), "Query(((SYNONYM WILDCARD OR foo) AND mai@2))");
    qobj = qp.parse_query("+mai +foo*", flags);
    TEST_STRINGS_EQUAL(qobj.get_description(), "Query((mai@1 AND (SYNONYM WILDCARD OR foo)))");
    qobj = qp.parse_query("-foo* main", flags);
    TEST_STRINGS_EQUAL(qobj.get_description(), "Query((main@2 AND_NOT (SYNONYM WILDCARD OR foo)))");
    qobj = qp.parse_query("main -foo*", flags);
    TEST_STRINGS_EQUAL(qobj.get_description(), "Query((main@1 AND_NOT (SYNONYM WILDCARD OR foo)))");
    qobj = qp.parse_query("main -foo* -bar", flags);
    TEST_STRINGS_EQUAL(qobj.get_description(), "Query((main@1 AND_NOT ((SYNONYM WILDCARD OR foo) OR bar@3)))");
    qobj = qp.parse_query("main -bar -foo*", flags);
    TEST_STRINGS_EQUAL(qobj.get_description(), "Query((main@1 AND_NOT (bar@2 OR (SYNONYM WILDCARD OR foo))))");
    // Check with OP_AND too.
    qp.set_default_op(Xapian::Query::OP_AND);
    qobj = qp.parse_query("foo* main", flags);
    TEST_STRINGS_EQUAL(qobj.get_description(), "Query(((SYNONYM WILDCARD OR foo) AND main@2))");
    qobj = qp.parse_query("main foo*", flags);
    TEST_STRINGS_EQUAL(qobj.get_description(), "Query((main@1 AND (SYNONYM WILDCARD OR foo)))");
    qp.set_default_op(Xapian::Query::OP_AND);
    qobj = qp.parse_query("+foo* main", flags);
    TEST_STRINGS_EQUAL(qobj.get_description(), "Query(((SYNONYM WILDCARD OR foo) AND main@2))");
    qobj = qp.parse_query("main +foo*", flags);
    TEST_STRINGS_EQUAL(qobj.get_description(), "Query((main@1 AND (SYNONYM WILDCARD OR foo)))");
    qobj = qp.parse_query("-foo* main", flags);
    TEST_STRINGS_EQUAL(qobj.get_description(), "Query((main@2 AND_NOT (SYNONYM WILDCARD OR foo)))");
    qobj = qp.parse_query("main -foo*", flags);
    TEST_STRINGS_EQUAL(qobj.get_description(), "Query((main@1 AND_NOT (SYNONYM WILDCARD OR foo)))");
    // Check empty wildcard followed by negation.
    qobj = qp.parse_query("foo* -main", Xapian::QueryParser::FLAG_LOVEHATE|Xapian::QueryParser::FLAG_WILDCARD);
    TEST_STRINGS_EQUAL(qobj.get_description(), "Query(((SYNONYM WILDCARD OR foo) AND_NOT main@2))");
    // Regression test for bug#484 fixed in 1.2.1 and 1.0.21.
    qobj = qp.parse_query("abc muscl* main", flags);
    TEST_STRINGS_EQUAL(qobj.get_description(), "Query(((abc@1 AND (SYNONYM WILDCARD OR muscl)) AND main@3))");
    return true;
}

// Test right truncation with prefixes.
DEFINE_TESTCASE(qp_flag_wildcard2, writable) {
    Xapian::WritableDatabase db = get_writable_database();
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
    TEST_STRINGS_EQUAL(qobj.get_description(), "Query((SYNONYM WILDCARD OR Ah))");
    qobj = qp.parse_query("author:h* test", Xapian::QueryParser::FLAG_WILDCARD);
    TEST_STRINGS_EQUAL(qobj.get_description(), "Query(((SYNONYM WILDCARD OR Ah) OR test@2))");
    return true;
}

static void
test_qp_flag_wildcard1_helper(const Xapian::Database &db,
			      Xapian::termcount max_expansion,
			      const string & query_string)
{
    Xapian::QueryParser qp;
    qp.set_database(db);
    qp.set_max_expansion(max_expansion);
    Xapian::Enquire e(db);
    e.set_query(qp.parse_query(query_string, Xapian::QueryParser::FLAG_WILDCARD));
    // The exception for expanding too much may happen at parse time or later
    // so we need to calculate the MSet too.
    e.get_mset(0, 10);
}

// Test right truncation with a limit on expansion.
DEFINE_TESTCASE(qp_flag_wildcard3, writable) {
    Xapian::WritableDatabase db = get_writable_database();
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
    TEST_EXCEPTION(Xapian::WildcardError,
	test_qp_flag_wildcard1_helper(db, 1, "muscle*"));
    TEST_EXCEPTION(Xapian::WildcardError,
	test_qp_flag_wildcard1_helper(db, 3, "musc*"));
    TEST_EXCEPTION(Xapian::WildcardError,
	test_qp_flag_wildcard1_helper(db, 3, "mus*"));
    TEST_EXCEPTION(Xapian::WildcardError,
	test_qp_flag_wildcard1_helper(db, 4, "mu*"));
    TEST_EXCEPTION(Xapian::WildcardError,
	test_qp_flag_wildcard1_helper(db, 5, "m*"));

    return true;
}

// Test partial queries.
DEFINE_TESTCASE(qp_flag_partial1, writable) {
    Xapian::WritableDatabase db = get_writable_database();
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
    TEST_STRINGS_EQUAL(qobj.get_description(), "Query(((SYNONYM WILDCARD OR a) OR Za@1))");
    qobj = qp.parse_query("ab", Xapian::QueryParser::FLAG_PARTIAL);
    TEST_STRINGS_EQUAL(qobj.get_description(), "Query(((SYNONYM WILDCARD OR ab) OR Zab@1))");
    qobj = qp.parse_query("muscle", Xapian::QueryParser::FLAG_PARTIAL);
    TEST_STRINGS_EQUAL(qobj.get_description(), "Query(((SYNONYM WILDCARD OR muscle) OR Zmuscl@1))");
    qobj = qp.parse_query("meat", Xapian::QueryParser::FLAG_PARTIAL);
    TEST_STRINGS_EQUAL(qobj.get_description(), "Query(((SYNONYM WILDCARD OR meat) OR Zmeat@1))");
    qobj = qp.parse_query("musc", Xapian::QueryParser::FLAG_PARTIAL);
    TEST_STRINGS_EQUAL(qobj.get_description(), "Query(((SYNONYM WILDCARD OR musc) OR Zmusc@1))");
    qobj = qp.parse_query("mutt", Xapian::QueryParser::FLAG_PARTIAL);
    TEST_STRINGS_EQUAL(qobj.get_description(), "Query(((SYNONYM WILDCARD OR mutt) OR Zmutt@1))");
    qobj = qp.parse_query("abc musc", Xapian::QueryParser::FLAG_PARTIAL);
    TEST_STRINGS_EQUAL(qobj.get_description(), "Query((Zabc@1 OR ((SYNONYM WILDCARD OR musc) OR Zmusc@2)))");
    qobj = qp.parse_query("a* mutt", Xapian::QueryParser::FLAG_PARTIAL | Xapian::QueryParser::FLAG_WILDCARD);
    TEST_STRINGS_EQUAL(qobj.get_description(), "Query(((SYNONYM WILDCARD OR a) OR ((SYNONYM WILDCARD OR mutt) OR Zmutt@2)))");

    // Check behaviour with stemmed terms, and stem strategy STEM_SOME.
    qobj = qp.parse_query("o", Xapian::QueryParser::FLAG_PARTIAL);
    TEST_STRINGS_EQUAL(qobj.get_description(), "Query(((SYNONYM WILDCARD OR o) OR Zo@1))");
    qobj = qp.parse_query("ou", Xapian::QueryParser::FLAG_PARTIAL);
    TEST_STRINGS_EQUAL(qobj.get_description(), "Query(((SYNONYM WILDCARD OR ou) OR Zou@1))");
    qobj = qp.parse_query("out", Xapian::QueryParser::FLAG_PARTIAL);
    TEST_STRINGS_EQUAL(qobj.get_description(), "Query(((SYNONYM WILDCARD OR out) OR Zout@1))");
    qobj = qp.parse_query("outs", Xapian::QueryParser::FLAG_PARTIAL);
    TEST_STRINGS_EQUAL(qobj.get_description(), "Query(((SYNONYM WILDCARD OR outs) OR Zout@1))");
    qobj = qp.parse_query("outsi", Xapian::QueryParser::FLAG_PARTIAL);
    TEST_STRINGS_EQUAL(qobj.get_description(), "Query(((SYNONYM WILDCARD OR outsi) OR Zoutsi@1))");
    qobj = qp.parse_query("outsid", Xapian::QueryParser::FLAG_PARTIAL);
    TEST_STRINGS_EQUAL(qobj.get_description(), "Query(((SYNONYM WILDCARD OR outsid) OR Zoutsid@1))");
    qobj = qp.parse_query("outside", Xapian::QueryParser::FLAG_PARTIAL);
    TEST_STRINGS_EQUAL(qobj.get_description(), "Query(((SYNONYM WILDCARD OR outside) OR Zoutsid@1))");

    // Check behaviour with capitalised terms, and stem strategy STEM_SOME.
    qobj = qp.parse_query("Out", Xapian::QueryParser::FLAG_PARTIAL);
    TEST_STRINGS_EQUAL(qobj.get_description(), "Query(((SYNONYM WILDCARD OR out) OR out@1))");
    qobj = qp.parse_query("Outs", Xapian::QueryParser::FLAG_PARTIAL);
    TEST_STRINGS_EQUAL(qobj.get_description(), "Query(((SYNONYM WILDCARD OR outs) OR outs@1))");
    qobj = qp.parse_query("Outside", Xapian::QueryParser::FLAG_PARTIAL);
    TEST_STRINGS_EQUAL(qobj.get_description(), "Query(((SYNONYM WILDCARD OR outside) OR outside@1))");
    // FIXME: Used to be this, but we aren't currently doing this change:
    // TEST_STRINGS_EQUAL(qobj.get_description(), "Query(outside@1#2)");

    // And now with stemming strategy STEM_ALL.
    qp.set_stemming_strategy(Xapian::QueryParser::STEM_ALL);
    qobj = qp.parse_query("Out", Xapian::QueryParser::FLAG_PARTIAL);
    TEST_STRINGS_EQUAL(qobj.get_description(), "Query(((SYNONYM WILDCARD OR out) OR out@1))");
    qobj = qp.parse_query("Outs", Xapian::QueryParser::FLAG_PARTIAL);
    TEST_STRINGS_EQUAL(qobj.get_description(), "Query(((SYNONYM WILDCARD OR outs) OR out@1))");
    qobj = qp.parse_query("Outside", Xapian::QueryParser::FLAG_PARTIAL);
    TEST_STRINGS_EQUAL(qobj.get_description(), "Query(((SYNONYM WILDCARD OR outside) OR outsid@1))");

    // And now with stemming strategy STEM_ALL_Z.
    qp.set_stemming_strategy(Xapian::QueryParser::STEM_ALL_Z);
    qobj = qp.parse_query("Out", Xapian::QueryParser::FLAG_PARTIAL);
    TEST_STRINGS_EQUAL(qobj.get_description(), "Query(((SYNONYM WILDCARD OR out) OR Zout@1))");
    qobj = qp.parse_query("Outs", Xapian::QueryParser::FLAG_PARTIAL);
    TEST_STRINGS_EQUAL(qobj.get_description(), "Query(((SYNONYM WILDCARD OR outs) OR Zout@1))");
    qobj = qp.parse_query("Outside", Xapian::QueryParser::FLAG_PARTIAL);
    TEST_STRINGS_EQUAL(qobj.get_description(), "Query(((SYNONYM WILDCARD OR outside) OR Zoutsid@1))");

    // Check handling of a case with a prefix.
    qp.set_stemming_strategy(Xapian::QueryParser::STEM_SOME);
    qobj = qp.parse_query("title:cow", Xapian::QueryParser::FLAG_PARTIAL);
    TEST_STRINGS_EQUAL(qobj.get_description(), "Query(((SYNONYM WILDCARD OR XTcow) OR ZXTcow@1))");
    qobj = qp.parse_query("title:cows", Xapian::QueryParser::FLAG_PARTIAL);
    TEST_STRINGS_EQUAL(qobj.get_description(), "Query(((SYNONYM WILDCARD OR XTcows) OR ZXTcow@1))");
    qobj = qp.parse_query("title:Cow", Xapian::QueryParser::FLAG_PARTIAL);
    TEST_STRINGS_EQUAL(qobj.get_description(), "Query(((SYNONYM WILDCARD OR XTcow) OR XTcow@1))");
    qobj = qp.parse_query("title:Cows", Xapian::QueryParser::FLAG_PARTIAL);
    TEST_STRINGS_EQUAL(qobj.get_description(), "Query(((SYNONYM WILDCARD OR XTcows) OR XTcows@1))");
    // FIXME: Used to be this, but we aren't currently doing this change:
    // TEST_STRINGS_EQUAL(qobj.get_description(), "Query(XTcows@1#2)");

    // Regression test - the initial version of the multi-prefix code would
    // inflate the wqf of the "parsed as normal" version of a partial term
    // by multiplying it by the number of prefixes mapped to.
    qobj = qp.parse_query("double:vision", Xapian::QueryParser::FLAG_PARTIAL);
    TEST_STRINGS_EQUAL(qobj.get_description(), "Query(((WILDCARD OR XONEvision SYNONYM WILDCARD OR XTWOvision) OR (ZXONEvision@1 SYNONYM ZXTWOvision@1)))");

    // Test handling of FLAG_PARTIAL when there's more than one prefix.
    qobj = qp.parse_query("double:part", Xapian::QueryParser::FLAG_PARTIAL);
    TEST_STRINGS_EQUAL(qobj.get_description(), "Query(((WILDCARD OR XONEpart SYNONYM WILDCARD OR XTWOpart) OR (ZXONEpart@1 SYNONYM ZXTWOpart@1)))");

    // Test handling of FLAG_PARTIAL when there's more than one prefix, without
    // stemming.
    qp.set_stemming_strategy(Xapian::QueryParser::STEM_NONE);
    qobj = qp.parse_query("double:part", Xapian::QueryParser::FLAG_PARTIAL);
    TEST_STRINGS_EQUAL(qobj.get_description(), "Query(((WILDCARD OR XONEpart SYNONYM WILDCARD OR XTWOpart) OR (XONEpart@1 SYNONYM XTWOpart@1)))");
    qobj = qp.parse_query("double:partial", Xapian::QueryParser::FLAG_PARTIAL);
    TEST_STRINGS_EQUAL(qobj.get_description(), "Query(((WILDCARD OR XONEpartial SYNONYM WILDCARD OR XTWOpartial) OR (XONEpartial@1 SYNONYM XTWOpartial@1)))");

    return true;
}

// Tests for document counts for wildcard queries.
// Regression test for bug fixed in 1.0.0.
DEFINE_TESTCASE(wildquery1, backend) {
    Xapian::QueryParser queryparser;
    unsigned flags = Xapian::QueryParser::FLAG_WILDCARD |
		     Xapian::QueryParser::FLAG_LOVEHATE;
    queryparser.set_stemmer(Xapian::Stem("english"));
    queryparser.set_stemming_strategy(Xapian::QueryParser::STEM_ALL);
    Xapian::Database db = get_database("apitest_simpledata");
    queryparser.set_database(db);
    Xapian::Enquire enquire(db);

    Xapian::Query qobj = queryparser.parse_query("th*", flags);
    tout << qobj.get_description() << endl;
    enquire.set_query(qobj);
    Xapian::MSet mymset = enquire.get_mset(0, 10);
    // Check that 6 documents were returned.
    TEST_MSET_SIZE(mymset, 6);

    qobj = queryparser.parse_query("notindb* \"this\"", flags);
    tout << qobj.get_description() << endl;
    enquire.set_query(qobj);
    mymset = enquire.get_mset(0, 10);
    // Check that 6 documents were returned.
    TEST_MSET_SIZE(mymset, 6);

    qobj = queryparser.parse_query("+notindb* \"this\"", flags);
    tout << qobj.get_description() << endl;
    enquire.set_query(qobj);
    mymset = enquire.get_mset(0, 10);
    // Check that 0 documents were returned.
    TEST_MSET_SIZE(mymset, 0);

    return true;
}

DEFINE_TESTCASE(qp_flag_bool_any_case1, !backend) {
    using Xapian::QueryParser;
    Xapian::QueryParser qp;
    Xapian::Query qobj;
    qobj = qp.parse_query("to and fro", QueryParser::FLAG_BOOLEAN | QueryParser::FLAG_BOOLEAN_ANY_CASE);
    TEST_STRINGS_EQUAL(qobj.get_description(), "Query((to@1 AND fro@2))");
    qobj = qp.parse_query("to and fro", QueryParser::FLAG_BOOLEAN);
    TEST_STRINGS_EQUAL(qobj.get_description(), "Query((to@1 OR and@2 OR fro@3))");
    // Regression test for bug in 0.9.4 and earlier.
    qobj = qp.parse_query("to And fro", QueryParser::FLAG_BOOLEAN | QueryParser::FLAG_BOOLEAN_ANY_CASE);
    TEST_STRINGS_EQUAL(qobj.get_description(), "Query((to@1 AND fro@2))");
    qobj = qp.parse_query("to And fro", QueryParser::FLAG_BOOLEAN);
    TEST_STRINGS_EQUAL(qobj.get_description(), "Query((to@1 OR and@2 OR fro@3))");
    return true;
}

static const test test_stop_queries[] = {
    { "test the queryparser", "(test@1 AND queryparser@3)" },
    // Regression test for bug in 0.9.6 and earlier.  This would fail to
    // parse.
    { "test AND the AND queryparser", "((test@1 AND the@2) AND queryparser@3)" },
    // 0.9.6 and earlier ignored a stopword even if it was the only term.
    // More recent versions don't ever treat a single term as a stopword.
    { "the", "the@1" },
    // 1.2.2 and earlier ignored an all-stopword query with multiple terms,
    // which prevents 'to be or not to be' for being searchable unless the
    // user made it into a phrase query or prefixed all terms with '+'
    // (ticket#245).
    { "an the a", "(an@1 AND the@2 AND a@3)" },
    // Regression test for bug in initial version of the patch for the
    // "all-stopword" case.
    { "the AND a an", "(the@1 AND (a@2 AND an@3))" },
    { NULL, NULL }
};

DEFINE_TESTCASE(qp_stopper1, !backend) {
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
	    expect = string("Query(") + expect + ')';
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
    { "NOT windows", "(<alldocuments> AND_NOT Zwindow@1)" },
    { "a AND (NOT b)", "(Za@1 AND (<alldocuments> AND_NOT Zb@2))" },
    { "AND NOT windows", "Syntax: <expression> AND NOT <expression>" },
    { "gordian NOT", "Syntax: <expression> NOT <expression>" },
    { "gordian AND NOT", "Syntax: <expression> AND NOT <expression>" },
    { NULL, NULL }
};

DEFINE_TESTCASE(qp_flag_pure_not1, !backend) {
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
	    expect = string("Query(") + expect + ')';
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
DEFINE_TESTCASE(qp_unstem_boolean_prefix, !backend) {
    Xapian::QueryParser qp;
    qp.add_boolean_prefix("test", "XTEST");
    Xapian::Query q = qp.parse_query("hello test:foo");
    TEST_STRINGS_EQUAL(q.get_description(), "Query((hello@1 FILTER XTESTfoo))");
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
    { "$50..$99", "VALUE_RANGE 1 $50 $99" },
    { "$50..$100", "" }, // start > range
    { "02/03/1979..10/12/1980", "VALUE_RANGE 1 02/03/1979 10/12/1980" },
    { "a..b hello", "(hello@1 FILTER VALUE_RANGE 1 a b)" },
    { "hello a..b", "(hello@1 FILTER VALUE_RANGE 1 a b)" },
    { "hello a..b world", "((hello@1 OR world@2) FILTER VALUE_RANGE 1 a b)" },
    { "hello a..b test:foo", "(hello@1 FILTER (VALUE_RANGE 1 a b AND XTESTfoo))" },
    { "hello a..b test:foo test:bar", "(hello@1 FILTER (VALUE_RANGE 1 a b AND (XTESTfoo OR XTESTbar)))" },
    { "hello a..b c..d test:foo", "(hello@1 FILTER ((VALUE_RANGE 1 a b OR VALUE_RANGE 1 c d) AND XTESTfoo))" },
    { "hello a..b c..d test:foo test:bar", "(hello@1 FILTER ((VALUE_RANGE 1 a b OR VALUE_RANGE 1 c d) AND (XTESTfoo OR XTESTbar)))" },
    { "-5..7", "VALUE_RANGE 1 -5 7" },
    { "hello -5..7", "(hello@1 FILTER VALUE_RANGE 1 -5 7)" },
    { "-5..7 hello", "(hello@1 FILTER VALUE_RANGE 1 -5 7)" },
    { "\"time flies\" 09:00..12:30", "((time@1 PHRASE 2 flies@2) FILTER VALUE_RANGE 1 09:00 12:30)" },
    // Feature test for single-ended ranges (ticket#480):
    { "..b", "VALUE_LE 1 b" },
    { "a..", "VALUE_GE 1 a" },
    // Test for expanded set of characters allowed in range start:
    { "10:30+1300..11:00+1300", "VALUE_RANGE 1 10:30+1300 11:00+1300" },
    { NULL, NULL }
};

// Simple test of ValueRangeProcessor class.
DEFINE_TESTCASE(qp_value_range1, !backend) {
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
	    expect = string("Query(") + expect + ')';
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

// Simple test of RangeProcessor class.
DEFINE_TESTCASE(qp_range1, !backend) {
    Xapian::QueryParser qp;
    qp.add_boolean_prefix("test", "XTEST");
    Xapian::RangeProcessor rp(1);
    qp.add_rangeprocessor(&rp);
    for (const test *p = test_value_range1_queries; p->query; ++p) {
	string expect, parsed;
	if (p->expect)
	    expect = p->expect;
	else
	    expect = "parse error";
	try {
	    Xapian::Query qobj = qp.parse_query(p->query);
	    parsed = qobj.get_description();
	    expect = string("Query(") + expect + ')';
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
    { "1..12", "VALUE_RANGE 2 \\xa0 \\xae" },
    { "20070201..20070228", "VALUE_RANGE 1 20070201 20070228" },
    { "$10..20", "VALUE_RANGE 4 \\xad \\xb1" },
    { "$10..$20", "VALUE_RANGE 4 \\xad \\xb1" },
    // Feature test for single-ended ranges (ticket#480):
    { "$..20", "VALUE_LE 4 \\xb1" },
    { "..$20", "VALUE_LE 3 $20" }, // FIXME: probably should parse as $..20
    { "$10..", "VALUE_GE 4 \\xad" },
    { "12..42kg", "VALUE_RANGE 5 \\xae \\xb5@" },
    { "12kg..42kg", "VALUE_RANGE 5 \\xae \\xb5@" },
    { "12kg..42", "VALUE_RANGE 3 12kg 42" },
    { "10..$20", "" }, // start > end
    { "1999-03-12..2020-12-30", "VALUE_RANGE 1 19990312 20201230" },
    { "1999/03/12..2020/12/30", "VALUE_RANGE 1 19990312 20201230" },
    { "1999.03.12..2020.12.30", "VALUE_RANGE 1 19990312 20201230" },
    // Feature test for single-ended ranges (ticket#480):
    { "..2020.12.30", "VALUE_LE 1 20201230" },
    { "1999.03.12..", "VALUE_GE 1 19990312" },
    { "12/03/99..12/04/01", "VALUE_RANGE 1 19990312 20010412" },
    { "03-12-99..04-14-01", "VALUE_RANGE 1 19990312 20010414" },
    { "(test:a..test:b hello)", "(hello@1 FILTER VALUE_RANGE 3 test:a test:b)" },
    { "12..42kg 5..6kg 1..12", "0 * (VALUE_RANGE 2 \\xa0 \\xae AND (VALUE_RANGE 5 \\xae \\xb5@ OR VALUE_RANGE 5 \\xa9 \\xaa))" },
    // Check that a VRP which fails to match doesn't remove a prefix or suffix.
    // 1.0.13/1.1.1 and earlier got this wrong in some cases.
    { "$12a..13", "VALUE_RANGE 3 $12a 13" },
    { "$12..13b", "VALUE_RANGE 3 $12 13b" },
    { "$12..12kg", "VALUE_RANGE 3 $12 12kg" },
    { "12..b12kg", "VALUE_RANGE 3 12 b12kg" },
    { NULL, NULL }
};

// Test chaining of ValueRangeProcessor classes.
DEFINE_TESTCASE(qp_value_range2, !backend) {
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
	    expect = string("Query(") + expect + ')';
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

// Test chaining of RangeProcessor classes.
DEFINE_TESTCASE(qp_range2, !backend) {
    using Xapian::RP_REPEATED;
    using Xapian::RP_SUFFIX;
    Xapian::QueryParser qp;
    qp.add_boolean_prefix("test", "XTEST");
    Xapian::DateRangeProcessor rp_date(1);
    Xapian::NumberRangeProcessor rp_num(2);
    Xapian::RangeProcessor rp_str(3);
    Xapian::NumberRangeProcessor rp_cash(4, "$", RP_REPEATED);
    Xapian::NumberRangeProcessor rp_weight(5, "kg", RP_SUFFIX|RP_REPEATED);
    qp.add_rangeprocessor(&rp_date);
    qp.add_rangeprocessor(&rp_num);
    qp.add_rangeprocessor(&rp_cash);
    qp.add_rangeprocessor(&rp_weight);
    qp.add_rangeprocessor(&rp_str);
    for (const test *p = test_value_range2_queries; p->query; ++p) {
	string expect, parsed;
	if (p->expect)
	    expect = p->expect;
	else
	    expect = "parse error";
	try {
	    Xapian::Query qobj = qp.parse_query(p->query);
	    parsed = qobj.get_description();
	    expect = string("Query(") + expect + ')';
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
DEFINE_TESTCASE(qp_value_range3, writable) {
    Xapian::WritableDatabase db = get_writable_database();
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
}

// Test NumberRangeProcessors with actual data.
DEFINE_TESTCASE(qp_range3, writable) {
    Xapian::WritableDatabase db = get_writable_database();
    double low = -10;
    int steps = 60;
    double step = 0.5;

    for (int i = 0; i <= steps; ++i) {
	double v = low + i * step;
	Xapian::Document doc;
	doc.add_value(1, Xapian::sortable_serialise(v));
	db.add_document(doc);
    }

    Xapian::NumberRangeProcessor rp_num(1);
    Xapian::QueryParser qp;
    qp.add_rangeprocessor(&rp_num);

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
}

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
DEFINE_TESTCASE(qp_value_range4, !backend) {
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
	    expect = string("Query(") + expect + ')';
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

/** Test a boolean filter which happens to contain "..".
 *
 *  Regression test for bug fixed in 1.2.3.
 *
 *  Also test that the same prefix can be set for a range and filter.
 */
DEFINE_TESTCASE(qp_range4, !backend) {
    Xapian::QueryParser qp;
    qp.add_boolean_prefix("id", "Q");
    qp.add_boolean_prefix("hello", "XHELLO");
    Xapian::RangeProcessor rp_str(1, "hello:");
    qp.add_rangeprocessor(&rp_str);
    for (const test *p = test_value_range4_queries; p->query; ++p) {
	string expect, parsed;
	if (p->expect)
	    expect = p->expect;
	else
	    expect = "parse error";
	try {
	    Xapian::Query qobj = qp.parse_query(p->query);
	    parsed = qobj.get_description();
	    expect = string("Query(") + expect + ')';
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
DEFINE_TESTCASE(qp_value_daterange1, !backend) {
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
	    expect = string("Query(") + expect + ')';
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

// Test DateRangeProcessor
DEFINE_TESTCASE(qp_daterange1, !backend) {
    Xapian::QueryParser qp;
    Xapian::DateRangeProcessor rp_date(1, Xapian::RP_DATE_PREFER_MDY, 1960);
    qp.add_rangeprocessor(&rp_date);
    for (const test *p = test_value_daterange1_queries; p->query; ++p) {
	string expect, parsed;
	if (p->expect)
	    expect = p->expect;
	else
	    expect = "parse error";
	try {
	    Xapian::Query qobj = qp.parse_query(p->query);
	    parsed = qobj.get_description();
	    expect = string("Query(") + expect + ')';
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
DEFINE_TESTCASE(qp_value_daterange2, !backend) {
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
	    expect = string("Query(") + expect + ')';
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

// Feature test DateRangeProcessor with prefixes (added in 1.1.2).
DEFINE_TESTCASE(qp_daterange2, !backend) {
    using Xapian::RP_DATE_PREFER_MDY;
    Xapian::QueryParser qp;
    Xapian::DateRangeProcessor rp_cdate(1, "created:", RP_DATE_PREFER_MDY, 1970);
    Xapian::DateRangeProcessor rp_mdate(2, "modified:", RP_DATE_PREFER_MDY, 1970);
    Xapian::DateRangeProcessor rp_adate(3, "accessed:", RP_DATE_PREFER_MDY, 1970);
    // Regression test - here a const char * was taken as a bool rather than a
    // std::string when resolving the overloaded forms.  Fixed in 1.2.13 and
    // 1.3.1.
    Xapian::DateRangeProcessor rp_ddate(4, "deleted:");
    qp.add_rangeprocessor(&rp_cdate);
    qp.add_rangeprocessor(&rp_mdate);
    qp.add_rangeprocessor(&rp_adate);
    qp.add_rangeprocessor(&rp_ddate);
    for (const test *p = test_value_daterange2_queries; p->query; ++p) {
	string expect, parsed;
	if (p->expect)
	    expect = p->expect;
	else
	    expect = "parse error";
	try {
	    Xapian::Query qobj = qp.parse_query(p->query);
	    parsed = qobj.get_description();
	    expect = string("Query(") + expect + ')';
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
DEFINE_TESTCASE(qp_value_stringrange1, !backend) {
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
	    expect = string("Query(") + expect + ')';
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

// Feature test RangeProcessor with prefixes.
DEFINE_TESTCASE(qp_stringrange1, !backend) {
    Xapian::QueryParser qp;
    Xapian::RangeProcessor rp_default(0);
    Xapian::RangeProcessor rp_tag(1, "tag:");
    qp.add_rangeprocessor(&rp_tag);
    qp.add_rangeprocessor(&rp_default);
    for (const test *p = test_value_stringrange1_queries; p->query; ++p) {
	string expect, parsed;
	if (p->expect)
	    expect = p->expect;
	else
	    expect = "parse error";
	try {
	    Xapian::Query qobj = qp.parse_query(p->query);
	    parsed = qobj.get_description();
	    expect = string("Query(") + expect + ')';
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

static const test test_value_customrange1_queries[] = {
    { "mars author:Asimov..Bradbury", "(mars@1 FILTER VALUE_RANGE 4 asimov bradbury)" },
    { NULL, NULL }
};

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

// Test custom ValueRangeProcessor subclass.
DEFINE_TESTCASE(qp_value_customrange1, !backend) {
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
	    expect = string("Query(") + expect + ')';
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

struct AuthorRangeProcessor : public Xapian::RangeProcessor {
    AuthorRangeProcessor() : Xapian::RangeProcessor(4, "author:") { }

    Xapian::Query operator()(const std::string& b, const std::string& e)
    {
	string begin = Xapian::Unicode::tolower(b);
	string end = Xapian::Unicode::tolower(e);
	return Xapian::RangeProcessor::operator()(begin, end);
    }
};

// Test custom RangeProcessor subclass.
DEFINE_TESTCASE(qp_customrange1, !backend) {
    Xapian::QueryParser qp;
    AuthorRangeProcessor rp_author;
    qp.add_rangeprocessor(&rp_author);
    for (const test *p = test_value_customrange1_queries; p->query; ++p) {
	string expect, parsed;
	if (p->expect)
	    expect = p->expect;
	else
	    expect = "parse error";
	try {
	    Xapian::Query qobj = qp.parse_query(p->query);
	    parsed = qobj.get_description();
	    expect = string("Query(") + expect + ')';
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

class TitleFieldProcessor : public Xapian::FieldProcessor {
    Xapian::Query operator()(const std::string & str) {
	if (str == "all")
	    return Xapian::Query::MatchAll;
	return Xapian::Query("S" + str);
    }
};

class HostFieldProcessor : public Xapian::FieldProcessor {
    Xapian::Query operator()(const std::string & str) {
	if (str == "*")
	    return Xapian::Query::MatchAll;
	string res = "H";
	for (string::const_iterator i = str.begin(); i != str.end(); ++i)
	    res += C_tolower(*i);
	return Xapian::Query(res);
    }
};

static const test test_fieldproc1_queries[] = {
    { "title:test", "Stest" },
    { "title:all", "<alldocuments>" },
    { "host:Xapian.org", "0 * Hxapian.org" },
    { "host:*", "0 * <alldocuments>" },
    { "host:\"Space Station.Example.Org\"", "0 * Hspace station.example.org" },
    { NULL, NULL }
};

// FieldProcessor test.
DEFINE_TESTCASE(qp_fieldproc1, !backend) {
    Xapian::QueryParser qp;
    TitleFieldProcessor title_fproc;
    HostFieldProcessor host_fproc;
    qp.add_prefix("title", &title_fproc);
    qp.add_boolean_prefix("host", &host_fproc);
    for (const test *p = test_fieldproc1_queries; p->query; ++p) {
	string expect, parsed;
	if (p->expect)
	    expect = p->expect;
	else
	    expect = "parse error";
	try {
	    Xapian::Query qobj = qp.parse_query(p->query);
	    parsed = qobj.get_description();
	    expect = string("Query(") + expect + ')';
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

class DateRangeFieldProcessor : public Xapian::FieldProcessor {
    Xapian::Query operator()(const std::string & str) {
	// In reality, these would be built from the current date, but for
	// testing it is much simpler to fix the date.
	if (str == "today")
	    return Xapian::Query(Xapian::Query::OP_VALUE_GE, 1, "20120725");
	if (str == "this week")
	    return Xapian::Query(Xapian::Query::OP_VALUE_GE, 1, "20120723");
	if (str == "this month")
	    return Xapian::Query(Xapian::Query::OP_VALUE_GE, 1, "20120701");
	if (str == "this year")
	    return Xapian::Query(Xapian::Query::OP_VALUE_GE, 1, "20120101");
	if (str == "this decade")
	    return Xapian::Query(Xapian::Query::OP_VALUE_GE, 1, "20100101");
	if (str == "this century")
	    return Xapian::Query(Xapian::Query::OP_VALUE_GE, 1, "20000101");
	throw Xapian::QueryParserError("Didn't understand date specification '" + str + "'");
    }
};

static const test test_fieldproc2_queries[] = {
    { "date:\"this week\"", "VALUE_GE 1 20120723" },
    { "date:23/7/2012..25/7/2012", "VALUE_RANGE 1 20120723 20120725" },
    { NULL, NULL }
};

// Test using FieldProcessor and ValueRangeProcessor together.
DEFINE_TESTCASE(qp_fieldproc2, !backend) {
    Xapian::QueryParser qp;
    DateRangeFieldProcessor date_fproc;
    qp.add_boolean_prefix("date", &date_fproc);
    Xapian::DateValueRangeProcessor vrp_date(1, "date:");
    qp.add_valuerangeprocessor(&vrp_date);
    for (const test *p = test_fieldproc2_queries; p->query; ++p) {
	string expect, parsed;
	if (p->expect)
	    expect = p->expect;
	else
	    expect = "parse error";
	try {
	    Xapian::Query qobj = qp.parse_query(p->query);
	    parsed = qobj.get_description();
	    expect = string("Query(") + expect + ')';
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

// Test using FieldProcessor and RangeProcessor together.
DEFINE_TESTCASE(qp_fieldproc3, !backend) {
    Xapian::QueryParser qp;
    DateRangeFieldProcessor date_fproc;
    qp.add_boolean_prefix("date", &date_fproc);
    Xapian::DateRangeProcessor rp_date(1, "date:");
    qp.add_rangeprocessor(&rp_date);
    for (const test *p = test_fieldproc2_queries; p->query; ++p) {
	string expect, parsed;
	if (p->expect)
	    expect = p->expect;
	else
	    expect = "parse error";
	try {
	    Xapian::Query qobj = qp.parse_query(p->query);
	    parsed = qobj.get_description();
	    expect = string("Query(") + expect + ')';
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

DEFINE_TESTCASE(qp_stoplist1, !backend) {
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
DEFINE_TESTCASE(qp_spell1, spelling) {
    Xapian::WritableDatabase db = get_writable_database();

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
			   Xapian::QueryParser::FLAG_BOOLEAN);
	tout << "Query: " << p->query << endl;
	TEST_STRINGS_EQUAL(qp.get_corrected_query_string(), p->expect);
    }

    return true;
}

// Test spelling correction in the QueryParser with multiple databases.
DEFINE_TESTCASE(qp_spell2, spelling)
{
    Xapian::WritableDatabase db1 = get_writable_database();

    db1.add_spelling("document");
    db1.add_spelling("search");
    db1.commit();

    Xapian::WritableDatabase db2 = get_named_writable_database("qp_spell2a");

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
			   Xapian::QueryParser::FLAG_BOOLEAN);
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
DEFINE_TESTCASE(qp_spellwild1, spelling) {
    Xapian::WritableDatabase db = get_writable_database();

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
DEFINE_TESTCASE(qp_spellpartial1, spelling) {
    Xapian::WritableDatabase db = get_writable_database();

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
    { "searching", "(Zsearch@1 SYNONYM Zfind@1 SYNONYM Zlocate@1)" },
    { "search", "(Zsearch@1 SYNONYM find@1)" },
    { "Search", "(search@1 SYNONYM find@1)" },
    { "Searching", "searching@1" },
    { "searching OR terms", "((Zsearch@1 SYNONYM Zfind@1 SYNONYM Zlocate@1) OR Zterm@2)" },
    { "search OR terms", "((Zsearch@1 SYNONYM find@1) OR Zterm@2)" },
    { "search +terms", "(Zterm@2 AND_MAYBE (Zsearch@1 SYNONYM find@1))" },
    { "search -terms", "((Zsearch@1 SYNONYM find@1) AND_NOT Zterm@2)" },
    { "+search terms", "((Zsearch@1 SYNONYM find@1) AND_MAYBE Zterm@2)" },
    { "-search terms", "(Zterm@2 AND_NOT (Zsearch@1 SYNONYM find@1))" },
    { "search terms", "((Zsearch@1 SYNONYM find@1) OR Zterm@2)" },
    // Shouldn't trigger synonyms:
    { "\"search terms\"", "(search@1 PHRASE 2 terms@2)" },
    // Check that setting FLAG_AUTO_SYNONYMS doesn't enable multi-word
    // synonyms.  Regression test for bug fixed in 1.3.0 and 1.2.9.
    { "regression test", "(Zregress@1 OR Ztest@2)" },
    { NULL, NULL }
};

// Test single term synonyms in the QueryParser.
DEFINE_TESTCASE(qp_synonym1, spelling) {
    Xapian::WritableDatabase db = get_writable_database();

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
	string expect = "Query(";
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
    { "sun OR tan OR cream", "((Zsun@1 OR Ztan@2) OR Zcream@3)" },
    { "sun tan", "((Zsun@1 OR Ztan@2) SYNONYM bathe@1)" },
    { "sun tan cream", "((Zsun@1 OR Ztan@2 OR Zcream@3) SYNONYM lotion@1)" },
    { "beach sun tan holiday", "(Zbeach@1 OR ((Zsun@2 OR Ztan@3) SYNONYM bathe@2) OR Zholiday@4)" },
    { "sun tan sun tan cream", "(((Zsun@1 OR Ztan@2) SYNONYM bathe@1) OR ((Zsun@3 OR Ztan@4 OR Zcream@5) SYNONYM lotion@3))" },
    { "single", "(Zsingl@1 SYNONYM record@1)" },
    { NULL, NULL }
};

// Test multi term synonyms in the QueryParser.
DEFINE_TESTCASE(qp_synonym2, synonyms) {
    Xapian::WritableDatabase db = get_writable_database();

    db.add_synonym("sun tan cream", "lotion");
    db.add_synonym("sun tan", "bathe");
    db.add_synonym("single", "record");

    db.commit();

    Xapian::QueryParser qp;
    qp.set_stemmer(Xapian::Stem("english"));
    qp.set_stemming_strategy(Xapian::QueryParser::STEM_SOME);
    qp.set_database(db);

    for (const test *p = test_multi_synonym_queries; p->query; ++p) {
	string expect = "Query(";
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
    { "searching", "Zsearch@1" },
    { "~searching", "(Zsearch@1 SYNONYM Zfind@1 SYNONYM Zlocate@1)" },
    { "~search", "(Zsearch@1 SYNONYM find@1)" },
    { "~Search", "(search@1 SYNONYM find@1)" },
    { "~Searching", "searching@1" },
    { "~searching OR terms", "((Zsearch@1 SYNONYM Zfind@1 SYNONYM Zlocate@1) OR Zterm@2)" },
    { "~search OR terms", "((Zsearch@1 SYNONYM find@1) OR Zterm@2)" },
    { "~search +terms", "(Zterm@2 AND_MAYBE (Zsearch@1 SYNONYM find@1))" },
    { "~search -terms", "((Zsearch@1 SYNONYM find@1) AND_NOT Zterm@2)" },
    { "+~search terms", "((Zsearch@1 SYNONYM find@1) AND_MAYBE Zterm@2)" },
    { "-~search terms", "(Zterm@2 AND_NOT (Zsearch@1 SYNONYM find@1))" },
    { "~search terms", "((Zsearch@1 SYNONYM find@1) OR Zterm@2)" },
    { "~foo:search", "(ZXFOOsearch@1 SYNONYM prefixated@1)" },
    // FIXME: should look for multi-term synonym...
    { "~\"search terms\"", "(search@1 PHRASE 2 terms@2)" },
    { NULL, NULL }
};

// Test the synonym operator in the QueryParser.
DEFINE_TESTCASE(qp_synonym3, synonyms) {
    Xapian::WritableDatabase db = get_writable_database();

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
	string expect = "Query(";
	expect += p->expect;
	expect += ')';
	Xapian::Query q;
	q = qp.parse_query(p->query,
			   Xapian::QueryParser::FLAG_SYNONYM |
			   Xapian::QueryParser::FLAG_BOOLEAN |
			   Xapian::QueryParser::FLAG_LOVEHATE |
			   Xapian::QueryParser::FLAG_PHRASE);
	tout << "Query: " << p->query << endl;
	TEST_STRINGS_EQUAL(q.get_description(), expect);
    }

    return true;
}

static const test test_stem_all_queries[] = {
    { "\"chemical engineers\"", "(chemic@1 PHRASE 2 engin@2)" },
    { "chemical NEAR engineers", "(chemic@1 NEAR 11 engin@2)" },
    { "chemical engineers", "(chemic@1 OR engin@2)" },
    { "title:(chemical engineers)", "(XTchemic@1 OR XTengin@2)" },
    { NULL, NULL }
};

DEFINE_TESTCASE(qp_stem_all1, !backend) {
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
	    expect = string("Query(") + expect + ')';
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
    { "\"chemical engineers\"", "(Zchemic@1 PHRASE 2 Zengin@2)" },
    { "chemical NEAR engineers", "(Zchemic@1 NEAR 11 Zengin@2)" },
    { "chemical engineers", "(Zchemic@1 OR Zengin@2)" },
    { "title:(chemical engineers)", "(ZXTchemic@1 OR ZXTengin@2)" },
    { NULL, NULL }
};

DEFINE_TESTCASE(qp_stem_all_z1, !backend) {
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
	    expect = string("Query(") + expect + ')';
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
DEFINE_TESTCASE(qp_scale1, synonyms) {
    Xapian::WritableDatabase db = get_writable_database();

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
    { "simple-example", "(simple@1 PHRASE 2 example@2)" },
    { "stock -cooking", "(Zstock@1 AND_NOT Zcook@2)" },
// FIXME: these give NEAR 2
//    { "foo -baz bar", "((foo@1 NEAR 11 bar@3) AND_NOT Zbaz@2)" },
//    { "one +two three", "(Ztwo@2 AND_MAYBE (one@1 NEAR 11 three@3))" },
    { "foo bar", "(foo@1 NEAR 11 bar@2)" },
    { "foo bar baz", "(foo@1 NEAR 12 bar@2 NEAR 12 baz@3)" },
    { "gtk+ -gnome", "(Zgtk+@1 AND_NOT Zgnome@2)" },
    { "c++ -d--", "(Zc++@1 AND_NOT Zd@2)" },
    { "\"c++ library\"", "(c++@1 PHRASE 2 library@2)" },
    { "author:orwell animal farm", "(Aorwell@1 NEAR 12 animal@2 NEAR 12 farm@3)" },
    { "author:Orwell Animal Farm", "(Aorwell@1 NEAR 12 animal@2 NEAR 12 farm@3)" },
    { "beer NOT \"orange juice\"", "(Zbeer@1 AND_NOT (orange@2 PHRASE 2 juice@3))" },
    { "beer AND NOT lager", "(Zbeer@1 AND_NOT Zlager@2)" },
    { "A OR B NOT C", "(a@1 OR (b@2 AND_NOT c@3))" },
    { "A OR B AND NOT C", "(a@1 OR (b@2 AND_NOT c@3))" },
    { "A OR B XOR C", "(a@1 OR (b@2 XOR c@3))" },
    { "A XOR B NOT C", "(a@1 XOR (b@2 AND_NOT c@3))" },
    { "one AND two", "(Zone@1 AND Ztwo@2)" },
    { "NOT windows", "Syntax: <expression> NOT <expression>" },
    { "a AND (NOT b)", "Syntax: <expression> NOT <expression>" },
    { "AND NOT windows", "Syntax: <expression> AND NOT <expression>" },
    { "gordian NOT", "Syntax: <expression> NOT <expression>" },
    { "gordian AND NOT", "Syntax: <expression> AND NOT <expression>" },
    { "foo OR (something AND)", "Syntax: <expression> AND <expression>" },
    { "OR foo", "Syntax: <expression> OR <expression>" },
    { "XOR", "Syntax: <expression> XOR <expression>" },
    { "hard\xa0space", "(hard@1 NEAR 11 space@2)" },
    { NULL, NULL }
};

DEFINE_TESTCASE(qp_near1, !backend) {
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
    queryparser.add_boolean_prefix("dogegory", "XDOG", false);
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
	    expect = string("Query(") + expect + ')';
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
    { "simple-example", "(simple@1 PHRASE 2 example@2)" },
    { "stock -cooking", "(Zstock@1 AND_NOT Zcook@2)" },
// FIXME: these give PHRASE 2
//    { "foo -baz bar", "((foo@1 PHRASE 11 bar@3) AND_NOT Zbaz@2)" },
//    { "one +two three", "(Ztwo@2 AND_MAYBE (one@1 PHRASE 11 three@3))" },
    { "foo bar", "(foo@1 PHRASE 11 bar@2)" },
    { "foo bar baz", "(foo@1 PHRASE 12 bar@2 PHRASE 12 baz@3)" },
    { "gtk+ -gnome", "(Zgtk+@1 AND_NOT Zgnome@2)" },
    { "c++ -d--", "(Zc++@1 AND_NOT Zd@2)" },
    { "\"c++ library\"", "(c++@1 PHRASE 2 library@2)" },
    { "author:orwell animal farm", "(Aorwell@1 PHRASE 12 animal@2 PHRASE 12 farm@3)" },
    { "author:Orwell Animal Farm", "(Aorwell@1 PHRASE 12 animal@2 PHRASE 12 farm@3)" },
    { "beer NOT \"orange juice\"", "(Zbeer@1 AND_NOT (orange@2 PHRASE 2 juice@3))" },
    { "beer AND NOT lager", "(Zbeer@1 AND_NOT Zlager@2)" },
    { "A OR B NOT C", "(a@1 OR (b@2 AND_NOT c@3))" },
    { "A OR B AND NOT C", "(a@1 OR (b@2 AND_NOT c@3))" },
    { "A OR B XOR C", "(a@1 OR (b@2 XOR c@3))" },
    { "A XOR B NOT C", "(a@1 XOR (b@2 AND_NOT c@3))" },
    { "one AND two", "(Zone@1 AND Ztwo@2)" },
    { "NOT windows", "Syntax: <expression> NOT <expression>" },
    { "a AND (NOT b)", "Syntax: <expression> NOT <expression>" },
    { "AND NOT windows", "Syntax: <expression> AND NOT <expression>" },
    { "gordian NOT", "Syntax: <expression> NOT <expression>" },
    { "gordian AND NOT", "Syntax: <expression> AND NOT <expression>" },
    { "foo OR (something AND)", "Syntax: <expression> AND <expression>" },
    { "OR foo", "Syntax: <expression> OR <expression>" },
    { "XOR", "Syntax: <expression> XOR <expression>" },
    { "hard\xa0space", "(hard@1 PHRASE 11 space@2)" },
    // FIXME: this isn't what we want, but fixing phrase to work with
    // subqueries first might be the best approach.
    // FIXME: this isn't currently reimplemented:
    // { "(one AND two) three", "((Zone@1 PHRASE 11 Zthree@3) AND (Ztwo@2 PHRASE 11 Zthree@3))" },
    { NULL, NULL }
};

DEFINE_TESTCASE(qp_phrase1, !backend) {
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
    queryparser.add_boolean_prefix("dogegory", "XDOG", false);
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
	    expect = string("Query(") + expect + ')';
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
    { "this is a test", "test@4" },
    { "test*", "(SYNONYM WILDCARD OR test)" },
    { "a test*", "(SYNONYM WILDCARD OR test)" },
    { "is a test*", "(SYNONYM WILDCARD OR test)" },
    { "this is a test*", "(SYNONYM WILDCARD OR test)" },
    { "this is a us* test*", "((SYNONYM WILDCARD OR us) OR (SYNONYM WILDCARD OR test))" },
    { "this is a user test*", "(user@4 OR (SYNONYM WILDCARD OR test))" },
    { NULL, NULL }
};

static const test test_stopword_group_and_queries[] = {
    { "this is a test", "test@4" },
    { "test*", "(SYNONYM WILDCARD OR test)" },
    { "a test*", "(SYNONYM WILDCARD OR test)" },
    // Two stopwords + one wildcard failed in 1.0.16
    { "is a test*", "(SYNONYM WILDCARD OR test)" },
    // Three stopwords + one wildcard failed in 1.0.16
    { "this is a test*", "(SYNONYM WILDCARD OR test)" },
    // Three stopwords + two wildcards failed in 1.0.16
    { "this is a us* test*", "((SYNONYM WILDCARD OR us) AND (SYNONYM WILDCARD OR test))" },
    { "this is a user test*", "(user@4 AND (SYNONYM WILDCARD OR test))" },
    { NULL, NULL }
};

// Regression test for bug fixed in 1.0.17 and 1.1.3.
DEFINE_TESTCASE(qp_stopword_group1, writable) {
    Xapian::WritableDatabase db = get_writable_database();
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
		expect = string("Query(") + expect + ')';
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
}

/// Check that QueryParser::set_default_op() rejects inappropriate ops.
DEFINE_TESTCASE(qp_default_op2, !backend) {
    Xapian::QueryParser qp;
    static const Xapian::Query::op ops[] = {
	Xapian::Query::OP_AND_NOT,
	Xapian::Query::OP_XOR,
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
	TEST_EXCEPTION(Xapian::InvalidArgumentError,
		       qp.set_default_op(*p));
	TEST_EQUAL(qp.get_default_op(), Xapian::Query::OP_OR);
    }
    return true;
}

struct qp_default_op3_test {
    Xapian::Query::op op;
    const char *expect;
};

/// Check that QueryParser::set_default_op() accepts appropriate ops.
DEFINE_TESTCASE(qp_default_op3, !backend) {
    Xapian::QueryParser qp;
    static const qp_default_op3_test tests[] = {
	{ Xapian::Query::OP_AND,
	  "Query((a@1 AND b@2 AND c@3))" },
	{ Xapian::Query::OP_OR,
	  "Query((a@1 OR b@2 OR c@3))" },
	{ Xapian::Query::OP_PHRASE,
	  "Query((a@1 PHRASE 12 b@2 PHRASE 12 c@3))" },
	{ Xapian::Query::OP_NEAR,
	  "Query((a@1 NEAR 12 b@2 NEAR 12 c@3))" },
	{ Xapian::Query::OP_ELITE_SET,
	  "Query((a@1 ELITE_SET 10 b@2 ELITE_SET 10 c@3))" },
	{ Xapian::Query::OP_SYNONYM,
	  "Query((a@1 SYNONYM b@2 SYNONYM c@3))" },
    };
    const qp_default_op3_test * p;
    for (p = tests; p - tests != sizeof(tests) / sizeof(*tests); ++p) {
	tout << p->op << endl;
	qp.set_default_op(p->op);
	// Check that get_default_op() returns what we just set.
	TEST_EQUAL(qp.get_default_op(), p->op);
	TEST_EQUAL(qp.parse_query("A B C").get_description(), p->expect);
    }
    return true;
}

/// Test that the default strategy is now STEM_SOME (as of 1.3.1).
DEFINE_TESTCASE(qp_defaultstrategysome1, !backend) {
    Xapian::QueryParser qp;
    qp.set_stemmer(Xapian::Stem("en"));
    TEST_EQUAL(qp.parse_query("testing").get_description(), "Query(Ztest@1)");
    return true;
}
