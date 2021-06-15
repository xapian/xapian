/** @file
 * @brief index arbitrary data as described by an index script
 */
/* Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2001 Sam Liddicott
 * Copyright 2001,2002 Ananova Ltd
 * Copyright 2002,2003,2004,2005,2006,2007,2008,2009,2010,2011,2014,2015,2017,2018,2019 Olly Betts
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
#include <fstream>
#include <iostream>
#include <list>
#include <map>
#include <memory>
#include <string>
#include <unordered_set>
#include <vector>
#include <cstring>

#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <ctime>

#include "commonhelp.h"
#include "datetime.h"
#include "genericxmlparser.h"
#include "hashterm.h"
#include "htmlparser.h"
#include "loadfile.h"
#include "parseint.h"
#include "setenv.h"
#include "str.h"
#include "stringutils.h"
#include "timegm.h"
#include "utf8truncate.h"
#include "values.h"

#ifndef HAVE_STRPTIME
#include "portability/strptime.h"
#endif

#include "gnu_getopt.h"

using namespace std;

#define PROG_NAME "scriptindex"
#define PROG_DESC "index arbitrary data as described by an index script"

static bool verbose;
static int addcount;
static int repcount;
static int delcount;

static inline bool
prefix_needs_colon(const string & prefix, unsigned ch)
{
    if (!C_isupper(ch) && ch != ':') return false;
    string::size_type len = prefix.length();
    return (len > 1 && prefix[len - 1] != ':');
}

const char * action_names[] = {
    // Actions used internally:
    "bad",
    "new",
    // Actual actions:
    "boolean",
    "date",
    "field",
    "gap",
    "hash",
    "hextobin",
    "index",
    "indexnopos",
    "load",
    "lower",
    "ltrim",
    "parsedate",
    "rtrim",
    "spell",
    "split",
    "squash",
    "trim",
    "truncate",
    "unhtml",
    "unique",
    "unxml",
    "value",
    "valuenumeric",
    "valuepacked",
    "weight"
};

// For debugging:
#define DUMP_ACTION(A) cout << action_names[(A).get_action()] << "(" << (A).get_string_arg() << "," << (A).get_num_arg() << ")" << endl

class Action {
  public:
    typedef enum {
	// Actions used internally:
	BAD,
	NEW,
	// Actual actions:
	BOOLEAN,
	DATE,
	FIELD,
	GAP,
	HASH,
	HEXTOBIN,
	INDEX,
	INDEXNOPOS,
	LOAD,
	LOWER,
	LTRIM,
	PARSEDATE,
	RTRIM,
	SPELL,
	SPLIT,
	SQUASH,
	TRIM,
	TRUNCATE,
	UNHTML,
	UNIQUE,
	UNXML,
	VALUE,
	VALUENUMERIC,
	VALUEPACKED,
	WEIGHT
    } type;
    enum { SPLIT_NONE, SPLIT_DEDUP, SPLIT_SORT, SPLIT_PREFIXES };
  private:
    type action;
    int num_arg = 0;
    string string_arg;
    // Offset into indexscript line.
    size_t pos;
  public:
    Action(type action_, size_t pos_)
	: action(action_), pos(pos_) { }
    Action(type action_, size_t pos_, const string & arg)
	: action(action_), string_arg(arg), pos(pos_) {
	num_arg = atoi(string_arg.c_str());
    }
    Action(type action_, size_t pos_, const string & arg, int num)
	: action(action_), num_arg(num), string_arg(arg), pos(pos_) { }
    type get_action() const { return action; }
    int get_num_arg() const { return num_arg; }
    void set_num_arg(int num) { num_arg = num; }
    const string & get_string_arg() const { return string_arg; }
    size_t get_pos() const { return pos; }
};

// These allow searching for an Action with a particular Action::type using
// std::find().

inline bool
operator==(const Action& a, Action::type t) { return a.get_action() == t; }

inline bool
operator==(Action::type t, const Action& a) { return a.get_action() == t; }

inline bool
operator!=(const Action& a, Action::type t) { return !(a == t); }

inline bool
operator!=(Action::type t, const Action& a) { return !(t == a); }

static void
ltrim(string& s, const string& chars)
{
    auto i = s.find_first_not_of(chars);
    if (i) s.erase(0, i);
}

static void
rtrim(string& s, const string& chars)
{
    s.resize(s.find_last_not_of(chars) + 1);
}

static void
squash(string& s, const string& chars)
{
    string output;
    output.reserve(s.size());
    string::size_type i = 0;
    while ((i = s.find_first_not_of(chars, i)) != string::npos) {
	auto j = s.find_first_of(chars, i);
	if (!output.empty()) output += ' ';
	output.append(s, i, j - i);
	i = j;
    }
    s = std::move(output);
}

enum diag_type { DIAG_ERROR, DIAG_WARN, DIAG_NOTE };

static void
report_location(enum diag_type type,
		const string& filename,
		size_t line = 0,
		size_t pos = string::npos)
{
    cerr << filename;
    if (line != 0) {
	cerr << ':' << line;
    }
    if (pos != string::npos) {
	// The first column is numbered 1.
	cerr << ':' << pos + 1;
    }
    switch (type) {
	case DIAG_ERROR:
	    cerr << ": error: ";
	    break;
	case DIAG_WARN:
	    cerr << ": warning: ";
	    break;
	case DIAG_NOTE:
	    cerr << ": note: ";
	    break;
    }
}

static void
report_useless_action(const string &file, size_t line, size_t pos,
		      const string &action)
{
    report_location(DIAG_WARN, file, line, pos);
    cerr << "Index action '" << action << "' has no effect" << endl;

    static bool given_left_to_right_warning = false;
    if (!given_left_to_right_warning) {
	given_left_to_right_warning = true;
	report_location(DIAG_NOTE, file, line, pos);
	cerr << "Actions are executed from left to right" << endl;
    }
}

static map<string, vector<Action>> index_spec;

static void
parse_index_script(const string &filename)
{
    ifstream script(filename.c_str());
    if (!script.is_open()) {
	report_location(DIAG_ERROR, filename);
	cerr << strerror(errno) << endl;
	exit(1);
    }
    string line;
    size_t line_no = 0;
    // Line number where we saw a `unique` action, or -1 if we haven't.
    int unique_line_no = -1;
    while (getline(script, line)) {
	++line_no;
	vector<string> fields;
	vector<Action> actions;
	string::const_iterator i, j;
	const string &s = line;
	i = find_if(s.begin(), s.end(), [](char ch) { return !C_isspace(ch); });
	if (i == s.end() || *i == '#') {
	    // Blank line or comment.
	    continue;
	}
	while (true) {
	    if (!C_isalnum(*i)) {
		report_location(DIAG_ERROR, filename, line_no, i - s.begin());
		cerr << "field name must start with alphanumeric" << endl;
		exit(1);
	    }
	    j = find_if(i, s.end(),
			[](char ch) { return !C_isalnum(ch) && ch != '_'; });
	    fields.push_back(string(i, j));
	    i = find_if(j, s.end(), [](char ch) { return !C_isspace(ch); });
	    if (i == s.end()) break;
	    if (*i == ':') {
		++i;
		i = find_if(i, s.end(), [](char ch) { return !C_isspace(ch); });
		break;
	    }
	    if (i == j) {
		report_location(DIAG_ERROR, filename, line_no, i - s.begin());
		cerr << "bad character '" << *i << "' in fieldname" << endl;
		exit(1);
	    }
	}
	Xapian::termcount weight = 1;
	size_t useless_weight_pos = string::npos;
	map<string, Action::type> boolmap;
	j = i;
	while (j != s.end()) {
	    size_t action_pos = j - s.begin();
	    i = find_if(j, s.end(), [](char ch) { return !C_isalnum(ch); });
	    string action(s, j - s.begin(), i - j);
	    Action::type code = Action::BAD;
	    unsigned min_args = 0, max_args = 0;
	    bool takes_integer_argument = false;
	    if (!action.empty()) {
		switch (action[0]) {
		    case 'b':
			if (action == "boolean") {
			    code = Action::BOOLEAN;
			    max_args = 1;
			}
			break;
		    case 'd':
			if (action == "date") {
			    code = Action::DATE;
			    min_args = max_args = 1;
			}
			break;
		    case 'f':
			if (action == "field") {
			    code = Action::FIELD;
			    max_args = 1;
			}
			break;
		    case 'g':
			if (action == "gap") {
			    code = Action::GAP;
			    max_args = 1;
			    takes_integer_argument = true;
			}
			break;
		    case 'h':
			if (action == "hash") {
			    code = Action::HASH;
			    max_args = 1;
			    takes_integer_argument = true;
			} else if (action == "hextobin") {
			    code = Action::HEXTOBIN;
			}
			break;
		    case 'i':
			if (action == "index") {
			    code = Action::INDEX;
			    max_args = 1;
			} else if (action == "indexnopos") {
			    code = Action::INDEXNOPOS;
			    max_args = 1;
			}
			break;
		    case 'l':
			if (action == "lower") {
			    code = Action::LOWER;
			} else if (action == "load") {
			    code = Action::LOAD;
			} else if (action == "ltrim") {
			    code = Action::LTRIM;
			    max_args = 1;
			}
			break;
		    case 'p':
			if (action == "parsedate") {
			    code = Action::PARSEDATE;
			    min_args = max_args = 1;
			}
			break;
		    case 'r':
			if (action == "rtrim") {
			    code = Action::RTRIM;
			    max_args = 1;
			}
			break;
		    case 's':
			if (action == "spell") {
			    code = Action::SPELL;
			} else if (action == "split") {
			    code = Action::SPLIT;
			    min_args = 1;
			    max_args = 2;
			} else if (action == "squash") {
			    code = Action::SQUASH;
			    max_args = 1;
			}
			break;
		    case 't':
			if (action == "truncate") {
			    code = Action::TRUNCATE;
			    min_args = max_args = 1;
			    takes_integer_argument = true;
			} else if (action == "trim") {
			    code = Action::TRIM;
			    max_args = 1;
			}
			break;
		    case 'u':
			if (action == "unhtml") {
			    code = Action::UNHTML;
			} else if (action == "unique") {
			    code = Action::UNIQUE;
			    min_args = max_args = 1;
			} else if (action == "unxml") {
			    code = Action::UNXML;
			}
			break;
		    case 'v':
			if (action == "value") {
			    code = Action::VALUE;
			    min_args = max_args = 1;
			    takes_integer_argument = true;
			} else if (action == "valuenumeric") {
			    code = Action::VALUENUMERIC;
			    min_args = max_args = 1;
			    takes_integer_argument = true;
			} else if (action == "valuepacked") {
			    code = Action::VALUEPACKED;
			    min_args = max_args = 1;
			    takes_integer_argument = true;
			}
			break;
		    case 'w':
			if (action == "weight") {
			    code = Action::WEIGHT;
			    min_args = max_args = 1;
			    takes_integer_argument = true;
			}
			break;
		}
	    }
	    if (code == Action::BAD) {
		report_location(DIAG_ERROR, filename, line_no, action_pos);
		cerr << "Unknown index action '" << action << "'" << endl;
		exit(1);
	    }
	    auto i_after_action = i;
	    i = find_if(i, s.end(), [](char ch) { return !C_isspace(ch); });

	    if (i != s.end() && *i == '=') {
		if (i != i_after_action) {
		    report_location(DIAG_WARN, filename, line_no,
				    i_after_action - s.begin());
		    cerr << "putting spaces between the action and '=' is "
			    "deprecated." << endl;
		}

		if (max_args == 0) {
		    report_location(DIAG_ERROR, filename, line_no,
				    i - s.begin());
		    cerr << "Index action '" << action
			 << "' doesn't take an argument" << endl;
		    exit(1);
		}

		++i;
		j = find_if(i, s.end(), [](char ch) { return !C_isspace(ch); });
		if (i != j) {
		    report_location(DIAG_WARN, filename, line_no,
				    i - s.begin());
		    cerr << "putting spaces between '=' and the argument is "
			    "deprecated." << endl;
		}

		vector<string> vals;
		while (true) {
		    if (j != s.end() && *j == '"') {
			// Quoted argument.
			++j;
			string arg;
			while (true) {
			    i = find_if(j, s.end(),
					[](char ch) {
					    return ch == '"' || ch == '\\';
					});
			    if (i == s.end()) {
				report_location(DIAG_ERROR, filename, line_no,
						s.size());
				cerr << "No closing quote" << endl;
				exit(1);
			    }
			    arg.append(j, i);
			    if (*i++ == '"')
				break;

			    // Escape sequence.
			    if (i == s.end()) {
bad_escaping:
				report_location(DIAG_ERROR, filename, line_no,
						i - s.begin());
				cerr << "Bad escaping in quoted action argument"
				     << endl;
				exit(1);
			    }

			    char ch = *i;
			    switch (ch) {
				case '\\':
				case '"':
				    break;
				case '0':
				    ch = '\0';
				    break;
				case 'n':
				    ch = '\n';
				    break;
				case 'r':
				    ch = '\r';
				    break;
				case 't':
				    ch = '\t';
				    break;
				case 'x': {
				    if (++i == s.end())
					goto bad_escaping;
				    char ch1 = *i;
				    if (++i == s.end())
					goto bad_escaping;
				    char ch2 = *i;
				    if (!C_isxdigit(ch1) ||
					!C_isxdigit(ch2))
					goto bad_escaping;
				    ch = hex_digit(ch1) << 4 |
					 hex_digit(ch2);
				    break;
				}
				default:
				    goto bad_escaping;
			    }
			    arg += ch;
			    j = i + 1;
			}
			vals.emplace_back(std::move(arg));
			if (i == s.end() || C_isspace(*i)) break;
			if (*i != ',') {
			    report_location(DIAG_ERROR, filename, line_no,
					    i - s.begin());
			    cerr << "Unexpected character '" << *i
				 << "' after closing quote" << endl;
			    exit(1);
			}
			++i;
		    } else if (max_args > 1) {
			// Unquoted argument, split on comma.
			i = find_if(j, s.end(),
				    [](char ch) {
					return C_isspace(ch) || ch == ',';
				    });
			vals.emplace_back(j, i);
			if (*i != ',') break;
			++i;
		    } else {
			// Unquoted argument, including any commas.
			i = find_if(j, s.end(),
				    [](char ch) { return C_isspace(ch); });
			vals.emplace_back(j, i);
			break;
		    }
		    j = i;

		    if (vals.size() == max_args) {
			report_location(DIAG_ERROR, filename, line_no,
					i - s.begin());
			cerr << "Index action '" << action
			     << "' takes at most " << max_args << " arguments"
			     << endl;
			exit(1);
		    }
		}

		if (vals.size() < min_args) {
		    report_location(DIAG_ERROR, filename, line_no,
				    i - s.begin());
		    if (min_args == max_args) {
			cerr << "Index action '" << action
			     << "' requires " << min_args << " arguments"
			     << endl;
			exit(1);
		    }
		    cerr << "Index action '" << action
			 << "' requires at least " << min_args << " arguments"
			 << endl;
		    exit(1);
		}

		string val;
		if (!vals.empty()) {
		    val = vals.front();
		}

		if (takes_integer_argument) {
		    auto dot = val.find('.');
		    if (dot != string::npos) {
			report_location(DIAG_WARN, filename, line_no,
					j - s.begin() + dot);
			cerr << "Index action '" << action
			     << "' takes an integer argument" << endl;
		    }
		}
		switch (code) {
		    case Action::DATE:
			if (val != "unix" &&
			    val != "unixutc" &&
			    val != "yyyymmdd") {
			    report_location(DIAG_ERROR, filename, line_no);
			    cerr << "Invalid parameter '" << val << "' for "
				    "action 'date'" << endl;
			    exit(1);
			}
			actions.emplace_back(code, action_pos, val);
			break;
		    case Action::INDEX:
		    case Action::INDEXNOPOS:
			actions.emplace_back(code, action_pos, val, weight);
			useless_weight_pos = string::npos;
			break;
		    case Action::WEIGHT:
			// We don't push an Action for WEIGHT - instead we
			// store it ready to use in the INDEX and INDEXNOPOS
			// Actions.
			if (!parse_unsigned(val.c_str(), weight)) {
			    report_location(DIAG_WARN, filename, line_no);
			    cerr << "Index action 'weight' takes a "
				    "non-negative integer argument" << endl;
			}
			if (useless_weight_pos != string::npos) {
			    report_useless_action(filename, line_no,
						  useless_weight_pos, action);
			}
			useless_weight_pos = action_pos;
			break;
		    case Action::PARSEDATE: {
			if (val.find("%Z") != val.npos) {
			    report_location(DIAG_ERROR, filename, line_no);
			    cerr << "Parsing timezone names with %Z is not supported" << endl;
			    exit(1);
			}
#ifndef HAVE_STRUCT_TM_TM_GMTOFF
			if (val.find("%z") != val.npos) {
			    report_location(DIAG_ERROR, filename, line_no);
			    cerr << "Parsing timezone offsets with %z is not supported on "
				    "this platform" << endl;
			    exit(1);
			}
#endif
			actions.emplace_back(code, action_pos, val);
			break;
		    }
		    case Action::SPLIT: {
			if (val.empty()) {
			    report_location(DIAG_ERROR, filename, line_no);
			    cerr << "Split delimiter can't be empty" << endl;
			    exit(1);
			}
			int operation = Action::SPLIT_NONE;
			if (vals.size() >= 2) {
			    if (vals[1] == "dedup") {
				operation = Action::SPLIT_DEDUP;
			    } else if (vals[1] == "sort") {
				operation = Action::SPLIT_SORT;
			    } else if (vals[1] == "none") {
				operation = Action::SPLIT_NONE;
			    } else if (vals[1] == "prefixes") {
				operation = Action::SPLIT_PREFIXES;
			    } else {
				report_location(DIAG_ERROR, filename, line_no);
				cerr << "Bad split operation '" << vals[1]
				     << "'" << endl;
				exit(1);
			    }
			}
			actions.emplace_back(code, action_pos, val, operation);
			break;
		    }
		    case Action::TRUNCATE:
			if (!actions.empty() &&
			    actions.back().get_action() == Action::LOAD) {
			    /* Turn "load truncate=n" into "load" with
			     * num_arg n, so that we don't needlessly
			     * allocate memory and read data we're just
			     * going to ignore.
			     */
			    actions.pop_back();
			    code = Action::LOAD;
			}
			actions.emplace_back(code, action_pos, val);
			break;
		    case Action::UNIQUE:
			if (unique_line_no >= 0) {
			    report_location(DIAG_ERROR, filename, line_no,
					    action_pos);
			    cerr << "Index action 'unique' used more than once"
				 << endl;
			    report_location(DIAG_NOTE, filename,
					    unique_line_no);
			    cerr << "Previously used here" << endl;
			    exit(1);
			}
			unique_line_no = line_no;
			if (boolmap.find(val) == boolmap.end())
			    boolmap[val] = Action::UNIQUE;
			actions.emplace_back(code, action_pos, val);
			break;
		    case Action::GAP: {
			actions.emplace_back(code, action_pos, val);
			auto& obj = actions.back();
			auto gap_size = obj.get_num_arg();
			if (gap_size <= 0) {
			    report_location(DIAG_ERROR, filename, line_no,
					    obj.get_pos() + 3 + 1);
			    cerr << "Index action 'gap' takes a strictly "
				    "positive integer argument" << endl;
			    exit(1);
			}
			break;
		    }
		    case Action::HASH: {
			actions.emplace_back(code, action_pos, val);
			auto& obj = actions.back();
			auto max_length = obj.get_num_arg();
			if (max_length < 6) {
			    report_location(DIAG_ERROR, filename, line_no,
					    obj.get_pos() + 4 + 1);
			    cerr << "Index action 'hash' takes an integer "
				    "argument which must be at least 6" << endl;
			    exit(1);
			}
			break;
		    }
		    case Action::LTRIM:
		    case Action::RTRIM:
		    case Action::SQUASH:
		    case Action::TRIM:
			for (unsigned char ch : val) {
			    if (ch >= 0x80) {
				auto column = actions.back().get_pos() +
					      strlen(action_names[code]) + 1;
				report_location(DIAG_ERROR, filename, line_no,
						column);
				cerr << "Index action '" << action_names[code]
				     << "' only support ASCII characters "
					"currently\n";
				exit(1);
			    }
			}
			actions.emplace_back(code, action_pos, val);
			break;
		    case Action::BOOLEAN:
			boolmap[val] = Action::BOOLEAN;
			/* FALLTHRU */
		    default:
			actions.emplace_back(code, action_pos, val);
		}
		i = find_if(i, s.end(), [](char ch) { return !C_isspace(ch); });
	    } else {
		if (min_args > 0) {
		    report_location(DIAG_ERROR, filename, line_no,
				    i_after_action - s.begin());
		    if (min_args == max_args) {
			cerr << "Index action '" << action << "' requires "
			     << min_args << " arguments" << endl;
			exit(1);
		    }
		    cerr << "Index action '" << action << "' requires at least "
			 << min_args << " arguments" << endl;
		    exit(1);
		}
		switch (code) {
		    case Action::INDEX:
		    case Action::INDEXNOPOS:
			useless_weight_pos = string::npos;
			actions.emplace_back(code, action_pos, "", weight);
			break;
		    case Action::GAP:
			actions.emplace_back(code, action_pos, "", 100);
			break;
		    case Action::HASH:
			actions.emplace_back(code, action_pos, "",
					     MAX_SAFE_TERM_LENGTH - 1);
			break;
		    case Action::LTRIM:
		    case Action::RTRIM:
		    case Action::SQUASH:
		    case Action::TRIM:
			actions.emplace_back(code, action_pos, " \t\f\v\r\n");
			break;
		    default:
			actions.emplace_back(code, action_pos);
			break;
		}
	    }
	    j = i;
	}

	if (useless_weight_pos != string::npos) {
	    report_useless_action(filename, line_no, useless_weight_pos,
				  "weight");
	}

	while (!actions.empty()) {
	    bool done = true;
	    Action::type action = actions.back().get_action();
	    switch (action) {
		case Action::HASH:
		case Action::HEXTOBIN:
		case Action::LOWER:
		case Action::LTRIM:
		case Action::PARSEDATE:
		case Action::RTRIM:
		case Action::SPELL:
		case Action::SQUASH:
		case Action::TRIM:
		case Action::TRUNCATE:
		case Action::UNHTML:
		case Action::UNXML:
		    done = false;
		    report_useless_action(filename, line_no,
					  actions.back().get_pos(),
					  action_names[action]);
		    actions.pop_back();
		    break;
		default:
		    break;
	    }
	    if (done) break;
	}

	map<string, Action::type>::const_iterator boolpfx;
	for (boolpfx = boolmap.begin(); boolpfx != boolmap.end(); ++boolpfx) {
	    if (boolpfx->second == Action::UNIQUE) {
		report_location(DIAG_WARN, filename, line_no);
		cerr << "Index action 'unique=" << boolpfx->first
		     << "' without 'boolean=" << boolpfx->first << "'" << endl;
		static bool given_doesnt_imply_boolean_warning = false;
		if (!given_doesnt_imply_boolean_warning) {
		    given_doesnt_imply_boolean_warning = true;
		    report_location(DIAG_NOTE, filename, line_no);
		    cerr << "'unique' doesn't implicitly add a boolean term"
			 << endl;
		}
	    }
	}

	vector<string>::const_iterator field;
	for (field = fields.begin(); field != fields.end(); ++field) {
	    vector<Action> &v = index_spec[*field];
	    if (v.empty()) {
		if (fields.size() == 1) {
		    // Optimise common case where there's only one fieldname
		    // for a list of actions.
		    v = std::move(actions);
		} else {
		    v = actions;
		}
	    } else {
		v.emplace_back(Action::NEW, string::npos);
		v.insert(v.end(), actions.begin(), actions.end());
	    }
	}
    }

    if (index_spec.empty()) {
	report_location(DIAG_ERROR, filename, line_no);
	cerr << "No rules found in index script" << endl;
	exit(1);
    }
}

static bool
run_actions(vector<Action>::const_iterator action_it,
	    vector<Action>::const_iterator action_end,
	    Xapian::WritableDatabase& database,
	    Xapian::TermGenerator& indexer,
	    const string& old_value,
	    bool& this_field_is_content, Xapian::Document& doc,
	    map<string, list<string>>& fields,
	    string& field, const char* fname,
	    size_t line_no, Xapian::docid& docid)
{
    string value = old_value;
    while (action_it != action_end) {
	auto& action = *action_it++;
	switch (action.get_action()) {
	    case Action::BAD:
		abort();
	    case Action::NEW:
		value = old_value;
		break;
	    case Action::FIELD:
		if (!value.empty()) {
		    string f = action.get_string_arg();
		    if (f.empty()) f = field;
		    // replace newlines with spaces
		    string s = value;
		    string::size_type j = 0;
		    while ((j = s.find('\n', j)) != string::npos)
			s[j] = ' ';
		    fields[f].push_back(s);
		}
		break;
	    case Action::INDEX:
		indexer.index_text(value,
				   action.get_num_arg(),
				   action.get_string_arg());
		break;
	    case Action::INDEXNOPOS:
		// No positional information so phrase searching won't work.
		// However, the database will use much less diskspace.
		indexer.index_text_without_positions(value,
						     action.get_num_arg(),
						     action.get_string_arg());
		break;
	    case Action::BOOLEAN: {
		// Do nothing if there's no text.
		if (value.empty()) break;

		string term = action.get_string_arg();
		if (prefix_needs_colon(term, value[0])) term += ':';
		term += value;

		doc.add_boolean_term(term);
		break;
	    }
	    case Action::GAP:
		indexer.increase_termpos(action.get_num_arg());
		break;
	    case Action::HASH: {
		unsigned int max_length = action.get_num_arg();
		if (value.length() > max_length)
		    value = hash_long_term(value, max_length);
		break;
	    }
	    case Action::HEXTOBIN: {
		size_t len = value.length();
		if (len & 1) {
		    report_location(DIAG_ERROR, fname, line_no);
		    cerr << "hextobin: input must have even length"
			 << endl;
		} else {
		    string output;
		    output.reserve(len / 2);
		    for (size_t j = 0; j < len; j += 2) {
			char a = value[j];
			char b = value[j + 1];
			if (!C_isxdigit(a) || !C_isxdigit(b)) {
			    report_location(DIAG_ERROR, fname, line_no);
			    cerr << "hextobin: input must be all hex "
				    "digits" << endl;
			    goto badhex;
			}
			char r = (hex_digit(a) << 4) | hex_digit(b);
			output.push_back(r);
		    }
		    value = std::move(output);
		}
badhex:
		break;
	    }
	    case Action::LOWER:
		value = Xapian::Unicode::tolower(value);
		break;
	    case Action::LTRIM:
		ltrim(value, action.get_string_arg());
		break;
	    case Action::RTRIM:
		rtrim(value, action.get_string_arg());
		break;
	    case Action::TRIM:
		rtrim(value, action.get_string_arg());
		ltrim(value, action.get_string_arg());
		break;
	    case Action::SQUASH:
		squash(value, action.get_string_arg());
		break;
	    case Action::LOAD: {
		// If there's no input, just issue a warning.
		if (value.empty()) {
		    report_location(DIAG_WARN, fname, line_no);
		    cerr << "Empty filename in LOAD action" << endl;
		    break;
		}
		bool truncated = false;
		string filename = std::move(value);
		// FIXME: Use NOATIME if we own the file or are root.
		if (!load_file(filename, action.get_num_arg(), NOCACHE,
			       value, truncated)) {
		    report_location(DIAG_ERROR, fname, line_no);
		    cerr << "Couldn't load file '" << filename << "': "
			 << strerror(errno) << endl;
		    value.resize(0);
		    break;
		}
		if (!truncated) break;
	    }
	    /* FALLTHRU */
	    case Action::TRUNCATE:
		utf8_truncate(value, action.get_num_arg());
		break;
	    case Action::SPELL:
		indexer.set_flags(indexer.FLAG_SPELLING);
		break;
	    case Action::SPLIT: {
		// Find the end of the actions which split should execute.
		auto split_end = find(action_it, action_end, Action::NEW);

		int split_type = action.get_num_arg();
		if (value.empty()) {
		    // Nothing to do.
		} else if (split_type != Action::SPLIT_SORT) {
		    // Generate split as we consume it.
		    const string& delimiter = action.get_string_arg();

		    unique_ptr<unordered_set<string>> seen;
		    if (split_type == Action::SPLIT_DEDUP) {
			seen.reset(new unordered_set<string>);
		    }

		    if (delimiter.size() == 1) {
			// Special case for common single character delimiter.
			char ch = delimiter[0];
			string::size_type i = 0;
			while (true) {
			    string::size_type j = value.find(ch, i);
			    if (split_type == Action::SPLIT_PREFIXES) {
				if (j > 0) {
				    string val(value, 0, j);
				    run_actions(action_it, split_end,
						database, indexer,
						val,
						this_field_is_content, doc,
						fields,
						field, fname, line_no,
						docid);
				}
			    } else if (i != j) {
				string val(value, i, j - i);
				if (!seen.get() || seen->insert(val).second) {
				    run_actions(action_it, split_end,
						database, indexer,
						val,
						this_field_is_content, doc,
						fields,
						field, fname, line_no,
						docid);
				}
			    }
			    if (j == string::npos) break;
			    i = j + 1;
			}
		    } else {
			string::size_type i = 0;
			while (true) {
			    string::size_type j = value.find(delimiter, i);
			    if (split_type == Action::SPLIT_PREFIXES) {
				if (j > 0) {
				    string val(value, 0, j);
				    run_actions(action_it, split_end,
						database, indexer,
						val,
						this_field_is_content, doc,
						fields,
						field, fname, line_no,
						docid);
				}
			    } else if (i != j) {
				string val(value, i, j - i);
				if (!seen.get() || seen->insert(val).second) {
				    run_actions(action_it, split_end,
						database, indexer,
						val,
						this_field_is_content, doc,
						fields,
						field, fname, line_no,
						docid);
				}
			    }
			    if (j == string::npos) break;
			    i = j + delimiter.size();
			}
		    }
		} else {
		    vector<string> split_values;
		    const string& delimiter = action.get_string_arg();
		    if (delimiter.size() == 1) {
			// Special case for common single character delimiter.
			char ch = delimiter[0];
			string::size_type i = 0;
			while (true) {
			    string::size_type j = value.find(ch, i);
			    if (i != j) {
				split_values.emplace_back(value, i, j - i);
			    }
			    if (j == string::npos) break;
			    i = j + 1;
			}
		    } else {
			string::size_type i = 0;
			while (true) {
			    string::size_type j = value.find(delimiter, i);
			    if (i != j) {
				split_values.emplace_back(value, i, j - i);
			    }
			    if (j == string::npos) break;
			    i = j + delimiter.size();
			}
		    }

		    sort(split_values.begin(), split_values.end());

		    for (auto&& val : split_values) {
			run_actions(action_it, split_end,
				    database, indexer, val,
				    this_field_is_content, doc, fields,
				    field, fname, line_no,
				    docid);
		    }
		}

		action_it = split_end;
		break;
	    }
	    case Action::UNHTML: {
		HtmlParser p;
		try {
		    // Default HTML character set is latin 1, though
		    // not specifying one is deprecated these days.
		    p.parse(value, "iso-8859-1", false);
		} catch (const string & newcharset) {
		    p.reset();
		    p.parse(value, newcharset, true);
		}
		if (p.indexing_allowed)
		    value = p.dump;
		else
		    value = "";
		break;
	    }
	    case Action::UNXML: {
		GenericXmlParser p;
		p.parse(value);
		value = std::move(p.dump);
		break;
	    }
	    case Action::UNIQUE: {
		// If there's no text, just issue a warning.
		if (value.empty()) {
		    report_location(DIAG_WARN, fname, line_no);
		    cerr << "Ignoring UNIQUE action on empty text"
			 << endl;
		    break;
		}

		// Ensure that the value of this field is unique.
		// If a record already exists with the same value,
		// it will be replaced with the new record.

		// Unique fields aren't considered content - if
		// there are no other fields in the document, the
		// document is to be deleted.
		this_field_is_content = false;

		// Argument is the prefix to add to the field value
		// to get the unique term.
		string t = action.get_string_arg();
		if (prefix_needs_colon(t, value[0])) t += ':';
		t += value;
		Xapian::PostingIterator p = database.postlist_begin(t);
		if (p != database.postlist_end(t)) {
		    docid = *p;
		}
		break;
	    }
	    case Action::VALUE:
		if (!value.empty())
		    doc.add_value(action.get_num_arg(), value);
		break;
	    case Action::VALUENUMERIC: {
		if (value.empty()) break;
		char * end;
		double dbl = strtod(value.c_str(), &end);
		if (*end) {
		    report_location(DIAG_WARN, fname, line_no);
		    cerr << "Trailing characters in VALUENUMERIC: '"
			 << value << "'" << endl;
		}
		doc.add_value(action.get_num_arg(),
			      Xapian::sortable_serialise(dbl));
		break;
	    }
	    case Action::VALUEPACKED: {
		uint32_t word = 0;
		if (value.empty() || !C_isdigit(value[0])) {
		    // strtoul() accepts leading whitespace and negated
		    // values, neither of which we want to allow.
		    errno = EINVAL;
		} else {
		    errno = 0;
		    char* q;
		    word = strtoul(value.c_str(), &q, 10);
		    if (!errno && *q != '\0') {
			// Trailing characters after converted value.
			errno = EINVAL;
		    }
		}
		if (errno) {
		    report_location(DIAG_WARN, fname, line_no);
		    cerr << "valuepacked \"" << value << "\" ";
		    if (errno == ERANGE) {
			cerr << "out of range";
		    } else {
			cerr << "not an unsigned integer";
		    }
		    cerr << endl;
		}
		int valueslot = action.get_num_arg();
		doc.add_value(valueslot, int_to_binary_string(word));
		break;
	    }
	    case Action::DATE: {
		// Do nothing for empty input.
		if (value.empty()) break;

		const string & type = action.get_string_arg();
		string yyyymmdd;
		if (type == "unix") {
		    time_t t;
		    if (!parse_signed(value.c_str(), t)) {
			report_location(DIAG_WARN, fname, line_no);
			cerr << "Date value (in secs) for action DATE "
				"must be an integer - ignoring" << endl;
			break;
		    }
		    struct tm *tm = localtime(&t);
		    int y = tm->tm_year + 1900;
		    int m = tm->tm_mon + 1;
		    yyyymmdd = date_to_string(y, m, tm->tm_mday);
		} else if (type == "unixutc") {
		    time_t t;
		    if (!parse_signed(value.c_str(), t)) {
			report_location(DIAG_WARN, fname, line_no);
			cerr << "Date value (in secs) for action DATE "
				"must be an integer - ignoring" << endl;
			break;
		    }
		    struct tm *tm = gmtime(&t);
		    int y = tm->tm_year + 1900;
		    int m = tm->tm_mon + 1;
		    yyyymmdd = date_to_string(y, m, tm->tm_mday);
		} else if (type == "yyyymmdd") {
		    if (value.length() != 8) {
			report_location(DIAG_WARN, fname, line_no);
			cerr << "date=yyyymmdd expects an 8 character value "
				"- ignoring" << endl;
			break;
		    }
		    yyyymmdd = value;
		}

		// Date (YYYYMMDD)
		doc.add_boolean_term("D" + yyyymmdd);
		yyyymmdd.resize(6);
		// Month (YYYYMM)
		doc.add_boolean_term("M" + yyyymmdd);
		yyyymmdd.resize(4);
		// Year (YYYY)
		doc.add_boolean_term("Y" + yyyymmdd);
		break;
	    }
	    case Action::PARSEDATE: {
		string dateformat = action.get_string_arg();
		struct tm tm;
		memset(&tm, 0, sizeof(tm));
		auto ret = strptime(value.c_str(), dateformat.c_str(), &tm);
		if (ret == NULL) {
		    report_location(DIAG_WARN, fname, line_no);
		    cerr << "\"" << value << "\" doesn't match format "
			    "\"" << dateformat << '\"' << endl;
		    break;
		}

		if (*ret != '\0') {
		    report_location(DIAG_WARN, fname, line_no);
		    cerr << "\"" << value << "\" not fully matched by "
			    "format \"" << dateformat << "\" "
			    "(\"" << ret << "\" left over) but "
			    "indexing anyway" << endl;
		}
#ifdef HAVE_STRUCT_TM_TM_GMTOFF
		auto gmtoff = tm.tm_gmtoff;
#endif
		auto secs_since_epoch = timegm(&tm);
#ifdef HAVE_STRUCT_TM_TM_GMTOFF
		secs_since_epoch -= gmtoff;
#endif
		value = str(secs_since_epoch);
		break;
	    }
	    default:
		/* Empty default case to avoid "unhandled enum value"
		 * warnings. */
		break;
	}
    }
    return true;
}

static void
index_file(const char *fname, istream &stream,
	   Xapian::WritableDatabase &database, Xapian::TermGenerator &indexer)
{
    string line;
    size_t line_no = 0;
    while (!stream.eof() && getline(stream, line)) {
	++line_no;
	Xapian::Document doc;
	indexer.set_document(doc);
	Xapian::docid docid = 0;
	map<string, list<string>> fields;
	bool seen_content = false;
	while (!line.empty()) {
	    // Cope with files from MS Windows (\r\n end of lines).
	    // Trim multiple \r characters, since that seems the best way
	    // to handle that case.
	    string::size_type last = line.find_last_not_of('\r');
	    if (last == string::npos) break;
	    line.resize(last + 1);

	    string::size_type eq = line.find('=');
	    if (eq == string::npos && !line.empty()) {
		report_location(DIAG_ERROR, fname, line_no, line.size());
		cerr << "expected = somewhere in this line" << endl;
		// FIXME: die or what?
	    }
	    string field(line, 0, eq);
	    string value(line, eq + 1, string::npos);
	    while (getline(stream, line)) {
		++line_no;
		if (line.empty() || line[0] != '=') break;
		// Cope with files from MS Windows (\r\n end of lines).
		// Trim multiple \r characters, since that seems the best way
		// to handle that case.
		last = line.find_last_not_of('\r');
		// line[0] == '=', so last != string::npos.
		// Replace the '=' with a '\n' so we don't have to use substr.
		line[0] = '\n';
		line.resize(last + 1);
		value += line;
	    }

	    // Default to not indexing spellings.
	    indexer.set_flags(Xapian::TermGenerator::flags(0));

	    bool this_field_is_content = true;
	    const vector<Action>& v = index_spec[field];
	    run_actions(v.begin(), v.end(),
			database, indexer, value,
			this_field_is_content, doc, fields,
			field, fname, line_no,
			docid);
	    if (this_field_is_content) seen_content = true;
	    if (stream.eof()) break;
	}

	// If we haven't seen any fields (other than unique identifiers)
	// the document is to be deleted.
	if (!seen_content) {
	    if (docid) {
		database.delete_document(docid);
		if (verbose) cout << "Del: " << docid << endl;
		++delcount;
	    }
	} else {
	    string data;
	    for (auto&& i : fields) {
		for (auto&& field_val : i.second) {
		    data += i.first;
		    data += '=';
		    data += field_val;
		    data += '\n';
		}
	    }

	    // Put the data in the document
	    doc.set_data(data);

	    // Add the document to the database
	    if (docid) {
		database.replace_document(docid, doc);
		if (verbose) cout << "Replace: " << docid << endl;
		++repcount;
	    } else {
		docid = database.add_document(doc);
		if (verbose) cout << "Add: " << docid << endl;
		++addcount;
	    }
	}
    }

    // Commit after each file to make sure all changes from that file make it
    // in.
    if (verbose) cout << "Committing: " << endl;
    database.commit();
}

[[noreturn]]
static void
show_help(int exit_code)
{
    cout << PROG_NAME " - " PROG_DESC "\n"
"Usage: " PROG_NAME " [OPTIONS] DATABASE INDEXER_SCRIPT [INPUT_FILE]...\n"
"\n"
"Creates or updates a Xapian database with the data from the input files listed\n"
"on the command line.  If no files are specified, data is read from stdin.\n"
"\n"
"See https://xapian.org/docs/omega/scriptindex.html for documentation of the\n"
"format for INDEXER_SCRIPT.\n"
"\n"
"Options:\n"
"  -v, --verbose       display additional messages to aid debugging\n"
"      --overwrite     create the database anew (the default is to update if\n"
"                      the database already exists)\n";
    print_stemmer_help("");
    print_help_and_version_help("");
    exit(exit_code);
}

int
main(int argc, char **argv)
try {
    // If the database already exists, default to updating not overwriting.
    int database_mode = Xapian::DB_CREATE_OR_OPEN;
    verbose = false;
    Xapian::Stem stemmer("english");

    // Without this, strptime() seems to treat formats without a timezone as
    // being local time, including %s.
    setenv("TZ", "UTC", 1);

    constexpr auto NO_ARG = no_argument;
    constexpr auto REQ_ARG = required_argument;
    static const struct option longopts[] = {
	{ "help",	NO_ARG,		NULL, 'h' },
	{ "version",	NO_ARG,		NULL, 'V' },
	{ "stemmer",	REQ_ARG,	NULL, 's' },
	{ "overwrite",	NO_ARG,		NULL, 'o' },
	{ "verbose",	NO_ARG,		NULL, 'v' },
	{ 0, 0, NULL, 0 }
    };

    int getopt_ret;
    while ((getopt_ret = gnu_getopt_long(argc, argv, "vs:hV",
					 longopts, NULL)) != -1) {
	switch (getopt_ret) {
	    default:
		show_help(1);
		break;
	    case 'h': // --help
		show_help(0);
		break;
	    case 'V': // --version
		print_package_info(PROG_NAME);
		return 0;
	    case 'o': // --overwrite
		database_mode = Xapian::DB_CREATE_OR_OVERWRITE;
		break;
	    case 'v':
		verbose = true;
		break;
	    case 's':
		try {
		    stemmer = Xapian::Stem(optarg);
		} catch (const Xapian::InvalidArgumentError &) {
		    cerr << "Unknown stemming language '" << optarg << "'.\n";
		    cerr << "Available language names are: "
			 << Xapian::Stem::get_available_languages() << endl;
		    return 1;
		}
		break;
	}
    }

    argv += optind;
    argc -= optind;
    if (argc < 2) {
	show_help(1);
    }

    parse_index_script(argv[1]);

    // Open the database.  If another process is currently updating the
    // database, wait for the lock to become available.
    auto flags = database_mode | Xapian::DB_RETRY_LOCK;
    Xapian::WritableDatabase database(argv[0], flags);

    Xapian::TermGenerator indexer;
    indexer.set_stemmer(stemmer);
    // Set the database for spellings to be added to by the "spell" action.
    indexer.set_database(database);

    addcount = 0;
    repcount = 0;
    delcount = 0;

    if (argc == 2) {
	// Read from stdin.
	index_file("<stdin>", cin, database, indexer);
    } else {
	// Read file(s) listed on the command line.
	for (int i = 2; i < argc; ++i) {
	    ifstream stream(argv[i]);
	    if (stream) {
		index_file(argv[i], stream, database, indexer);
	    } else {
		cerr << "Can't open file " << argv[i] << endl;
	    }
	}
    }

    cout << "records (added, replaced, deleted) = (" << addcount << ", "
	 << repcount << ", " << delcount << ")" << endl;
} catch (const Xapian::Error &error) {
    cerr << "Exception: " << error.get_description() << endl;
    exit(1);
} catch (const std::bad_alloc &) {
    cerr << "Exception: std::bad_alloc" << endl;
    exit(1);
} catch (...) {
    cerr << "Unknown Exception" << endl;
    exit(1);
}
