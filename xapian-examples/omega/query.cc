/* query.cc: query executor for omega
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

#include <algorithm>
#include <vector>

#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include <time.h>

#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>

#include "omega.h"
#include "query.h"
#include "cgiparam.h"

static bool done_query = false;
static om_docid last = 0;

// declared in parsequery.ll
extern void parse_prob(const string&);

list<om_termname> new_terms_list;
static set<om_termname> new_terms;

static vector<OmQuery *> pluses;
static vector<OmQuery *> minuses;
static vector<OmQuery *> normals;

char *fmtstr =
"<TR><TD VALIGN=top><IMG "
"SRC=\"http://www.euroferret.com/fx-gif/score-$score.gif\" "
"ALT=\"$percentage%\" HEIGHT=35 WIDTH=35></TD>\n"
"<TD VALIGN=top><TABLE BORDER=0 CELLPADDING=1><TR><TD BGCOLOR=\"#ccffcc\">"
"<INPUT TYPE=checkbox NAME=R VALUE=$id$if{$relevant, CHECKED}>"
"</TD></TR></TABLE></TD>\n"
"<TD>\n"
"<B><A HREF=\"$field{url}\">"
"$html{$or{$field{caption},$field{url},Untitled}}"
"</A></B><BR>\n"
"$html{$field{sample}}$if{$field{sample},...}"
"<BR>\n"
"<A HREF=\"$field{url}\">$html{$field{url}}</A><br>\n"
//"<small>Language: <b>$html{$or{$field{language},unknown}}</b>\n"
//"Size: <b>$html{$or{$filesize{$field{size}},unknown}}</b>\n"		       
//"Last modified: <b>$html{$or{$date{$field{modified}},unknown}}</b><br>\n"
"$percentage% relevant, matching: <i>$list{$html{$terms},$. ,</i> and <i>}</i></small>\n"
"</TD></TR>\n";

char *pagefmtstr =
"$set{thousand,$.}$set{decimal,.}"
"<html>\n"
"<script language=javascript><!--\n"
"function C(c) {var i, o;\n"
"o = document.P.P.value;\n"
"if (c.checked){\n"
"document.P.P.value = o+\" \"+c.value;\n"
"} else {\n"
"o = \" \"+o+\" \";i = o.lastIndexOf(\" \"+c.value+\" \");\n"
"if (i!=-1) {\n"
"document.P.P.value =\n"
"o.substring(1,i)+o.substring(i+c.value.length+1,o.length-1);\n"
"}}}\n"
"// -->\n"
"</script>\n"
"<head>\n"
"<title>Omega: search for `$html{$query}'</title>\n"
"</head>\n"
"<body bgcolor=white>\n"
"<FORM NAME=P METHOD=GET "
"ACTION=\"$html{$or{$env{SCRIPT_NAME},ompf}}\" TARGET=\"_top\">\n"
"<center>\n"
"<INPUT NAME=P VALUE=\"$query\" SIZE=65>\n"
"<INPUT TYPE=IMAGE VALUE=\"Search\" BORDER=0 WIDTH=56 HEIGHT=56 ALIGN=middle "
"SRC=\"http://www.euroferret.com/fx-gif/find.gif\">\n"
"<table><tr><td bgcolor=\"#ccffcc\">\n"
"$topterms\n"
"$if{$topterms,<BR><NOSCRIPT><INPUT TYPE=hidden NAME=ADD VALUE=1></NOSCRIPT>}\n"
"</td></tr></table>\n"
//FIXME: uncomment once $freqs works "$list{$freqs,<hr>,$. ,<hr>}\n"
"<hr>\n"
"$ifeq{$msize,0,$if{$queryterms,No documents match your query}}"
"$ifneq{$msize,0,$if{$lastpage,$ifeq{$topdoc,0,All $msize matches}}}"
"$ifneq{$msize,0,$if{$lastpage,$ifneq{$topdoc,0,$add{$topdoc,1}-$msize of exactly $msize matches}}}"
"$ifneq{$msize,0,$ifnot{$lastpage,$add{$topdoc,1}-$add{$topdoc,$maxhits} of at least $msize matches}}"
"\n"
"</center>\n"
"<table>\n"
"$hitlist\n"
"</table>\n"
//\PAGES.G
// FIXME: what if multiple DB parameters?
"$if{$cgi{DB},<INPUT TYPE=hidden NAME=DB VALUE=\"$html{$cgi{DB}}\">}\n"
"$ifeq{$topdoc,0,<INPUT TYPE=hidden NAME=TOPDOC VALUE=$topdoc>}\n"
"$ifneq{$maxhits,10,<INPUT TYPE=hidden NAME=MAXHITS VALUE=$maxhits>}\n"
"$if{$fmt,<INPUT TYPE=hidden NAME=FMT VALUE=\"$html{$fmt}\">}\n"
"$list{$queryterms,<INPUT TYPE=hidden NAME=OLDP VALUE=\",.,.\">}\n"
"$list{$relevants,<INPUT TYPE=hidden NAME=R VALUE=,><INPUT TYPE=hidden NAME=R VALUE=,>}\n"
"</FORM>\n"
"<hr><div align=right><i><small>$html{$version}</small></i></div>\n"
"</body>\n"
"</html>\n";

// STATLINE -> $if{$or{$ifneq{$msize,0,ok},$queryterms},...}

// lookup in table (e.g. en -> English)

static void ensure_match();

string raw_prob;
map<om_docid, bool> ticked;

string query_string;

om_queryop op = OM_MOP_OR; // default matching mode

querytype
set_probabilistic(const string &newp, const string &oldp)
{
    int first_nonspace = 0;
    /* skip leading whitespace */
    while (isspace(newp[first_nonspace])) first_nonspace++;
    int len = newp.length();
    /* and strip trailing whitespace */
    while (len > first_nonspace && isspace(newp[len - 1])) len--;
    raw_prob = newp.substr(first_nonspace, len - first_nonspace);
    parse_prob(raw_prob);

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
    if (new_terms.size() < n_old_terms) return NEW_QUERY;
    
    while ((pend = strchr(term, '.')) != NULL) {
	if (new_terms.find(string(term, pend - term)) == new_terms.end())
	    return NEW_QUERY;
	term = pend + 1;
    }
    if (new_terms.size() > n_old_terms) return EXTENDED_QUERY;
    return SAME_QUERY;
}

static om_termpos querypos = 1;

/* if term is in the database, add it to the term list */
void check_term(const string &name, termtype type) {
    new_terms_list.push_back(name);
    new_terms.insert(name);
    OmQuery *q = new OmQuery(name, 1, querypos++);
    switch (type) {
     case PLUS:
	pluses.push_back(q);
	break;
     case MINUS:
	minuses.push_back(q);
	break;
     case NORMAL:
	normals.push_back(q);
	break;
    }
}

// FIXME: multimap for general use?
map<char, string> filter_map;

void add_bterm(const string &term) {
    filter_map[term[0]] = term;
}

/**************************************************************/
static void
run_query()
{
    OmQuery query(OM_MOP_AND_NOT,
		  OmQuery(OM_MOP_AND_MAYBE,
			  OmQuery(OM_MOP_AND,
				  pluses.begin(),
				  pluses.end()),
			  OmQuery(OM_MOP_OR,
				  normals.begin(),
				  normals.end())),
		  OmQuery(OM_MOP_OR,
			  minuses.begin(),
			  minuses.end()));

    // a vector is more convenient than a map for constructing
    // queries.
    vector<om_termname> filter_vec;
    for (map<char,string>::const_iterator ft = filter_map.begin();
	 ft != filter_map.end();
	 ++ft) {
	filter_vec.push_back(ft->second);
    }

    if (!filter_vec.empty()) {
	query = OmQuery(OM_MOP_FILTER,
			query,
			OmQuery(OM_MOP_AND,
				filter_vec.begin(),
				filter_vec.end()));
    }

    enquire->set_query(query);

    // We could use the value of topdoc as first parameter, but we
    // might need to know the first few items on the mset to fake
    // a relevance set for topterms
    mset = enquire->get_mset(0, topdoc + list_size, rset); // FIXME - set msetcmp to reverse
}

/* generate a sorted picker */
extern void
do_picker(char prefix, const char **opts)
{
    const char **p;
    char buf[16] = "BOOL-X";
    bool do_current = false;
    string current;
    vector<string> picker;
    
    map <char, string>::const_iterator i = filter_map.find(prefix);
    if (i != filter_map.end() && i->second.length() > 1) {
	current = i->second.substr(1);
	do_current = true;
    }

    cout << "<SELECT NAME=B>\n<OPTION VALUE=\"\"";

    if (!do_current) cout << " SELECTED";

    cout << '>';

    buf[5] = prefix;

    string tmp = option[buf];
    if (!tmp.empty())
	cout << tmp;
    else
	cout << "-Any-";
    
    for (p = opts; *p; p++) {
	strcpy(buf+6, *p);
	string trans = option[buf];
	if (trans.empty()) {
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
	size_t j = (*i2).find('\t');
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

extern void
print_page_links(char type, long int hits_per_page)
{
   int page;
   om_docid new_first;

   /* suppress page links if there's only one page */
   if (mset.mbound <= hits_per_page) return;

   if (type == 'T') {
      for (page = 0; page < 10; page++) {
	  new_first = page * hits_per_page;

	  if (new_first > mset.mbound - 1) break;
	  cout << "<INPUT TYPE=submit NAME=\"F " << new_first << "\" VALUE="
	       << page + 1 << ">\n";
      }
   } else {
      long int plh, plw, have_selected_page_gifs;
      // If not specified, don't default plh and plw since the page
      // gifs may not all be the same size
      plh = atoi(option["pagelink_height"].c_str());
      plw = atoi(option["pagelink_width"].c_str());
      have_selected_page_gifs = atoi(option["selected_pages"].c_str());

      for ( page = 0; page < 10; page++ ) {
	 new_first = page * hits_per_page;

	 if (new_first > mset.mbound - 1) break;

	 if (new_first == topdoc) {
	     cout << "<IMG SRC=\"" << option["gif_dir"] << "/page-"
		  << page + 1;
	     if (have_selected_page_gifs) cout << 's';
	     cout << ".gif\"";
	 } else {
	     cout << "<INPUT TYPE=image NAME=\"F " << new_first << "\" VALUE="
		  << page + 1 << " SRC=\"" << option["gif_dir"] << "/page-"
		  << page + 1 << ".gif\" BORDER=0";
	 }
	 if (plh) cout << " HEIGHT=" << plh;
	 if (plw) cout << " WIDTH=" << plw;
	 cout << '>';
      }
      cout << endl;
   }
}

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

string
html_escape(const string &str)
{
    string res;
    size_t p = 0;
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

static void print_query_string(const char *after) {		      
    if (after && strncmp(after, "&B=", 3) == 0) {
	char prefix = after[3];
	size_t start = 0, amp = 0;
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

static string
display_date(const string &date_string)
{
    char buf[64] = "";
    if (date_string.size()) {
	time_t date = atoi(date_string.c_str());
	if (date != (time_t)-1) {
	    struct tm *then;
	    then = gmtime(&date);
	    strftime(buf, sizeof buf, "%Y-%m-%d", then);
	}
    }
    return string(buf);
}

/* return a sane (1-100) percentage value for num/denom */
static int percentage(double num, double denom) {
    /* default to 100 so pure boolean queries give 100% not 0%) */
    long int percent = 100;
    if (denom) {
	percent = (long)( (100.0 * num) / denom + 0.5 );
	if (percent > 100) percent = 100;
	else if (percent < 1) percent = 1;
    }
    return (int)percent;
}

/* pretty print numbers with thousands separated */
/* NB only handles %ld and %d with no width or flag specifiers... */
static string
pretty_sprintf(const char *p, int *a)
{
    string res;
    char ch;    
    while ((ch = *p++)) {
	if (ch == '%') {
	    ch = *p++;
	    if (ch == 'l') ch = *p++;
	    if (ch == 'd') {
		char buf[16];
		char *q;
		int len;
		sprintf(buf, "%d", *a++);
		len = strlen(buf);
		q = buf;
		while ((ch = *q++)) {
		    res += ch;
		    if (--len && len % 3 == 0) res += option["thousand"];
		}
		continue;
	    }
	}
	res += ch;
    }
    return res;
}

static map<string, string> field;
static om_docid q0;
static int percent;

static string print_caption(om_docid m);

static bool relevant_cached = false;

static string
eval(const string &fmt)
{
    string res;
    size_t p = 0, q;
    while ((q = fmt.find('$', p)) != string::npos) {
	res += fmt.substr(p, q - p);
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
	if (!isalpha(fmt[q])) {
	    string msg = "Unknown $ code in: $" + fmt.substr(q);
	    throw msg;
	}
	p = fmt.find_first_not_of("ABCDEFGHIJKLMNOPQRSTUVWXYZ"
				  "abcdefghijklmnopqrstuvwxyz"
				  "0123456789_", q);
	if (p == string::npos) p = fmt.size();
	string var = fmt.substr(q, p - q);
	vector<string> args;
	if (fmt[p] == '{') {
	    p++;
	    q = p;
	    int nest = 1;
	    while (1) {
		p = fmt.find_first_of(",{}", p + 1);
		if (p == string::npos) throw "missing } in " + fmt;
		if (fmt[p] == '{') {
		    ++nest;
		} else {
		    if (nest == 1) {
			args.push_back(fmt.substr(q, p - q));
			q = p + 1;
		    }
		    if (fmt[p] == '}' && --nest == 0) break;
		}
	    }
	    p++;
	}

	bool ok = false;
	string value;
	char tmp[20];
	switch (var[0]) {
	 case 'a':
	    if (var == "add") {
		ok = true;
		unsigned int total = 0;
		vector<string>::const_iterator i;
		for (i = args.begin(); i != args.end(); i++) {
		    value = eval(*i);
		    unsigned int i = atoi(value.c_str());
		    total += i;
	        }
		sprintf(tmp, "%u", total);
		value = tmp;
		break;
	    }
	    break;
	 case 'c':
	    if (var == "cgi") {
		ok = true;
		MCI i = cgi_params.find(eval(args[0]));
		if (i != cgi_params.end()) value = i->second;
		break;
	    }
	    if (var == "cgilist") {
		ok = true;
		pair<MCI, MCI> g;
		g = cgi_params.equal_range(eval(args[0]));
		for (MCI i = g.first; i != g.second; i++)
		    value = value + i->second + '\t';
		if (value.size()) value.erase(value.size() - 1);
		break;
	    }
	    break;
	 case 'd':
	    if (var == "date") {
		ok = true;
		value = display_date(eval(args[0]));
		break;
	    }
	    break;
	 case 'e':
	    if (var == "env") {
		ok = true;
		var = eval(args[0]);
		char *env = getenv(var.c_str());
		if (env != NULL) value = env;
		break;
	    }
	    break;
	 case 'f':
	    if (var == "field") {
		ok = true;
		value = field[eval(args[0])];
		break;
	    }
	    if (var == "freqs") {
		ensure_match();
		ok = true;
		if (!new_terms_list.empty()) {
		    static string val;
		    if (val.size() == 0) {
			list<om_termname>::const_iterator i;
			for (i = new_terms_list.begin();
			     i != new_terms_list.end(); i++) {
			    int freq = 42; // FIXME: enquire->get_termfreq(*i);
			    val = val + *i + ":&nbsp;"
				+ pretty_sprintf("%d", &freq) + '\t';
			}		    
			if (val.size()) val.erase(val.size() - 1);
		    }
		    value = val;
		}
		break;
	    }
	    if (var == "filesize") {
		ok = true;
		// FIXME: rounding?
		// FIXME: for smaller sizes give decimal fractions, e.g. "1.4K"
		int size = atoi(eval(args[0]).c_str());
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
	    if (var == "fmt") {
		ok = true;
		value = fmtname;
		break;
	    }
	    break;
	 case 'h':
	    if (var == "html") {
		ok = true;
	        value = html_escape(eval(args[0]));
		break;
	    }
	    if (var == "hitlist") {
#if 0
		const char *q;
		int ch;
		
		query_string = "?DB=";
		query_string += db_name;
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
		/* add any boolean terms */
		map <char, string>::const_iterator i;			 
		for (i = filter_map.begin(); i != filter_map.end(); i++) {
		    query_string += "&B=";
		    query_string += i->second;
		}
		
		struct stat st;
		int fd = open(fmtfile.c_str(), O_RDONLY);
		if (fd >= 0) {
		    if (fstat(fd, &st) == 0 && st.st_size) {
			char *p;
			p = (char*)malloc(st.st_size + 1);
			if (p) {
			    if (read(fd, p, st.st_size) == st.st_size) {
				p[st.st_size] = '\0';
				fmtstr = p;
			    }
			}
		    }
		    close(fd);
		}
#endif
		ensure_match();		
		for (om_docid m = topdoc; m < last; m++)
		    value += print_caption(m);

		ok = true;
		break;
	    }
	    break;
	 case 'i':
	    if (var == "id") {
		// document id
		sprintf(tmp, "%u", q0);
		value = tmp;
		ok = true;
		break;
	    }
	    if (var == "if") {
		ok = true;
		if (eval(args[0]).size()) value = eval(args[1]);
		break;
	    }
	    if (var == "ifnot") {
		ok = true;
		if (eval(args[0]).size() == 0) value = eval(args[1]);
		break;
	    }
	    if (var == "ifeq") {
		ok = true;
		if (eval(args[0]) == eval(args[1]))
		    value = eval(args[2]);
		break;
	    }
	    if (var == "ifneq") {
		ok = true;
		if (eval(args[0]) != eval(args[1]))
		    value = eval(args[2]);
		break;
	    }
	    break;
	 case 'l':
	    if (var == "lastpage") {
		ensure_match();		
		// "true" if last page, empty otherwise
		if (last >= mset.mbound) value = "true";
		ok = true;
		break;
	    }
	    if (var == "list") {
		ok = true;
		string list = eval(args[0]);
		string pre, inter, interlast, post;
		switch (args.size()) {
		 case 2:
		    inter = interlast = eval(args[1]);
		    break;
		 case 3:
		    inter = eval(args[1]);
		    interlast = eval(args[2]);
		    break;
		 case 4:
		    pre = eval(args[1]);
		    inter = interlast = eval(args[2]);
		    post = eval(args[3]);
		    break;
		 case 5:
		    pre = eval(args[1]);
		    inter = eval(args[2]);
		    interlast = eval(args[3]);
		    post = eval(args[4]);
		    break;
		}
		if (list.size()) {
		    value += pre;
		    size_t split = 0, split2;
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
	    break;
	 case 'm':
	    if (var == "msize") {
		ensure_match();		
		// number of matches
		sprintf(tmp, "%u", mset.mbound);
		value = tmp;
		ok = true;
		break;
	    }
	    if (var == "maxhits") {
		sprintf(tmp, "%u", list_size);
		value = tmp;
		ok = true;
		break;
	    }
	    break;
	 case 'o':
	    if (var == "or") {
		ok = true;
		vector<string>::const_iterator i;
		for (i = args.begin(); i != args.end(); i++) {
		    value = eval(*i);
		    if (value.size()) break;
	        }
		break;
	    }
	    if (var == "opt") {
		ok = true;
		value = option[eval(args[0])];
		break;
	    }
	    break;
	 case 'p':
	    if (var == "percentage") {
		// percentage score
		sprintf(tmp, "%u", percent);
		value = tmp;
		ok = true;
		break;
	    }
	    break;
	 case 'q':
	    if (var == "query") {
		// query
		value = raw_prob;
		ok = true;
		break;
	    }
	    if (var == "queryterms") {
		ok = true;
		if (!new_terms_list.empty()) {
		    static string val;
		    if (val.size() == 0) {
			list<om_termname>::const_iterator i;
			for (i = new_terms_list.begin();
			     i != new_terms_list.end(); i++) {
			    val = val + *i + '\t';
			}		    
			if (val.size()) val.erase(val.size() - 1);
		    }
		    value = val;
		}
		break;
	    }
	    break;
	 case 'r':
	    if (var == "relevant") {
		ok = true;
		static string val;
		if (!relevant_cached) {
		    relevant_cached = true;
		    // document id if relevant; empty otherwise
		    if (ticked[q0]) {
			ticked[q0] = false; // icky side-effect
			sprintf(tmp, "%u", q0);
			val = tmp;
		    } else {
			val = "";
		    }
		}
		value = val;
		break;
	    }
	    if (var == "relevants") {
		ensure_match();		
		ok = true;
		map <om_docid, bool>::const_iterator i;
		for (i = ticked.begin(); i != ticked.end(); i++) {
		    if (i->second) {
			sprintf(tmp, "%u", i->first);
			value = value + tmp + '\t';
		    }
		}
		if (value.size()) value.erase(value.size() - 1);
		break;
	    }
	    break;
	 case 's':
	    if (var == "score") {
		// Score (0 to 10)
		sprintf(tmp, "%u", percent / 10);
		value = tmp;
		ok = true;
		break;
	    }
	    if (var == "set") {
		ok = true;
		option[eval(args[0])] = eval(args[1]);
		break;
	    }
	    break;
	 case 't':
	    if (var == "terms") {
		ensure_match();		
		// list of matching terms
		ok = true;
		om_termname_list matching = enquire->get_matching_terms(q0);
		list<om_termname>::const_iterator term;
		for (term = matching.begin(); term != matching.end(); term++)
		    value = value + *term + '\t';

		if (value.size()) value.erase(value.size() - 1);
		break;
	    }
	    if (var == "topdoc") {
		// first document on current page of hit list (counting from 0)
		ok = true;
		sprintf(tmp, "%u", topdoc);
		value = tmp;
		break;
	    }
	    if (var == "topterms") {
		ok = true;
		static string val;
		if (val.size() == 0) {
		    ensure_match();
		    
		    // Present a clickable list of expand terms
		    if (mset.mbound) {
			OmESet eset;
			ExpandDeciderOmega decider;
			
			if (rset->items.size()) {
			    eset = enquire->get_eset(20, *rset, 0, &decider);
			} else {
			    // invent an rset
			    OmRSet tmp;
			    
			    for (int m = min(4, int(mset.mbound) - 1); m >= 0; m--)
				tmp.add_document(mset.items[m].did);
			    
			    eset = enquire->get_eset(20, tmp, 0, &decider);
			}
		    
			vector<OmESetItem>::const_iterator i;
			for (i = eset.items.begin();
			     i != eset.items.end(); i++) {
			    string tname = i->tname;
			    val = val
				+ "<INPUT TYPE=checkbox NAME=X VALUE=\""
				+ tname + ".\" onClick=\"C(this)\">&nbsp;"
				+ tname + ". ";			     
			}
		    }
		}
		value = val;
		break;
	    }
	    break;
	 case 'u':
	    if (var == "url") {
		ok = true;
	        value = percent_encode(eval(args[0]));
		break;
	    }
	    break;
	 case 'v':
	    if (var == "version") {
		value = PROGRAM_NAME" - "PACKAGE" "VERSION;
		ok = true;
		break;
	    }
	    break;
	}
	if (!ok) throw "Unknown variable `" + var + "'";
        res += value;
    }
	     
    res += fmt.substr(p);
    return res;
}

static string
print_caption(om_docid m)
{
    relevant_cached = false;

    q0 = mset.items[m].did;
    percent = percentage((double)mset.items[m].wt, mset.max_possible);

    OmDocument doc = enquire->get_doc(q0);
    OmData data = doc.get_data();
    string text = data.value;

    // parse record
    field.clear();
    size_t i = 0;
    while (1) {
	size_t old_i = i;
	i = text.find('\n', i);
	string line = text.substr(old_i, i);
	size_t j = line.find('=');
	if (j != string::npos) {
	    field[line.substr(0, j)] = line.substr(j + 1);
	} else if (line != "") {
	    // FIXME: bodge for now
	    if (field["caption"].size() == 0) field["caption"] = line;
	    field["sample"] += line;
	}
	if (i == string::npos) break;
	i++;
    }

    return eval(fmtstr);
}

#if 0
\\PAGES.[A-Z] { // T for text, G for graphical
    print_page_links(yytext[7], size);
}
\\STATLINE {
    if (mset.mbound == 0 && new_terms_list.empty()) {
	// eat to next newline (or EOF)
	int c;
	do c = yyinput(); while (c != '\n' && c != EOF);
    }
}
\\MAXHITS[0-9]+ {
    // item in max hits selector box */
    // expect 3 digits in FX, but allow more or less
    string num = string(yytext + 8, yyleng - 8);
    if (size == atoi(num.c_str())) cout << "SELECTED";
}
\\OSELECT[AO] {
    if ((op == OM_MOP_AND) ^! (yytext[8] == 'A')) cout << "SELECTED";
}
	  case 'Q': /* query url */
	     print_query_string(q + 2);
	     break;
	 case 'l': /* language (with link unless "unknown") */
	    if (language_code == "x") {
		cout << language;
	    } else {
		cout << "<A HREF=\"/";
		print_query_string("&B=L");
		cout << "&B=L" << language_code << "\">" << language
		    << "</A>";
	    }
	    break;
#endif

static void
print_query_page(const string &page)
{
    // FIXME: read fmt from file...
    cout << eval(pagefmtstr);
}

om_doccount
do_match()
{
    print_query_page("query");
    return mset.mbound;
}

// run query if we haven't already
static void
ensure_match()
{			    
    if (done_query) return;
    
    run_query();
    done_query = true;
    if (topdoc > mset.mbound) topdoc = 0;
    
    if (topdoc + list_size < mset.mbound)
	last = topdoc + list_size;
    else
	last = mset.mbound;
}
