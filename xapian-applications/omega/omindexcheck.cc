/** @file
 * @brief Auxiliary program of omindextest
 */
/* Copyright (C) 2019 Bruno Baruffaldi
 * Copyright (C) 2020 Parth Kapadia
 * Copyright (C) 2021,2022 Olly Betts
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

#include <algorithm>
#include <iostream>
#include <string>
#include <unordered_map>

using namespace std;

enum test_result { PASS, FAIL, SKIP };
enum test_flag { SKIP_IF_NO_TERMS = 1 };

struct testcase {
    vector<string> terms;
    unsigned int flags;

    testcase(vector<string> v, unsigned int f = 0) : terms(move(v)), flags(f) {}
};


static unordered_map<string, testcase> tests;

static void
index_test()
{
    tests.insert({"test.txt",
		  {{"Zjoey", "Zfood", "Zедой", "Z喬伊不分享食物"}}});
    tests.insert({"test-csv.csv",
		  {{"ZFcsv", "Zbreak", "Zwere"}}});
    tests.insert({"test-html.html",
		  {{"Ajeroen", "ZAoom", "ZSworld", "Shello", "Zchapter"}}});
#if defined HAVE_POPPLER
    tests.insert({"pdf/poppler.pdf",
		  {{"ZFpoppler", "Zsub", "Ztext", "Ztitl", "Zpie"}}});
#endif
#if defined HAVE_LIBEBOOK
    tests.insert({"fb2/hello.fb2",
		  {{"Ajeroen", "Aooms", "Zbeauti", "Zchapter", "Zdocument",
		    "Zooms3", "Zsubsect", "Ztoday", "ZFhello", "Zoutlin"}}});
    tests.insert({"fb2/lang-name.fb2",
		  {{"Adavid", "Atardon", "ZAdavid", "ZFlang", "Zeel",
		    "Zhovercraft", "Zlanguag"}}});
    tests.insert({"fb2/lang2.fb2",
		  {{"Adavid", "Atardon", "ZAtardon", "Zúhořů", "Zhovercraft",
		    "Zlanguag", "Zmój", "Zof", "Zpełen", "Zwęgorzi",
		    "Zpoduszkowiec", "Zvznášedlo"}}});
    tests.insert({"lrf/hello.lrf",
		  {{"ZFhello", "Zhello", "Zworld"}}});
    tests.insert({"pdb/PalmDOC-hello.pdb",
		  {{"Shello", "Sworld", "ZFpalmdoc", "Zworld"}}});
    tests.insert({"pdb/PeanutPress-hello.pdb",
		  {{"Fpeanutpress", "ZFhello", "Zhello"}}});
    tests.insert({"pdb/test.pdb",
		  {{"Sdemodemo", "ZFtest", "Zwherearew"}}});
#endif
#if defined HAVE_LIBETONYEK
    tests.insert({"keynotes/test-keynote.key",
		  {{"ZFkeynot", "Zbold", "Znow", "Zsubtitl"}}});
    tests.insert({"pages/test-pages.pages",
		  {{"ZFpage", "Zfull", "Zhovercraft", "Zwęgorzi"}}});
#endif
#if defined HAVE_TESSERACT
    tests.insert({"img/Test1.gif",
		  {{"Znoisyimag", "Zocr", "Ztesseract"}}});
    tests.insert({"img/Test2.pgm",
		  {{"ZFtest2", "Znoisyimag", "Ztesseract"}}});
    tests.insert({"img/Test3.ppm",
		  {{"ZFtest3", "Zocr", "Ztest"}}});
    tests.insert({"img/Test4.tiff",
		  {{"Znoisyimag", "Ztesseract", "Zto"}}});
    tests.insert({"img/Test5.webp",
		  {{"Znoisyimag", "Ztesseract", "Ztest"}}});
    tests.insert({"img/poster-2.jpg",
		  {{"ZFposter", "Zby", "Zproperti", "Zsurveil", "Zvideo"}}});
    tests.insert({"img/poster.jpg",
		  {{"Zbicycl", "Zride", "Zroller", "Zskateboard"}}});
    tests.insert({"img/scan-page.png",
		  {{"Zannual", "Zfed", "Zreturn", "Zwhile"}}});
#endif
#if defined HAVE_LIBARCHIVE
    // blank file
    // pass the test if no terms are found
    tests.insert({"odf/blank.odt",
		  {{}}});
    // corrupted file (ODP)
    // tests.insert({"corrupt_file.odp", {"ZSnatur"}});

    // ODF
    tests.insert({"odf/test.odt",
		  {{"Zедой", "Z喬伊不分享食物"}}});
    tests.insert({"odf/libarchive_text.odt",
		  {{"Stesttitle", "Aolly", "Zsampl", "Zhead", "Ztext",
		    "Zhello", "Zworld"}}});
    tests.insert({"odf/libarchive_text_template.ott",
		  {{"Zjane", "Zdoe", "Zstructur"}}});
    tests.insert({"odf/libarchive_presentation.odp",
		  {{"Zfascin", "Zfact", "Zpustak", "Zmahal", "Zmillion",
		    "Zpeopl", "Zbirthday", "501"}}});
    tests.insert({"odf/libarchive_presentation_template.otp",
		  {{"ZSalizarin", "Zhead", "Zworld", "Ztext"}}});
    tests.insert({"odf/libarchive_spreadsheet.ods",
		  {{"Zhello", "Zworld", "Zsampl", "2"}}});
    tests.insert({"odf/libarchive_spreadsheet_template.ots",
		  {{"Zfood", "Zpasta", "Zpercentag", "40"}}});
    tests.insert({"odf/libarchive_draw.odg",
		  {{"Zparth", "Zkapadia"}}});

    // Apache OpenOffice
    tests.insert({"sof/libarchive_openoffice_calc.sxc",
		  {{"Ztoy", "Zproduct", "Zcost", "Zquantiti", "Zcardboard"}}});
    tests.insert({"sof/libarchive_openoffice_calc_template.stc",
		  {{"ZSpurchas", "ZStemplat", "Zproduct", "Zquantiti",
		    "Zsampl"}}});
    tests.insert({"sof/libarchive_openoffice_text.sxw",
		  {{"Zhello", "Zsampl", "Zopenoffic", "Zwriter"}}});
    tests.insert({"sof/libarchive_openoffice_text_template.stw",
		  {{"Zhello", "Zworld", "Zsampl", "Zhead", "ZStemplat",
		    "ZStext"}}});
    tests.insert({"sof/libarchive_openoffice_presentation.sxi",
		  {{"Zhead", "Zhello", "Zopenoffic", "Zimpress"}}});
    tests.insert({"sof/libarchive_openoffice_presentation_template.sti",
		  {{"ZSproject", "ZSresearch", "Zhead", "Ztext"}}});

    // OOXML formats
    tests.insert({"ooxml/Book.xlsx",
		  {{"Zmodi", "Zgood", "Zemploye"}}});
    tests.insert({"ooxml/2sheets.xlsx",
		  {{"0.123456", "123.456", "15", "2021", "3.14159265358979",
		    "43", "55", "Aolly", "Ssheet", "Stitle", "xmas"}}});
    tests.insert({"ooxml/Doc.docx",
		  {{"Zедой", "Z喬伊不分享食物", "ZSbakeri"}}});
    tests.insert({"ooxml/Nature.pptx",
		  {{"ZSnatur", "Zbeauti", "Zsampl"}}});
#endif
#if defined HAVE_LIBABW
    // Title term is not being tested here because some older versions of Libabw
    // lack a bug fix for the title to be handled properly. (< libabw-0.1.2)
    tests.insert({"abw/test.abw",
		  {{"ZAparth", "Zabiword", "Zsampl", "Zdocument"}}});
    tests.insert({"abw/macbeth.zabw",
		  {{"Ashakespeare", "Awilliam", "Smacbeth",
		    "ambition", "macduff", "shall"}}});
#else
    // Indexed using AbiwordParser class, which doesn't currently handle metadata.
    tests.insert({"abw/test.abw",
		  {{"Zabiword", "Zsampl", "Zdocument"}}});
    tests.insert({"abw/macbeth.zabw",
		  {{"ambition", "macduff", "shall"}}});
#endif
    tests.insert({"abw/test1.abw",
		  {{"Zедой", "Z喬伊不分享食物"}}});
    tests.insert({"abw/Friendly-Letter.awt",
		  {{"address", "addressee", "body", "dear", "sincerely"}}});
#if defined HAVE_LIBCDR
    // .cdr versions >= 16 are not included in the tests as they will work
    // correctly only with libcdr >= 0.1.6
    tests.insert({"cdr/test1.cdr",
		  {{"Zalgerian", "Zcalibri"}}});
    tests.insert({"cdr/test2.cdr",
		  {{"Zедой", "Z喬伊不分享食物", "Zdocument"}}});
#endif
#if defined HAVE_LIBEXTRACTOR
    // skip the test if no terms are not found
    // tests for libextractor may be skipped if proper plugins are not installed
    tests.insert({"video/file_example_OGG_480_1_7mg.ogg",
		  {{"lavf58.29.100", "Zogg"}, SKIP_IF_NO_TERMS}});
    tests.insert({"video/file_example_AVI_480_750kB.avi",
		  {{"Zcodec", "Zh264", "480x270", "msvideo", "30", "fps"},
		   SKIP_IF_NO_TERMS}});
    tests.insert({"audio/file_example_OOG_1MG.ogg",
		  {{"Akevin", "Amacleod", "Simpact", "ZSmoderato", "Zlibrari",
		    "Zcinemat"}, SKIP_IF_NO_TERMS}});
    tests.insert({"audio/file_example_WAV_1MG.wav",
		  {{"Zstereo", "wav", "Zms"}, SKIP_IF_NO_TERMS}});
#endif
    tests.insert({"application/vnd.ms-xpsdocument_xpstest.xps",
		 {{"second", "header", "footer"}}});
}

static test_result
compare_test(testcase& test, const Xapian::Document& doc, const string& file)
{
    // when all terms are found - PASS
    // when only some terms are found - FAIL
    // when no terms are found - SKIP/PASS/FAIL depending on the value of
    // testcase flags
    // FAIL - !terms.empty()
    // PASS - terms.empty()
    // SKIP - flag & SKIP_IF_NO_TERMS != 0
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
    if (test.flags & SKIP_IF_NO_TERMS)
	return SKIP;
    if (!test.terms.empty())
	return FAIL;
    else
	return PASS;
}

int
main(int argc, char** argv)
{
    test_result result = PASS;
    if (argc <= 1)
	return 1;

    Xapian::Database db(argv[1]);

    index_test();
    for (auto t = db.allterms_begin("U"); t != db.allterms_end("U"); ++t) {
	const string& term = *t;
	string url(term, 2);
	Xapian::PostingIterator p = db.postlist_begin(term);
	if (p == db.postlist_end(term)) {
	    // This shouldn't be possible.
	    cerr << "Term " << term << " doesn't index anything?!\n";
	    result = FAIL;
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
	    tests.erase(iter);
	}
    }

    for (auto t : tests) {
	cerr << "Testcase for URL " << t.first << " wasn't exercised\n";
	result = FAIL;
    }

    // exit status of 77 to denote a skipped test (standard for automake)
    if (result == PASS)
	return 0;
    if (result == FAIL)
	return 1;
    return 77;
}
