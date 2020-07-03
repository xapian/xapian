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
#include "stringutils.h"

#include <iostream>
#include <string>
#include <unordered_map>
#include <xapian.h>

using namespace std;

typedef vector<string> testcase;

static unordered_map<string, testcase> tests;

static void
index_test()
{
    tests.insert({"test.txt",
		  {"Zjoey", "Zfood", "Zедой", "Z喬伊不分享食物"}});
    tests.insert({"test-csv.csv",
		  {"ZFcsv", "Zbreak", "Zwere"}});
    tests.insert({"test-html.html",
		  {"Ajeroen", "ZAoom", "ZSworld", "Shello", "Zchapter"}});
#if defined HAVE_POPPLER
    tests.insert({"pdf/poppler.pdf",
		  {"ZFpoppler", "Zsub", "Ztext", "Ztitl", "Zpie"}});
#endif
#if defined HAVE_LIBEBOOK
    tests.insert({"fb2/hello.fb2",
		  {"Ajeroen", "Aooms", "Zbeauti", "Zchapter", "Zdocument",
		   "Zooms3", "Zsubsect", "Ztoday", "ZFhello", "Zoutlin"}});
    tests.insert({"fb2/lang-name.fb2",
		  {"Adavid", "Atardon", "ZAdavid", "ZFlang", "Zeel",
		   "Zhovercraft", "Zlanguag"}});
    tests.insert({"fb2/lang2.fb2",
		  {"Adavid", "Atardon", "ZAtardon", "Zúhořů", "Zhovercraft",
		   "Zlanguag", "Zmój", "Zof", "Zpełen", "Zpoduszkowiec",
		   "Zvznášedlo", "Zwęgorzi"}});
    tests.insert({"lrf/hello.lrf",
		  {"ZFhello", "Zhello", "Zworld"}});
    tests.insert({"pdb/PalmDOC-hello.pdb",
		  {"Shello", "Sworld", "ZFpalmdoc", "Zworld"}});
    tests.insert({"pdb/PeanutPress-hello.pdb",
		  {"Fpeanutpress", "ZFhello", "Zhello"}});
    tests.insert({"pdb/test.pdb",
		  {"Sdemodemo", "ZFtest", "Zwherearew"}});
#endif
#if defined HAVE_LIBETONYEK
    tests.insert({"keynotes/test-keynote.key",
		  {"ZFkeynot", "Zbold", "Znow", "Zsubtitl"}});
    tests.insert({"pages/test-pages.pages",
		  {"ZFpage", "Zfull", "Zhovercraft", "Zwęgorzi"}});
#endif
#if defined HAVE_TESSERACT
    tests.insert({"img/Test1.gif",
		  {"Znoisyimag", "Zocr", "Ztesseract"}});
    tests.insert({"img/Test2.pgm",
		  {"ZFtest2", "Znoisyimag", "Ztesseract"}});
    tests.insert({"img/Test3.ppm",
		  {"ZFtest3", "Zocr", "Ztest"}});
    tests.insert({"img/Test4.tiff",
		  {"Znoisyimag", "Ztesseract", "Zto"}});
    tests.insert({"img/Test5.webp",
		  {"Znoisyimag", "Ztesseract", "Ztest"}});
    tests.insert({"img/poster-2.jpg",
		  {"ZFposter", "Zby", "Zproperti", "Zsurveil", "Zvideo"}});
    tests.insert({"img/poster.jpg",
		  {"Zbicycl", "Zride", "Zroller", "Zskateboard"}});
    tests.insert({"img/scan-page.png",
		  {"Zannual", "Zfed", "Zreturn", "Zwhile"}});
#endif
#if defined HAVE_LIBARCHIVE
    // blank file
    tests.insert({"odf/blank.odt", {}});
    // corrupted file (ODP)
    // tests.insert({"corrupt_file.odp", {"ZSnatur"}});

    // ODF
    tests.insert({"odf/test.odt",
		  {"Zедой", "Z喬伊不分享食物"}});
    tests.insert({"odf/libarchive_text.odt",
		  {"Stesttitle", "Atestauthor", "Zsampl", "Zhead", "Ztext",
		   "Zhello", "Zworld"}});
    tests.insert({"odf/libarchive_text_template.ott",
		  {"Zjane", "Zdoe", "Zstructur"}});
    tests.insert({"odf/libarchive_presentation.odp",
		  {"Zfascin", "Zfact", "Zpustak", "Zmahal", "Zmillion",
		   "Zpeopl", "Zbirthday", "501"}});
    tests.insert({"odf/libarchive_presentation_template.otp",
		  {"ZSalizarin", "Zhead", "Zworld", "Ztext"}});
    tests.insert({"odf/libarchive_spreadsheet.ods",
		  {"Zhello", "Zworld", "Zsampl", "2"}});
    tests.insert({"odf/libarchive_spreadsheet_template.ots",
		  {"Zfood", "Zpasta", "Zpercentag", "40"}});
    tests.insert({"odf/libarchive_draw.odg", {"Zparth", "Zkapadia"}});

    // Apache OpenOffice
    tests.insert({"sof/libarchive_openoffice_calc.sxc",
		  {"Ztoy", "Zproduct", "Zcost", "Zquantiti", "Zcardboard"}});
    tests.insert({"sof/libarchive_openoffice_calc_template.stc",
		  {"ZSpurchas", "ZStemplat", "Zproduct", "Zquantiti",
		   "Zsampl"}});
    tests.insert({"sof/libarchive_openoffice_text.sxw",
		  {"Zhello", "Zsampl", "Zopenoffic", "Zwriter"}});
    tests.insert({"sof/libarchive_openoffice_text_template.stw",
		  {"Zhello", "Zworld", "Zsampl", "Zhead", "ZStemplat",
		   "ZStext"}});
    tests.insert({"sof/libarchive_openoffice_presentation.sxi",
		  {"Zhead", "Zhello", "Zopenoffic", "Zimpress"}});
    tests.insert({"sof/libarchive_openoffice_presentation_template.sti",
		  {"ZSproject", "ZSresearch", "Zhead", "Ztext"}});
#endif
#if defined HAVE_LIBARCHIVE
    // blank file
    tests.insert({"odf/blank.odt", {}});
    // corrupted file (ODP)
    // "no text extracted from document body, but indexing metadata anyway"
    // error on searching for any term - term not found
    // tests.insert({"corrupt_file.odp", {"ZSnatur"}});

    // ODF
    tests.insert({"odf/test.odt", {"Zедой", "Z喬伊不分享食物"}});
    tests.insert({"odf/libarchive_text.odt", {"Stesttitle", "Atestauthor",
		  "Zsampl", "Zhead", "Ztext", "Zhello",
		  "Zworld"}});
    tests.insert({"odf/libarchive_text_template.ott", {"Zjane", "Zdoe",
		  "Zstructur"}});
    tests.insert({"odf/libarchive_presentation.odp", {"Zfascin", "Zfact",
		  "Zpustak", "Zmahal", "Zmillion", "Zpeopl", "Zbirthday",
		  "501"}});
    tests.insert({"odf/libarchive_presentation_template.otp", {"ZSalizarin",
		  "Zhead", "Zworld", "Ztext"}});
    tests.insert({"odf/libarchive_spreadsheet.ods", {"Zhello", "Zworld",
		  "Zsampl", "2"}});
    tests.insert({"odf/libarchive_spreadsheet_template.ots", {"Zfood", "Zpasta",
		  "Zpercentag", "40"}});
    tests.insert({"odf/libarchive_draw.odg", {"Zparth", "Zkapadia"}});

    // Apache OpenOffice
    tests.insert({"sof/libarchive_openoffice_calc.sxc", {"Ztoy", "Zproduct",
		  "Zcost", "Zquantiti", "Zcardboard"}});
    tests.insert({"sof/libarchive_openoffice_calc_template.stc", {"ZSpurchas",
		  "ZStemplat", "Zproduct", "Zquantiti", "Zsampl"}});
    tests.insert({"sof/libarchive_openoffice_text.sxw", {"Zhello", "Zsampl",
		  "Zopenoffic", "Zwriter"}});
    tests.insert({"sof/libarchive_openoffice_text_template.stw", {"Zhello",
		  "Zworld", "Zsampl", "Zhead", "ZStemplat", "ZStext"}});
    tests.insert({"sof/libarchive_openoffice_presentation.sxi", {"Zhead",
		  "Zhello", "Zopenoffic", "Zimpress"}});
    tests.insert({"sof/libarchive_openoffice_presentation_template.sti",
		 {"ZSproject", "ZSresearch", "Zhead", "Ztext"}});
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
    if (argc <= 1)
	return 1;
    db.add_database(Xapian::Database(argv[1]));

    index_test();
    for (auto t = db.allterms_begin("U"); t != db.allterms_end("U"); ++t) {
	const string& term = *t;
	string url(term, 2);
	Xapian::PostingIterator p = db.postlist_begin(term);
	if (p == db.postlist_end(term)) {
	    continue;
	}
	Xapian::docid did = *p;
	Xapian::Document doc = db.get_document(did);
	auto iter = tests.find(url);
	if (iter != tests.end()) {
	    succeed &= compare_test(iter->second, doc, url);
	}
    }
    return succeed ? 0 : 1;
}
