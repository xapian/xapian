/** @file
 * @brief Auxiliary program of omindextest
 */
/* Copyright (C) 2019 Bruno Baruffaldi
 * Copyright (C) 2020 Parth Kapadia
 * Copyright (C) 2021,2022,2023,2025 Olly Betts
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
#include <iomanip>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

#include "values.h"

using namespace std;

enum test_result { PASS, FAIL };

// Macro to mark optional terms.
//
// If there are optional terms in a testcase, either all or none must be
// present.  This is useful with libextractor where the required plugin
// may not be installed, but libextractor will still return generic metadata.
//
// This works by appending '\xff', which we remove before comparing.  This
// means terms ending with this byte can't be used in testcases, but it's
// invalid in UTF-8 so probably not a problematic limitation.
#define OPT(T) (T "\xff")

struct testcase {
    vector<string> terms;

    vector<pair<Xapian::valueno, string>> values;

    testcase(vector<string> v)
	: terms(std::move(v)) {}

    testcase(vector<string> v, vector<pair<Xapian::valueno, string>> v2)
	: terms(std::move(v)), values(std::move(v2)) {}
};


static unordered_map<string, testcase> tests;

static void
index_test()
{
    tests.insert({"plaintext/iso88591.txt",
		  {{"à", "d'après", "françois", "idée", "réalisation"}}});
    tests.insert({"plaintext/utf16be-bom.txt",
		  {{"Zjoey", "Zfood", "Zедой", "Z喬伊不分享食物"}}});
    tests.insert({"plaintext/utf16le-bom.txt",
		  {{"Zjoey", "Zfood", "Zедой", "Z喬伊不分享食物"}}});
    tests.insert({"plaintext/utf8.txt",
		  {{"Zjoey", "Zfood", "Zедой", "Z喬伊不分享食物"}}});
    tests.insert({"plaintext/utf8-bom.txt",
		  {{"Zjoey", "Zfood", "Zедой", "Z喬伊不分享食物"}}});
    tests.insert({"test-csv.csv",
		  {{"ZFcsv", "Zbreak", "Zwere"}}});
    tests.insert({"test-html.html",
		  {{"Ajeroen", "ZAoom", "ZSworld", "Shello", "Zchapter"}}});
    tests.insert({"svg/diagram.svg",
		  {{"Sdiagram", "Timage/svg+xml", "Zstart"}}});
    tests.insert({"svg/diagram.svgz",
		  {{"Sdiagram", "Timage/svg+xml-compressed", "Zstart"}}});
#ifdef HAVE_GMIME
    tests.insert({"email/html.eml",
		  {{"Aexample", "Ame", "Aorg", "Auser", "Shtml",
		    "Tmessage/rfc822",
		    "XMID:E1p1II7-008OVw-1w@example.org",
		    "XTOada", "XTOexample", "XTOorg", "XTOuser",
		    "html", "message", "test"},
		  {{{VALUE_CREATED, "c\x8a\xb4\xb3"}, // 1670034611
		    {VALUE_SIZE, Xapian::sortable_serialise(450)},
		    {VALUE_MD5, "y.<0RW\xb0\xf4\xd2+\xa8\x09\xde\xff|\x0d"}
		   }}}});
    tests.insert({"email/text.eml",
		  {{"Aexample", "Ame", "Aorg", "Auser", "Stext", "Tmessage/rfc822",
		    "XMID:E1p1II7-008OVw-1v@example.org",
		    "XTOexample", "XTOorg", "XTOuser",
		    "comment1", "comment2", "keyword1", "keyword2",
		    "message", "plain", "text"},
		  {{{VALUE_CREATED, "c\x8a\xb4\xb3"}, // 1670034611
		    {VALUE_SIZE, Xapian::sortable_serialise(477)},
		    {VALUE_MD5,
		     "C\x7f\x17;;\x87\x91\x5c\x05?\x83\x14\xec\xaa\xad\x94"}
		   }}}});
#endif
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
    tests.insert({"iwork/test-keynote.key",
		  {{"ZFkeynot", "Zbold", "Znow", "Zsubtitl"}}});
    tests.insert({"iwork/test-pages.pages",
		  {{"ZFpage", "Zfull", "Zhovercraft", "Zwęgorzi"}}});
#endif
#if defined HAVE_TESSERACT
    tests.insert({"image/Test1.gif",
		  {{"Znoisyimag", "Zocr", "Ztesseract"}}});
    tests.insert({"image/Test2.pgm",
		  {{"ZFtest2", "Znoisyimag", "Ztesseract"}}});
    tests.insert({"image/Test3.ppm",
		  {{"ZFtest3", "Zocr", "Ztest"}}});
    tests.insert({"image/Test4.tiff",
		  {{"Znoisyimag", "Ztesseract", "Zto"}}});
    tests.insert({"image/Test5.webp",
		  {{"Znoisyimag", "Ztesseract", "Ztest"}}});
    tests.insert({"image/poster-2.jpg",
		  {{"ZFposter", "Zby", "Zproperti", "Zsurveil", "Zvideo"}}});
    tests.insert({"image/poster.jpg",
		  {{"Zbicycl", "Zride", "Zroller", "Zskateboard"}}});
    tests.insert({"image/scan-page.png",
		  {{"Zannual", "Zfed", "Zreturn", "Zwhile"}}});
#endif
#define OFFICE_TESTCASES(PREFIX) \
    /* blank file */ \
    /* pass the test if no terms are found */ \
    tests.insert({PREFIX "opendoc/blank.odt", \
		  {{}}}); \
    /* corrupted file (ODP) */ \
    /* tests.insert({PREFIX "corrupt_file.odp", {"ZSnatur"}}); */ \
    \
    /* ODF */ \
    tests.insert({PREFIX "opendoc/test.odt", \
		  {{"Zедой", "Z喬伊不分享食物"}}}); \
    tests.insert({PREFIX "opendoc/text.odt", \
		  {{"Stesttitle", NOTLO("Aolly",) "Zsampl", "Zhead", "Ztext", \
		    "Zhello", "Zworld"}}}); \
    tests.insert({PREFIX "opendoc/text_template.ott", \
		  {{"Zjane", "Zdoe", "Zstructur"}}}); \
    NOTLO(\
    tests.insert({PREFIX "opendoc/presentation.odp", \
		  {{"Zfascin", "Zfact", "Zpustak", "Zmahal", "Zmillion", \
		    "Zpeopl", "Zbirthday", "501"}}}); \
    )\
    tests.insert({PREFIX "opendoc/presentation_template.otp", \
		  {{"ZSalizarin", "Zhead", "Zworld", "Ztext"}}}); \
    tests.insert({PREFIX "opendoc/spreadsheet.ods", \
		  {{"Zhello", "Zworld", "Zsampl", "2"}}}); \
    tests.insert({PREFIX "opendoc/spreadsheet_template.ots", \
		  {{"Zfood", "Zpasta", "Zpercentag", "40"}}}); \
    tests.insert({PREFIX "opendoc/draw.odg", \
		  {{"Zparth", "Zkapadia"}}}); \
    \
    /* Apache OpenOffice */ \
    tests.insert({PREFIX "staroffice/calc.sxc", \
		  {{"Ztoy", "Zproduct", "Zcost", "Zquantiti", "Zcardboard"}}}); \
    tests.insert({PREFIX "staroffice/calc_template.stc", \
		  {{NOTLO("ZSpurchas", "ZStemplat",) "Zproduct", "Zquantiti", \
		    "Zsampl"}}}); \
    tests.insert({PREFIX "staroffice/text.sxw", \
		  {{"Zhello", "Zsampl", "Zopenoffic", "Zwriter"}}}); \
    tests.insert({PREFIX "staroffice/text_template.stw", \
		  {{"Zhello", "Zworld", "Zsampl", "Zhead", \
		    NOTLO("ZStemplat", "ZStext")}}}); \
    tests.insert({PREFIX "staroffice/presentation.sxi", \
		  {{"Zhead", "Zhello", "Zopenoffic", "Zimpress"}}}); \
    tests.insert({PREFIX "staroffice/presentation_template.sti", \
		  {{NOTLO("ZSproject", "ZSresearch",) "Zhead", "Ztext"}}}); \
    \
    /* Microsoft XML formats */ \
    tests.insert({PREFIX "msxml/Book.xlsx", \
		  {{"Zmodi", "Zgood", "Zemploye"}}}); \
    tests.insert({PREFIX "msxml/2sheets.xlsx", \
		  {{NOTLO("0.123456",) LO("0.12346",) \
		    "123.456", "15", "2021", \
		    NOTLO("3.14159265358979",) LO("3.14159",) \
		    "43", "55", "Aolly", "Ssheet", "Stitle", "xmas"}}}); \
    tests.insert({PREFIX "msxml/Doc.docx", \
		  {{"Zедой", "Z喬伊不分享食物", "ZSbakeri"}}}); \
    tests.insert({PREFIX "msxml/Nature.pptx", \
		  {{"ZSnatur", "Zbeauti", "Zsampl"}}}); \
    tests.insert({PREFIX "msxml/vnd.ms-xpsdocument_xpstest.xps", \
		 {{"second", "header", "footer"}}});
#if defined HAVE_LIBARCHIVE
# define LO(...)
# define NOTLO(...) __VA_ARGS__
    OFFICE_TESTCASES("")
# undef NOTLO
# undef LO
#endif
#if defined HAVE_LIBREOFFICEKIT_LIBREOFFICEKIT_HXX
# define LO(...) __VA_ARGS__
# define NOTLO(...)
    OFFICE_TESTCASES("lok-")
# undef NOTLO
# undef LO
#endif
#if defined HAVE_LIBABW
    tests.insert({"abw/test.abw",
		  {{"Sabiword", "Stitle", "ZAparth",
		    "Zabiword", "Zsampl", "Zdocument"}}});
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
    // Testcase for libextractor need to allow for the required plugin not
    // being installed as libextractor still returns generic metadata.
    tests.insert({"video/file_example_OGG_480_1_7mg.ogg",
		  {{"Eogg", "Tvideo/ogg",
		    OPT("Searth"), OPT("Splanet")}}});
    tests.insert({"video/file_example_AVI_480_750kB.avi",
		  {{"Eavi", "Tvideo/x-msvideo",
		    OPT("Zcodec"), OPT("Zh264"),
		    OPT("480x270"), OPT("msvideo"), OPT("30"), OPT("fps")}}});
    tests.insert({"audio/file_example_OOG_1MG.ogg",
		  {{"Eogg", "Taudio/ogg",
		    OPT("Akevin"), OPT("Amacleod"),
		    OPT("Simpact"), OPT("ZSmoderato")}}});
    tests.insert({"audio/file_example_WAV_1MG.wav",
		  {{"Ewav", "Taudio/x-wav",
		    OPT("Zstereo"), OPT("wav"), OPT("Zms")}}});
#endif
#if defined HAVE_LIBGEPUB
    tests.insert({"epub/epub2test.epub",
		  {{"Eepub", "Tapplication/epub+zip", "Aolly",
		    "book", "chapter", "welcome"}}});
    tests.insert({"epub/epub3test.epub",
		  {{"Eepub", "Tapplication/epub+zip", "Aolly",
		    "book", "chapter", "welcome"}}});
#endif
#if defined HAVE_LIBMWAW
    tests.insert({"apple_works/test_word.cwk",
		  {{"Aparth", "Sword", "Zhello", "Zdocument"}}});
    tests.insert({"apple_works/test_spreadsheet.cwk",
		  {{"Aparth", "Sspreadsheet", "Zpizza", "220"}}});
    tests.insert({"apple_works/test_draw.cwk",
		  {{"Zdraw", "Zsampl", "Zgraphic"}}});
#endif
}

static void
escape(const string& s, std::ostream& stream)
{
    for (unsigned char ch : s) {
	if (ch >= 0x20 && ch < 127 && ch != '\\') {
	    stream << ch;
	} else {
	    stream << "\\x"
		   << std::hex << std::setfill('0') << std::setw(2) << int(ch);
	}
    }
}

static test_result
compare_test(testcase& test, const Xapian::Document& doc, const string& file)
{
    sort(test.terms.begin(), test.terms.end());
    Xapian::TermIterator term_iterator = doc.termlist_begin();
    bool all_required_terms_exist = true;
    string missing_optional;
    bool no_optional = true;
    for (auto& i : test.terms) {
	if (i.back() == '\xff') {
	    string t(i, 0, i.size() - 1);
	    term_iterator.skip_to(t);
	    // Optional term.
	    if (term_iterator == doc.termlist_end() || *term_iterator != t) {
		missing_optional += ' ';
		missing_optional += t;
	    } else {
		no_optional = false;
	    }
	} else {
	    auto t = i;
	    term_iterator.skip_to(t);
	    if (term_iterator == doc.termlist_end() || *term_iterator != t) {
		cerr << file << ": error: Term " << t
		     << " should index this file but doesn't\n";
		all_required_terms_exist = false;
	    }
	}
    }

    bool values_ok = true;
    for (auto& i : test.values) {
	const string& v = doc.get_value(i.first);
	if (v != i.second) {
	    cerr << file << ": error: Value slot " << i.first << " should be ";
	    escape(i.second, cerr);
	    cerr << " not ";
	    escape(v, cerr);
	    cerr << '\n';
	    values_ok = false;
	}
    }
    if (!values_ok) {
	return FAIL;
    }
    if (!missing_optional.empty() && !no_optional) {
	cerr << file << ": error: Only some of the optional terms index this "
			"file, missing:" << missing_optional << '\n';
    } else if (all_required_terms_exist) {
	// All terms found (including degenerate case where no terms are listed
	// to check for).
	return PASS;
    }
    cerr << "Expected at least these terms:";
    for (auto& t : test.terms) {
	if (t.back() == '\xff') {
	    // Optional term.
	    cerr << " OPT(" << t.substr(0, t.size() - 1) << ')';
	} else {
	    cerr << ' ' << t;
	}
    }
    cerr << "\nFull list of terms actually present:";
    for (term_iterator = doc.termlist_begin();
	 term_iterator != doc.termlist_end();
	 ++term_iterator) {
	cerr << ' ' << *term_iterator;
    }
    cerr << '\n';
    return FAIL;
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
	    tests.erase(iter);
	}
	if (++p != db.postlist_end(term)) {
	    cerr << "URL term " << term << " indexes more than one document\n";
	    result = FAIL;
	}
    }

    for (auto t : tests) {
	cerr << "Testcase for URL " << t.first << " wasn't exercised\n";
	result = FAIL;
    }

    return result == FAIL ? 1 : 0;
}
