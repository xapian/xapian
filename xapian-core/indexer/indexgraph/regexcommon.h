/* regexcommon.h: Wrappers for regular expression functions
 *
 * ----START-LICENCE----
 * Copyright 1999,2000 BrightStation PLC
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 * USA
 * -----END-LICENCE-----
 */
#ifndef OM_HGUARD_REGEXCOMMON_H
#define OM_HGUARD_REGEXCOMMON_H

#include "config.h"

extern "C" {

/* This is a grotty workaround for an interaction between rxposix.h
 * and <map> - the fact that pair is defined confused the compiler
 * in one of the regex structs.  So we move it out of the way first...
 */
#define pair PAIR_BY_ANOTHER_NAME

#ifdef HAVE_RXPOSIX_H
#include <rxposix.h>
#define HAVE_REGEX
#else // not HAVE_RXPOSIX_H
#ifdef HAVE_RX_RXPOSIX_H
#include <rx/rxposix.h>
#define HAVE_REGEX
#endif // HAVE_RX_RXPOSIX_H
#endif // HAVE_RXPOSIX_H

/* End of grotty hack - see comment above. */
#undef pair
};

#ifdef HAVE_REGEX
#include "om/omerror.h"
#include "omassert.h"

class Regex {
    public:
	Regex()
		: compiled(false),
		  regmatches(0),
		  regmatch_size(0) {};

	void set(const std::string &expr) {
	    if (compiled) {
		regfree(&re);
		compiled = false;
	    }
	    /* FIXME: regncomp isn't POSIX - may have to use
	     * regcomp sometimes
	     */
	    int result = regncomp(&re,
				  expr.c_str(), expr.size(),
				  0);
	    if (result == 0) {
		// success
		regmatch_size = re.re_nsub + 1;
		if (regmatches) {
		    delete [] regmatches;
		}
		regmatches = new regmatch_t[regmatch_size];
		compiled = true;
	    } else {
		std::string message("Invalid regular expression `");
		message += expr;
		message += "': ";
		message += geterrstring(result);
		throw OmInvalidDataError(message);
	    }
	}

	bool matches(const std::string &s) {
	    Assert(compiled);

	    str = s;

	    int result = regnexec(&re, str.c_str(), str.length(),
				  regmatch_size,
				  &regmatches, 0);
	    bool retval = (result == 0);
	    if (result == REG_ESPACE) {
		throw std::bad_alloc();
	    }
	    return retval;
	}

	regoff_t match_start(size_t match_no) const {
	    if (match_no >= regmatch_size) {
		return -1;
	    } else {
		return regmatches[match_no].rm_so;
	    }
	}

	regoff_t match_end(size_t match_no) const {
	    if (match_no >= regmatch_size) {
		return -1;
	    } else {
		return regmatches[match_no].rm_eo;
	    }
	}

	std::string match_string(size_t match_no) const {
	    if (match_no >= regmatch_size ||
		regmatches[match_no].rm_so == -1) {
		return std::string("");
	    } else {
		return str.substr(regmatches[match_no].rm_so,
				  regmatches[match_no].rm_eo -
				  regmatches[match_no].rm_so);
	    }
	}

	~Regex() {
	    if (compiled) {
		regfree(&re);
		compiled = false;
	    }
	    if (regmatches) {
		delete [] regmatches;
	    }
	}
    private:
	bool compiled;
	regex_t re;

	regmatch_t *regmatches;
	size_t regmatch_size;

	std::string str;

	std::string geterrstring(int errcode) {
	    size_t len = regerror(errcode, &re, NULL, 0);
	    char *buf = 0;
	    std::string result;
	    try {
		buf = new char[len];
		regerror(errcode, &re, buf, len);
		result = buf;
		delete [] buf;
		buf = 0;
	    } catch (...) {
		delete [] buf;
		buf = 0;
	    }
	    return result;
	}
};
#endif // HAVE_REGEX

#endif // OM_HGUARD_REGEXCOMMON_H
