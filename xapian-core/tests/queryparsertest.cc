/* queryparsertest.cc: Tests of Xapian::QueryParser
 *
 * Copyright (C) 2002,2003,2004,2005,2006 Olly Betts
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
#include <iostream>
#include <string>

using namespace std;

#define TESTCASE(S) {#S, test_##S}
#define END_OF_TESTCASES {0, 0}

#include "testsuite.h"

struct test {
    const char *query;
    const char *expect;
};

static test test_or_queries[] = {
    { "simple-example", "(simpl:(pos=1) PHRASE 2 exampl:(pos=2))" },
    { "time_t", "(time:(pos=1) PHRASE 2 t:(pos=2))" },
    { "stock -cooking", "(stock:(pos=1) AND_NOT cook:(pos=2))" },
    { "d- school report", "(d-:(pos=1) OR school:(pos=2) OR report:(pos=3))" },
    { "gtk+ -gnome", "(gtk+:(pos=1) AND_NOT gnome:(pos=2))" },
    { "c++ -d--", "(c++:(pos=1) AND_NOT d--:(pos=2))" },
    { "Mg2+ Cl-", "(Rmg2+:(pos=1) OR Rcl-:(pos=2))" },
    { "\"c++ library\"", "(c++:(pos=1) PHRASE 2 librari:(pos=2))" },
    { "A&L A&RMCO AD&D", "(Ra&l:(pos=1) OR Ra&rmco:(pos=2) OR Rad&d:(pos=3))" },
    { "C# vs C++", "(Rc#:(pos=1) OR vs:(pos=2) OR Rc++:(pos=3))" },
    { "j##", "j#:(pos=1)" },
    { "a#b", "(a:(pos=1) OR b:(pos=2))" },
    { "O.K. U.N.C.L.E XY.Z.", "(Rok:(pos=1) OR Runcle:(pos=2) OR (Rxy:(pos=3) PHRASE 2 Rz:(pos=4)))" },
    { "author:orwell animal farm", "(Aorwel:(pos=1) OR anim:(pos=2) OR farm:(pos=3))" },
    { "h\xf6hle", "hoehl:(pos=1)" },
    { "one +two three", "(two:(pos=2) AND_MAYBE (one:(pos=1) OR three:(pos=3)))" },
    { "subject:test other", "(XTtest:(pos=1) OR other:(pos=2))" },
    { "subject:\"space flight\"", "(XTspace:(pos=1) PHRASE 2 XTflight:(pos=2))" },
    { "author:(twain OR poe) OR flight", "(Atwain:(pos=1) OR Apoe:(pos=2) OR flight:(pos=3))" },
    { "author:(twain OR title:pit OR poe)", "(Atwain:(pos=1) OR XTpit:(pos=2) OR Apoe:(pos=3))" },
    { "title:2001 title:space", "(XT2001:(pos=1) OR XTspace:(pos=2))" },
    { "(title:help)", "XThelp:(pos=1)" },
    { "beer NOT \"orange juice\"", "(beer:(pos=1) AND_NOT (orang:(pos=2) PHRASE 2 juic:(pos=3)))" },
    { "beer AND NOT lager", "(beer:(pos=1) AND_NOT lager:(pos=2))" },
    { "one AND two", "(one:(pos=1) AND two:(pos=2))" },
    { "one A.N.D. two", "(one:(pos=1) OR Rand:(pos=2) OR two:(pos=3))" },
    { "one \xc1ND. two", "(one:(pos=1) OR Rand:(pos=2) OR two:(pos=3))" },
    { "one author:AND two", "(one:(pos=1) OR ARand:(pos=2) OR two:(pos=3))" },
    { "author:hyphen-ated", "(Ahyphen:(pos=1) PHRASE 2 Aate:(pos=2))" },
    { "cvs site:xapian.org", "(cvs:(pos=1) FILTER Hxapian.org)" },
    { "cvs -site:xapian.org", "(cvs:(pos=1) AND_NOT Hxapian.org)" },
    { "site:xapian.org mail", "(mail:(pos=1) FILTER Hxapian.org)" },
    { "-site:xapian.org mail", "(mail:(pos=1) AND_NOT Hxapian.org)" },
    { "site:xapian.org", "Hxapian.org" },
    { "mug +site:xapian.org -site:cvs.xapian.org", "((mug:(pos=1) AND_NOT Hcvs.xapian.org) FILTER Hxapian.org)" },
    { "mug -site:cvs.xapian.org +site:xapian.org", "((mug:(pos=1) AND_NOT Hcvs.xapian.org) FILTER Hxapian.org)" },
    { "NOT windows", "Syntax: <expression> NOT <expression>" },
    { "AND NOT windows", "Syntax: <expression> AND NOT <expression>" },
    { "gordian NOT", "Syntax: <expression> NOT <expression>" },
    { "gordian AND NOT", "Syntax: <expression> AND NOT <expression>" },
    { "foo OR (something AND)", "Syntax: <expression> AND <expression>" },
    { "OR foo", "Syntax: <expression> OR <expression>" },
    { "XOR", "Syntax: <expression> XOR <expression>" },
    { "hard\xa0space", "(hard:(pos=1) OR space:(pos=2))" },
    { " white\r\nspace\ttest ", "(white:(pos=1) OR space:(pos=2) OR test:(pos=3))" },
    { "one AND two/three", "(one:(pos=1) AND (two:(pos=2) PHRASE 2 three:(pos=3)))" },
    { "one AND /two/three", "(one:(pos=1) AND (two:(pos=2) PHRASE 2 three:(pos=3)))" },
    { "one AND/two/three", "(one:(pos=1) AND (two:(pos=2) PHRASE 2 three:(pos=3)))" },
    { "one +/two/three", "((two:(pos=2) PHRASE 2 three:(pos=3)) AND_MAYBE one:(pos=1))" },
    { "one//two", "(one:(pos=1) PHRASE 2 two:(pos=2))" },
    { "\"missing quote", "(miss:(pos=1) PHRASE 2 quot:(pos=2))" },
    { "DVD+RW", "(Rdvd:(pos=1) OR Rrw:(pos=2))" }, // Would a phrase be better?
    { "+\"must have\" optional", "((must:(pos=1) PHRASE 2 have:(pos=2)) AND_MAYBE option:(pos=3))" },
    // Real world examples from tweakers.net:
    { "Call to undefined function: imagecreate()", "(Rcall:(pos=1) OR to:(pos=2) OR undefin:(pos=3) OR function:(pos=4) OR imagecr:(pos=5))" },
    { "mysql_fetch_row(): supplied argument is not a valid MySQL result resource", "((mysql:(pos=1) PHRASE 3 fetch:(pos=2) PHRASE 3 row:(pos=3)) OR suppli:(pos=4) OR argument:(pos=5) OR is:(pos=6) OR not:(pos=7) OR a:(pos=8) OR valid:(pos=9) OR Rmysql:(pos=10) OR result:(pos=11) OR resourc:(pos=12))" },
    { "php date() nedelands", "(php:(pos=1) OR date:(pos=2) OR nedeland:(pos=3))" },
    { "wget domein --http-user", "(wget:(pos=1) OR domein:(pos=2) OR (http:(pos=3) PHRASE 2 user:(pos=4)))" },
    { "@home problemen", "(home:(pos=1) OR problemen:(pos=2))" },
    { "'ipacsum'", "ipacsum:(pos=1)" },
    { "canal + ", "canal:(pos=1)" },
    { "/var/run/mysqld/mysqld.sock", "(var:(pos=1) PHRASE 5 run:(pos=2) PHRASE 5 mysqld:(pos=3) PHRASE 5 mysqld:(pos=4) PHRASE 5 sock:(pos=5))" },
    { "\"QSI-161 drivers\"", "(Rqsi:(pos=1) PHRASE 3 161:(pos=2) PHRASE 3 driver:(pos=3))" },
    { "\"e-cube\" barebone", "((e:(pos=1) PHRASE 2 cube:(pos=2)) OR barebon:(pos=3))" },
    { "\"./httpd: symbol not found: dlopen\"", "(httpd:(pos=1) PHRASE 5 symbol:(pos=2) PHRASE 5 not:(pos=3) PHRASE 5 found:(pos=4) PHRASE 5 dlopen:(pos=5))" },
    { "ERROR 2003: Can't connect to MySQL server on 'localhost' (10061)",
      "(Rerror:(pos=1) OR 2003:(pos=2) OR (Rcan:(pos=3) PHRASE 2 t:(pos=4)) OR connect:(pos=5) OR to:(pos=6) OR Rmysql:(pos=7) OR server:(pos=8) OR on:(pos=9) OR localhost:(pos=10) OR 10061:(pos=11))" },
    { "location.href = \"\"", "(locat:(pos=1) PHRASE 2 href:(pos=2))" },
    { "method=\"post\" action=\"\">", "(method:(pos=1) OR post:(pos=2) OR action:(pos=3))" },
    { "behuizing 19\" inch", "(behuiz:(pos=1) OR 19:(pos=2) OR inch:(pos=3))" },
    { "19\" rack", "(19:(pos=1) OR rack:(pos=2))" },
    { "3,5\" mainboard", "(3:(pos=1) OR 5:(pos=2) OR mainboard:(pos=3))" },
    { "553 sorry, that domain isn't in my list of allowed rcpthosts (#5.7.1)",
      "(553:(pos=1) OR sorri:(pos=2) OR that:(pos=3) OR domain:(pos=4) OR (isn:(pos=5) PHRASE 2 t:(pos=6)) OR in:(pos=7) OR my:(pos=8) OR list:(pos=9) OR of:(pos=10) OR allow:(pos=11) OR rcpthost:(pos=12) OR (5:(pos=13) PHRASE 3 7:(pos=14) PHRASE 3 1:(pos=15)))" },
    { "data error (clic redundancy check)", "(data:(pos=1) OR error:(pos=2) OR clic:(pos=3) OR redund:(pos=4) OR check:(pos=5))" },
    { "? mediaplayer 9\"", "(mediaplay:(pos=1) OR 9:(pos=2))" },
    { "date(\"w\")", "(date:(pos=1) OR w:(pos=2))" },
    { "Syntaxisfout (operator ontbreekt ASP", "(Rsyntaxisfout:(pos=1) OR oper:(pos=2) OR ontbreekt:(pos=3) OR Rasp:(pos=4))" },
    { "Request.ServerVariables(\"logon_user\")", "((Rrequest:(pos=1) PHRASE 2 Rservervariables:(pos=2)) OR (logon:(pos=3) PHRASE 2 user:(pos=4)))" },
    { "ASP \"request.form\" van \\\"enctype=\"MULTIPART/FORM-DATA\"\\\"",
      "(Rasp:(pos=1) OR (request:(pos=2) PHRASE 2 form:(pos=3)) OR van:(pos=4) OR enctyp:(pos=5) OR (Rmultipart:(pos=6) PHRASE 3 Rform:(pos=7) PHRASE 3 Rdata:(pos=8)))" },
    { "USER ftp (Login failed): Invalid shell: /sbin/nologin", "(Ruser:(pos=1) OR ftp:(pos=2) OR Rlogin:(pos=3) OR fail:(pos=4) OR Rinvalid:(pos=5) OR shell:(pos=6) OR (sbin:(pos=7) PHRASE 2 nologin:(pos=8)))" },
    { "ip_masq_new(proto=TCP)", "((ip:(pos=1) PHRASE 3 masq:(pos=2) PHRASE 3 new:(pos=3)) OR proto:(pos=4) OR Rtcp:(pos=5))" },
    { "\"document.write(\"", "(document:(pos=1) PHRASE 2 write:(pos=2))" },
    { "ERROR 1045: Access denied for user: 'root@localhost' (Using password: NO)", "(Rerror:(pos=1) OR 1045:(pos=2) OR Raccess:(pos=3) OR deni:(pos=4) OR for:(pos=5) OR user:(pos=6) OR (root:(pos=7) PHRASE 2 localhost:(pos=8)) OR Rusing:(pos=9) OR password:(pos=10) OR Rno:(pos=11))" },
    { "TIP !! subtitles op TV-out (via DVD max g400)", "(Rtip:(pos=1) OR subtitl:(pos=2) OR op:(pos=3) OR (Rtv:(pos=4) PHRASE 2 out:(pos=5)) OR via:(pos=6) OR Rdvd:(pos=7) OR max:(pos=8) OR g400:(pos=9))" },
    { "Gigabyte 8PE667 (de Ultra versie) of Asus A7N8X Deluxe", "(Rgigabyte:(pos=1) OR 8pe667:(pos=2) OR de:(pos=3) OR Rultra:(pos=4) OR versi:(pos=5) OR of:(pos=6) OR Rasus:(pos=7) OR Ra7n8x:(pos=8) OR Rdeluxe:(pos=9))" },
    { "\"1) Ze testen 8x AF op de GFFX tegen \"", "(1:(pos=1) PHRASE 9 Rze:(pos=2) PHRASE 9 testen:(pos=3) PHRASE 9 8x:(pos=4) PHRASE 9 Raf:(pos=5) PHRASE 9 op:(pos=6) PHRASE 9 de:(pos=7) PHRASE 9 Rgffx:(pos=8) PHRASE 9 tegen:(pos=9))" },
    { "\") Ze houden geen rekening met de kwaliteit van AF. Als ze dat gedaan hadden dan waren ze tot de conclusie gekomen dat Performance AF (dus Bilinear AF) op de 9700Pro goed te vergelijken is met Balanced AF op de GFFX. En dan hadden ze ook gezien dat de GFFX niet kan tippen aan de Quality AF van de 9700Pro.\"", "(Rze:(pos=1) PHRASE 59 houden:(pos=2) PHRASE 59 geen:(pos=3) PHRASE 59 reken:(pos=4) PHRASE 59 met:(pos=5) PHRASE 59 de:(pos=6) PHRASE 59 kwaliteit:(pos=7) PHRASE 59 van:(pos=8) PHRASE 59 Raf:(pos=9) PHRASE 59 Rals:(pos=10) PHRASE 59 ze:(pos=11) PHRASE 59 dat:(pos=12) PHRASE 59 gedaan:(pos=13) PHRASE 59 hadden:(pos=14) PHRASE 59 dan:(pos=15) PHRASE 59 waren:(pos=16) PHRASE 59 ze:(pos=17) PHRASE 59 tot:(pos=18) PHRASE 59 de:(pos=19) PHRASE 59 conclusi:(pos=20) PHRASE 59 gekomen:(pos=21) PHRASE 59 dat:(pos=22) PHRASE 59 Rperformance:(pos=23) PHRASE 59 Raf:(pos=24) PHRASE 59 dus:(pos=25) PHRASE 59 Rbilinear:(pos=26) PHRASE 59 Raf:(pos=27) PHRASE 59 op:(pos=28) PHRASE 59 de:(pos=29) PHRASE 59 9700pro:(pos=30) PHRASE 59 go:(pos=31) PHRASE 59 te:(pos=32) PHRASE 59 vergelijken:(pos=33) PHRASE 59 is:(pos=34) PHRASE 59 met:(pos=35) PHRASE 59 Rbalanced:(pos=36) PHRASE 59 Raf:(pos=37) PHRASE 59 op:(pos=38) PHRASE 59 de:(pos=39) PHRASE 59 Rgffx:(pos=40) PHRASE 59 Ren:(pos=41) PHRASE 59 dan:(pos=42) PHRASE 59 hadden:(pos=43) PHRASE 59 ze:(pos=44) PHRASE 59 ook:(pos=45) PHRASE 59 gezien:(pos=46) PHRASE 59 dat:(pos=47) PHRASE 59 de:(pos=48) PHRASE 59 Rgffx:(pos=49) PHRASE 59 niet:(pos=50) PHRASE 59 kan:(pos=51) PHRASE 59 tippen:(pos=52) PHRASE 59 aan:(pos=53) PHRASE 59 de:(pos=54) PHRASE 59 Rquality:(pos=55) PHRASE 59 Raf:(pos=56) PHRASE 59 van:(pos=57) PHRASE 59 de:(pos=58) PHRASE 59 9700pro:(pos=59))" },
    { "\"Ze houden geen rekening met de kwaliteit van AF. Als ze dat gedaan hadden dan waren ze tot de conclusie gekomen dat Performance AF (dus Bilinear AF) op de 9700Pro goed te vergelijken is met Balanced AF op de GFFX. En dan hadden ze ook gezien dat de GFFX niet kan tippen aan de Quality AF van de 9700Pro.\"", "(Rze:(pos=1) PHRASE 59 houden:(pos=2) PHRASE 59 geen:(pos=3) PHRASE 59 reken:(pos=4) PHRASE 59 met:(pos=5) PHRASE 59 de:(pos=6) PHRASE 59 kwaliteit:(pos=7) PHRASE 59 van:(pos=8) PHRASE 59 Raf:(pos=9) PHRASE 59 Rals:(pos=10) PHRASE 59 ze:(pos=11) PHRASE 59 dat:(pos=12) PHRASE 59 gedaan:(pos=13) PHRASE 59 hadden:(pos=14) PHRASE 59 dan:(pos=15) PHRASE 59 waren:(pos=16) PHRASE 59 ze:(pos=17) PHRASE 59 tot:(pos=18) PHRASE 59 de:(pos=19) PHRASE 59 conclusi:(pos=20) PHRASE 59 gekomen:(pos=21) PHRASE 59 dat:(pos=22) PHRASE 59 Rperformance:(pos=23) PHRASE 59 Raf:(pos=24) PHRASE 59 dus:(pos=25) PHRASE 59 Rbilinear:(pos=26) PHRASE 59 Raf:(pos=27) PHRASE 59 op:(pos=28) PHRASE 59 de:(pos=29) PHRASE 59 9700pro:(pos=30) PHRASE 59 go:(pos=31) PHRASE 59 te:(pos=32) PHRASE 59 vergelijken:(pos=33) PHRASE 59 is:(pos=34) PHRASE 59 met:(pos=35) PHRASE 59 Rbalanced:(pos=36) PHRASE 59 Raf:(pos=37) PHRASE 59 op:(pos=38) PHRASE 59 de:(pos=39) PHRASE 59 Rgffx:(pos=40) PHRASE 59 Ren:(pos=41) PHRASE 59 dan:(pos=42) PHRASE 59 hadden:(pos=43) PHRASE 59 ze:(pos=44) PHRASE 59 ook:(pos=45) PHRASE 59 gezien:(pos=46) PHRASE 59 dat:(pos=47) PHRASE 59 de:(pos=48) PHRASE 59 Rgffx:(pos=49) PHRASE 59 niet:(pos=50) PHRASE 59 kan:(pos=51) PHRASE 59 tippen:(pos=52) PHRASE 59 aan:(pos=53) PHRASE 59 de:(pos=54) PHRASE 59 Rquality:(pos=55) PHRASE 59 Raf:(pos=56) PHRASE 59 van:(pos=57) PHRASE 59 de:(pos=58) PHRASE 59 9700pro:(pos=59))" },
    { "$structure = imap_header($mbox, $tt);", "(structur:(pos=1) OR (imap:(pos=2) PHRASE 2 header:(pos=3)) OR mbox:(pos=4) OR tt:(pos=5))" },
    { "\"ifup: Could not get a valid interface name: -> skipped\"", "(ifup:(pos=1) PHRASE 9 Rcould:(pos=2) PHRASE 9 not:(pos=3) PHRASE 9 get:(pos=4) PHRASE 9 a:(pos=5) PHRASE 9 valid:(pos=6) PHRASE 9 interfac:(pos=7) PHRASE 9 name:(pos=8) PHRASE 9 skip:(pos=9))" },
    { "Er kan geen combinatie van filters worden gevonden om de gegevensstroom te genereren. (Error=80040218)", "(Rer:(pos=1) OR kan:(pos=2) OR geen:(pos=3) OR combinati:(pos=4) OR van:(pos=5) OR filter:(pos=6) OR worden:(pos=7) OR gevonden:(pos=8) OR om:(pos=9) OR de:(pos=10) OR gegevensstroom:(pos=11) OR te:(pos=12) OR genereren:(pos=13) OR Rerror:(pos=14) OR 80040218:(pos=15))" },
    { "ereg_replace(\"\\\\\",\"\\/\"", "(ereg:(pos=1) PHRASE 2 replac:(pos=2))" },
    { "\\\\\"divx+geen+geluid\\\\\"", "(divx:(pos=1) PHRASE 3 geen:(pos=2) PHRASE 3 geluid:(pos=3))" },
    { "lcase(\"string\")", "(lcase:(pos=1) OR string:(pos=2))" },
    { "isEmpty( )  functie in visual basic", "(isempti:(pos=1) OR functi:(pos=2) OR in:(pos=3) OR visual:(pos=4) OR basic:(pos=5))" },
    { "*** stop: 0x0000001E (0xC0000005,0x00000000,0x00000000,0x00000000)", "(stop:(pos=1) OR 0x0000001e:(pos=2) OR 0xc0000005:(pos=3) OR 0x00000000:(pos=4) OR 0x00000000:(pos=5) OR 0x00000000:(pos=6))" },
    { "\"ctrl+v+c+a fout\"", "(ctrl:(pos=1) PHRASE 5 v:(pos=2) PHRASE 5 c:(pos=3) PHRASE 5 a:(pos=4) PHRASE 5 fout:(pos=5))" },
    { "Server.CreateObject(\"ADODB.connection\")", "((Rserver:(pos=1) PHRASE 2 Rcreateobject:(pos=2)) OR (Radodb:(pos=3) PHRASE 2 connect:(pos=4)))" },
    { "Presario 6277EA-XP model P4/28 GHz-120GB-DVD-CDRW (512MBWXP) (470048-012)", "(Rpresario:(pos=1) OR (6277ea:(pos=2) PHRASE 2 Rxp:(pos=3)) OR model:(pos=4) OR (Rp4:(pos=5) PHRASE 2 28:(pos=6)) OR (Rghz:(pos=7) PHRASE 4 120gb:(pos=8) PHRASE 4 Rdvd:(pos=9) PHRASE 4 Rcdrw:(pos=10)) OR 512mbwxp:(pos=11) OR (470048:(pos=12) PHRASE 2 012:(pos=13)))" },
    { "Failed to connect agent. (AGENT=dbaxchg2, EC=UserId =NUll)", "(Rfailed:(pos=1) OR to:(pos=2) OR connect:(pos=3) OR agent:(pos=4) OR Ragent:(pos=5) OR dbaxchg2:(pos=6) OR Rec:(pos=7) OR Ruserid:(pos=8) OR Rnull:(pos=9))" },
    { "delphi CreateOleObject(\"MSXML2.DomDocument\")", "(delphi:(pos=1) OR Rcreateoleobject:(pos=2) OR (Rmsxml2:(pos=3) PHRASE 2 Rdomdocument:(pos=4)))" },
    { "Unhandled exeption in IEXPLORE.EXE (FTAPP.DLL)", "(Runhandled:(pos=1) OR exept:(pos=2) OR in:(pos=3) OR (Riexplore:(pos=4) PHRASE 2 Rexe:(pos=5)) OR (Rftapp:(pos=6) PHRASE 2 Rdll:(pos=7)))" },
    { "IBM High Rate Wireless LAN PCI Adapter (Low Profile Enabled)", "(Ribm:(pos=1) OR Rhigh:(pos=2) OR Rrate:(pos=3) OR Rwireless:(pos=4) OR Rlan:(pos=5) OR Rpci:(pos=6) OR Radapter:(pos=7) OR Rlow:(pos=8) OR Rprofile:(pos=9) OR Renabled:(pos=10))" },
    { "asp ' en \"", "(asp:(pos=1) OR en:(pos=2))" },
    { "Hercules 3D Prophet 8500 LE 64MB (OEM, Radeon 8500 LE)", "(Rhercules:(pos=1) OR 3d:(pos=2) OR Rprophet:(pos=3) OR 8500:(pos=4) OR Rle:(pos=5) OR 64mb:(pos=6) OR Roem:(pos=7) OR Rradeon:(pos=8) OR 8500:(pos=9) OR Rle:(pos=10))" },
    { "session_set_cookie_params(echo \"hoi\")", "((session:(pos=1) PHRASE 4 set:(pos=2) PHRASE 4 cooki:(pos=3) PHRASE 4 param:(pos=4)) OR echo:(pos=5) OR hoi:(pos=6))" },
    { "windows update werkt niet (windows se", "(window:(pos=1) OR updat:(pos=2) OR werkt:(pos=3) OR niet:(pos=4) OR window:(pos=5) OR se:(pos=6))" },
    { "De statuscode van de fout is ( 0 x 4 , 0 , 0 , 0 )", "(Rde:(pos=1) OR statuscod:(pos=2) OR van:(pos=3) OR de:(pos=4) OR fout:(pos=5) OR is:(pos=6) OR 0:(pos=7) OR x:(pos=8) OR 4:(pos=9) OR 0:(pos=10) OR 0:(pos=11) OR 0:(pos=12))" },
    { "sony +(u20 u-20)", "((u20:(pos=2) OR (u:(pos=3) PHRASE 2 20:(pos=4))) AND_MAYBE soni:(pos=1))" },
    { "[crit] (17)File exists: unable to create scoreboard (name-based shared memory failure)", "(crit:(pos=1) OR 17:(pos=2) OR Rfile:(pos=3) OR exist:(pos=4) OR unabl:(pos=5) OR to:(pos=6) OR creat:(pos=7) OR scoreboard:(pos=8) OR (name:(pos=9) PHRASE 2 base:(pos=10)) OR share:(pos=11) OR memori:(pos=12) OR failur:(pos=13))" },
    { "directories lokaal php (uitlezen OR inladen)", "(directori:(pos=1) OR lokaal:(pos=2) OR php:(pos=3) OR uitlezen:(pos=4) OR inladen:(pos=5))" },
    { "(multi pc modem)+ (line sync)", "(multi:(pos=1) OR pc:(pos=2) OR modem:(pos=3) OR line:(pos=4) OR sync:(pos=5))" },
    { "xp 5.1.2600.0 (xpclient.010817-1148)", "(xp:(pos=1) OR (5:(pos=2) PHRASE 4 1:(pos=3) PHRASE 4 2600:(pos=4) PHRASE 4 0:(pos=5)) OR (xpclient:(pos=6) PHRASE 3 010817:(pos=7) PHRASE 3 1148:(pos=8)))" },
    { "DirectDraw test results: Failure at step 5 (User verification of rectangles): HRESULT = 0x00000000 (error code) Direct3D 7 test results: Failure at step 32 (User verification of Direct3D rendering): HRESULT = 0x00000000 (error code) Direct3D 8 test results: Failure at step 32 (User verification of Direct3D rendering): HRESULT = 0x00000000 (error code) Direct3D 9 test results: Failure at step 32 (User verification of Direct3D rendering): HRESULT = 0x00000000 (error code)", "(Rdirectdraw:(pos=1) OR test:(pos=2) OR result:(pos=3) OR Rfailure:(pos=4) OR at:(pos=5) OR step:(pos=6) OR 5:(pos=7) OR Ruser:(pos=8) OR verif:(pos=9) OR of:(pos=10) OR rectangl:(pos=11) OR Rhresult:(pos=12) OR 0x00000000:(pos=13) OR error:(pos=14) OR code:(pos=15) OR Rdirect3d:(pos=16) OR 7:(pos=17) OR test:(pos=18) OR result:(pos=19) OR Rfailure:(pos=20) OR at:(pos=21) OR step:(pos=22) OR 32:(pos=23) OR Ruser:(pos=24) OR verif:(pos=25) OR of:(pos=26) OR Rdirect3d:(pos=27) OR render:(pos=28) OR Rhresult:(pos=29) OR 0x00000000:(pos=30) OR error:(pos=31) OR code:(pos=32) OR Rdirect3d:(pos=33) OR 8:(pos=34) OR test:(pos=35) OR result:(pos=36) OR Rfailure:(pos=37) OR at:(pos=38) OR step:(pos=39) OR 32:(pos=40) OR Ruser:(pos=41) OR verif:(pos=42) OR of:(pos=43) OR Rdirect3d:(pos=44) OR render:(pos=45) OR Rhresult:(pos=46) OR 0x00000000:(pos=47) OR error:(pos=48) OR code:(pos=49) OR Rdirect3d:(pos=50) OR 9:(pos=51) OR test:(pos=52) OR result:(pos=53) OR Rfailure:(pos=54) OR at:(pos=55) OR step:(pos=56) OR 32:(pos=57) OR Ruser:(pos=58) OR verif:(pos=59) OR of:(pos=60) OR Rdirect3d:(pos=61) OR render:(pos=62) OR Rhresult:(pos=63) OR 0x00000000:(pos=64) OR error:(pos=65) OR code:(pos=66))" },
    { "Thermaltake Aquarius II waterkoeling (kompleet voor P4 en XP)", "(Rthermaltake:(pos=1) OR Raquarius:(pos=2) OR Rii:(pos=3) OR waterkoel:(pos=4) OR kompleet:(pos=5) OR voor:(pos=6) OR Rp4:(pos=7) OR en:(pos=8) OR Rxp:(pos=9))" },
    { "E3501 unable to add job to database (EC=-2005)", "(Re3501:(pos=1) OR unabl:(pos=2) OR to:(pos=3) OR add:(pos=4) OR job:(pos=5) OR to:(pos=6) OR databas:(pos=7) OR Rec:(pos=8) OR 2005:(pos=9))" },
    { "\"arp -s\" ip veranderen", "((arp:(pos=1) PHRASE 2 s:(pos=2)) OR ip:(pos=3) OR veranderen:(pos=4))" },
    { "header(\"content-type: application/octet-stream\");", "(header:(pos=1) OR (content:(pos=2) PHRASE 2 type:(pos=3)) OR (applic:(pos=4) PHRASE 3 octet:(pos=5) PHRASE 3 stream:(pos=6)))" },
    { "$datum = date(\"d-m-Y\");", "(datum:(pos=1) OR date:(pos=2) OR (d:(pos=3) PHRASE 3 m:(pos=4) PHRASE 3 Ry:(pos=5)))" },
    { "\"'\" +asp", "asp:(pos=1)" },
    { "+session +[", "session:(pos=1)" },
    { "Dit apparaat kan niet starten. (Code 10)", "(Rdit:(pos=1) OR apparaat:(pos=2) OR kan:(pos=3) OR niet:(pos=4) OR starten:(pos=5) OR Rcode:(pos=6) OR 10:(pos=7))" },
    { "\"You cannot use the Administration program while the Domino Server is running. Either shut down the Domino Server (but keep the file server running) or choose the ican labeled 'Lotus Notes' instead.\"", "(Ryou:(pos=1) PHRASE 32 cannot:(pos=2) PHRASE 32 use:(pos=3) PHRASE 32 the:(pos=4) PHRASE 32 Radministration:(pos=5) PHRASE 32 program:(pos=6) PHRASE 32 while:(pos=7) PHRASE 32 the:(pos=8) PHRASE 32 Rdomino:(pos=9) PHRASE 32 Rserver:(pos=10) PHRASE 32 is:(pos=11) PHRASE 32 running:(pos=12) PHRASE 32 Reither:(pos=13) PHRASE 32 shut:(pos=14) PHRASE 32 down:(pos=15) PHRASE 32 the:(pos=16) PHRASE 32 Rdomino:(pos=17) PHRASE 32 Rserver:(pos=18) PHRASE 32 but:(pos=19) PHRASE 32 keep:(pos=20) PHRASE 32 the:(pos=21) PHRASE 32 file:(pos=22) PHRASE 32 server:(pos=23) PHRASE 32 run:(pos=24) PHRASE 32 or:(pos=25) PHRASE 32 choos:(pos=26) PHRASE 32 the:(pos=27) PHRASE 32 ican:(pos=28) PHRASE 32 label:(pos=29) PHRASE 32 Rlotus:(pos=30) PHRASE 32 Rnotes:(pos=31) PHRASE 32 instead:(pos=32))" },
    { "\"+irq +veranderen +xp\"", "(irq:(pos=1) PHRASE 3 veranderen:(pos=2) PHRASE 3 xp:(pos=3))" },
    { "\"is not a member of 'operator``global namespace''' + c++", "(is:(pos=1) PHRASE 9 not:(pos=2) PHRASE 9 a:(pos=3) PHRASE 9 member:(pos=4) PHRASE 9 of:(pos=5) PHRASE 9 oper:(pos=6) PHRASE 9 global:(pos=7) PHRASE 9 namespac:(pos=8) PHRASE 9 c++:(pos=9))" },
    { "mkdir() failed (File exists) php", "(mkdir:(pos=1) OR fail:(pos=2) OR Rfile:(pos=3) OR exist:(pos=4) OR php:(pos=5))" },
    { "laatsteIndex(int n)", "(laatsteindex:(pos=1) OR int:(pos=2) OR n:(pos=3))" },
    { "\"line+in\" OR \"c8783\"", "((line:(pos=1) PHRASE 2 in:(pos=2)) OR c8783:(pos=3))" },
    { "if ($_POST['Submit'])", "(if:(pos=1) OR Rpost:(pos=2) OR Rsubmit:(pos=3))" },
    { "NEC DVD+-RW ND-1300A", "(Rnec:(pos=1) OR Rdvd:(pos=2) OR Rrw:(pos=3) OR (Rnd:(pos=4) PHRASE 2 1300a:(pos=5)))" },
    { "*String not found* (*String not found*.)", "(Rstring:(pos=1) OR not:(pos=2) OR found:(pos=3) OR Rstring:(pos=4) OR not:(pos=5) OR found:(pos=6))" },
    { "MSI G4Ti4200-TD 128MB (GeForce4 Ti4200)", "(Rmsi:(pos=1) OR (Rg4ti4200:(pos=2) PHRASE 2 Rtd:(pos=3)) OR 128mb:(pos=4) OR Rgeforce4:(pos=5) OR Rti4200:(pos=6))" },
    { "href=\"#\"", "href:(pos=1)" },
    { "Request.ServerVariables(\"REMOTE_USER\") javascript", "((Rrequest:(pos=1) PHRASE 2 Rservervariables:(pos=2)) OR (Rremote:(pos=3) PHRASE 2 Ruser:(pos=4)) OR javascript:(pos=5))" },
    { "XF86Config(-4) waar", "(Rxf86config:(pos=1) OR 4:(pos=2) OR waar:(pos=3))" },
    { "Unknown (tag 2000)", "(Runknown:(pos=1) OR tag:(pos=2) OR 2000:(pos=3))" },
    { "KT4V(MS-6712)", "(Rkt4v:(pos=1) OR (Rms:(pos=2) PHRASE 2 6712:(pos=3)))" },
    { "scheduled+AND+nieuwsgroepen+AND+updaten", "(schedul:(pos=1) AND nieuwsgroepen:(pos=2) AND updaten:(pos=3))" },
    { "137(netbios-ns)", "(137:(pos=1) OR (netbio:(pos=2) PHRASE 2 ns:(pos=3)))" },
    { "HARWARE ERROR, TRACKING SERVO (4:0X09:0X01)", "(Rharware:(pos=1) OR Rerror:(pos=2) OR Rtracking:(pos=3) OR Rservo:(pos=4) OR (4:(pos=5) PHRASE 3 0x09:(pos=6) PHRASE 3 0x01:(pos=7)))" },
    { "Chr(10) wat is code van \" teken", "(Rchr:(pos=1) OR 10:(pos=2) OR wat:(pos=3) OR is:(pos=4) OR code:(pos=5) OR van:(pos=6) OR teken:(pos=7))" },
    { "wat is code van \" teken", "(wat:(pos=1) OR is:(pos=2) OR code:(pos=3) OR van:(pos=4) OR teken:(pos=5))" },
    { "The Jet VBA file (VBAJET.dll for 16-bit version, VBAJET32.dll version", "(Rthe:(pos=1) OR Rjet:(pos=2) OR Rvba:(pos=3) OR file:(pos=4) OR (Rvbajet:(pos=5) PHRASE 2 dll:(pos=6)) OR for:(pos=7) OR (16:(pos=8) PHRASE 2 bit:(pos=9)) OR version:(pos=10) OR (Rvbajet32:(pos=11) PHRASE 2 dll:(pos=12)) OR version:(pos=13))" },
    { "Permission denied (publickey,password,keyboard-interactive).", "(Rpermission:(pos=1) OR deni:(pos=2) OR publickey:(pos=3) OR password:(pos=4) OR (keyboard:(pos=5) PHRASE 2 interact:(pos=6)))" },
    { "De lees- of schrijfbewerking (\"written\") op het geheugen is mislukt", "(Rde:(pos=1) OR lees-:(pos=2) OR of:(pos=3) OR schrijfbewerk:(pos=4) OR written:(pos=5) OR op:(pos=6) OR het:(pos=7) OR geheugen:(pos=8) OR is:(pos=9) OR mislukt:(pos=10))" },
    { "Primary IDE channel no 80 conductor cable installed\"", "(Rprimary:(pos=1) OR Ride:(pos=2) OR channel:(pos=3) OR no:(pos=4) OR 80:(pos=5) OR conductor:(pos=6) OR cabl:(pos=7) OR instal:(pos=8))" },
    { "\"2020 NEAR zoom\"", "(2020:(pos=1) PHRASE 3 Rnear:(pos=2) PHRASE 3 zoom:(pos=3))" },
    { "setcookie(\"naam\",\"$user\");", "(setcooki:(pos=1) OR naam:(pos=2) OR user:(pos=3))" },
    { "MSI 645 Ultra (MS-6547) Ver1", "(Rmsi:(pos=1) OR 645:(pos=2) OR Rultra:(pos=3) OR (Rms:(pos=4) PHRASE 2 6547:(pos=5)) OR Rver1:(pos=6))" },
    { "if ($HTTP", "(if:(pos=1) OR Rhttp:(pos=2))" },
    { "data error(cyclic redundancy check)", "(data:(pos=1) OR error:(pos=2) OR cyclic:(pos=3) OR redund:(pos=4) OR check:(pos=5))" },
    { "UObject::StaticAllocateObject <- (NULL None) <- UObject::StaticConstructObject <- InitEngine", "((Ruobject:(pos=1) PHRASE 2 Rstaticallocateobject:(pos=2)) OR Rnull:(pos=3) OR Rnone:(pos=4) OR (Ruobject:(pos=5) PHRASE 2 Rstaticconstructobject:(pos=6)) OR Rinitengine:(pos=7))" },
    { "Failure at step 8 (Creating 3D Device)", "(Rfailure:(pos=1) OR at:(pos=2) OR step:(pos=3) OR 8:(pos=4) OR Rcreating:(pos=5) OR 3d:(pos=6) OR Rdevice:(pos=7))" },
    { "Call Shell(\"notepad.exe\",", "(Rcall:(pos=1) OR Rshell:(pos=2) OR (notepad:(pos=3) PHRASE 2 exe:(pos=4)))" },
    { "2.5\" harddisk converter", "((2:(pos=1) PHRASE 2 5:(pos=2)) OR (harddisk:(pos=3) PHRASE 2 convert:(pos=4)))" }, // FIXME better if " didn't generate a phrase here...
    { "creative labs \"dvd+rw\"", "(creativ:(pos=1) OR lab:(pos=2) OR (dvd:(pos=3) PHRASE 2 rw:(pos=4)))" },
    { "\"het beleid van deze computer staat u niet toe interactief", "(het:(pos=1) PHRASE 10 beleid:(pos=2) PHRASE 10 van:(pos=3) PHRASE 10 deze:(pos=4) PHRASE 10 comput:(pos=5) PHRASE 10 staat:(pos=6) PHRASE 10 u:(pos=7) PHRASE 10 niet:(pos=8) PHRASE 10 toe:(pos=9) PHRASE 10 interactief:(pos=10))" },
    { "ati radeon \"driver cleaner", "(ati:(pos=1) OR radeon:(pos=2) OR (driver:(pos=3) PHRASE 2 cleaner:(pos=4)))" },
    { "\"../\" path", "path:(pos=1)" },
    { "(novell client) workstation only", "(novel:(pos=1) OR client:(pos=2) OR workstat:(pos=3) OR onli:(pos=4))" },
    { "Unable to find libgd.(a|so) anywhere", "(Runable:(pos=1) OR to:(pos=2) OR find:(pos=3) OR libgd:(pos=4) OR a:(pos=5) OR so:(pos=6) OR anywher:(pos=7))" },
    { "\"libstdc++-libc6.1-1.so.2\"", "(libstdc:(pos=1) PHRASE 6 libc6:(pos=2) PHRASE 6 1:(pos=3) PHRASE 6 1:(pos=4) PHRASE 6 so:(pos=5) PHRASE 6 2:(pos=6))" },
    { "ipsec_setup (/etc/ipsec.conf, line 1) cannot open configuration file \"/etc/ipsec.conf\" -- `' aborted", "((ipsec:(pos=1) PHRASE 2 setup:(pos=2)) OR (etc:(pos=3) PHRASE 3 ipsec:(pos=4) PHRASE 3 conf:(pos=5)) OR line:(pos=6) OR 1:(pos=7) OR cannot:(pos=8) OR open:(pos=9) OR configur:(pos=10) OR file:(pos=11) OR (etc:(pos=12) PHRASE 3 ipsec:(pos=13) PHRASE 3 conf:(pos=14)) OR abort:(pos=15))" },
    { "Forwarden van domeinnaam (naar HTTP adres)", "(Rforwarden:(pos=1) OR van:(pos=2) OR domeinnaam:(pos=3) OR naar:(pos=4) OR Rhttp:(pos=5) OR adr:(pos=6))" },
    { "Compaq HP, 146.8 GB (MPN-286716-B22) Hard Drives", "(Rcompaq:(pos=1) OR Rhp:(pos=2) OR (146:(pos=3) PHRASE 2 8:(pos=4)) OR Rgb:(pos=5) OR (Rmpn:(pos=6) PHRASE 3 286716:(pos=7) PHRASE 3 Rb22:(pos=8)) OR Rhard:(pos=9) OR Rdrives:(pos=10))" },
    { "httpd (no pid file) not running", "(httpd:(pos=1) OR no:(pos=2) OR pid:(pos=3) OR file:(pos=4) OR not:(pos=5) OR run:(pos=6))" },
    { "apache httpd (pid file) not running", "(apach:(pos=1) OR httpd:(pos=2) OR pid:(pos=3) OR file:(pos=4) OR not:(pos=5) OR run:(pos=6))" },
    { "Klasse is niet geregistreerd  (Fout=80040154).", "(Rklasse:(pos=1) OR is:(pos=2) OR niet:(pos=3) OR geregistreerd:(pos=4) OR Rfout:(pos=5) OR 80040154:(pos=6))" },
    { "\"dvd+r\" \"dvd-r\"", "((dvd:(pos=1) PHRASE 2 r:(pos=2)) OR (dvd:(pos=3) PHRASE 2 r:(pos=4)))" },
    { "\"=\" tekens uit csvfile", "(teken:(pos=1) OR uit:(pos=2) OR csvfile:(pos=3))" },
    { "libc.so.6(GLIBC_2.3)", "((libc:(pos=1) PHRASE 3 so:(pos=2) PHRASE 3 6:(pos=3)) OR (Rglibc:(pos=4) PHRASE 3 2:(pos=5) PHRASE 3 3:(pos=6)))" },
    { "Sitecom Broadband xDSL / Cable Router 4S (DC-202)", "(Rsitecom:(pos=1) OR Rbroadband:(pos=2) OR xdsl:(pos=3) OR Rcable:(pos=4) OR Rrouter:(pos=5) OR 4s:(pos=6) OR (Rdc:(pos=7) PHRASE 2 202:(pos=8)))" },
    { "(t-mobile) bereik", "((t:(pos=1) PHRASE 2 mobil:(pos=2)) OR bereik:(pos=3))" },
    { "error LNK2001: unresolved external symbol \"public", "(error:(pos=1) OR Rlnk2001:(pos=2) OR unresolv:(pos=3) OR extern:(pos=4) OR symbol:(pos=5) OR public:(pos=6))" },
    { "patch linux exploit -p)", "(patch:(pos=1) OR linux:(pos=2) OR exploit:(pos=3) OR p:(pos=4))" },
    { "MYD not found (Errcode: 2)", "(Rmyd:(pos=1) OR not:(pos=2) OR found:(pos=3) OR Rerrcode:(pos=4) OR 2:(pos=5))" },
    { "ob_start(\"ob_gzhandler\"); file download", "((ob:(pos=1) PHRASE 2 start:(pos=2)) OR (ob:(pos=3) PHRASE 2 gzhandler:(pos=4)) OR file:(pos=5) OR download:(pos=6))" },
    { "ECS Elitegroup K7VZA (VIA VT8363/VT8363A)", "(Recs:(pos=1) OR Relitegroup:(pos=2) OR Rk7vza:(pos=3) OR Rvia:(pos=4) OR (Rvt8363:(pos=5) PHRASE 2 Rvt8363a:(pos=6)))" },
    { "ASUS A7V8X (LAN + Serial-ATA + Firewire + Raid + Audio)", "(Rasus:(pos=1) OR Ra7v8x:(pos=2) OR Rlan:(pos=3) OR (Rserial:(pos=4) PHRASE 2 Rata:(pos=5)) OR Rfirewire:(pos=6) OR Rraid:(pos=7) OR Raudio:(pos=8))" },
    { "Javascript:history.go(-1)", "((Rjavascript:(pos=1) PHRASE 3 histori:(pos=2) PHRASE 3 go:(pos=3)) OR 1:(pos=4))" },
    { "java :) als icon", "(java:(pos=1) OR al:(pos=2) OR icon:(pos=3))" },
    { "onmouseover=setPointer(this", "(onmouseov:(pos=1) OR setpoint:(pos=2) OR this:(pos=3))" },
    { "\" in vbscript", "(in:(pos=1) PHRASE 2 vbscript:(pos=2))" },
    { "IRC (FAQ OR (hulp NEAR bij))", "(Rirc:(pos=1) OR Rfaq:(pos=2) OR (hulp:(pos=3) NEAR 11 bij:(pos=4)))" },
    { "setProperty(\"McSquare\"+i, _xscale, _xscale++);", "(setproperti:(pos=1) OR Rmcsquare:(pos=2) OR i:(pos=3) OR xscale:(pos=4) OR xscale++:(pos=5))" },
    { "[warn] Apache does not support line-end comments. Consider using quotes around argument: \"#-1\"", "(warn:(pos=1) OR Rapache:(pos=2) OR doe:(pos=3) OR not:(pos=4) OR support:(pos=5) OR (line:(pos=6) PHRASE 2 end:(pos=7)) OR comments:(pos=8) OR Rconsider:(pos=9) OR use:(pos=10) OR quot:(pos=11) OR around:(pos=12) OR argument:(pos=13) OR 1:(pos=14))" },
    { "(php.ini) (memory_limit)", "((php:(pos=1) PHRASE 2 ini:(pos=2)) OR (memori:(pos=3) PHRASE 2 limit:(pos=4)))" },
    { "line 8: syntax error near unexpected token `kernel_thread(f'", "(line:(pos=1) OR 8:(pos=2) OR syntax:(pos=3) OR error:(pos=4) OR near:(pos=5) OR unexpect:(pos=6) OR token:(pos=7) OR (kernel:(pos=8) PHRASE 2 thread:(pos=9)) OR f:(pos=10))" },
    { "VXD NAVEX()@)", "(Rvxd:(pos=1) OR Rnavex:(pos=2))" },
    { "\"Iiyama AS4314UT 17\" \"", "(Riiyama:(pos=1) PHRASE 3 Ras4314ut:(pos=2) PHRASE 3 17:(pos=3))" },
    { "include (\"$id.html\");", "(includ:(pos=1) OR (id:(pos=2) PHRASE 2 html:(pos=3)))" },
    { "include id.Today's date is: <? print (date (\"M d, Y\")); ?>hp", "(includ:(pos=1) OR (id:(pos=2) PHRASE 3 Rtoday:(pos=3) PHRASE 3 s:(pos=4)) OR date:(pos=5) OR is:(pos=6) OR print:(pos=7) OR date:(pos=8) OR (Rm:(pos=9) PHRASE 3 d:(pos=10) PHRASE 3 Ry:(pos=11)) OR hp:(pos=12))" },
    { "(program files\\common) opstarten", "(program:(pos=1) OR (file:(pos=2) PHRASE 2 common:(pos=3)) OR opstarten:(pos=4))" },
    { "java \" string", "(java:(pos=1) OR string:(pos=2))" },
    { "+=", "" },
    { "php +=", "php:(pos=1)" },
    { "[php] ereg_replace(\".\"", "(php:(pos=1) OR (ereg:(pos=2) PHRASE 2 replac:(pos=3)))" },
    { "\"echo -e\" kleur", "((echo:(pos=1) PHRASE 2 e:(pos=2)) OR kleur:(pos=3))" },
    { "adobe premiere \"-1\"", "(adob:(pos=1) OR premier:(pos=2) OR 1:(pos=3))" },
    { "DVD brander \"+\" en \"-\"", "(Rdvd:(pos=1) OR brander:(pos=2) OR en:(pos=3))" },
    { "inspirion \"dvd+R\"", "(inspirion:(pos=1) OR (dvd:(pos=2) PHRASE 2 Rr:(pos=3)))" },
    { "asp 0x80040E14)", "(asp:(pos=1) OR 0x80040e14:(pos=2))" },
    { "\"e-tech motorola router", "(e:(pos=1) PHRASE 4 tech:(pos=2) PHRASE 4 motorola:(pos=3) PHRASE 4 router:(pos=4))" },
    { "bluetooth '1.3.2.19\"", "(bluetooth:(pos=1) OR (1:(pos=2) PHRASE 4 3:(pos=3) PHRASE 4 2:(pos=4) PHRASE 4 19:(pos=5)))" },
    { "ms +-connect", "(ms:(pos=1) OR connect:(pos=2))" },
    { "php+print+\"", "(php:(pos=1) OR print+:(pos=2))" },
    { "athlon 1400 :welke videokaart\"", "(athlon:(pos=1) OR 1400:(pos=2) OR welk:(pos=3) OR videokaart:(pos=4))" },
    { "+-dvd", "dvd:(pos=1)" },
    { "glftpd \"-new-\"", "(glftpd:(pos=1) OR new-:(pos=2))" },
    { "\"scandisk + dos5.0", "(scandisk:(pos=1) PHRASE 3 dos5:(pos=2) PHRASE 3 0:(pos=3))" },
    { "socket\\(\\)", "socket:(pos=1)" },
    { "msn (e-tech) router", "(msn:(pos=1) OR (e:(pos=2) PHRASE 2 tech:(pos=3)) OR router:(pos=4))" },
    { "Het grote Epox 8k3a+ ervaring/prob topic\"", "(Rhet:(pos=1) OR grote:(pos=2) OR Repox:(pos=3) OR 8k3a+:(pos=4) OR (ervar:(pos=5) PHRASE 2 prob:(pos=6)) OR topic:(pos=7))" },
    { "\"CF+bluetooth\"", "(Rcf:(pos=1) PHRASE 2 bluetooth:(pos=2))" },
    { "kwaliteit (s-video) composite verschil tv out", "(kwaliteit:(pos=1) OR (s:(pos=2) PHRASE 2 video:(pos=3)) OR composit:(pos=4) OR verschil:(pos=5) OR tv:(pos=6) OR out:(pos=7))" },
    { "Wie kan deze oude hardware nog gebruiken\" Deel", "(Rwie:(pos=1) OR kan:(pos=2) OR deze:(pos=3) OR oud:(pos=4) OR hardwar:(pos=5) OR nog:(pos=6) OR gebruiken:(pos=7) OR Rdeel:(pos=8))" },
    { "Public Declare Sub Sleep Lib \"kernel32\" (ByVal dwMilliseconds As Long)", "(Rpublic:(pos=1) OR Rdeclare:(pos=2) OR Rsub:(pos=3) OR Rsleep:(pos=4) OR Rlib:(pos=5) OR kernel32:(pos=6) OR Rbyval:(pos=7) OR dwmillisecond:(pos=8) OR Ras:(pos=9) OR Rlong:(pos=10))" },
    { "for inclusion (include_path='.:/usr/share/php')", "(for:(pos=1) OR inclus:(pos=2) OR (includ:(pos=3) PHRASE 2 path:(pos=4)) OR (usr:(pos=5) PHRASE 3 share:(pos=6) PHRASE 3 php:(pos=7)))" },
    { "\"muziek 2x zo snel\"\"", "(muziek:(pos=1) PHRASE 4 2x:(pos=2) PHRASE 4 zo:(pos=3) PHRASE 4 snel:(pos=4))" },
    { "execCommand('inserthorizontalrule'", "(execcommand:(pos=1) OR inserthorizontalrul:(pos=2))" },
    { "specs: IBM PS/2, Intel 8086 @ 25 mhz!!, 2 mb intern, 50 mb hd, 5.5\" floppy drive, toetsenbord en geen muis", "(spec:(pos=1) OR Ribm:(pos=2) OR (Rps:(pos=3) PHRASE 2 2:(pos=4)) OR Rintel:(pos=5) OR 8086:(pos=6) OR 25:(pos=7) OR mhz:(pos=8) OR 2:(pos=9) OR mb:(pos=10) OR intern:(pos=11) OR 50:(pos=12) OR mb:(pos=13) OR hd:(pos=14) OR (5:(pos=15) PHRASE 2 5:(pos=16)) OR (floppi:(pos=17) PHRASE 6 drive:(pos=18) PHRASE 6 toetsenbord:(pos=19) PHRASE 6 en:(pos=20) PHRASE 6 geen:(pos=21) PHRASE 6 mui:(pos=22)))" },
    { "History: GetEventTool <- GetMusicManager <- GetMusicScript <- DMCallRoutine <- AMusicScriptEvent::execCallRoutine <- UObject::execClassContext <- (U2GameInfo M08A1.U2GameInfo0 @ Function U2.U2GameInfo.NotifyLevelChangeEnd : 0075 line 744) <- UObject::ProcessEvent <- (U2GameInfo M08A1.U2GameInfo0, Function U2.U2GameInfo.NotifyLevelChangeEnd) <- UGameEngine::LoadMap <- LocalMapURL <- UGameEngine::Browse <- ServerTravel <- UGameEngine::Tick <- UpdateWorld <- MainLoop", "(Rhistory:(pos=1) OR Rgeteventtool:(pos=2) OR Rgetmusicmanager:(pos=3) OR Rgetmusicscript:(pos=4) OR Rdmcallroutine:(pos=5) OR (Ramusicscriptevent:(pos=6) PHRASE 2 execcallroutin:(pos=7)) OR (Ruobject:(pos=8) PHRASE 2 execclasscontext:(pos=9)) OR Ru2gameinfo:(pos=10) OR (Rm08a1:(pos=11) PHRASE 2 Ru2gameinfo0:(pos=12)) OR Rfunction:(pos=13) OR (Ru2:(pos=14) PHRASE 3 Ru2gameinfo:(pos=15) PHRASE 3 Rnotifylevelchangeend:(pos=16)) OR 0075:(pos=17) OR line:(pos=18) OR 744:(pos=19) OR (Ruobject:(pos=20) PHRASE 2 Rprocessevent:(pos=21)) OR Ru2gameinfo:(pos=22) OR (Rm08a1:(pos=23) PHRASE 2 Ru2gameinfo0:(pos=24)) OR Rfunction:(pos=25) OR (Ru2:(pos=26) PHRASE 3 Ru2gameinfo:(pos=27) PHRASE 3 Rnotifylevelchangeend:(pos=28)) OR (Rugameengine:(pos=29) PHRASE 2 Rloadmap:(pos=30)) OR Rlocalmapurl:(pos=31) OR (Rugameengine:(pos=32) PHRASE 2 Rbrowse:(pos=33)) OR Rservertravel:(pos=34) OR (Rugameengine:(pos=35) PHRASE 2 Rtick:(pos=36)) OR Rupdateworld:(pos=37) OR Rmainloop:(pos=38))" },
    { "Support AMD XP 2400+ & 2600+ (K7T Turbo2 only)", "(Rsupport:(pos=1) OR Ramd:(pos=2) OR Rxp:(pos=3) OR 2400+:(pos=4) OR 2600+:(pos=5) OR Rk7t:(pos=6) OR Rturbo2:(pos=7) OR onli:(pos=8))" },
    { "'\"><br>bla</br>", "(br:(pos=1) PHRASE 3 bla:(pos=2) PHRASE 3 br:(pos=3))" },
    { "The instruction at \"0x30053409\" referenced memory at \"0x06460504\". The memory could not be \"read'. Click OK to terminate the application.", "(Rthe:(pos=1) OR instruct:(pos=2) OR at:(pos=3) OR 0x30053409:(pos=4) OR referenc:(pos=5) OR memori:(pos=6) OR at:(pos=7) OR 0x06460504:(pos=8) OR Rthe:(pos=9) OR memori:(pos=10) OR could:(pos=11) OR not:(pos=12) OR be:(pos=13) OR (read:(pos=14) PHRASE 7 Rclick:(pos=15) PHRASE 7 Rok:(pos=16) PHRASE 7 to:(pos=17) PHRASE 7 termin:(pos=18) PHRASE 7 the:(pos=19) PHRASE 7 application:(pos=20)))" },
    { "\"(P5A-b)\"", "(Rp5a:(pos=1) PHRASE 2 b:(pos=2))" },
    { "(13,5 > 13) == no-go!", "(13:(pos=1) OR 5:(pos=2) OR 13:(pos=3) OR (no:(pos=4) PHRASE 2 go:(pos=5)))" },
    { "eth not found \"ifconfig -a\"", "(eth:(pos=1) OR not:(pos=2) OR found:(pos=3) OR (ifconfig:(pos=4) PHRASE 2 a:(pos=5)))" },
    { "<META NAME=\"ROBOTS", "(Rmeta:(pos=1) OR Rname:(pos=2) OR Rrobots:(pos=3))" },
    { "lp0: using parport0 (interrupt-driven)", "(lp0:(pos=1) OR use:(pos=2) OR parport0:(pos=3) OR (interrupt:(pos=4) PHRASE 2 driven:(pos=5)))" },
    { "ULTRA PC-TUNING, COOLING & MODDING (4,6)", "(Rultra:(pos=1) OR (Rpc:(pos=2) PHRASE 2 Rtuning:(pos=3)) OR Rcooling:(pos=4) OR Rmodding:(pos=5) OR 4:(pos=6) OR 6:(pos=7))" },
    { "512MB PC2700 DDR SDRAM Rood (Dane-Elec)", "(512mb:(pos=1) OR Rpc2700:(pos=2) OR Rddr:(pos=3) OR Rsdram:(pos=4) OR Rrood:(pos=5) OR (Rdane:(pos=6) PHRASE 2 Relec:(pos=7)))" },
    { "header(\"Content Type: text/html\");", "(header:(pos=1) OR Rcontent:(pos=2) OR Rtype:(pos=3) OR (text:(pos=4) PHRASE 2 html:(pos=5)))" },
    { "\"-RW\" \"+RW\"", "(Rrw:(pos=1) OR Rrw:(pos=2))" },
    { "\"cresta digital answering machine", "(cresta:(pos=1) PHRASE 4 digit:(pos=2) PHRASE 4 answer:(pos=3) PHRASE 4 machin:(pos=4))" },
    { "Arctic Super Silent PRO TC (Athlon/P3 - 2,3 GHz)", "(Rarctic:(pos=1) OR Rsuper:(pos=2) OR Rsilent:(pos=3) OR Rpro:(pos=4) OR Rtc:(pos=5) OR (Rathlon:(pos=6) PHRASE 2 Rp3:(pos=7)) OR 2:(pos=8) OR 3:(pos=9) OR Rghz:(pos=10))" },
    { "c++ fopen \"r+t\"", "(c++:(pos=1) OR fopen:(pos=2) OR (r:(pos=3) PHRASE 2 t:(pos=4)))" },
    { "c++ fopen (r+t)", "(c++:(pos=1) OR fopen:(pos=2) OR r:(pos=3) OR t:(pos=4))" },
    { "\"DVD+R\"", "(Rdvd:(pos=1) PHRASE 2 Rr:(pos=2))" },
    { "Class.forName(\"jdbc.odbc.JdbcOdbcDriver\");", "((Rclass:(pos=1) PHRASE 2 fornam:(pos=2)) OR (jdbc:(pos=3) PHRASE 3 odbc:(pos=4) PHRASE 3 Rjdbcodbcdriver:(pos=5)))" },
    { "perl(find.pl)", "(perl:(pos=1) OR (find:(pos=2) PHRASE 2 pl:(pos=3)))" },
    { "\"-5v\" voeding", "(5v:(pos=1) OR voed:(pos=2))" },
    { "\"-5v\" power supply", "(5v:(pos=1) OR power:(pos=2) OR suppli:(pos=3))" },
    { "An Error occurred whie attempting to initialize the Borland Database Engine (error $2108)", "(Ran:(pos=1) OR Rerror:(pos=2) OR occur:(pos=3) OR whie:(pos=4) OR attempt:(pos=5) OR to:(pos=6) OR initi:(pos=7) OR the:(pos=8) OR Rborland:(pos=9) OR Rdatabase:(pos=10) OR Rengine:(pos=11) OR error:(pos=12) OR 2108:(pos=13))" },
    { "(error $2108) Borland", "(error:(pos=1) OR 2108:(pos=2) OR Rborland:(pos=3))" },
    { "On Friday 04 April 2003 09:32, Edwin van Eersel wrote: > ik voel me eigenlijk wel behoorlijk kut :)", "(Ron:(pos=1) OR Rfriday:(pos=2) OR 04:(pos=3) OR Rapril:(pos=4) OR 2003:(pos=5) OR (09:(pos=6) PHRASE 2 32:(pos=7)) OR Redwin:(pos=8) OR van:(pos=9) OR Reersel:(pos=10) OR wrote:(pos=11) OR ik:(pos=12) OR voel:(pos=13) OR me:(pos=14) OR eigenlijk:(pos=15) OR wel:(pos=16) OR behoorlijk:(pos=17) OR kut:(pos=18))" },
    { "Elektrotechniek + \"hoe bevalt het?\"\"", "(Relektrotechniek:(pos=1) OR (hoe:(pos=2) PHRASE 3 bevalt:(pos=3) PHRASE 3 het:(pos=4)))" },
    { "Shortcuts in menu (java", "(Rshortcuts:(pos=1) OR in:(pos=2) OR menu:(pos=3) OR java:(pos=4))" },
    { "detonator+settings\"", "(deton:(pos=1) OR set:(pos=2))" },
    { "(ez-bios) convert", "((ez:(pos=1) PHRASE 2 bio:(pos=2)) OR convert:(pos=3))" },
    { "Sparkle 7100M4 64MB (GeForce4 MX440)", "(Rsparkle:(pos=1) OR 7100m4:(pos=2) OR 64mb:(pos=3) OR Rgeforce4:(pos=4) OR Rmx440:(pos=5))" },
    { "freebsd \"boek OR newbie\"", "(freebsd:(pos=1) OR (boek:(pos=2) PHRASE 3 Ror:(pos=3) PHRASE 3 newbi:(pos=4)))" },
    { "for (;;) c++", "(for:(pos=1) OR c++:(pos=2))" },
    { "1700+-2100+", "(1700:(pos=1) OR 2100+:(pos=2))" },
    { "PHP Warning:  Invalid library (maybe not a PHP library) 'libmysqlclient.so'", "(Rphp:(pos=1) OR Rwarning:(pos=2) OR Rinvalid:(pos=3) OR librari:(pos=4) OR mayb:(pos=5) OR not:(pos=6) OR a:(pos=7) OR Rphp:(pos=8) OR librari:(pos=9) OR (libmysqlcli:(pos=10) PHRASE 2 so:(pos=11)))" },
    { "NEC DV-5800B (Bul", "(Rnec:(pos=1) OR (Rdv:(pos=2) PHRASE 2 5800b:(pos=3)) OR Rbul:(pos=4))" },
    { "org.jdom.input.SAXBuilder.<init>(SAXBuilder.java)", "((org:(pos=1) PHRASE 4 jdom:(pos=2) PHRASE 4 input:(pos=3) PHRASE 4 Rsaxbuilder:(pos=4)) OR init:(pos=5) OR (Rsaxbuilder:(pos=6) PHRASE 2 java:(pos=7)))" },
    { "AMD Athlon XP 2500+ (1,83GHz, 512KB)", "(Ramd:(pos=1) OR Rathlon:(pos=2) OR Rxp:(pos=3) OR 2500+:(pos=4) OR 1:(pos=5) OR 83ghz:(pos=6) OR 512kb:(pos=7))" },
    { "'q ben\"", "(q:(pos=1) OR ben:(pos=2))" },
    { "getsmbfilepwent: malformed password entry (uid not number)", "(getsmbfilepw:(pos=1) OR malform:(pos=2) OR password:(pos=3) OR entri:(pos=4) OR uid:(pos=5) OR not:(pos=6) OR number:(pos=7))" },
    { "\xf6ude onderdelen\"", "(oeud:(pos=1) OR onderdelen:(pos=2))" },
    { "Heeft iemand enig idee waarom de pioneer (zelf met originele firmware van pioneer) bij mij niet wil flashen ?" "?", "(Rheeft:(pos=1) OR iemand:(pos=2) OR enig:(pos=3) OR ide:(pos=4) OR waarom:(pos=5) OR de:(pos=6) OR pioneer:(pos=7) OR zelf:(pos=8) OR met:(pos=9) OR originel:(pos=10) OR firmwar:(pos=11) OR van:(pos=12) OR pioneer:(pos=13) OR bij:(pos=14) OR mij:(pos=15) OR niet:(pos=16) OR wil:(pos=17) OR flashen:(pos=18))" }, // Split ? and ? to avoid trigram problems
    { "asus a7v266 bios nieuw -(a7v266-e)", "((asus:(pos=1) OR a7v266:(pos=2) OR bio:(pos=3) OR nieuw:(pos=4)) AND_NOT (a7v266:(pos=5) PHRASE 2 e:(pos=6)))" },
    { "cybercom \"dvd+r\"", "(cybercom:(pos=1) OR (dvd:(pos=2) PHRASE 2 r:(pos=3)))" },
    { "AMD PCNET Family Ethernet Adapter (PCI-ISA)", "(Ramd:(pos=1) OR Rpcnet:(pos=2) OR Rfamily:(pos=3) OR Rethernet:(pos=4) OR Radapter:(pos=5) OR (Rpci:(pos=6) PHRASE 2 Risa:(pos=7)))" },
    { "relais +/-", "relai:(pos=1)" },
    { "formules (slepen OR doortrekken) excel", "(formul:(pos=1) OR slepen:(pos=2) OR doortrekken:(pos=3) OR excel:(pos=4))" },
    { "\"%English", "Renglish:(pos=1)" },
    { "select max( mysql", "(select:(pos=1) OR max:(pos=2) OR mysql:(pos=3))" },
    { "leejow(saait", "(leejow:(pos=1) OR saait:(pos=2))" },
    { "'Windows 2000 Advanced Server\" netwerkverbinding valt steeds weg", "(Rwindows:(pos=1) OR 2000:(pos=2) OR Radvanced:(pos=3) OR Rserver:(pos=4) OR (netwerkverbind:(pos=5) PHRASE 4 valt:(pos=6) PHRASE 4 steed:(pos=7) PHRASE 4 weg:(pos=8)))" },
    { "K7T Turbo 2  (MS-6330)", "(Rk7t:(pos=1) OR Rturbo:(pos=2) OR 2:(pos=3) OR (Rms:(pos=4) PHRASE 2 6330:(pos=5)))" },
    { "failed to receive data from the client agent. (ec=1)", "(fail:(pos=1) OR to:(pos=2) OR receiv:(pos=3) OR data:(pos=4) OR from:(pos=5) OR the:(pos=6) OR client:(pos=7) OR agent:(pos=8) OR ec:(pos=9) OR 1:(pos=10))" },
    { "\"cannot find -lz\"", "(cannot:(pos=1) PHRASE 3 find:(pos=2) PHRASE 3 lz:(pos=3))" },
    { "undefined reference to `mysql_drop_db'\"", "(undefin:(pos=1) OR refer:(pos=2) OR to:(pos=3) OR (mysql:(pos=4) PHRASE 3 drop:(pos=5) PHRASE 3 db:(pos=6)))" },
    { "search form asp \"%'", "(search:(pos=1) OR form:(pos=2) OR asp:(pos=3))" },
    { "(dvd+r) kwaliteit", "(dvd:(pos=1) OR r:(pos=2) OR kwaliteit:(pos=3))" },
    { "Fatal error: Allowed memory size of 8388608 bytes exhausted (tried to allocate 35 bytes)", "(Rfatal:(pos=1) OR error:(pos=2) OR Rallowed:(pos=3) OR memori:(pos=4) OR size:(pos=5) OR of:(pos=6) OR 8388608:(pos=7) OR byte:(pos=8) OR exhaust:(pos=9) OR tri:(pos=10) OR to:(pos=11) OR alloc:(pos=12) OR 35:(pos=13) OR byte:(pos=14))" },
    { "geluid (schokt OR hapert)", "(geluid:(pos=1) OR schokt:(pos=2) OR hapert:(pos=3))" },
    { "Het wordt pas echt leuk als het hard staat!! >:)", "(Rhet:(pos=1) OR wordt:(pos=2) OR pas:(pos=3) OR echt:(pos=4) OR leuk:(pos=5) OR al:(pos=6) OR het:(pos=7) OR hard:(pos=8) OR staat:(pos=9))" },
    { "Uw configuratie bestand bevat instellingen (root zonder wachtwoord) die betrekking hebben tot de standaard MySQL account. Uw MySQL server draait met deze standaard waardes, en is open voor ongewilde toegang, het wordt dus aangeraden dit op te lossen", "(Ruw:(pos=1) OR configurati:(pos=2) OR bestand:(pos=3) OR bevat:(pos=4) OR instellingen:(pos=5) OR root:(pos=6) OR zonder:(pos=7) OR wachtwoord:(pos=8) OR die:(pos=9) OR betrekk:(pos=10) OR hebben:(pos=11) OR tot:(pos=12) OR de:(pos=13) OR standaard:(pos=14) OR Rmysql:(pos=15) OR account:(pos=16) OR Ruw:(pos=17) OR Rmysql:(pos=18) OR server:(pos=19) OR draait:(pos=20) OR met:(pos=21) OR deze:(pos=22) OR standaard:(pos=23) OR waard:(pos=24) OR en:(pos=25) OR is:(pos=26) OR open:(pos=27) OR voor:(pos=28) OR ongewild:(pos=29) OR toegang:(pos=30) OR het:(pos=31) OR wordt:(pos=32) OR dus:(pos=33) OR aangeraden:(pos=34) OR dit:(pos=35) OR op:(pos=36) OR te:(pos=37) OR lossen:(pos=38))" },
    { "(library qt-mt) not found", "(librari:(pos=1) OR (qt:(pos=2) PHRASE 2 mt:(pos=3)) OR not:(pos=4) OR found:(pos=5))" },
    { "Qt (>= Qt 3.0.3) (library qt-mt) not found", "(Rqt:(pos=1) OR Rqt:(pos=2) OR (3:(pos=3) PHRASE 3 0:(pos=4) PHRASE 3 3:(pos=5)) OR librari:(pos=6) OR (qt:(pos=7) PHRASE 2 mt:(pos=8)) OR not:(pos=9) OR found:(pos=10))" },
    { "setup was unable to find (or could not read) the language specific setup resource dll, unable to continue. Please reboot and try again.", "(setup:(pos=1) OR was:(pos=2) OR unabl:(pos=3) OR to:(pos=4) OR find:(pos=5) OR or:(pos=6) OR could:(pos=7) OR not:(pos=8) OR read:(pos=9) OR the:(pos=10) OR languag:(pos=11) OR specif:(pos=12) OR setup:(pos=13) OR resourc:(pos=14) OR dll:(pos=15) OR unabl:(pos=16) OR to:(pos=17) OR continue:(pos=18) OR Rplease:(pos=19) OR reboot:(pos=20) OR and:(pos=21) OR tri:(pos=22) OR again:(pos=23))" },
    { "Titan TTC-D5TB(4/CU35)", "(Rtitan:(pos=1) OR (Rttc:(pos=2) PHRASE 2 Rd5tb:(pos=3)) OR (4:(pos=4) PHRASE 2 Rcu35:(pos=5)))" },
    { "[php] date( min", "(php:(pos=1) OR date:(pos=2) OR min:(pos=3))" },
    { "EPOX EP-8RDA+ (nForce2 SPP+MCP-T) Rev. 1.1", "(Repox:(pos=1) OR (Rep:(pos=2) PHRASE 2 8rda+:(pos=3)) OR nforce2:(pos=4) OR Rspp:(pos=5) OR (Rmcp:(pos=6) PHRASE 2 Rt:(pos=7)) OR Rrev:(pos=8) OR (1:(pos=9) PHRASE 2 1:(pos=10)))" },
    { "554 5.4.6 Too many hops 53 (25 max)", "(554:(pos=1) OR (5:(pos=2) PHRASE 3 4:(pos=3) PHRASE 3 6:(pos=4)) OR Rtoo:(pos=5) OR mani:(pos=6) OR hop:(pos=7) OR 53:(pos=8) OR 25:(pos=9) OR max:(pos=10))" },
    { "ik had toch nog een vraagje: er zijn nu eigenlijk alleen maar schijfjes van 4.7GB alleen straks zullen er vast schijfjes van meer dan 4.7GB komen. Zal deze brander dit wel kunnen schijven?" "?(na bijvoorbeeld een firmware update?) ben erg benieuwd", "(ik:(pos=1) OR had:(pos=2) OR toch:(pos=3) OR nog:(pos=4) OR een:(pos=5) OR vraagj:(pos=6) OR er:(pos=7) OR zijn:(pos=8) OR nu:(pos=9) OR eigenlijk:(pos=10) OR alleen:(pos=11) OR maar:(pos=12) OR schijfj:(pos=13) OR van:(pos=14) OR (4:(pos=15) PHRASE 2 7gb:(pos=16)) OR alleen:(pos=17) OR strak:(pos=18) OR zullen:(pos=19) OR er:(pos=20) OR vast:(pos=21) OR schijfj:(pos=22) OR van:(pos=23) OR meer:(pos=24) OR dan:(pos=25) OR (4:(pos=26) PHRASE 2 7gb:(pos=27)) OR komen:(pos=28) OR Rzal:(pos=29) OR deze:(pos=30) OR brander:(pos=31) OR dit:(pos=32) OR wel:(pos=33) OR kunnen:(pos=34) OR schijven:(pos=35) OR na:(pos=36) OR bijvoorbeeld:(pos=37) OR een:(pos=38) OR firmwar:(pos=39) OR updat:(pos=40) OR ben:(pos=41) OR erg:(pos=42) OR benieuwd:(pos=43))" }, // Split ? and ? to avoid trigram problems
    { "ati linux drivers (4.3.0)", "(ati:(pos=1) OR linux:(pos=2) OR driver:(pos=3) OR (4:(pos=4) PHRASE 3 3:(pos=5) PHRASE 3 0:(pos=6)))" },
    { "ENCAPSED_AND_WHITESPACE", "(Rencapsed:(pos=1) PHRASE 3 Rand:(pos=2) PHRASE 3 Rwhitespace:(pos=3))" },
    { "lpadmin: add-printer (set device) failed: client-error-not-possible", "(lpadmin:(pos=1) OR (add:(pos=2) PHRASE 2 printer:(pos=3)) OR set:(pos=4) OR devic:(pos=5) OR fail:(pos=6) OR (client:(pos=7) PHRASE 4 error:(pos=8) PHRASE 4 not:(pos=9) PHRASE 4 possibl:(pos=10)))" },
    { "welke dvd \"+r\" media", "(welk:(pos=1) OR dvd:(pos=2) OR r:(pos=3) OR media:(pos=4))" },
    { "Warning: stat failed for fotos(errno=2 - No such file or directory)", "(Rwarning:(pos=1) OR stat:(pos=2) OR fail:(pos=3) OR for:(pos=4) OR foto:(pos=5) OR errno:(pos=6) OR 2:(pos=7) OR Rno:(pos=8) OR such:(pos=9) OR file:(pos=10) OR or:(pos=11) OR directori:(pos=12))" },
    { "dvd +/-", "dvd:(pos=1)" },
    { "7vaxp +voltage mod\"", "(voltag:(pos=2) AND_MAYBE (7vaxp:(pos=1) OR mod:(pos=3)))" },
    { "lpt port (SPP/EPP) is enabled", "(lpt:(pos=1) OR port:(pos=2) OR (Rspp:(pos=3) PHRASE 2 Repp:(pos=4)) OR is:(pos=5) OR enabl:(pos=6))" },
    { "getenv(\"HTTP_REFERER\")", "(getenv:(pos=1) OR (Rhttp:(pos=2) PHRASE 2 Rreferer:(pos=3)))" },
    { "Error setting display mode: CreateDevice failed (D3DERR_DRIVERINTERNALERROR)", "(Rerror:(pos=1) OR set:(pos=2) OR display:(pos=3) OR mode:(pos=4) OR Rcreatedevice:(pos=5) OR fail:(pos=6) OR (Rd3derr:(pos=7) PHRASE 2 Rdriverinternalerror:(pos=8)))" },
    { "Exception number: c0000005 (access violation)", "(Rexception:(pos=1) OR number:(pos=2) OR c0000005:(pos=3) OR access:(pos=4) OR violat:(pos=5))" },
    { "header(\"Content-type:application/octetstream\");", "(header:(pos=1) OR (Rcontent:(pos=2) PHRASE 4 type:(pos=3) PHRASE 4 applic:(pos=4) PHRASE 4 octetstream:(pos=5)))" },
    { "java.security.AccessControlException: access denied (java.lang.RuntimePermission accessClassInPackage.sun.jdbc.odbc)", "((java:(pos=1) PHRASE 3 secur:(pos=2) PHRASE 3 Raccesscontrolexception:(pos=3)) OR access:(pos=4) OR deni:(pos=5) OR (java:(pos=6) PHRASE 3 lang:(pos=7) PHRASE 3 Rruntimepermission:(pos=8)) OR (accessclassinpackag:(pos=9) PHRASE 4 sun:(pos=10) PHRASE 4 jdbc:(pos=11) PHRASE 4 odbc:(pos=12)))" },
    { "(001.part.met", "(001:(pos=1) PHRASE 3 part:(pos=2) PHRASE 3 met:(pos=3))" },
    { "Warning: mail(): Use the -f option (5th param) to include valid reply-to address ! in /usr/home/vdb/www/mail.php on line 79", "(Rwarning:(pos=1) OR mail:(pos=2) OR Ruse:(pos=3) OR the:(pos=4) OR f:(pos=5) OR option:(pos=6) OR 5th:(pos=7) OR param:(pos=8) OR to:(pos=9) OR includ:(pos=10) OR valid:(pos=11) OR (repli:(pos=12) PHRASE 2 to:(pos=13)) OR address:(pos=14) OR in:(pos=15) OR (usr:(pos=16) PHRASE 6 home:(pos=17) PHRASE 6 vdb:(pos=18) PHRASE 6 www:(pos=19) PHRASE 6 mail:(pos=20) PHRASE 6 php:(pos=21)) OR on:(pos=22) OR line:(pos=23) OR 79:(pos=24))" },
    { "PHP Use the -f option (5th param)", "((Rphp:(pos=1) OR Ruse:(pos=2) OR the:(pos=3) OR option:(pos=5) OR 5th:(pos=6) OR param:(pos=7)) AND_NOT f:(pos=4))" },
    { "dvd \"+\" \"-\"", "dvd:(pos=1)" },
    { "bericht  ( %)", "bericht:(pos=1)" },
    { "2500+ of 2600+ (niett OC)", "(2500+:(pos=1) OR of:(pos=2) OR 2600+:(pos=3) OR niett:(pos=4) OR Roc:(pos=5))" },
    { "maxtor windows xp werkt The drivers for this device are not installed. (Code 28)", "(maxtor:(pos=1) OR window:(pos=2) OR xp:(pos=3) OR werkt:(pos=4) OR Rthe:(pos=5) OR driver:(pos=6) OR for:(pos=7) OR this:(pos=8) OR devic:(pos=9) OR are:(pos=10) OR not:(pos=11) OR installed:(pos=12) OR Rcode:(pos=13) OR 28:(pos=14))" },
    { "Warning: stat failed for /mnt/web/react/got/react/board/non-www/headlines/tnet-headlines.txt (errno=2 - No such file or directory) in /mnt/web/react/got/react/global/non-www/templates/got/functions.inc.php on line 303", "(Rwarning:(pos=1) OR stat:(pos=2) OR fail:(pos=3) OR for:(pos=4) OR (mnt:(pos=5) PHRASE 12 web:(pos=6) PHRASE 12 react:(pos=7) PHRASE 12 got:(pos=8) PHRASE 12 react:(pos=9) PHRASE 12 board:(pos=10) PHRASE 12 non:(pos=11) PHRASE 12 www:(pos=12) PHRASE 12 headlin:(pos=13) PHRASE 12 tnet:(pos=14) PHRASE 12 headlin:(pos=15) PHRASE 12 txt:(pos=16)) OR errno:(pos=17) OR 2:(pos=18) OR Rno:(pos=19) OR such:(pos=20) OR file:(pos=21) OR or:(pos=22) OR directori:(pos=23) OR in:(pos=24) OR (mnt:(pos=25) PHRASE 13 web:(pos=26) PHRASE 13 react:(pos=27) PHRASE 13 got:(pos=28) PHRASE 13 react:(pos=29) PHRASE 13 global:(pos=30) PHRASE 13 non:(pos=31) PHRASE 13 www:(pos=32) PHRASE 13 templat:(pos=33) PHRASE 13 got:(pos=34) PHRASE 13 function:(pos=35) PHRASE 13 inc:(pos=36) PHRASE 13 php:(pos=37)) OR on:(pos=38) OR line:(pos=39) OR 303:(pos=40))" },
    { "apm: BIOS version 1.2 Flags 0x03 (Driver version 1.16)", "(apm:(pos=1) OR Rbios:(pos=2) OR version:(pos=3) OR (1:(pos=4) PHRASE 2 2:(pos=5)) OR Rflags:(pos=6) OR 0x03:(pos=7) OR Rdriver:(pos=8) OR version:(pos=9) OR (1:(pos=10) PHRASE 2 16:(pos=11)))" },
    { "GA-8IHXP(3.0)", "((Rga:(pos=1) PHRASE 2 8ihxp:(pos=2)) OR (3:(pos=3) PHRASE 2 0:(pos=4)))" },
    { "8IHXP(3.0)", "(8ihxp:(pos=1) OR (3:(pos=2) PHRASE 2 0:(pos=3)))" },
    { "na\xb7si (de ~ (m.))", "(na:(pos=1) OR si:(pos=2) OR de:(pos=3) OR m:(pos=4))" },
    { "header(\"Content-Disposition: attachment;", "(header:(pos=1) OR (Rcontent:(pos=2) PHRASE 3 Rdisposition:(pos=3) PHRASE 3 attach:(pos=4)))" },
    { "\"header(\"Content-Disposition: attachment;\"", "(header:(pos=1) OR (Rcontent:(pos=2) PHRASE 2 Rdisposition:(pos=3)) OR attach:(pos=4))" },
    { "\"Beep -f\"", "(Rbeep:(pos=1) PHRASE 2 f:(pos=2))" },
    { "kraan NEAR (Elektrisch OR Electrisch)", "(kraan:(pos=1) OR Rnear:(pos=2) OR Relektrisch:(pos=3) OR Ror:(pos=4) OR Relectrisch:(pos=5))" },
    { "checking for Qt... configure: error: Qt (>= Qt 3.0.2) (headers and libraries) not found. Please check your installation!", "(check:(pos=1) OR for:(pos=2) OR Rqt:(pos=3) OR configur:(pos=4) OR error:(pos=5) OR Rqt:(pos=6) OR Rqt:(pos=7) OR (3:(pos=8) PHRASE 3 0:(pos=9) PHRASE 3 2:(pos=10)) OR header:(pos=11) OR and:(pos=12) OR librari:(pos=13) OR not:(pos=14) OR found:(pos=15) OR Rplease:(pos=16) OR check:(pos=17) OR your:(pos=18) OR instal:(pos=19))" },
    { "parse error, unexpected '\\\"', expecting T_STRING or T_VARIABLE or T_NUM_STRING", "(pars:(pos=1) OR error:(pos=2) OR unexpect:(pos=3) OR (expect:(pos=4) PHRASE 10 Rt:(pos=5) PHRASE 10 Rstring:(pos=6) PHRASE 10 or:(pos=7) PHRASE 10 Rt:(pos=8) PHRASE 10 Rvariable:(pos=9) PHRASE 10 or:(pos=10) PHRASE 10 Rt:(pos=11) PHRASE 10 Rnum:(pos=12) PHRASE 10 Rstring:(pos=13)))" },
    { "ac3 (0x2000) \"Dolby Laboratories,", "(ac3:(pos=1) OR 0x2000:(pos=2) OR (Rdolby:(pos=3) PHRASE 2 Rlaboratories:(pos=4)))" },
    { "Movie.FileName=(\"../../../~animations/\"+lesson1.recordset.fields('column3')+\"Intro.avi\")", "((Rmovie:(pos=1) PHRASE 2 Rfilename:(pos=2)) OR anim:(pos=3) OR (lesson1:(pos=4) PHRASE 3 recordset:(pos=5) PHRASE 3 field:(pos=6)) OR column3:(pos=7) OR (Rintro:(pos=8) PHRASE 2 avi:(pos=9)))" },
    { "502 Permission Denied - Permission Denied - news.chello.nl -- http://www.chello.nl/ (Typhoon v1.2.3)", "(502:(pos=1) OR Rpermission:(pos=2) OR Rdenied:(pos=3) OR Rpermission:(pos=4) OR Rdenied:(pos=5) OR (news:(pos=6) PHRASE 3 chello:(pos=7) PHRASE 3 nl:(pos=8)) OR (http:(pos=9) PHRASE 4 www:(pos=10) PHRASE 4 chello:(pos=11) PHRASE 4 nl:(pos=12)) OR Rtyphoon:(pos=13) OR (v1:(pos=14) PHRASE 3 2:(pos=15) PHRASE 3 3:(pos=16)))" },
    { "Motion JPEG (MJPEG codec)", "(Rmotion:(pos=1) OR Rjpeg:(pos=2) OR Rmjpeg:(pos=3) OR codec:(pos=4))" },
    { ": zoomtext\"", "zoomtext:(pos=1)" },
    { "Your SORT command does not seem to support the \"-r -n -k 7\"", "(Ryour:(pos=1) OR Rsort:(pos=2) OR command:(pos=3) OR doe:(pos=4) OR not:(pos=5) OR seem:(pos=6) OR to:(pos=7) OR support:(pos=8) OR the:(pos=9) OR (r:(pos=10) PHRASE 4 n:(pos=11) PHRASE 4 k:(pos=12) PHRASE 4 7:(pos=13)))" },
    { "Geef de naam van de MSDOS prompt op C:\\\\WINDOWS.COM\\\"", "(Rgeef:(pos=1) OR de:(pos=2) OR naam:(pos=3) OR van:(pos=4) OR de:(pos=5) OR Rmsdos:(pos=6) OR prompt:(pos=7) OR op:(pos=8) OR (Rc:(pos=9) PHRASE 3 Rwindows:(pos=10) PHRASE 3 Rcom:(pos=11)))" },
    { "\"\"wa is fase\"", "(wa:(pos=1) OR is:(pos=2) OR fase:(pos=3))" },
    { "<v:imagedata src=\"", "((v:(pos=1) PHRASE 2 imagedata:(pos=2)) OR src:(pos=3))" },
    { "system(play ringin.wav); ?>", "(system:(pos=1) OR play:(pos=2) OR (ringin:(pos=3) PHRASE 2 wav:(pos=4)))" },
    { "\"perfect NEAR systems\"", "(perfect:(pos=1) PHRASE 3 Rnear:(pos=2) PHRASE 3 system:(pos=3))" },
    { "LoadLibrary(\"mainta/gamex86.dll\") failed", "(Rloadlibrary:(pos=1) OR (mainta:(pos=2) PHRASE 3 gamex86:(pos=3) PHRASE 3 dll:(pos=4)) OR fail:(pos=5))" },
    { "DATE_FORMAT('1997-10-04 22:23:00', '%W %M %Y');", "((Rdate:(pos=1) PHRASE 2 Rformat:(pos=2)) OR (1997:(pos=3) PHRASE 3 10:(pos=4) PHRASE 3 04:(pos=5)) OR (22:(pos=6) PHRASE 3 23:(pos=7) PHRASE 3 00:(pos=8)) OR Rw:(pos=9) OR Rm:(pos=10) OR Ry:(pos=11))" },
    { "secundaire IDE-controller (dubbele fifo)", "(secundair:(pos=1) OR (Ride:(pos=2) PHRASE 2 control:(pos=3)) OR dubbel:(pos=4) OR fifo:(pos=5))" },
    { "\"Postal2+Explorer.exe\"", "(Rpostal2:(pos=1) PHRASE 3 Rexplorer:(pos=2) PHRASE 3 exe:(pos=3))" },
    { "COUNT(*)", "Rcount:(pos=1)" },
    { "Nuttige Windows progs   (1/11)", "(Rnuttige:(pos=1) OR Rwindows:(pos=2) OR prog:(pos=3) OR (1:(pos=4) PHRASE 2 11:(pos=5)))" },
    { "if(usercode==passcode==)", "(if:(pos=1) OR usercod:(pos=2) OR passcod:(pos=3))" },
    { "lg 8160b (dvd+r)", "(lg:(pos=1) OR 8160b:(pos=2) OR dvd:(pos=3) OR r:(pos=4))" },
    { "iPAQ Pocket PC 2002 End User Update (EUU - Service Pack)", "(ipaq:(pos=1) OR Rpocket:(pos=2) OR Rpc:(pos=3) OR 2002:(pos=4) OR Rend:(pos=5) OR Ruser:(pos=6) OR Rupdate:(pos=7) OR Reuu:(pos=8) OR Rservice:(pos=9) OR Rpack:(pos=10))" },
    { "'ipod pakt tags niet\"", "(ipod:(pos=1) OR pakt:(pos=2) OR tag:(pos=3) OR niet:(pos=4))" },
    { "\"DVD+/-R\"", "(Rdvd+:(pos=1) PHRASE 2 Rr:(pos=2))" },
    { "\"DVD+R DVD-R\"", "(Rdvd:(pos=1) PHRASE 4 Rr:(pos=2) PHRASE 4 Rdvd:(pos=3) PHRASE 4 Rr:(pos=4))" },
    { "php ;)  in een array zetten", "(php:(pos=1) OR in:(pos=2) OR een:(pos=3) OR array:(pos=4) OR zetten:(pos=5))" },
    { "De inhoud van uw advertentie is niet geschikt voor plaatsing op marktplaats! (001", "(Rde:(pos=1) OR inhoud:(pos=2) OR van:(pos=3) OR uw:(pos=4) OR advertenti:(pos=5) OR is:(pos=6) OR niet:(pos=7) OR geschikt:(pos=8) OR voor:(pos=9) OR plaats:(pos=10) OR op:(pos=11) OR marktplaat:(pos=12) OR 001:(pos=13))" },
    { "creative (soundblaster OR sb) 128", "(creativ:(pos=1) OR soundblast:(pos=2) OR sb:(pos=3) OR 128:(pos=4))" },
    { "Can't open file: (errno: 145)", "((Rcan:(pos=1) PHRASE 2 t:(pos=2)) OR open:(pos=3) OR file:(pos=4) OR errno:(pos=5) OR 145:(pos=6))" },
    { "Formateren lukt niet(98,XP)", "(Rformateren:(pos=1) OR lukt:(pos=2) OR niet:(pos=3) OR 98:(pos=4) OR Rxp:(pos=5))" },
    { "access denied (java.io.", "(access:(pos=1) OR deni:(pos=2) OR (java:(pos=3) PHRASE 2 io:(pos=4)))" },
    { "(access denied (java.io.)", "(access:(pos=1) OR deni:(pos=2) OR (java:(pos=3) PHRASE 2 io:(pos=4)))" },
    { "wil niet installeren ( crc fouten)", "(wil:(pos=1) OR niet:(pos=2) OR installeren:(pos=3) OR crc:(pos=4) OR fouten:(pos=5))" },
    { "(DVD+RW) brandsoftware meerdere", "(Rdvd:(pos=1) OR Rrw:(pos=2) OR brandsoftwar:(pos=3) OR meerder:(pos=4))" },
    { "(database OF databases) EN geheugen", "(databas:(pos=1) OR Rof:(pos=2) OR databas:(pos=3) OR Ren:(pos=4) OR geheugen:(pos=5))" },
    { "(server 2003) winroute", "(server:(pos=1) OR 2003:(pos=2) OR winrout:(pos=3))" },
    { "54MHz (kanaal 2 VHF) tot tenminste 806 MHz (kanaal 69 UHF)", "(54mhz:(pos=1) OR kanaal:(pos=2) OR 2:(pos=3) OR Rvhf:(pos=4) OR tot:(pos=5) OR tenminst:(pos=6) OR 806:(pos=7) OR Rmhz:(pos=8) OR kanaal:(pos=9) OR 69:(pos=10) OR Ruhf:(pos=11))" },
    { "(draadloos OR wireless) netwerk", "(draadloo:(pos=1) OR wireless:(pos=2) OR netwerk:(pos=3))" },
    { "localtime(time(NULL));", "(localtim:(pos=1) OR time:(pos=2) OR Rnull:(pos=3))" },
    { "ob_start(\"ob_gzhandler\");", "((ob:(pos=1) PHRASE 2 start:(pos=2)) OR (ob:(pos=3) PHRASE 2 gzhandler:(pos=4)))" },
    { "PPP Closed : LCP Time-out (VPN-0)", "(Rppp:(pos=1) OR Rclosed:(pos=2) OR Rlcp:(pos=3) OR (Rtime:(pos=4) PHRASE 2 out:(pos=5)) OR (Rvpn:(pos=6) PHRASE 2 0:(pos=7)))" },
    { "COM+-gebeurtenissysteem", "(Rcom:(pos=1) OR gebeurtenissysteem:(pos=2))" },
    { "rcpthosts (#5.7.1)", "(rcpthost:(pos=1) OR (5:(pos=2) PHRASE 3 7:(pos=3) PHRASE 3 1:(pos=4)))" },
    { "Dit apparaat werkt niet goed omdat Windows de voor dit apparaat vereiste stuurprogramma's niet kan laden.  (Code 31)", "(Rdit:(pos=1) OR apparaat:(pos=2) OR werkt:(pos=3) OR niet:(pos=4) OR go:(pos=5) OR omdat:(pos=6) OR Rwindows:(pos=7) OR de:(pos=8) OR voor:(pos=9) OR dit:(pos=10) OR apparaat:(pos=11) OR vereist:(pos=12) OR (stuurprogramma:(pos=13) PHRASE 2 s:(pos=14)) OR niet:(pos=15) OR kan:(pos=16) OR laden:(pos=17) OR Rcode:(pos=18) OR 31:(pos=19))" },
    { "window.open( scrollbar", "((window:(pos=1) PHRASE 2 open:(pos=2)) OR scrollbar:(pos=3))" },
    { "T68i truc ->", "(Rt68i:(pos=1) OR truc:(pos=2))" },
    { "T68i ->", "Rt68i:(pos=1)" },
    { "\"de lijn is bezet\"\"", "(de:(pos=1) PHRASE 4 lijn:(pos=2) PHRASE 4 is:(pos=3) PHRASE 4 bezet:(pos=4))" },
    { "if (eregi(\"", "(if:(pos=1) OR eregi:(pos=2))" },
    { "This device is not working properly because Windows cannot load the drivers required for this device. (Code 31)", "(Rthis:(pos=1) OR devic:(pos=2) OR is:(pos=3) OR not:(pos=4) OR work:(pos=5) OR proper:(pos=6) OR becaus:(pos=7) OR Rwindows:(pos=8) OR cannot:(pos=9) OR load:(pos=10) OR the:(pos=11) OR driver:(pos=12) OR requir:(pos=13) OR for:(pos=14) OR this:(pos=15) OR device:(pos=16) OR Rcode:(pos=17) OR 31:(pos=18))" },
    { "execCommand(\"Paste\");", "(execcommand:(pos=1) OR Rpaste:(pos=2))" },
    { "\"-1 unread\"", "(1:(pos=1) PHRASE 2 unread:(pos=2))" },
    { "\"www.historical-fire-engines", "(www:(pos=1) PHRASE 4 histor:(pos=2) PHRASE 4 fire:(pos=3) PHRASE 4 engin:(pos=4))" },
    { "\"DVD+RW\" erase", "((Rdvd:(pos=1) PHRASE 2 Rrw:(pos=2)) OR eras:(pos=3))" },
    { "[showjekamer)", "showjekam:(pos=1)" },
    { "The description for Event ID  1  in Source  True Vector Engine ) cannot be found. The local computer may not have the necessary registry information or message DLL files to display messages from a remote computer. You may be able to use the /AUXSOURC", "(Rthe:(pos=1) OR descript:(pos=2) OR for:(pos=3) OR Revent:(pos=4) OR Rid:(pos=5) OR 1:(pos=6) OR in:(pos=7) OR Rsource:(pos=8) OR Rtrue:(pos=9) OR Rvector:(pos=10) OR Rengine:(pos=11) OR cannot:(pos=12) OR be:(pos=13) OR found:(pos=14) OR Rthe:(pos=15) OR local:(pos=16) OR comput:(pos=17) OR may:(pos=18) OR not:(pos=19) OR have:(pos=20) OR the:(pos=21) OR necessari:(pos=22) OR registri:(pos=23) OR inform:(pos=24) OR or:(pos=25) OR messag:(pos=26) OR Rdll:(pos=27) OR file:(pos=28) OR to:(pos=29) OR display:(pos=30) OR messag:(pos=31) OR from:(pos=32) OR a:(pos=33) OR remot:(pos=34) OR computer:(pos=35) OR Ryou:(pos=36) OR may:(pos=37) OR be:(pos=38) OR abl:(pos=39) OR to:(pos=40) OR use:(pos=41) OR the:(pos=42) OR Rauxsourc:(pos=43))" },
    { "org.apache.jasper.JasperException: This absolute uri (http://java.sun.com/jstl/core) cannot be resolved in either web.xml or the jar files deployed with this application", "((org:(pos=1) PHRASE 4 apach:(pos=2) PHRASE 4 jasper:(pos=3) PHRASE 4 Rjasperexception:(pos=4)) OR Rthis:(pos=5) OR absolut:(pos=6) OR uri:(pos=7) OR (http:(pos=8) PHRASE 6 java:(pos=9) PHRASE 6 sun:(pos=10) PHRASE 6 com:(pos=11) PHRASE 6 jstl:(pos=12) PHRASE 6 core:(pos=13)) OR cannot:(pos=14) OR be:(pos=15) OR resolv:(pos=16) OR in:(pos=17) OR either:(pos=18) OR (web:(pos=19) PHRASE 2 xml:(pos=20)) OR or:(pos=21) OR the:(pos=22) OR jar:(pos=23) OR file:(pos=24) OR deploy:(pos=25) OR with:(pos=26) OR this:(pos=27) OR applic:(pos=28))" },
    { "This absolute uri (http://java.sun.com/jstl/core) cannot be resolved in either web.xml or the jar files deployed with this application", "(Rthis:(pos=1) OR absolut:(pos=2) OR uri:(pos=3) OR (http:(pos=4) PHRASE 6 java:(pos=5) PHRASE 6 sun:(pos=6) PHRASE 6 com:(pos=7) PHRASE 6 jstl:(pos=8) PHRASE 6 core:(pos=9)) OR cannot:(pos=10) OR be:(pos=11) OR resolv:(pos=12) OR in:(pos=13) OR either:(pos=14) OR (web:(pos=15) PHRASE 2 xml:(pos=16)) OR or:(pos=17) OR the:(pos=18) OR jar:(pos=19) OR file:(pos=20) OR deploy:(pos=21) OR with:(pos=22) OR this:(pos=23) OR applic:(pos=24))" },
    { "vervangen # \"/", "vervangen:(pos=1)" },
    { "vervangen # /\"", "vervangen:(pos=1)" },
    { "while(list($key, $val) = each($HTTP_POST_VARS))", "(while:(pos=1) OR list:(pos=2) OR key:(pos=3) OR val:(pos=4) OR each:(pos=5) OR (Rhttp:(pos=6) PHRASE 3 Rpost:(pos=7) PHRASE 3 Rvars:(pos=8)))" },
    { "PowerDVD does not support the current display mode. (DDraw Overlay mode is recommended)", "(Rpowerdvd:(pos=1) OR doe:(pos=2) OR not:(pos=3) OR support:(pos=4) OR the:(pos=5) OR current:(pos=6) OR display:(pos=7) OR mode:(pos=8) OR Rddraw:(pos=9) OR Roverlay:(pos=10) OR mode:(pos=11) OR is:(pos=12) OR recommend:(pos=13))" },
    { "Warning:  Unexpected character in input:  '' (ASCII=92) state=1  highlight", "(Rwarning:(pos=1) OR Runexpected:(pos=2) OR charact:(pos=3) OR in:(pos=4) OR input:(pos=5) OR Rascii:(pos=6) OR 92:(pos=7) OR state:(pos=8) OR 1:(pos=9) OR highlight:(pos=10))" },
    { "error: Qt-1.4 (headers and libraries) not found. Please check your installation!", "(error:(pos=1) OR (Rqt:(pos=2) PHRASE 3 1:(pos=3) PHRASE 3 4:(pos=4)) OR header:(pos=5) OR and:(pos=6) OR librari:(pos=7) OR not:(pos=8) OR found:(pos=9) OR Rplease:(pos=10) OR check:(pos=11) OR your:(pos=12) OR instal:(pos=13))" },
    { "Error while initializing the sound driver: device /dev/dsp can't be opened (No such device) The sound server will continue, using the null output device.", "(Rerror:(pos=1) OR while:(pos=2) OR initi:(pos=3) OR the:(pos=4) OR sound:(pos=5) OR driver:(pos=6) OR devic:(pos=7) OR (dev:(pos=8) PHRASE 2 dsp:(pos=9)) OR (can:(pos=10) PHRASE 2 t:(pos=11)) OR be:(pos=12) OR open:(pos=13) OR Rno:(pos=14) OR such:(pos=15) OR devic:(pos=16) OR Rthe:(pos=17) OR sound:(pos=18) OR server:(pos=19) OR will:(pos=20) OR continu:(pos=21) OR use:(pos=22) OR the:(pos=23) OR null:(pos=24) OR output:(pos=25) OR device:(pos=26))" },
    { "mag mijn waarschuwing nu weg ? ;)", "(mag:(pos=1) OR mijn:(pos=2) OR waarschuw:(pos=3) OR nu:(pos=4) OR weg:(pos=5))" },
    { "Abit NF7-S (nForce 2 Chipset) Rev 2.0", "(Rabit:(pos=1) OR (Rnf7:(pos=2) PHRASE 2 Rs:(pos=3)) OR nforc:(pos=4) OR 2:(pos=5) OR Rchipset:(pos=6) OR Rrev:(pos=7) OR (2:(pos=8) PHRASE 2 0:(pos=9)))" },
    { "Setup Could Not Verify the Integrity of the File\" Error Message Occurs When You Try to Install Windows XP Service Pack 1", "(Rsetup:(pos=1) OR Rcould:(pos=2) OR Rnot:(pos=3) OR Rverify:(pos=4) OR the:(pos=5) OR Rintegrity:(pos=6) OR of:(pos=7) OR the:(pos=8) OR Rfile:(pos=9) OR (Rerror:(pos=10) PHRASE 13 Rmessage:(pos=11) PHRASE 13 Roccurs:(pos=12) PHRASE 13 Rwhen:(pos=13) PHRASE 13 Ryou:(pos=14) PHRASE 13 Rtry:(pos=15) PHRASE 13 to:(pos=16) PHRASE 13 Rinstall:(pos=17) PHRASE 13 Rwindows:(pos=18) PHRASE 13 Rxp:(pos=19) PHRASE 13 Rservice:(pos=20) PHRASE 13 Rpack:(pos=21) PHRASE 13 1:(pos=22)))" },
    { "(browser 19) citrix", "(browser:(pos=1) OR 19:(pos=2) OR citrix:(pos=3))" },
    { "preg_replace (.*?)", "(preg:(pos=1) PHRASE 2 replac:(pos=2))" },
    { "formule excel #naam\"?\"", "(formul:(pos=1) OR excel:(pos=2) OR naam:(pos=3))" },
    { "->", "" },
    { "De instructie op 0x77f436f7 verwijst naar geheugen op 0x007f4778. De lees-of schrijfbewerking (\"written\") op het geheugen is mislukt", "(Rde:(pos=1) OR instructi:(pos=2) OR op:(pos=3) OR 0x77f436f7:(pos=4) OR verwijst:(pos=5) OR naar:(pos=6) OR geheugen:(pos=7) OR op:(pos=8) OR 0x007f4778:(pos=9) OR Rde:(pos=10) OR (lee:(pos=11) PHRASE 2 of:(pos=12)) OR schrijfbewerk:(pos=13) OR written:(pos=14) OR op:(pos=15) OR het:(pos=16) OR geheugen:(pos=17) OR is:(pos=18) OR mislukt:(pos=19))" },
    { "<iframe src=\"www.tweakers.net></iframe>", "(ifram:(pos=1) OR src:(pos=2) OR (www:(pos=3) PHRASE 4 tweaker:(pos=4) PHRASE 4 net:(pos=5) PHRASE 4 ifram:(pos=6)))" },
    { "\"rpm -e httpd\"", "(rpm:(pos=1) PHRASE 3 e:(pos=2) PHRASE 3 httpd:(pos=3))" },
    { "automatisch op All Flis (*.*)", "(automatisch:(pos=1) OR op:(pos=2) OR Rall:(pos=3) OR Rflis:(pos=4))" },
    { "(Windows; U; Windows NT 5.1; en-US; rv:1.3b) Gecko/20030210", "(Rwindows:(pos=1) OR Ru:(pos=2) OR Rwindows:(pos=3) OR Rnt:(pos=4) OR (5:(pos=5) PHRASE 2 1:(pos=6)) OR (en:(pos=7) PHRASE 2 Rus:(pos=8)) OR (rv:(pos=9) PHRASE 3 1:(pos=10) PHRASE 3 3b:(pos=11)) OR (Rgecko:(pos=12) PHRASE 2 20030210:(pos=13)))" },
    { "en-US; rv:1.3b) Gecko/20030210", "((en:(pos=1) PHRASE 2 Rus:(pos=2)) OR (rv:(pos=3) PHRASE 3 1:(pos=4) PHRASE 3 3b:(pos=5)) OR (Rgecko:(pos=6) PHRASE 2 20030210:(pos=7)))" },
    { "\"en-US; rv:1.3b) Gecko/20030210\"", "(en:(pos=1) PHRASE 7 Rus:(pos=2) PHRASE 7 rv:(pos=3) PHRASE 7 1:(pos=4) PHRASE 7 3b:(pos=5) PHRASE 7 Rgecko:(pos=6) PHRASE 7 20030210:(pos=7))" },
    { "(./) chmod.sh", "(chmod:(pos=1) PHRASE 2 sh:(pos=2))" },
    { "document.write(ssg(\" html", "((document:(pos=1) PHRASE 2 write:(pos=2)) OR ssg:(pos=3) OR html:(pos=4))" },
    { "superstack \"mac+adressen\"", "(superstack:(pos=1) OR (mac:(pos=2) PHRASE 2 adressen:(pos=3)))" },
    { "IIS getenv(REMOTE_HOST)_", "(Riis:(pos=1) OR getenv:(pos=2) OR (Rremote:(pos=3) PHRASE 2 Rhost:(pos=4)))" },
    { "IIS en getenv(REMOTE_HOST)", "(Riis:(pos=1) OR en:(pos=2) OR getenv:(pos=3) OR (Rremote:(pos=4) PHRASE 2 Rhost:(pos=5)))" },
    { "php getenv(\"HTTP_REFERER\")", "(php:(pos=1) OR getenv:(pos=2) OR (Rhttp:(pos=3) PHRASE 2 Rreferer:(pos=4)))" },
    { "nec+-1300", "(nec:(pos=1) OR 1300:(pos=2))" },
    { "smbpasswd script \"-s\"", "(smbpasswd:(pos=1) OR script:(pos=2) OR s:(pos=3))" },
    { "leestekens \" \xd6 \xeb", "(leesteken:(pos=1) OR (Roe:(pos=2) PHRASE 2 e:(pos=3)))" },
    { "freesco and (all seeing eye)", "(freesco:(pos=1) OR and:(pos=2) OR all:(pos=3) OR see:(pos=4) OR eye:(pos=5))" },
    { "('all seeing eye') and freesco", "(all:(pos=1) OR see:(pos=2) OR eye:(pos=3) OR and:(pos=4) OR freesco:(pos=5))" },
    { "\"[......\"", "" },
    { "Error = 11004 (500 No Data (Winsock error #11004))", "(Rerror:(pos=1) OR 11004:(pos=2) OR 500:(pos=3) OR Rno:(pos=4) OR Rdata:(pos=5) OR Rwinsock:(pos=6) OR error:(pos=7) OR 11004:(pos=8))" },
    { "gegevensfout (cyclishe redundantiecontrole)", "(gegevensfout:(pos=1) OR cyclish:(pos=2) OR redundantiecontrol:(pos=3))" },
    { "firmware versie waar NEC\"", "(firmwar:(pos=1) OR versi:(pos=2) OR waar:(pos=3) OR Rnec:(pos=4))" },
    { "nu.nl \"-1\"", "((nu:(pos=1) PHRASE 2 nl:(pos=2)) OR 1:(pos=3))" },
    { "provider+-webspace", "(provid:(pos=1) OR webspac:(pos=2))" },
    { "verschil \"dvd+rw\" \"dvd-rw\"", "(verschil:(pos=1) OR (dvd:(pos=2) PHRASE 2 rw:(pos=3)) OR (dvd:(pos=4) PHRASE 2 rw:(pos=5)))" },
    { "(dhcp client) + hangt", "(dhcp:(pos=1) OR client:(pos=2) OR hangt:(pos=3))" },
    { "MSI 875P Neo-FIS2R (Intel 875P)", "(Rmsi:(pos=1) OR 875p:(pos=2) OR (Rneo:(pos=3) PHRASE 2 Rfis2r:(pos=4)) OR Rintel:(pos=5) OR 875p:(pos=6))" },
    { "voeding passief gekoeld\"", "(voed:(pos=1) OR passief:(pos=2) OR gekoeld:(pos=3))" },
    { "if (mysql_num_rows($resultaat)==1)", "(if:(pos=1) OR (mysql:(pos=2) PHRASE 3 num:(pos=3) PHRASE 3 row:(pos=4)) OR resultaat:(pos=5) OR 1:(pos=6))" },
    { "Server.CreateObject(\"Persits.Upload.1\")", "((Rserver:(pos=1) PHRASE 2 Rcreateobject:(pos=2)) OR (Rpersits:(pos=3) PHRASE 3 Rupload:(pos=4) PHRASE 3 1:(pos=5)))" },
    { "if(cod>9999999)cod=parseInt(cod/64)", "(if:(pos=1) OR cod:(pos=2) OR 9999999:(pos=3) OR cod:(pos=4) OR parseint:(pos=5) OR (cod:(pos=6) PHRASE 2 64:(pos=7)))" },
    { "if (cod>9999999", "(if:(pos=1) OR cod:(pos=2) OR 9999999:(pos=3))" },
    { "\"rm -rf /bin/laden\"", "(rm:(pos=1) PHRASE 4 rf:(pos=2) PHRASE 4 bin:(pos=3) PHRASE 4 laden:(pos=4))" },
    { "\">>> 0) & 0xFF\"", "(0:(pos=1) PHRASE 2 0xff:(pos=2))" },
    { "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01//EN\"> document.body.scrollHeight", "(Rdoctype:(pos=1) OR Rhtml:(pos=2) OR Rpublic:(pos=3) OR (Rw3c:(pos=4) PHRASE 6 Rdtd:(pos=5) PHRASE 6 Rhtml:(pos=6) PHRASE 6 4:(pos=7) PHRASE 6 01:(pos=8) PHRASE 6 Ren:(pos=9)) OR (document:(pos=10) PHRASE 3 bodi:(pos=11) PHRASE 3 scrollheight:(pos=12)))" },
    { "<BR>window.resizeBy(offsetX,offsetY)<P>kweet", "(Rbr:(pos=1) OR (window:(pos=2) PHRASE 2 resizebi:(pos=3)) OR offsetx:(pos=4) OR offseti:(pos=5) OR Rp:(pos=6) OR kweet:(pos=7))" },
    { "linux humor :)", "(linux:(pos=1) OR humor:(pos=2))" },
    { "ClassFactory kan aangevraagde klasse niet leveren  (Fout=80040111)", "(Rclassfactory:(pos=1) OR kan:(pos=2) OR aangevraagd:(pos=3) OR klass:(pos=4) OR niet:(pos=5) OR leveren:(pos=6) OR Rfout:(pos=7) OR 80040111:(pos=8))" },
    { "remote_smtp defer (-44)", "((remot:(pos=1) PHRASE 2 smtp:(pos=2)) OR defer:(pos=3) OR 44:(pos=4))" },
    { "txtlogin.getText().trim().toUpperCase().intern() == inuser[2 * (i - 1) + 2].trim().toUpperCase().intern() && txtpass.getText().trim().toUpperCase().intern() == inuser[2 * (i - 1) + 3].trim().toUpperCase().intern())", "((txtlogin:(pos=1) PHRASE 2 gettext:(pos=2)) OR trim:(pos=3) OR touppercas:(pos=4) OR intern:(pos=5) OR inus:(pos=6) OR 2:(pos=7) OR i:(pos=8) OR 1:(pos=9) OR 2:(pos=10) OR trim:(pos=11) OR touppercas:(pos=12) OR intern:(pos=13) OR (txtpass:(pos=14) PHRASE 2 gettext:(pos=15)) OR trim:(pos=16) OR touppercas:(pos=17) OR intern:(pos=18) OR inus:(pos=19) OR 2:(pos=20) OR i:(pos=21) OR 1:(pos=22) OR 3:(pos=23) OR trim:(pos=24) OR touppercas:(pos=25) OR intern:(pos=26))" },
    { "Koper + amoniak (NH2", "(Rkoper:(pos=1) OR amoniak:(pos=2) OR Rnh2:(pos=3))" },
    { "nec dvd -/+r", "((nec:(pos=1) OR dvd:(pos=2)) AND_NOT r:(pos=3))" }, // Not ideal at all - "-" shouldn't fire here...
    { "er is een gereserveerde fout (-1104) opgetreden", "(er:(pos=1) OR is:(pos=2) OR een:(pos=3) OR gereserveerd:(pos=4) OR fout:(pos=5) OR 1104:(pos=6) OR opgetreden:(pos=7))" },
    { "Cor \\(CCN\\)'\" <cor.kloet@ccn.controlec.nl>", "(Rcor:(pos=1) OR Rccn:(pos=2) OR (cor:(pos=3) PHRASE 5 kloet:(pos=4) PHRASE 5 ccn:(pos=5) PHRASE 5 controlec:(pos=6) PHRASE 5 nl:(pos=7)))" },
    { "Warning: Failed opening for inclusion (include_path='') in Unknown on line 0", "(Rwarning:(pos=1) OR Rfailed:(pos=2) OR open:(pos=3) OR for:(pos=4) OR inclus:(pos=5) OR (includ:(pos=6) PHRASE 2 path:(pos=7)) OR in:(pos=8) OR Runknown:(pos=9) OR on:(pos=10) OR line:(pos=11) OR 0:(pos=12))" },
    { "\"~\" + \"c:\\\"", "c:(pos=1)" },
    { "mysql count(*)", "(mysql:(pos=1) OR count:(pos=2))" },
    { "for %f in (*.*) do", "(for:(pos=1) OR f:(pos=2) OR in:(pos=3) OR do:(pos=4))" },
    { "raar \"~\" bestand", "(raar:(pos=1) OR bestand:(pos=2))" },
    { "NEC DVD +-R/RW 1300", "(Rnec:(pos=1) OR Rdvd:(pos=2) OR (Rr:(pos=3) PHRASE 2 Rrw:(pos=4)) OR 1300:(pos=5))" },
    { "approved (ref: 38446-263)", "(approv:(pos=1) OR ref:(pos=2) OR (38446:(pos=3) PHRASE 2 263:(pos=4)))" },
    { "GA-7VRXP(2.0)", "((Rga:(pos=1) PHRASE 2 7vrxp:(pos=2)) OR (2:(pos=3) PHRASE 2 0:(pos=4)))" },
    { "~ Could not retrieve directory listing for \"/\"", "(Rcould:(pos=1) OR not:(pos=2) OR retriev:(pos=3) OR directori:(pos=4) OR list:(pos=5) OR for:(pos=6))" },
    { "asp CreateObject(\"Word.Document\")", "(asp:(pos=1) OR Rcreateobject:(pos=2) OR (Rword:(pos=3) PHRASE 2 Rdocument:(pos=4)))" },
    { "De lees- of schrijfbewerking (\"written\") op het geheugen is mislukt.", "(Rde:(pos=1) OR lees-:(pos=2) OR of:(pos=3) OR schrijfbewerk:(pos=4) OR written:(pos=5) OR op:(pos=6) OR het:(pos=7) OR geheugen:(pos=8) OR is:(pos=9) OR mislukt:(pos=10))" },
    { "putStr (map (\\x -> chr (round (21/2 * x^3 - 92 * x^2 + 503/2 * x - 105))) [1..4])", "(putstr:(pos=1) OR map:(pos=2) OR ((x:(pos=3) OR round:(pos=5) OR (21:(pos=6) PHRASE 2 2:(pos=7)) OR x:(pos=8) OR 3:(pos=9) OR 92:(pos=10) OR x:(pos=11) OR 2:(pos=12) OR (503:(pos=13) PHRASE 2 2:(pos=14)) OR x:(pos=15) OR 105:(pos=16)) AND_NOT chr:(pos=4)) OR (1:(pos=17) PHRASE 2 4:(pos=18)))" },
    { "parent.document.getElementById(\\\"leftmenu\\\").cols", "((parent:(pos=1) PHRASE 3 document:(pos=2) PHRASE 3 getelementbyid:(pos=3)) OR leftmenu:(pos=4) OR col:(pos=5))" },
    { "<% if not isEmpty(Request.QueryString) then", "(if:(pos=1) OR not:(pos=2) OR isempti:(pos=3) OR (Rrequest:(pos=4) PHRASE 2 Rquerystring:(pos=5)) OR then:(pos=6))" },
    { "Active Desktop (Hier issie)", "(Ractive:(pos=1) OR Rdesktop:(pos=2) OR Rhier:(pos=3) OR issi:(pos=4))" },
    { "Asus A7V8X (LAN + Sound)", "(Rasus:(pos=1) OR Ra7v8x:(pos=2) OR Rlan:(pos=3) OR Rsound:(pos=4))" },
    { "Novell This pentium class machine (or greater) lacks some required CPU feature(s", "(Rnovell:(pos=1) OR Rthis:(pos=2) OR pentium:(pos=3) OR class:(pos=4) OR machin:(pos=5) OR or:(pos=6) OR greater:(pos=7) OR lack:(pos=8) OR some:(pos=9) OR requir:(pos=10) OR Rcpu:(pos=11) OR featur:(pos=12) OR s:(pos=13))" },
    { "sql server install fails error code (-1)", "(sql:(pos=1) OR server:(pos=2) OR instal:(pos=3) OR fail:(pos=4) OR error:(pos=5) OR code:(pos=6) OR 1:(pos=7))" },
    { "session_register(\"login\");", "((session:(pos=1) PHRASE 2 regist:(pos=2)) OR login:(pos=3))" },
    { "\"kylix+ndmb\"", "(kylix:(pos=1) PHRASE 2 ndmb:(pos=2))" },
    { "Cannot find imap library (libc-client.a).", "(Rcannot:(pos=1) OR find:(pos=2) OR imap:(pos=3) OR librari:(pos=4) OR (libc:(pos=5) PHRASE 3 client:(pos=6) PHRASE 3 a:(pos=7)))" },
    { "If ($_SESSION[\"Login\"] == 1)", "(Rif:(pos=1) OR Rsession:(pos=2) OR Rlogin:(pos=3) OR 1:(pos=4))" },
    { "You have an error in your SQL syntax near '1')' at line 1", "(Ryou:(pos=1) OR have:(pos=2) OR an:(pos=3) OR error:(pos=4) OR in:(pos=5) OR your:(pos=6) OR Rsql:(pos=7) OR syntax:(pos=8) OR near:(pos=9) OR 1:(pos=10) OR at:(pos=11) OR line:(pos=12) OR 1:(pos=13))" },
    { "ASRock K7VT2 (incl. LAN)", "(Rasrock:(pos=1) OR Rk7vt2:(pos=2) OR incl:(pos=3) OR Rlan:(pos=4))" },
    { "+windows98 +(geen communicatie) +ie5", "(windows98:(pos=1) AND (geen:(pos=2) OR communicati:(pos=3)) AND ie5:(pos=4))" },
    { "\"xterm -fn\"", "(xterm:(pos=1) PHRASE 2 fn:(pos=2))" },
    { "IRQL_NOT_LESS_OR_EQUAL", "(Rirql:(pos=1) PHRASE 5 Rnot:(pos=2) PHRASE 5 Rless:(pos=3) PHRASE 5 Ror:(pos=4) PHRASE 5 Requal:(pos=5))" },
    { "access query \"NOT IN\"", "(access:(pos=1) OR queri:(pos=2) OR (Rnot:(pos=3) PHRASE 2 Rin:(pos=4)))" },
    { "\"phrase one \"phrase two\"", "((phrase:(pos=1) PHRASE 2 one:(pos=2)) OR phrase:(pos=3) OR two:(pos=4))" }, // FIXME: 2 phrases better?
    { "NEAR 207 46 249 27", "(Rnear:(pos=1) OR 207:(pos=2) OR 46:(pos=3) OR 249:(pos=4) OR 27:(pos=5))" },
    { "- NEAR 12V voeding", "(Rnear:(pos=1) OR 12v:(pos=2) OR voed:(pos=3))" },
    { "waarom \"~\" in directorynaam", "(waarom:(pos=1) OR in:(pos=2) OR directorynaam:(pos=3))" },
    { "cd'r NEAR toebehoren", "((cd:(pos=1) PHRASE 2 r:(pos=2)) OR Rnear:(pos=3) OR toebehoren:(pos=4))" }, // FIXME: Not ideal - should NEAR work on phrases?
    { NULL, NULL }
};

static test test_and_queries[] = {
    { "internet explorer title:(http www)", "(internet:(pos=1) AND explor:(pos=2) AND XThttp:(pos=3) AND XTwww:(pos=4))" },
    // Regression test for bug in 0.9.2 and earlier - this would give
    // (two:(pos=2) AND_MAYBE (one:(pos=1) AND three:(pos=3)))
    { "one +two three", "(one:(pos=1) AND two:(pos=2) AND three:(pos=3))" },
    { "hello -title:\"hello world\"", "(hello:(pos=1) AND_NOT (XThello:(pos=2) PHRASE 2 XTworld:(pos=3)))" },
    { NULL, NULL }
};

static test test_stop_queries[] = {
    { "test the queryparser", "(test:(pos=1) AND queryparser:(pos=3))" },
    // Regression test for bug in 0.9.6 and earlier.  This would fail to
    // parse.
    { "test AND the AND queryparser", "(test:(pos=1) AND the:(pos=2) AND queryparser:(pos=3))" },
    // 0.9.6 and earlier ignored a stopword even if it was the only term.
    // We don't ignore it in this case, which is probably better.  But
    // an all-stopword query with multiple terms doesn't work, which
    // prevents 'to be or not to be' for being searchable unless made
    // into a phrase query.
    { "the", "the:(pos=1)" },
    { NULL, NULL }
};

static bool test_queryparser1()
{
    Xapian::QueryParser queryparser;
    queryparser.set_stemmer(Xapian::Stem("english"));
    queryparser.set_stemming_strategy(Xapian::QueryParser::STEM_SOME);
    queryparser.add_prefix("author", "A");
    queryparser.add_prefix("title", "XT");
    queryparser.add_prefix("subject", "XT");
    queryparser.add_boolean_prefix("site", "H");
    for (test *p = test_or_queries; p->query; ++p) {
	string expect, parsed;
	if (p->expect)
	    expect = p->expect;
	else
	    expect = "parse error";
	try {
	    Xapian::Query qobj = queryparser.parse_query(p->query);
	    parsed = qobj.get_description();
	    expect = string("Xapian::Query(") + expect + ')';
	} catch (const Xapian::Error &e) {
	    parsed = e.get_msg();
	} catch (const char *s) {
	    parsed = s;
	} catch (...) {
	    parsed = "Unknown exception!";
	}
	tout << "Query: " << p->query << '\n';
	TEST_EQUAL(parsed, expect);
    }
    return true;
}

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
    for (test *p = test_and_queries; p->query; ++p) {
	string expect, parsed;
	if (p->expect)
	    expect = p->expect;
	else
	    expect = "parse error";
	try {
	    Xapian::Query qobj = queryparser.parse_query(p->query);
	    parsed = qobj.get_description();
	    expect = string("Xapian::Query(") + expect + ')';
	} catch (const Xapian::Error &e) {
	    parsed = e.get_msg();
	} catch (const char *s) {
	    parsed = s;
	} catch (...) {
	    parsed = "Unknown exception!";
	}
	tout << "Query: " << p->query << '\n';
	TEST_EQUAL(parsed, expect);
    }
    return true;
}

// Test query with odd characters in.
static bool test_qp_odd_chars1()
{
    Xapian::QueryParser queryparser;
    string query("\x01weird\x00stuff\x7f", 13);
    Xapian::Query qobj = queryparser.parse_query(query);
    tout << "Query:  " << query << '\n';
    TEST_EQUAL(qobj.get_description(), "Xapian::Query((weird:(pos=1) OR stuff:(pos=2)))");
    return true;
}

// Test right truncation.
static bool test_qp_flag_wildcard1()
{
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
    Xapian::QueryParser queryparser;
    queryparser.set_database(db);
    Xapian::Query qobj = queryparser.parse_query("ab*", Xapian::QueryParser::FLAG_WILDCARD);
    TEST_EQUAL(qobj.get_description(), "Xapian::Query(abc:(pos=1))");
    qobj = queryparser.parse_query("muscle*", Xapian::QueryParser::FLAG_WILDCARD);
    TEST_EQUAL(qobj.get_description(), "Xapian::Query((muscle:(pos=1) OR musclebound:(pos=1)))");
    qobj = queryparser.parse_query("meat*", Xapian::QueryParser::FLAG_WILDCARD);
    TEST_EQUAL(qobj.get_description(), "Xapian::Query()");
    qobj = queryparser.parse_query("musc*", Xapian::QueryParser::FLAG_WILDCARD);
    TEST_EQUAL(qobj.get_description(), "Xapian::Query((muscat:(pos=1) OR muscle:(pos=1) OR musclebound:(pos=1) OR muscular:(pos=1)))");
    qobj = queryparser.parse_query("mutt*", Xapian::QueryParser::FLAG_WILDCARD);
    TEST_EQUAL(qobj.get_description(), "Xapian::Query(mutton:(pos=1))");
    // Regression test (we weren't lowercasing terms before checking if they
    // were in the database or not):
    qobj = queryparser.parse_query("mUTTON++");
    TEST_EQUAL(qobj.get_description(), "Xapian::Query(mutton:(pos=1))");
    return true;
}

static bool test_qp_flag_bool_any_case1()
{
    using Xapian::QueryParser;
    Xapian::QueryParser qp;
    Xapian::Query qobj;
    qobj = qp.parse_query("to and fro", QueryParser::FLAG_BOOLEAN | QueryParser::FLAG_BOOLEAN_ANY_CASE);
    TEST_EQUAL(qobj.get_description(), "Xapian::Query((to:(pos=1) AND fro:(pos=2)))");
    qobj = qp.parse_query("to and fro", QueryParser::FLAG_BOOLEAN);
    TEST_EQUAL(qobj.get_description(), "Xapian::Query((to:(pos=1) OR and:(pos=2) OR fro:(pos=3)))");
    // Regression test for bug in 0.9.4 and earlier.
    qobj = qp.parse_query("to And fro", QueryParser::FLAG_BOOLEAN | QueryParser::FLAG_BOOLEAN_ANY_CASE);
    TEST_EQUAL(qobj.get_description(), "Xapian::Query((to:(pos=1) AND fro:(pos=2)))");
    qobj = qp.parse_query("to And fro", QueryParser::FLAG_BOOLEAN);
    TEST_EQUAL(qobj.get_description(), "Xapian::Query((to:(pos=1) OR and:(pos=2) OR fro:(pos=3)))");
    return true;
}

static bool test_qp_stopper1()
{
    Xapian::QueryParser qp;
    const char * stopwords[] = { "a", "an", "the" };
    Xapian::SimpleStopper stop(stopwords, stopwords + 3);
    qp.set_stopper(&stop);
    qp.set_default_op(Xapian::Query::OP_AND);
    for (test *p = test_stop_queries; p->query; ++p) {
	string expect, parsed;
	if (p->expect)
	    expect = p->expect;
	else
	    expect = "parse error";
	try {
	    Xapian::Query qobj = qp.parse_query(p->query);
	    parsed = qobj.get_description();
	    expect = string("Xapian::Query(") + expect + ')';
	} catch (const Xapian::Error &e) {
	    parsed = e.get_msg();
	} catch (const char *s) {
	    parsed = s;
	} catch (...) {
	    parsed = "Unknown exception!";
	}
	tout << "Query: " << p->query << '\n';
	TEST_EQUAL(parsed, expect);
    }
    return true;
}

/// Test cases for the QueryParser.
static test_desc tests[] = {
    TESTCASE(queryparser1),
    TESTCASE(qp_default_op1),
    TESTCASE(qp_odd_chars1),
    TESTCASE(qp_flag_wildcard1),
    TESTCASE(qp_flag_bool_any_case1),
    TESTCASE(qp_stopper1),
    END_OF_TESTCASES
};

int main(int argc, char **argv)
{
    test_driver::parse_command_line(argc, argv);
    return test_driver::run(tests);
}
