/** @file omindexcheck.cc
 * @brief Auxiliary program of omindextest
 */
/* Copyright (C) 2019 Bruno Baruffaldi
 * Copyright (C) 2020 Parth Kapadia
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

enum test_result { PASS, FAIL, SKIP };
enum test_flag { FAIL_IF_NO_TERMS = 1, SKIP_IF_NO_TERMS = 2, PASS_IF_NO_TERMS = 4 };

struct testcase {
    unsigned int flags;
    vector<string> terms;
};

static unordered_map<string, testcase> tests;

static void
index_test()
{
    tests.insert({"test.txt",
		  {FAIL_IF_NO_TERMS, {"Zjoey", "Zfood", "Zедой",
				      "Z喬伊不分享食物"}}});
    tests.insert({"test-csv.csv",
		  {FAIL_IF_NO_TERMS, {"ZFcsv", "Zbreak", "Zwere"}}});
    tests.insert({"test-html.html",
		  {FAIL_IF_NO_TERMS, {"Ajeroen", "ZAoom", "ZSworld", "Shello",
				      "Zchapter"}}});
#if defined HAVE_POPPLER
    tests.insert({"pdf/poppler.pdf",
		  {FAIL_IF_NO_TERMS, {"ZFpoppler", "Zsub", "Ztext", "Ztitl",
				      "Zpie"}}});
#endif
#if defined HAVE_LIBEBOOK
    tests.insert({"fb2/hello.fb2",
		  {FAIL_IF_NO_TERMS, {"Ajeroen", "Aooms", "Zbeauti", "Zchapter",
				      "Zdocument", "Zooms3", "Zsubsect",
				      "Ztoday", "ZFhello", "Zoutlin"}}});
    tests.insert({"fb2/lang-name.fb2",
		  {FAIL_IF_NO_TERMS, {"Adavid", "Atardon", "ZAdavid", "ZFlang",
				      "Zeel", "Zhovercraft", "Zlanguag"}}});
    tests.insert({"fb2/lang2.fb2",
		  {FAIL_IF_NO_TERMS, {"Adavid", "Atardon", "ZAtardon",
				      "Zúhořů", "Zhovercraft", "Zlanguag",
				      "Zmój", "Zof", "Zpełen", "Zwęgorzi",
				      "Zpoduszkowiec", "Zvznášedlo"}}});
    tests.insert({"lrf/hello.lrf",
		  {FAIL_IF_NO_TERMS, {"ZFhello", "Zhello", "Zworld"}}});
    tests.insert({"pdb/PalmDOC-hello.pdb",
		  {FAIL_IF_NO_TERMS, {"Shello", "Sworld", "ZFpalmdoc",
				      "Zworld"}}});
    tests.insert({"pdb/PeanutPress-hello.pdb",
		  {FAIL_IF_NO_TERMS, {"Fpeanutpress", "ZFhello", "Zhello"}}});
    tests.insert({"pdb/test.pdb",
		  {FAIL_IF_NO_TERMS, {"Sdemodemo", "ZFtest", "Zwherearew"}}});
#endif
#if defined HAVE_LIBETONYEK
    tests.insert({"keynotes/test-keynote.key",
		  {FAIL_IF_NO_TERMS, {"ZFkeynot", "Zbold", "Znow",
				      "Zsubtitl"}}});
    tests.insert({"pages/test-pages.pages",
		  {FAIL_IF_NO_TERMS, {"ZFpage", "Zfull", "Zhovercraft",
				      "Zwęgorzi"}}});
#endif
#if defined HAVE_TESSERACT
    tests.insert({"img/Test1.gif",
		  {FAIL_IF_NO_TERMS, {"Znoisyimag", "Zocr", "Ztesseract"}}});
    tests.insert({"img/Test2.pgm",
		  {FAIL_IF_NO_TERMS, {"ZFtest2", "Znoisyimag", "Ztesseract"}}});
    tests.insert({"img/Test3.ppm",
		  {FAIL_IF_NO_TERMS, {"ZFtest3", "Zocr", "Ztest"}}});
    tests.insert({"img/Test4.tiff",
		  {FAIL_IF_NO_TERMS, {"Znoisyimag", "Ztesseract", "Zto"}}});
    tests.insert({"img/Test5.webp",
		  {FAIL_IF_NO_TERMS, {"Znoisyimag", "Ztesseract", "Ztest"}}});
    tests.insert({"img/poster-2.jpg",
		  {FAIL_IF_NO_TERMS, {"ZFposter", "Zby", "Zproperti",
				      "Zsurveil", "Zvideo"}}});
    tests.insert({"img/poster.jpg",
		  {FAIL_IF_NO_TERMS, {"Zbicycl", "Zride", "Zroller",
				      "Zskateboard"}}});
    tests.insert({"img/scan-page.png",
		  {FAIL_IF_NO_TERMS, {"Zannual", "Zfed", "Zreturn",
				      "Zwhile"}}});
#endif
#if defined HAVE_LIBARCHIVE
    // blank file
    // pass the test if no terms are found
    tests.insert({"odf/blank.odt",
		  {PASS_IF_NO_TERMS, {}}});
    // corrupted file (ODP)
    // tests.insert({"corrupt_file.odp", {"ZSnatur"}});

    // ODF
    tests.insert({"odf/test.odt",
		  {FAIL_IF_NO_TERMS, {"Zедой", "Z喬伊不分享食物"}}});
    tests.insert({"odf/libarchive_text.odt",
		  {FAIL_IF_NO_TERMS, {"Stesttitle", "Atestauthor", "Zsampl",
				      "Zhead", "Ztext", "Zhello", "Zworld"}}});
    tests.insert({"odf/libarchive_text_template.ott",
		  {FAIL_IF_NO_TERMS, {"Zjane", "Zdoe", "Zstructur"}}});
    tests.insert({"odf/libarchive_presentation.odp",
		  {FAIL_IF_NO_TERMS, {"Zfascin", "Zfact", "Zpustak", "Zmahal",
				      "Zmillion", "Zpeopl", "Zbirthday",
				      "501"}}});
    tests.insert({"odf/libarchive_presentation_template.otp",
		  {FAIL_IF_NO_TERMS, {"ZSalizarin", "Zhead", "Zworld",
				      "Ztext"}}});
    tests.insert({"odf/libarchive_spreadsheet.ods",
		  {FAIL_IF_NO_TERMS, {"Zhello", "Zworld", "Zsampl", "2"}}});
    tests.insert({"odf/libarchive_spreadsheet_template.ots",
		  {FAIL_IF_NO_TERMS, {"Zfood", "Zpasta", "Zpercentag", "40"}}});
    tests.insert({"odf/libarchive_draw.odg",
		  {FAIL_IF_NO_TERMS, {"Zparth", "Zkapadia"}}});

    // Apache OpenOffice
    tests.insert({"sof/libarchive_openoffice_calc.sxc",
		  {FAIL_IF_NO_TERMS, {"Ztoy", "Zproduct", "Zcost", "Zquantiti",
				      "Zcardboard"}}});
    tests.insert({"sof/libarchive_openoffice_calc_template.stc",
		  {FAIL_IF_NO_TERMS, {"ZSpurchas", "ZStemplat", "Zproduct",
				      "Zquantiti", "Zsampl"}}});
    tests.insert({"sof/libarchive_openoffice_text.sxw",
		  {FAIL_IF_NO_TERMS, {"Zhello", "Zsampl", "Zopenoffic",
				      "Zwriter"}}});
    tests.insert({"sof/libarchive_openoffice_text_template.stw",
		  {FAIL_IF_NO_TERMS, {"Zhello", "Zworld", "Zsampl", "Zhead",
				      "ZStemplat", "ZStext"}}});
    tests.insert({"sof/libarchive_openoffice_presentation.sxi",
		  {FAIL_IF_NO_TERMS, {"Zhead", "Zhello", "Zopenoffic",
				      "Zimpress"}}});
    tests.insert({"sof/libarchive_openoffice_presentation_template.sti",
		  {FAIL_IF_NO_TERMS, {"ZSproject", "ZSresearch", "Zhead",
				      "Ztext"}}});

    // OOXML formats
    tests.insert({"ooxml/book.xlsx",
		  {FAIL_IF_NO_TERMS, {"Zmodi", "Zgood", "Zemploye"}}});
    tests.insert({"ooxml/doc.docx",
		  {FAIL_IF_NO_TERMS, {"Zедой", "Z喬伊不分享食物",
				      "ZSbakeri"}}});
    tests.insert({"ooxml/nature.pptx",
		  {FAIL_IF_NO_TERMS, {"ZSnatur", "Zbeauti", "Zsampl"}}});
#endif
#if defined HAVE_LIBABW
    // Title term is not being tested here because some older versions of Libabw
    // lack a bug fix for the title to be handled properly. (< libabw-0.1.2)
    tests.insert({"abw/test.abw",
		  {FAIL_IF_NO_TERMS, {"ZAparth", "Zabiword", "Zsampl",
				      "Zdocument"}}});
    tests.insert({"abw/test1.abw",
		  {FAIL_IF_NO_TERMS, {"Zедой", "Z喬伊不分享食物"}}});
#endif
#if defined HAVE_LIBCDR
    // .cdr versions >= 16 are not included in the tests as they will work
    // correctly only with libcdr >= 0.1.6
    tests.insert({"cdr/test1.cdr",
		  {FAIL_IF_NO_TERMS, {"Zalgerian", "Zcalibri"}}});
    tests.insert({"cdr/test2.cdr",
		  {FAIL_IF_NO_TERMS, {"Zедой", "Z喬伊不分享食物",
				      "Zdocument"}}});
#endif
#if defined HAVE_LIBEXTRACTOR
    // skip the test if no terms are not found
    // tests for libextractor may be skipped if proper plugins are not installed
    tests.insert({"video/file_example_OGG_480_1_7mg.ogg",
		  {SKIP_IF_NO_TERMS, {"lavf58.29.100", "Zogg"}}});
    tests.insert({"video/file_example_AVI_480_750kB.avi",
		  {SKIP_IF_NO_TERMS, {"Zcodec", "Zh264", "480x270", "msvideo",
				      "30", "fps"}}});
    tests.insert({"audio/file_example_OOG_1MG.ogg",
		  {SKIP_IF_NO_TERMS, {"Akevin", "Amacleod", "Simpact",
				      "ZSmoderato", "Zlibrari", "Zcinemat"}}});
    tests.insert({"audio/file_example_WAV_1MG.wav",
		  {SKIP_IF_NO_TERMS, {"Zstereo", "wav", "Zms"}}});
#endif
}

static test_result
compare_test(testcase& test, const Xapian::Document& doc, const string& file)
{
    // when all terms are found - PASS
    // when no terms are found - SKIP/PASS/FAIL depending on the value of
    // testcase flags.
    // when only some terms are found - FAIL
    sort(test.terms.begin(), test.terms.end());
    Xapian::TermIterator term_iterator = doc.termlist_begin();
    bool term_found = false, all_terms_exist = true;
    for (auto& t : test.terms) {
	term_iterator.skip_to(t);
	if (term_iterator == doc.termlist_end() || *term_iterator != t) {
	    cerr << "Error in " << file << ": Term " << t <<
		 " does not belong to this file" << endl;
	    all_terms_exist = false;
	} else {
	    term_found = true;
	}
    }
    if (term_found) {
	if (all_terms_exist)
	    return PASS;
	else
	    return FAIL;
    }
    // no terms found
    if (test.flags & FAIL_IF_NO_TERMS)
	return FAIL;
    if (test.flags & SKIP_IF_NO_TERMS)
	return SKIP;
    else
	return PASS;
}

int
main(int argc, char** argv)
{
    Xapian::Database db;
    test_result result = PASS;
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
	    test_result individual_result = compare_test(iter->second, doc,
							 url);
	    if (individual_result == FAIL)
		result = FAIL;
	    else if (result == PASS && individual_result == SKIP)
		result = SKIP;
	}
    }
    // exit status of 77 to denote a skipped test (standard for automake)
    if (result == PASS)
	return 0;
    else if (result == FAIL)
	return 1;
    else
	return 77;
}
