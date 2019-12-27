/** @file omindexcheck.cc
 * @brief Auxiliary program of omindextest
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

typedef vector<string> testcase;

unordered_map<string, testcase> tests;

static void
index_test()
{
    tests.insert({"test.txt", {"Zjoey", "Zfood",
		  "Zедой", "Z喬伊不分享食物"}});
    tests.insert({"test-csv.csv", {"ZFcsv", "Zbreak", "Zwere"}});
    tests.insert({"test-html.html", {"Ajeroen", "ZAoom", "ZSworld", "Shello",
		  "Zchapter"}});
#if defined HAVE_POPPLER
    tests.insert({"pdf/poppler.pdf", {"ZFpoppler", "Zsub", "Ztext", "Ztitl",
		  "Zpie"}});
#endif
#if defined HAVE_LIBEBOOK
    tests.insert({"fb2/hello.fb2", {"Ajeroen", "Aooms", "Zbeauti", "Zchapter",
		  "Zdocument", "Zooms3", "Zsubsect", "Ztoday", "ZFhello",
		  "Zoutlin"}});
    tests.insert({"fb2/lang-name.fb2", {"Adavid", "Atardon", "ZAdavid",
		  "ZFlang", "Zeel", "Zhovercraft", "Zlanguag"}});
    tests.insert({"fb2/lang2.fb2", {"Adavid", "Atardon", "ZAtardon",
		  "Zúhořů", "Zhovercraft", "Zlanguag", "Zmój", "Zof",
		  "Zpełen", "Zpoduszkowiec", "Zvznášedlo", "Zwęgorzi"}});
    tests.insert({"lrf/hello.lrf", {"ZFhello", "Zhello", "Zworld"}});
    tests.insert({"pdb/PalmDOC-hello.pdb", {"Shello", "Sworld", "ZFpalmdoc",
		  "Zworld"}});
    tests.insert({"pdb/PeanutPress-hello.pdb", {"Fpeanutpress", "ZFhello",
		  "Zhello"}});
    tests.insert({"pdb/test.pdb", {"Sdemodemo", "ZFtest", "Zwherearew"}});
#endif
#if defined HAVE_LIBETONYEK
    tests.insert({"keynotes/test-keynote.key", {"ZFkeynot", "Zbold", "Znow",
		  "Zsubtitl"}});
    tests.insert({"pages/test-pages.pages", {"ZFpage", "Zfull", "Zhovercraft",
		  "Zwęgorzi"}});
#endif
#if defined HAVE_TESSERACT
    tests.insert({"img/Test1.gif", {"Znoisyimag", "Zocr", "Ztesseract"}});
    tests.insert({"img/Test2.pgm", {"ZFtest2", "Znoisyimag", "Ztesseract"}});
    tests.insert({"img/Test3.ppm", {"ZFtest3", "Zocr", "Ztest"}});
    tests.insert({"img/Test4.tiff", {"Znoisyimag", "Ztesseract", "Zto"}});
    tests.insert({"img/Test5.webp", {"Znoisyimag", "Ztesseract", "Ztest"}});
    tests.insert({"img/poster-2.jpg", {"ZFposter", "Zby", "Zproperti",
		  "Zsurveil", "Zvideo"}});
    tests.insert({"img/poster.jpg", {"Zbicycl", "Zride", "Zroller",
		  "Zskateboard"}});
    tests.insert({"img/scan-page.png", {"Zannual", "Zfed", "Zreturn",
		  "Zwhile"}});
#endif
}

static bool
compare_test(testcase& test, const Xapian::Document& doc, const string& file)
{
    sort(test.begin(), test.end());
    Xapian::TermIterator term_iterator = doc.termlist_begin();
    for (auto& t : test) {
	term_iterator.skip_to(t);
	if (term_iterator == doc.termlist_end() || *term_iterator != t) {
	    cerr << "Error in " << file << ": Term " << t <<
		 " does not belong to this file" << endl;
	    return false;
	}
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
	auto it = tests.find(url);
	if (it != tests.end())
	    succeed &= compare_test(it->second, doc, url);
    }
    return succeed ? 0 : 1;
}
