/** @file omindexcheck.cc
 * @brief test omindex
 */
/* Copyright (C) 2019 Bruno Baruffaldi
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
#include "worker.h"

#include <xapian.h>
#include <iostream>
#include <string>
#include <unordered_map>
#include <cstdlib>

#include "gnu_getopt.h"
#include "hashterm.h"
#include "common/stringutils.h"

using namespace std;

typedef struct {
    string title;
    string author;
    string dump;
}testcase;

unordered_map<string, testcase> tests;

static void
index_test(void)
{
    tests.insert({"test.txt", {"", "",
    "Joey doesn't share food "
    "Джои не делится едой 喬伊不分享食物 "}});
    tests.insert({"test-csv.csv", {"", "",
    "We were on a break ! 1 2 3 4 5 6 0.1 0.2 0.3 0.01 40.2 $9,00"}});
    tests.insert({"test-html.html", {"Hello World", "Jeroen Ooms",
    "helloworldjeroenooms3/20/2018chapter1subsection1todayisabeautifulday!1"}});
#if defined HAVE_POPPLER
    tests.insert({"pdf/hello.pdf", {"Hello World", "Jeroen Ooms",
    "helloworldjeroenooms3/20/2018chapter1subsection1todayisabeautifulday!1"}});
    tests.insert({"pdf/poppler.pdf", {"PDFTITLE", "Someone",
    "Abstracthereisyourabstract1topicsometexttotestomindex.1"}});
#endif
#if defined HAVE_LIBEBOOK
    tests.insert({"fb2/hello.fb2", {"", "Jeroen Ooms",
    "helloworldjeroenooms3/20/2018chapter1subsection1todayisabeautifulday!1"
    "documentoutlinechapter1subsection1"}});
    tests.insert({"fb2/lang-name.fb2",
    {"", "David Tardon", "myhovercraftisfullofeels."}});
    tests.insert({"fb2/lang2.fb2", {"", "David Tardon",
    "myhovercraftisfullofeels.mojevznášedlojeplnéúhořů."
    "mójpoduszkowiecjestpełenwęgorzy."}});
    tests.insert({"lrf/hello.lrf",
    {"", "", "helloworld."}});
    tests.insert({"pdb/test.pdb",
    {"demodemo", "", "wherearewe"}});
    tests.insert({"pdb/PalmDOC-hello.pdb",
    {"helloworld", "", "helloworld."}});
    tests.insert({"pdb/PeanutPress-hello.pdb",
    {"", "", "helloworld."}});
#endif
#if defined HAVE_LIBETONYEK
    tests.insert({"keynotes/test-keynote.key", {"", "",
    "titleslidesubtitlecapitalizedtitletextinboldanditalic,"
    "continuinginitaliconly,untilnowcoloredtextmorecoloredtexthere"}});
    tests.insert({"pages/test-pages.pages", {"", "",
    "myhovercraftisfullofeels.mévznášedlojeplnéúhořů."
    "mójpoduszkowiecjestpełenwęgorzy."}});
#endif
#if defined HAVE_TESSERACT
    tests.insert({"img/poster.jpg", {"", "",
    "SKATEBOARDINGBICYCLERIDINGROLLERBLADINGSCOOTERRIDING®"}});
    tests.insert({"img/poster-2.jpg", {"", "",
    "THISPROPERTYISPROTECTEDBYVIDEOSURVEILLANCE"}});
    tests.insert({"img/scan-page.png", {"", "",
    "of9.5%annuallywhiletheFed-eratedjunkfundreturned"
    "11.9%fearoffinancialcollapse,"}});
    tests.insert({"img/Test1.gif", {"", "",
    "noisy image to test tesseract ocr"}});
    tests.insert({"img/Test2.pgm", {"", "",
    "noisy image to test tesseract ocr"}});
    tests.insert({"img/Test3.ppm", {"", "",
    "noisy image to test tesseract ocr"}});
    tests.insert({"img/Test4.tiff", {"", "",
    "noisy image to test tesseract ocr"}});
    tests.insert({"img/Test5.webp", {"", "",
    "noisy image to test tesseract ocr"}});
#endif
#if defined HAVE_GMIME
    tests.insert({"eml/test-correo1.eml", {"asunto", "Me<me@mail.com>",
    "prueba para descargar correos--atte.me"}});
    tests.insert({"eml/test-correo2.eml",
    {"полностью переписана статья", "Me<me@mail.com>",
    "Венгерский алгоритм решения задачи"
    "о назначениях--Atte.BrunoBaruffaldi"}});
#endif
#if defined HAVE_CATDOC
    tests.insert({"xls/test-xls.xls", {"", "",
    "I am testing catdoc with calc 1 2 3 0.4 5.01 1.33%"}});
#endif
}

static testcase
get_from_data(const string& data)
{
    testcase* t = new testcase;
    size_t len = data.length();
    const char* p = data.c_str();
    const char* end = p + len;

    while (p != end) {
	const char* start = p;
	size_t start_l = start - data.c_str();
	p = static_cast<const char*>(memchr(p, '\n', end - start));
	const char* eol;
	if (p)
	    eol = p++;
	else
	    p = eol = end;
	if ((end - start) > 8 && memcmp(start, "caption=", 8) == 0) {
	    start_l += 8;
	    t->title.assign(data, start_l, data.find('\n', start_l) - start_l);
	} else if ((end - start) > 7 && memcmp(start, "author=", 7) == 0) {
	    start_l += 7;
	    t->author.assign(data, start_l, data.find('\n', start_l) - start_l);
	} else if ((end - start) > 7 && memcmp(start, "sample=", 7) == 0) {
	    start_l += 7;
	    t->dump.assign(data, start_l, data.find('\n', start_l) - start_l);
	}
    }
    return *t;
}

static bool
compare_text(const char* s, const char* t)
{
    int s_it, t_it;
    for (s_it = t_it = 0; s[s_it] != '\0' && t[t_it] != '\0'; ++s_it, ++t_it) {
	while (isspace(s[s_it]) && s[s_it] != '\0') s_it++;
	while (isspace(t[t_it]) && t[t_it] != '\0') t_it++;
	if (s[s_it] == '\0' || t[t_it] == '\0')
	    break;
	if (tolower(s[s_it]) != tolower(t[t_it]))
	    return false;
    }
    while (isspace(s[s_it]) && s[s_it] != '\0') s_it++;
    while (isspace(t[t_it]) && t[t_it] != '\0') t_it++;
    return s[s_it] == '\0' && t[t_it] == '\0';
}

static bool
compare_test(const string& file, const testcase& x, const testcase& y)
{
    if (!compare_text(x.author.c_str(), y.author.c_str())) {
	cerr << "Error " << file << ": The author is wrong" << endl;
	cerr << "\tExpected: " << x.author << endl;
	cerr << "\tGot: " << y.author << endl;
	return false;
    }
    if (!compare_text(x.title.c_str(), y.title.c_str())) {
	cerr << "Error " << file << ": The title is wrong" << endl;
	cerr << "\tExpected: " << x.title << endl;
	cerr << "\tGot: " << y.title << endl;
	return false;
    }
    if (!compare_text(x.dump.c_str(), y.dump.c_str())) {
	cerr << "Error " << file << ": The dump is wrong" << endl;
	cerr << "\tExpected: " << x.dump << endl;
	cerr << "\tGot: " << y.dump << endl;
	return false;
    }
    return true;
}

int
main(int argc, char** argv)
{
    Xapian::Database db;
    bool succeed = true;
    if (argc <= 2)
	return 1;
    db.add_database(Xapian::Database(argv[2]));
    string url, current_dir(argv[1]);

    index_test();
    for (auto t = db.allterms_begin("U"); t != db.allterms_end("U"); ++t) {
	const string& term = *t;
	Xapian::PostingIterator p = db.postlist_begin(term);
	if (p == db.postlist_end(term)) {
	    continue;
	}
	Xapian::docid did = *p;
	Xapian::Document doc = db.get_document(did);
	auto data = doc.get_data();
	size_t start;
	if (startswith(data, "url=")) {
	    start = CONST_STRLEN("url=");
	} else {
	    start = data.find("\nurl=");
	    if (start == string::npos)
		continue;
	    start += CONST_STRLEN("\nurl=");
	}
	url.assign(data, start, data.find('\n', start) - start);
	start = url.find(current_dir) + current_dir.length();
	url = url.substr(start, url.length());
	if (tests.find(url) != tests.end()) {
	    testcase dc = get_from_data(data);
	    succeed &= compare_test(url, tests[url], dc);
	}
    }
    return succeed ? 0 : 1;
}
