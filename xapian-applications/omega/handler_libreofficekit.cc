/** @file
 * @brief Extract text and metadata using LibreOfficeKit
 */
/* Copyright (C) 2014-2023 Olly Betts
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <config.h>

#include "handler.h"

#include "htmlparser.h"
#include "loadfile.h"
#include "tmpdir.h"
#include "urlencode.h"

using namespace std;

#include <climits>
#include <cstdlib>
#include <exception>
#include <iostream>
#include <memory>

#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <sysexits.h>

#define LOK_USE_UNSTABLE_API // So we can use lok::Document::getParts().
#include <LibreOfficeKit/LibreOfficeKit.hxx>

using namespace std;
using namespace lok;

// Install location for Debian packages (also Fedora on 32-bit architectures):
#define LO_PATH_DEBIAN "/usr/lib/libreoffice/program"

// Install location for Fedora packages on 64-bit architectures:
#define LO_PATH_FEDORA64 "/usr/lib64/libreoffice/program"

// Install location on macOS.  May not actually work there currently though,
// see: https://gitlab.com/ojwb/lloconv/-/issues/11
#define LO_PATH_MACOS "/Applications/LibreOffice.app/Contents/Frameworks"

// Find a LibreOffice installation to use.
static const char*
get_lo_path()
{
    const char* lo_path = getenv("LO_PATH");
    if (lo_path) return lo_path;

    struct stat sb;
#define CHECK_DIR(P) if (stat(P"/versionrc", &sb) == 0 && S_ISREG(sb.st_mode)) return P
#ifdef __APPLE__
    CHECK_DIR(LO_PATH_MACOS);
#else
    CHECK_DIR(LO_PATH_DEBIAN);
    if constexpr(sizeof(void*) > 4) {
	CHECK_DIR(LO_PATH_FEDORA64);
    }
#endif

    // Check install locations for .deb files from libreoffice.org,
    // e.g. /opt/libreoffice6.3/program
    DIR* opt = opendir("/opt");
    if (opt) {
	// We require at least LibreOffice 4.3.
	unsigned long best_major = 4;
	unsigned long best_minor = 2;
	static string best_rc;
	struct dirent* d;
	while ((d = readdir(opt))) {
#ifdef DT_DIR
	    // Opportunistically skip non-directories if we can spot them
	    // just by looking at d_type.
	    if (d->d_type != DT_DIR && d->d_type != DT_UNKNOWN) {
		continue;
	    }
#endif
	    if (memcmp(d->d_name, "libreoffice", strlen("libreoffice")) != 0) {
		continue;
	    }

	    char* p = d->d_name + strlen("libreoffice");
	    unsigned long major = strtoul(p, &p, 10);
	    if (major == ULONG_MAX) continue;
	    unsigned long minor = 0;
	    if (*p == '.') {
		minor = strtoul(p + 1, &p, 10);
		if (minor == ULONG_MAX) continue;

		string rc = "/opt/";
		rc += d->d_name;
		rc += "/program";
		if (stat((rc + "/versionrc").c_str(), &sb) != 0 ||
		    !S_ISREG(sb.st_mode)) {
		    continue;
		}

		if (major > best_major ||
		    (major == best_major && minor > best_minor)) {
		    best_major = major;
		    best_minor = minor;
		    best_rc = std::move(rc);
		}
	    }
	}
	closedir(opt);
	if (!best_rc.empty()) {
	    return best_rc.c_str();
	}
    }

    cerr << "LibreOffice install not found\n"
	"Set LO_PATH in the environment to the 'program' directory - e.g.:\n"
	"LO_PATH=/opt/libreoffice/program\n"
	"export LO_PATH\n";
    _Exit(EX_UNAVAILABLE);
}

static string output_file;
static string output_url;

static Office* llo;

bool
initialise()
{
    output_file = get_tmpfile("tmp.html");
    if (output_file.empty()) {
	cerr << "Couldn't create temporary directory\n";
	return false;
    }
    url_encode_path(output_url, output_file);

    const char* lo_path = get_lo_path();
    llo = lok_cpp_init(lo_path);
    if (!llo) {
	cerr << "Failed to initialise LibreOfficeKit\n";
	return false;
    }
    return true;
}

void
extract(const string& filename, const string&)
try {
    const char* format = "html"; // FIXME or xhtml
    const char* options = "SkipImages";
    string input_url;
    url_encode_path(input_url, filename);
    unique_ptr<Document> lodoc(llo->documentLoad(input_url.c_str(), options));
    if (!lodoc.get()) {
	const char* errmsg = llo->getError();
	send_field(FIELD_ERROR, errmsg ? errmsg : "Failed to load document");
	return;
    }

    if (!lodoc->saveAs(output_url.c_str(), format, options)) {
	const char* errmsg = llo->getError();
	send_field(FIELD_ERROR, errmsg ? errmsg : "Failed to load export");
	return;
    }

    string html;
    if (!load_file(output_file, html)) {
	unlink(output_file.c_str());
	send_field(FIELD_ERROR, "Failed to load LibreOffice HTML output");
	return;
    }
    HtmlParser p;
    p.ignore_metarobots();
    p.parse(html, "utf-8", true);
    unlink(output_file.c_str());
    send_field(FIELD_BODY, p.dump);
    send_field(FIELD_TITLE, p.title);
    send_field(FIELD_KEYWORDS, p.keywords);
    send_field(FIELD_KEYWORDS, p.topic);
    send_field(FIELD_AUTHOR, p.author);
    send_field_created_date(p.created);
    // The documentation comment in LibreOfficeKit.hxx says this method
    // returns a count of "individual sheets in a Calc, or slides in Impress,
    // and has no relevance for Writer" but it actually seems to return a
    // page count for writer documents.
    send_field_page_count(lodoc->getParts());
} catch (const exception& e) {
    send_field(FIELD_ERROR, e.what());
}
