/* query.cc: query executor for omega
 *
 * ----START-LICENCE----
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2001 James Aylett
 * Copyright 2001 Ananova Ltd
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

#include <algorithm>
#include <vector>
#include <map>

#include <stdio.h>
#include <ctype.h>
#include <time.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "utils.h"
#include "omega.h"
#include "query.h"
#include "cgiparam.h"
#include "parsequery.h"

static bool done_query = false;
static om_docid last = 0;

static OmMSet mset;

static void ensure_match();

string raw_prob;
map<om_docid, bool> ticked;

static OmQuery query;
static string query_string;
OmQuery::op default_op = OmQuery::OP_OR; // default matching mode

static QueryParser qp;

static string eval_file(const string &fmtfile);

querytype
set_probabilistic(const string &newp, const string &oldp)
{
    // strip leading and trailing whitespace
    std::string::size_type first_nonspace = newp.find_first_not_of(" \t\r\n\v");
    if (first_nonspace == string::npos) {
	raw_prob = "";
    } else {
	std::string::size_type len = newp.find_last_not_of(" \t\r\n\v");
	raw_prob = newp.substr(first_nonspace, len + 1 - first_nonspace);
    }

    // call YACC generated parser
    qp.set_stemming_options(option["no_stem"] == "true" ? "" : "english",
			    option["all_stem"] == "true"); 
    query = qp.parse_query(raw_prob);

    // Check new query against the previous one
    const char *pend;
    const char *term;
    unsigned int n_old_terms = 0;

    if (oldp.empty()) return NEW_QUERY;

    // We used to use "word1#word2#" (with trailing #) but some broken old
    // browsers (versions of MSIE) don't quote # in form GET submissions
    // and everything after the # gets interpreted as an anchor.
    // So now we use "word1.word2." instead.
    term = oldp.c_str();
    pend = term;
    while ((pend = strchr(pend, '.')) != NULL) {
	pend++;
	n_old_terms++;
    }
    // short-cut: if the new query has fewer terms, it must be a new one
    if (qp.termset.size() < n_old_terms) return NEW_QUERY;
    
    while ((pend = strchr(term, '.')) != NULL) {
	if (qp.termset.find(string(term, pend - term)) == qp.termset.end())
	    return NEW_QUERY;
	term = pend + 1;
    }
    if (qp.termlist.size() > n_old_terms) return EXTENDED_QUERY;
    return SAME_QUERY;
}

static std::multimap<char, string> filter_map;

typedef std::multimap<char,string>::const_iterator FMCI;
    
void add_bterm(const string &term) {
    filter_map.insert(std::make_pair(term[0], term));
}

static int
last_day(int y, int m)
{
    static const int l[13] = {
	0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
    };
    if (m != 2) return l[m];
    return 28 + (y % 4 == 0); // good until 2100
}

static OmQuery
date_range_filter(int y1, int m1, int d1, int y2, int m2, int d2)
{
    char buf[9];
    sprintf(buf, "%04d%02d", y1, m1);
    std::vector<OmQuery> v;

    // Deal with any initial partial month
    if (d1 > 1 || (y1 == y2 && m1 == m2 && d2 < last_day(y2, m2))) {
    	for ( ; d1 <= 31 ; d1++) {
	    sprintf(buf + 6, "%02d", d1);
	    v.push_back(OmQuery('D' + std::string(buf)));
	}
    } else {
	v.push_back(OmQuery('M' + std::string(buf)));
    }
    
    if (y1 == y2 && m1 == m2) {
	return OmQuery(OmQuery::OP_OR, v.begin(), v.end());
    }

    int m_last = (y1 < y2) ? 12 : m2 - 1;
    while (++m1 <= m_last) {
	sprintf(buf + 4, "%02d", m1);
	v.push_back(OmQuery('M' + std::string(buf)));
    }
	
    if (y1 < y2) {
	while (++y1 < y2) {
	    sprintf(buf, "%04d", y1);
	    v.push_back(OmQuery('Y' + std::string(buf)));
	}
	sprintf(buf, "%04d", y2);
	for (m1 = 1; m1 < m2; m1++) {
	    sprintf(buf + 4, "%02d", m1);
	    v.push_back(OmQuery('M' + std::string(buf)));
	}
    }
	
    sprintf(buf + 4, "%02d", m2);

    // Deal with any final partial month
    if (d2 < last_day(y2, m2)) {
    	for (d1 = 1 ; d1 <= d2; d1++) {
	    sprintf(buf + 6, "%02d", d1);
	    v.push_back(OmQuery('D' + std::string(buf)));
	}
    } else {
	v.push_back(OmQuery('M' + std::string(buf)));
    }

    return OmQuery(OmQuery::OP_OR, v.begin(), v.end());
}

static void
parse_date(string date, int *y, int *m, int *d)
{
    // FIXME: for now only support YYYYMMDD (e.g. 20011119)
    // and don't error check
    *y = atoi(date.substr(0, 4).c_str());
    *m = atoi(date.substr(4, 2).c_str());
    *d = atoi(date.substr(6, 2).c_str());
}

static void
run_query()
{
    if (!filter_map.empty()) {
	// OR together filters with the same prefix, then AND together
	std::vector<OmQuery> filter_vec;
	std::vector<om_termname> and_vec;
	int current = -256;
	for (FMCI i = filter_map.begin(); ; i++) {
	    bool over = (i == filter_map.end());
	    if (over || i->first != current) {
		switch (and_vec.size()) {
		    case 0:
		        break;
		    case 1:
			filter_vec.push_back(OmQuery(and_vec[0]));
		        break;
		    default:
			filter_vec.push_back(OmQuery(OmQuery::OP_OR,
						     and_vec.begin(),
						     and_vec.end()));
		        break;
		}
		if (over)
		    break;
		and_vec.clear();
		current = i->first;
	    }
	    and_vec.push_back(i->second);
	}
	
	query = OmQuery(OmQuery::OP_FILTER,
			query,
			OmQuery(OmQuery::OP_AND,
				filter_vec.begin(),
				filter_vec.end()));
    }

    if (!date1.empty() || !date2.empty() || !daysminus.empty()) {
	int y1 = 1970, m1 = 1, d1 = 1;
	if (!date1.empty()) {
	    parse_date(date1, &y1, &m1, &d1);	
	}
	int y2, m2, d2;
	if (!daysminus.empty()) {
	    time_t now = time(NULL);
	    if (date1.empty()) {
		struct tm *t = localtime(&now);
		y2 = t->tm_year + 1900;
		m2 = t->tm_mon + 1;
		d2 = t->tm_mday;
	    } else {
		y2 = y1;
		m2 = m1;
		d2 = d1;
	    }
	    // FIXME: if DATE1 and DAYSMINUS set, use date1 not now...
	    time_t then = now - atoi(daysminus.c_str()) * 86400;
	    struct tm *t = localtime(&then);
	    y1 = t->tm_year + 1900;
	    m1 = t->tm_mon + 1;
	    d1 = t->tm_mday;
	} else if (date2.empty()) {
	    time_t now = time(NULL);
	    struct tm *t = localtime(&now);
	    y2 = t->tm_year + 1900;
	    m2 = t->tm_mon + 1;
	    d2 = t->tm_mday;
	} else {
	    parse_date(date2, &y2, &m2, &d2);
	}
	
	query = OmQuery(OmQuery::OP_FILTER,
		       	query,
			date_range_filter(y1, m1, d1, y2, m2, d2));
    }

    enquire->set_query(query);

    // We could use the value of topdoc as first parameter, but we
    // need to know the first few items on the mset to fake a
    // relevance set for topterms
    OmSettings opt;
    opt.set("match_percent_cutoff", threshold);
    // FIXME - set msetcmp to reverse?
    mset = enquire->get_mset(0, topdoc + hits_per_page, rset, &opt);
}

#if 0
// generate a sorted picker  FIXME: should be generatable by script language
//
// output looks like this:
//
// <SELECT NAME=B>
// <OPTION VALUE="" SELECTED>
// <OPTION VALUE="Mtext/html">HTML
// <OPTION VALUE="Mapplication/postscript">PostScript
// <OPTION VALUE="Mtext/plain">Text
// </SELECT>
static void
do_picker(char prefix, const char **opts)
{
    const char **p;
    bool do_current = false;
    string current;
    vector<string> picker;

    // FIXME: what if multiple values present for a prefix???
    FMCI i = filter_map.find(prefix);
    if (i != filter_map.end() && i->second.length() > 1) {
	current = i->second.substr(1);
	do_current = true;
    }

    cout << "<SELECT NAME=B>\n<OPTION VALUE=\"\"";

    // Some versions of MSIE don't default to selecting the first option,
    // so we explicitly have to
    if (!do_current) cout << " SELECTED";

    cout << '>';

    string tmp = option[std::string('B') + prefix];
    if (!tmp.empty())
	cout << tmp;
    else
	cout << "-Any-";
    
    for (p = opts; *p; p++) {
	string trans = option[std::string('B') + prefix + *p];
	if (trans.empty()) {
	    // FIXME: nasty special casing on prefix...
	    if (prefix == 'N')
		trans = string(".") + *p;
	    else 
		trans = "[" + string(*p) + "]";
	}
	if (do_current && current == *p) {
	    trans += '\n';
	    do_current = false;
	}
	picker.push_back(trans + '\t' + string(*p));
    }
   
    sort(picker.begin(), picker.end());

    vector<string>::const_iterator i2;
    for (i2 = picker.begin(); i2 != picker.end(); i2++) {
	std::string::size_type j = (*i2).find('\t');
	if (j == string::npos) continue;
	const char *p = (*i2).c_str();
	cout << "\n<OPTION VALUE=" << prefix << string(p + j + 1);
	if (j >= 1 && p[j - 1] == '\n') cout << " SELECTED";
	cout << '>' << string(p, j);
    }

    if (do_current) {
	cout << "\n<OPTION VALUE=\"" << prefix << current << "\" SELECTED>"
             << current;
    }
    cout << "</SELECT>\n";
}
#endif

static string
percent_encode(const string &str)
{
    string res;
    const char *p = str.c_str();
    while (1) {
	char ch = *p++;
	if (ch == 0) return res;
	if (ch <= 32 || ch >= 127 || strchr("#%&,/:;<=>?@[\\]^_{|}", ch)) {
	    char buf[4];
	    sprintf(buf, "%%%02x", ch);
	    res += buf;
	} else {
	    res += ch;
	}
    }
}

static string
html_escape(const string &str)
{
    string res;
    std::string::size_type p = 0;
    while (p < str.size()) {
	char ch = str[p++];
	switch (ch) {
 	    case '<':
	        res += "&lt;";
	        continue;
	    case '>':
	        res += "&gt;";
	        continue;
	    case '&':
	        res += "&amp;";
	        continue;
	    case '"':
	        res += "&quot;";
	        continue;
	    default:
	        res += ch;
	}
    }
    return res;
}

// FIXME split list into hash or map and use that rather than linear lookup?
static bool word_in_list(const string& test_word, const string& list)
{
    //    cerr << "word_in_list(" << test_word << ", '" << list << "'): ";
    std::string::size_type split = 0, split2;
    while ((split2 = list.find('\t', split)) != std::string::npos) {
	if (test_word == list.substr(split, split2 - split)) {
	    //	    cerr << "HIT." << endl;
	    return 1;
	} else {
	    split = split2 + 1;
	}
    }
    if (test_word == list.substr(split, split2 - split)) {
	//	cerr << "HIT." << endl;
	return 1;
    } else {
	//	cerr << "." << endl;
	return 0;
    }
}

// FIXME: this copied from om/indexer/index_utils.cc
static void
lowercase_term(om_termname &term)
{
    om_termname::iterator i = term.begin();
    while(i != term.end()) {
	*i = tolower(*i);
	i++;
    }
}

// FIXME: shares algorithm with omindex.cc!
static string
html_highlight(const string &str, const string &list)
{
    string res;
    std::string::size_type i, j = 0, k, l = 0;
    //    cerr << "html_highlight('" << str << "', '" << list << "'): ";
    while ((i = str.find_first_of("ABCDEFGHIJKLMNOPQRSTUVWXYZ"
				  "abcdefghijklmnopqrstuvwxyz", j))
	   != std::string::npos) {

	//	cerr << "[i=" << i << "]";
	
	j = str.find_first_not_of("ABCDEFGHIJKLMNOPQRSTUVWXYZ"
				  "abcdefghijklmnopqrstuvwxyz"
				  "0123456789", i);
	k = str.find_first_not_of("+-", j);
	if (k == string::npos || !isalnum(str[k])) j = k;

	//	cerr << "[j=" << j << "] ";
	
	//#if 0
	string word = str.substr(i, j - i);
	string term = word;
	lowercase_term(term);

	res += html_escape(str.substr(l, i - l));
	if (word_in_list(stemmer->stem_word(term), list)) {
	    res += "<strong>" + word + "</strong>";
	} else {
	    res += word;
	}
	l = j;
	//#endif
	//	i = j + 1;
    }
    //    cerr << endl;
    if (l != std::string::npos) {
	res += html_escape(str.substr(l));
    }
    return res;
}

#if 0
static void
print_query_string(const char *after)
{
    if (after && strncmp(after, "&B=", 3) == 0) {
	char prefix = after[3];
	std::string::size_type start = 0, amp = 0;
	while (1) {
	    amp = query_string.find('&', amp);
	    if (amp == string::npos) {
		cout << query_string.substr(start);
		return;
	    }
	    amp++;
	    while (query_string[amp] == 'B' &&
		   query_string[amp + 1] == '=' &&
		   query_string[amp + 2] == prefix) {
		cout << query_string.substr(start, amp - start - 1);
		start = query_string.find('&', amp + 3);
		if (start == string::npos) return;
		amp = start + 1;
	    }
	}
    }
    cout << query_string;
}
#endif

static map<string, string> field;
static om_docid q0;
static int percent;

static string print_caption(om_docid m, const string &fmt, const string &loopvar);

enum tagval {
CMD_,
CMD_add,
CMD_allterms,
CMD_and,
CMD_cgi,
CMD_cgilist,
CMD_date,
CMD_dbname,
CMD_defaultop,
CMD_eq,
CMD_env,
CMD_field,
CMD_filesize,
CMD_fmt,
CMD_freq,
CMD_freqs,
CMD_highlight,
CMD_hitlist,
CMD_hitsperpage,
CMD_html,
CMD_id,
CMD_if,
CMD_include,
CMD_last,
CMD_lastpage,
CMD_list,
CMD_map,
CMD_max,
CMD_min,
CMD_msize,
CMD_ne,
CMD_nice,
CMD_not,
CMD_opt,
CMD_or,
CMD_percentage,
CMD_query,
CMD_queryterms,
CMD_range,
CMD_record,
CMD_relevant,
CMD_relevants,
CMD_score,
CMD_set,
CMD_setmap,
CMD_setrelevant,
CMD_terms,
CMD_thispage,
CMD_topdoc,
CMD_topterms,
CMD_url,
CMD_version
};

struct func_attrib {
    int tag;
    int minargs, maxargs, evalargs;
    bool ensure_match;
    bool cache; // FIXME: implement cache
};

#define STRINGIZE(N) _STRINGIZE(N)
#define _STRINGIZE(N) #N
    
#define T(F) STRINGIZE(F), { CMD_##F
struct func_desc {
    const char *name;
    struct func_attrib a;
};

#define N -1
static struct func_desc func_tab[] = {
//name minargs maxargs evalargs ensure_match cache
{"",{CMD_,	N, N, 0, 0, 0}}, // commented out code
{T(add),	0, N, N, 0, 0}}, // add a list of numbers
{T(allterms),	0, 1, N, 0, 0}}, // list of all terms matching document
{T(and),	1, N, 0, 0, 0}}, // logical shortcutting and of a list of values
{T(cgi),	1, 1, N, 0, 0}}, // return cgi parameter value
{T(cgilist),	1, 1, N, 0, 0}}, // return list of values for cgi parameter
{T(date),	1, 2, N, 1, 0}}, // convert time_t to strftime format (default: YYYY-MM-DD)
{T(dbname),	0, 0, N, 0, 0}}, // database name
{T(defaultop),	0, 0, N, 0, 0}}, // default operator: "and" or "or"
{T(env),	1, 1, N, 0, 0}}, // environment variable
{T(eq),		2, 2, N, 0, 0}}, // test equality
{T(field),	1, 1, N, 0, 0}}, // lookup field in record
{T(filesize),	1, 1, N, 0, 0}}, // pretty printed filesize
{T(fmt),	0, 0, N, 0, 0}}, // name of current format
{T(freq),	1, 1, N, 0, 0}}, // frequency of a term
{T(freqs),	0, 0, N, 1, 1}}, // return HTML string listing query terms and frequencies
{T(highlight),	2, 2, N, 0, 0}}, // html escape and highlight words from list
{T(hitlist),	N, N, 0, 1, 0}}, // display hitlist using format in argument
{T(hitsperpage),0, 0, N, 0, 0}}, // hits per page
{T(html),	1, 1, N, 0, 0}}, // html escape string (<>&)
{T(id),		0, 0, N, 0, 0}}, // docid of current doc
{T(if),		2, 3, 1, 0, 0}}, // conditional
{T(include),	1, 1, 1, 0, 0}}, // include another file
{T(last),	0, 0, N, 1, 0}}, // m-set number of last hit on page
{T(lastpage),	0, 0, N, 1, 0}}, // number of last hit page
{T(list),	2, 5, N, 0, 0}}, // pretty print list
{T(map),	1, N, 1, 0, 0}}, // map a list into another list
{T(max),	1, N, N, 0, 0}}, // maximum of a list of values
{T(min),	1, N, N, 0, 0}}, // minimum of a list of values
{T(msize),	0, 0, N, 1, 0}}, // number of matches
{T(ne), 	2, 2, N, 0, 0}}, // test not equal
{T(nice),	1, 1, N, 0, 0}}, // pretty print integer (with thousands sep)
{T(not),	1, 1, N, 0, 0}}, // logical not
{T(opt),	1, 1, N, 0, 0}}, // lookup an option value
{T(or),		1, N, 0, 0, 0}}, // logical shortcutting or of a list of values
{T(percentage),	0, 0, N, 0, 0}}, // percentage score of current hit
{T(query),	0, 0, N, 0, 0}}, // query
{T(queryterms),	0, 0, N, 0, 1}}, // list of query terms
{T(range),	2, 2, N, 0, 0}}, // return list of values between start and end
{T(record),	0, 1, N, 1, 0}}, // record contents of document
{T(relevant),	0, 1, N, 1, 0}}, // is document relevant?
{T(relevants),	0, 0, N, 1, 0}}, // return list of relevant documents
{T(score),	0, 0, N, 0, 0}}, // score (0-10) of current hit
{T(set),	2, 2, N, 0, 0}}, // set option value
{T(setmap),	1, N, N, 0, 0}}, // set map of option values
{T(setrelevant),0, 1, N, 0, 0}}, // set rset
{T(terms),	0, 0, N, 1, 0}}, // list of matching terms
{T(thispage),	0, 0, N, 1, 0}}, // page number of current page
{T(topdoc),	0, 0, N, 0, 0}}, // first document on current page of hit list (counting from 0)
// FIXME: cache really needs to be smart about parameter value...
{T(topterms),	0, 1, N, 1, 1}}, // list of up to N top relevance feedback terms (default 20)
{T(url),	1, 1, N, 0, 0}}, // url encode argument
{T(version),	0, 0, N, 0, 0}}, // omega version string
{ NULL,{0,      0, 0, 0, 0, 0}}
};

static string
eval(const string &fmt, const string &loopvar)
{
    static map<string, const struct func_attrib *> func_map;
    if (func_map.empty()) {
	struct func_desc *p;
	for (p = func_tab; p->name != NULL; p++) {
	    func_map[string(p->name)] = &(p->a);
	}
    }
    string res;
    std::string::size_type p = 0, q;
    while ((q = fmt.find('$', p)) != string::npos) {
	res += fmt.substr(p, q - p);
	std::string::size_type code_start = q; // note down for error reporting
	q++;
	if (q >= fmt.size()) break;
	// Magic sequences:
	// `$$' -> `$', `$(' -> `{', `$)' -> `}', `$.' -> `,'
	char lit = '\0';
	switch (fmt[q]) {
	 case '$': lit = '$'; break;
	 case '(': lit = '{'; break;
	 case ')': lit = '}'; break;
	 case '.': lit = ','; break;
	}
	if (lit) {
	    res += lit;
	    p = q + 1;
	    continue;
	}
	if (fmt[q] == '_') {
	    res += loopvar;
            p = q + 1;
            continue;
	}
	if (fmt[q] != '{' && !isalpha(fmt[q])) {
	    string msg = "Unknown $ code in: $" + fmt.substr(q);
	    throw msg;
	}
	p = fmt.find_first_not_of("ABCDEFGHIJKLMNOPQRSTUVWXYZ"
				  "abcdefghijklmnopqrstuvwxyz"
				  "0123456789_", q);
	if (p == string::npos) p = fmt.size();
	string var = fmt.substr(q, p - q);
	map<string, const struct func_attrib *>::const_iterator i;
	i = func_map.find(var);
	if (i == func_map.end()) {
	    throw "Unknown function `" + var + "'";
	}
	vector<string> args;
	if (fmt[p] == '{') {
	    q = p + 1;
	    int nest = 1;
	    while (1) {
		p = fmt.find_first_of(",{}", p + 1);
		if (p == string::npos)
		    throw "missing } in " + fmt.substr(code_start);
		if (fmt[p] == '{') {
		    ++nest;
		} else {
		    if (nest == 1) {
			// should we split the args
			if (i->second->minargs != N) {
			    args.push_back(fmt.substr(q, p - q));
			    q = p + 1;
			}
		    }
		    if (fmt[p] == '}' && --nest == 0) break;
		}
	    }
	    if (i->second->minargs == N) args.push_back(fmt.substr(q, p - q));
	    p++;
	}

	// allow "$thispage{}s.gif" to work...
	if (i->second->maxargs == 0 && args.size() == 1 && args[0].empty())
	    args.clear();

	if (i->second->minargs != N) {
	    if ((int)args.size() < i->second->minargs)
		throw "too few arguments to $" + var;
	    if (i->second->maxargs != N &&
		(int)args.size() > i->second->maxargs)
		throw "too many arguments to $" + var;

	    std::vector<string>::size_type n;
	    if (i->second->evalargs != N)
		n = i->second->evalargs;
	    else
		n = args.size();
	    
	    for (std::vector<string>::size_type j = 0; j < n; j++)
		args[j] = eval(args[j], loopvar);
	}
	if (i->second->ensure_match) ensure_match();
	string value;
	switch (i->second->tag) {
	    case CMD_:
	        break;
	    case CMD_add: {
		int total = 0;
		vector<string>::const_iterator i;
		for (i = args.begin(); i != args.end(); i++)
		    total += string_to_int(*i);
		value = int_to_string(total);
		break;
	    }
	    case CMD_allterms: {
		// list of all terms indexing document
		int id = q0;
		if (!args.empty()) id = string_to_int(args[0]);
		OmTermIterator term = omdb->termlist_begin(id);
		for ( ; term != omdb->termlist_end(id); term++)
		    value = value + *term + '\t';

		if (!value.empty()) value.erase(value.size() - 1);
		break;
	    }
	    case CMD_and:
		for (vector<string>::const_iterator i = args.begin();
		     i != args.end(); i++) {
		    if (eval(*i, loopvar).empty()) {
			value = "true";
			break;
		    }
	        }
		break;
	    case CMD_cgi: {
		MCI i = cgi_params.find(args[0]);
		if (i != cgi_params.end()) value = i->second;
		break;
	    }
	    case CMD_cgilist: {
		std::pair<MCI, MCI> g;
		g = cgi_params.equal_range(args[0]);
		for (MCI i = g.first; i != g.second; i++)
		    value = value + i->second + '\t';
		if (!value.empty()) value.erase(value.size() - 1);
		break;
	    }
	    case CMD_date:
		value = args[0];
		if (!value.empty()) {
		    char buf[64] = "";
		    time_t date = string_to_int(value);
		    if (date != (time_t)-1) {
			struct tm *then;
			then = gmtime(&date);
			string fmt = "%Y-%m-%d";
			if (args.size() > 1) fmt = eval(args[1], loopvar);
			strftime(buf, sizeof buf, fmt.c_str(), then);
		    }
		    value = buf;
		}
		break;
	    case CMD_dbname:
		if (dbname != default_dbname) value = dbname;
		break;
	    case CMD_defaultop:
		if (default_op == OmQuery::OP_AND) {
		    value = "and";
		} else {
		    value = "or";
		}
		break;
	    case CMD_eq:
		if (args[0] == args[1]) value = "true";
		break;
	    case CMD_env: {
		char *env = getenv(args[0].c_str());
		if (env != NULL) value = env;
		break;
	    }
	    case CMD_field:
		value = field[args[0]];
		break;
	    case CMD_filesize: {
		// FIXME: rounding?
		// FIXME: for smaller sizes give decimal fractions, e.g. "1.4K"
		int size = string_to_int(args[0]);
		char buf[200] = "";
		if (size && size < 1024) {
		    sprintf(buf, "%d bytes", size);
		} else if (size < 1024*1024) {
		    sprintf(buf, "%dK", int(size/1024));
		} else if (size < 1024*1024*1024) {
		    sprintf(buf, "%dM", int(size/1024/1024));
		} else {
		    sprintf(buf, "%dG", int(size/1024/1024/1024));
		}
		value = buf;
		break;
	    }
	    case CMD_fmt:
		value = fmtname;
		break;
	    case CMD_freq: 
		try {
		    value = int_to_string(mset.get_termfreq(args[0]));
		} catch (...) {
		    value = int_to_string(omdb->get_termfreq(args[0]));
		}
		break;
	    case CMD_freqs:
		// for backward compatibility
		value = eval("$map{$queryterms,$_:&nbsp;$nice{$freq{$_}}}",
			     loopvar);
		break;
	    case CMD_hitlist:
#if 0
		const char *q;
		int ch;
		
		query_string = "?DB=";
		query_string += dbname;
		query_string += "&P=";
		q = raw_prob.c_str();
		while ((ch = *q++) != '\0') {
		    switch (ch) {
		     case '+':
			query_string += "%2b";
			break;
		     case '"':
			query_string += "%22";
			break;
		     case ' ':
			ch = '+';
			/* fall through */
		     default:
			query_string += ch;
		    }
		}
	        // add any boolean terms
		for (FMCI i = filter_map.begin(); i != filter_map.end(); i++) {
		    query_string += "&B=";
		    query_string += i->second;
		}
#endif
		for (om_docid m = topdoc; m < last; m++)
		    value += print_caption(m, args[0], loopvar);
		break;
	    case CMD_hitsperpage:
		value = int_to_string(hits_per_page);
		break;
	    case CMD_highlight:
		value = html_highlight(args[0], args[1]);
		break;
	    case CMD_html:
	        value = html_escape(args[0]);
		break;
	    case CMD_id:
		// document id
		value = int_to_string(q0);
		break;
	    case CMD_if:
		if (!args[0].empty())
		    value = eval(args[1], loopvar);
		else if (args.size() > 2)
		    value = eval(args[2], loopvar);
		break;
	    case CMD_include:
	        value = eval_file(args[0]);
	        break;
	    case CMD_last:
		value = int_to_string(last);
		break;
	    case CMD_lastpage:
		value = int_to_string((mset.get_matches_lower_bound() - 1) /
				      hits_per_page + 1);
		break;
	    case CMD_list: {
		if (!args[0].empty()) {
		    string pre, inter, interlast, post;
		    switch (args.size()) {
		     case 2:
			inter = interlast = args[1];
			break;
		     case 3:
			inter = args[1];
			interlast = args[2];
			break;
		     case 4:
			pre = args[1];
			inter = interlast = args[2];
			post = args[3];
			break;
		     case 5:
			pre = args[1];
			inter = args[2];
			interlast = args[3];
			post = args[4];
			break;
		    }
		    value += pre;
		    string list = args[0];
		    std::string::size_type split = 0, split2;
		    while ((split2 = list.find('\t', split)) != string::npos) {
			if (split) value += inter;
			value += list.substr(split, split2 - split);
			split = split2 + 1;
		    }
		    if (split) value += interlast;
		    value += list.substr(split);
		    value += post;
		}
		break;
	    }
	    case CMD_map:
		if (!args[0].empty()) {
		    string l = args[0], pat = args[1];
		    std::string::size_type i = 0, j;
		    while (1) {
			j = l.find('\t', i);
			value += eval(pat, eval(l.substr(i, j - i), loopvar));
			if (j == string::npos) break;
			value += '\t';
			i = j + 1;
		    }
		}
	        break;
	    case CMD_max: {
		vector<string>::const_iterator i = args.begin();
		int val = string_to_int(*i++);
		for (; i != args.end(); i++) {
		    int x = string_to_int(*i);
		    if (x > val) val = x;
	        }
		value = int_to_string(val);
		break;
	    }
	    case CMD_min: {
		vector<string>::const_iterator i = args.begin();
		int val = string_to_int(*i++);
		for (; i != args.end(); i++) {
		    int x = string_to_int(*i);
		    if (x < val) val = x;
	        }
		value = int_to_string(val);
		break;
	    }
	    case CMD_msize:
		// number of matches
		value = int_to_string(mset.get_matches_estimated());
		break;
            case CMD_ne:
		if (args[0] != args[1]) value = "true";
		break;
	    case CMD_nice: {
		std::string::const_iterator i = args[0].begin();
		int len = args[0].length();
		while (len) {
		    value += *i++;
		    if (--len && len % 3 == 0) res += option["thousand"];
		}
		break;
	    }
	    case CMD_not:
		if (args[0].empty()) value = "true";
		break;
	    case CMD_opt:
		value = option[args[0]];
		break;
	    case CMD_or:
		for (vector<string>::const_iterator i = args.begin();
		     i != args.end(); i++) {
		    value = eval(*i, loopvar);
		    if (!value.empty()) break;
	        }
		break;
	    case CMD_percentage:
		// percentage score
		value = int_to_string(percent);
		break;
	    case CMD_query:
		value = raw_prob;
		break;
	    case CMD_queryterms:
		if (!qp.termlist.empty()) {
		    list<om_termname>::const_iterator i;
		    for (i = qp.termlist.begin();
			 i != qp.termlist.end(); i++) {
			value = value + *i + '\t';
		    }		    
		    if (!value.empty()) value.erase(value.size() - 1);
		}
		break;
	    case CMD_range: {
		int start = string_to_int(args[0]);
		int end = string_to_int(args[1]);
	        while (start <= end) {
		    value = value + int_to_string(start);
		    if (start < end) value += '\t';
		    start++;
		}
		break;
	    }
	    case CMD_record: {
		int id = q0;
		if (!args.empty()) id = string_to_int(args[0]);
		value = omdb->get_document(id).get_data().value;
		break;
	    }
	    case CMD_relevant: {
		// document id if relevant; empty otherwise
		int id = q0;
		if (!args.empty()) id = string_to_int(args[0]);
		map<om_docid, bool>::iterator i = ticked.find(id);
		if (i != ticked.end()) {
		    i->second = false; // icky side-effect
		    value = int_to_string(id);
		}
		break;
	    }
	    case CMD_relevants:		
		for (map <om_docid, bool>::const_iterator i = ticked.begin();
		     i != ticked.end(); i++) {
		    if (i->second) value += int_to_string(i->first) + '\t';
		}
		if (!value.empty()) value.erase(value.size() - 1);
		break;
	    case CMD_score:
	        // Score (0 to 10)
		value = int_to_string(percent / 10);
		break;
	    case CMD_set:
		option[args[0]] = args[1];
		break;
	    case CMD_setmap: {
		string base = args[0] + ',';
		for (unsigned int i = 1; i + 1 < args.size(); i++) {
		    option[base + args[i]] = args[i + 1];
		}
		break;
	    }
	    case CMD_setrelevant: {
		std::string::size_type i = 0, j;
	    	while (1) {
		    j = args[0].find_first_not_of("0123456789", i);
	    	    om_docid id = atoi(args[0].substr(i, j - i).c_str());
		    if (id) {
			rset->add_document(id);
			ticked[id] = true;
		    }
	    	    if (j == string::npos) break;
		    i = j + 1;
		}
		break;
	    }			     
	    case CMD_terms: {
		// list of matching terms
		OmTermIterator term = enquire->get_matching_terms_begin(q0);
		for ( ; term != enquire->get_matching_terms_end(q0); term++)
		    value = value + *term + '\t';

		if (!value.empty()) value.erase(value.size() - 1);
		break;
	    }
	    case CMD_thispage:
		value = int_to_string(topdoc / hits_per_page + 1);
		break;
	    case CMD_topdoc:
		// first document on current page of hit list (counting from 0)
		value = int_to_string(topdoc);
		break;
	    case CMD_topterms: {
		int howmany = 20;
		if (!args.empty()) howmany = string_to_int(args[0]);
		if (howmany < 0) howmany = 0;
		    
		// List of expand terms
		OmESet eset;
		ExpandDeciderOmega decider;
		    
		if (!rset->empty()) {
		    eset = enquire->get_eset(howmany, *rset, 0, &decider);
	    	} else if (mset.size()) {
		    // invent an rset
		    OmRSet tmp;
			
	    	    int c = 5;
    		    // FIXME: what if mset does not start at first match?
		    OmMSetIterator m = mset.begin();
		    for ( ; m != mset.end(); ++m) {
			tmp.add_document(*m);
			if (--c == 0) break;
		    }
			
		    eset = enquire->get_eset(howmany, tmp, 0, &decider);
		}
		    
		for (OmESetIterator i = eset.begin(); i != eset.end(); i++)
		    value = value + *i + '\t';
		if (!value.empty()) value.erase(value.size() - 1);
		break;
	    }
	    case CMD_url:
	        value = percent_encode(args[0]);
		break;
	    case CMD_version:
		value = PROGRAM_NAME" - "PACKAGE" "VERSION;
		break;
	    default:
		// FIXME: should never get here, so assert this?
		throw "Unknown function `" + var + "'";
	}
        res += value;
    }
	     
    res += fmt.substr(p);
    return res;
}

static string
eval_file(const string &fmtfile)
{    
    // don't allow ".." in format names as this would allow people to open
    // a format "../../etc/passwd" or similar
    // FIXME: make this check more exact ("foo..bar" is safe)
    // FIXME: log when this check fails
    string::size_type i = fmtfile.find("..");
    if (i == string::npos) {
	string file = template_dir + "/" + fmtfile;
	struct stat st;
	int fd = open(file.c_str(), O_RDONLY);
	if (fd >= 0) {
	    if (fstat(fd, &st) == 0 && S_ISREG(st.st_mode)) {
		char *blk;
		blk = (char*)malloc(st.st_size);
		if (blk) {
		    char *p = blk;
		    int len = st.st_size;
		    while (len) {
			int r = read(fd, p, len);
			if (r < 0) break;
			p += r;
			len -= r;
		    }
		    if (len == 0) {
			string fmt = string(blk, st.st_size);
			free(blk);			
			close(fd);
			string empty;
			return eval(fmt, empty);
		    }
		    free(blk);
		}
	    }
	    close(fd);
	}
    }

    // FIXME: report why!
    string err = string("Couldn't read format template `") + fmtfile + '\'';
    throw err;
}

/* return a sane (1-100) percentage value for ratio */
static int
percentage(double ratio)
{
    /* default to 100 so pure boolean queries give 100% not 0%) */
    long int percent = 100;
    percent = (long)(100.0 * ratio + 0.5);
    if (percent > 100) percent = 100;
    else if (percent < 1) percent = 1;
    return (int)percent;
}

static string
print_caption(om_docid m, const string &fmt, const string &loopvar)
{
    q0 = *(mset[m]);

    percent = mset.convert_to_percent(mset[m]);

    OmDocument doc = omdb->get_document(q0);
    OmData data = doc.get_data();
    string text = data.value;

    // parse record
    field.clear();
    std::string::size_type i = 0;
    while (1) {
	std::string::size_type old_i = i;
	i = text.find('\n', i);
	string line = text.substr(old_i, i - old_i);
	std::string::size_type j = line.find('=');
	if (j != string::npos) {
	    field[line.substr(0, j)] = line.substr(j + 1);
	} else if (!line.empty()) {
	    // FIXME: bodge for now
	    if (field["caption"].empty()) field["caption"] = line;
	    field["sample"] += line;
	}
	if (i == string::npos) break;
	i++;
    }

    return eval(fmt, loopvar);
}

om_doccount
do_match()
{
    cout << eval_file(fmtname);
    return mset.get_matches_estimated();
}

// run query if we haven't already
static void
ensure_match()
{			    
    if (done_query) return;
    
    run_query();
    done_query = true;
    // FIXME: size()?
    if (topdoc > mset.get_matches_lower_bound()) topdoc = 0;
    
    if (topdoc + hits_per_page < mset.get_matches_lower_bound())
	last = topdoc + hits_per_page;
    else
	last = mset.get_matches_lower_bound();
}
