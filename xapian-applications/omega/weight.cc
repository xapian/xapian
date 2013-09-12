/** @file weight.cc
 * @brief Set the weighting scheme for Omega
 */
/* Copyright (C) 2009,2013 Olly Betts
 * Copyright (C) 2013 Aarsh Shah
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
 */

#include <config.h>

#include "weight.h"

#include "stringutils.h"

#include <cstdlib>
#include "safeerrno.h"
#include "common/noreturn.h"

#ifndef XAPIAN_AT_LEAST
#define XAPIAN_AT_LEAST(A,B,C) \
    (XAPIAN_MAJOR_VERSION > (A) || \
     (XAPIAN_MAJOR_VERSION == (A) && \
      (XAPIAN_MINOR_VERSION > (B) || \
       (XAPIAN_MINOR_VERSION == (B) && XAPIAN_REVISION >= (C)))))
#endif

using namespace std;

XAPIAN_NORETURN(static void
parameter_error(const char * param, const string & scheme));

static void
parameter_error(const char * msg, const string & scheme)
{
    string m(msg);
    m += ": '";
    m += scheme;
    m += "'";
    throw m;
}

static bool
double_param(const char ** p, double * ptr_val)
{
    char *end;
    errno = 0;
    double v = strtod(*p, &end);
    if (*p == end || errno) return false;
    *p = end;
    *ptr_val = v;
    return true;
}

void
set_weighting_scheme(Xapian::Enquire & enq, const map<string, string> & opt,
		     bool force_boolean)
{
    if (!force_boolean) {
	map<string, string>::const_iterator i = opt.find("weighting");
	if (i == opt.end()) return;

	const string & scheme = i->second;
	if (scheme.empty()) return;

	if (startswith(scheme, "bm25")) {
	    const char *p = scheme.c_str() + 4;
	    if (*p == '\0') {
		enq.set_weighting_scheme(Xapian::BM25Weight());
		return;
	    }
	    if (C_isspace((unsigned char)*p)) {
		double k1 = 1;
		double k2 = 0;
		double k3 = 1;
		double b = 0.5;
		double min_normlen = 0.5;
		if (!double_param(&p, &k1))
		    parameter_error("Parameter 1 (k1) is invalid", scheme);
		if (*p && !double_param(&p, &k2))
		    parameter_error("Parameter 2 (k2) is invalid", scheme);
		if (*p && !double_param(&p, &k3))
		    parameter_error("Parameter 3 (k3) is invalid", scheme);
		if (*p && !double_param(&p, &b))
		    parameter_error("Parameter 4 (b) is invalid", scheme);
		if (*p && !double_param(&p, &min_normlen))
		    parameter_error("Parameter 5 (min_normlen) is invalid", scheme);
		if (*p)
		    parameter_error("Extra data after parameter 5", scheme);
		Xapian::BM25Weight wt(k1, k2, k3, b, min_normlen);
		enq.set_weighting_scheme(wt);
		return;
	    }
	}

	if (startswith(scheme, "trad")) {
	    const char *p = scheme.c_str() + 4;
	    if (*p == '\0') {
		enq.set_weighting_scheme(Xapian::TradWeight());
		return;
	    }
	    if (C_isspace((unsigned char)*p)) {
		double k;
		if (!double_param(&p, &k))
		    parameter_error("Parameter is invalid", scheme);
		if (*p)
		    parameter_error("Extra data after parameter", scheme);
		enq.set_weighting_scheme(Xapian::TradWeight(k));
		return;
	    }
	}

#if XAPIAN_AT_LEAST(1,3,1)
	if (startswith(scheme, "tfidf")) {
	    const char *p = scheme.c_str() + 5;
	    if (*p == '\0') {
		enq.set_weighting_scheme(Xapian::TfIdfWeight());
		return;
	    }
	    if (C_isspace((unsigned char)*p)) {
		enq.set_weighting_scheme(Xapian::TfIdfWeight(p + 1));
		return;
	    }
	}
#endif

#if XAPIAN_AT_LEAST(1,3,2)
	if (startswith(scheme, "inl2")) {
	    const char *p = scheme.c_str() + 4;
	    if (*p == '\0') {
		enq.set_weighting_scheme(Xapian::InL2Weight());
		return;
	    }
	    if (C_isspace((unsigned char)*p)) {
		double k;
		if (!double_param(&p, &k))
		    parameter_error("Parameter is invalid", scheme);
		if (*p)
		    parameter_error("Extra data after parameter", scheme);
		enq.set_weighting_scheme(Xapian::InL2Weight(k));
		return;
	    }
	}

	if (startswith(scheme, "ifb2")) {
	    const char *p = scheme.c_str() + 4;
	    if (*p == '\0') {
		enq.set_weighting_scheme(Xapian::IfB2Weight());
		return;
	    }
	    if (C_isspace((unsigned char)*p)) {
		double k;
		if (!double_param(&p, &k))
		    parameter_error("Parameter is invalid", scheme);
		if (*p)
		    parameter_error("Extra data after parameter", scheme);
		enq.set_weighting_scheme(Xapian::IfB2Weight(k));
		return;
	    }
	}

	if (startswith(scheme, "ineb2")) {
	    const char *p = scheme.c_str() + 5;
	    if (*p == '\0') {
		enq.set_weighting_scheme(Xapian::IneB2Weight());
		return;
	    }
	    if (C_isspace((unsigned char)*p)) {
		double k;
		if (!double_param(&p, &k))
		    parameter_error("Parameter is invalid", scheme);
		if (*p)
		    parameter_error("Extra data after parameter", scheme);
		enq.set_weighting_scheme(Xapian::IneB2Weight(k));
		return;
	    }
	}

	if (startswith(scheme, "bb2")) {
	    const char *p = scheme.c_str() + 3;
	    if (*p == '\0') {
		enq.set_weighting_scheme(Xapian::BB2Weight());
		return;
	    }
	    if (C_isspace((unsigned char)*p)) {
		double k;
		if (!double_param(&p, &k))
		    parameter_error("Parameter is invalid", scheme);
		if (*p)
		    parameter_error("Extra data after parameter", scheme);
		enq.set_weighting_scheme(Xapian::BB2Weight(k));
		return;
	    }
	}

	if (startswith(scheme, "dlh")) {
	    const char *p = scheme.c_str() + 3;
	    if (*p == '\0') {
		enq.set_weighting_scheme(Xapian::DLHWeight());
		return;
	    }
	    if (C_isspace((unsigned char)*p)) {
		throw "No parameters are required for DLH";
	    }
	}

	if (startswith(scheme, "pl2")) {
	    const char *p = scheme.c_str() + 3;
	    if (*p == '\0') {
		enq.set_weighting_scheme(Xapian::PL2Weight());
		return;
	    }
	    if (C_isspace((unsigned char)*p)) {
		double k;
		if (!double_param(&p, &k))
		    parameter_error("Parameter is invalid", scheme);
		if (*p)
		    parameter_error("Extra data after parameter", scheme);
		enq.set_weighting_scheme(Xapian::PL2Weight(k));
		return;
	    }
	}

	if (startswith(scheme, "dph")) {
	    const char *p = scheme.c_str() + 3;
	    if (*p == '\0') {
		enq.set_weighting_scheme(Xapian::DPHWeight());
		return;
	    }
	    if (C_isspace((unsigned char)*p)) {
		throw "No parameters are required for DPH";
	    }
	}
#endif

	if (scheme != "bool") {
	    throw "Unknown $opt{weighting} setting: " + scheme;
	}
    }

    enq.set_weighting_scheme(Xapian::BoolWeight());
}
