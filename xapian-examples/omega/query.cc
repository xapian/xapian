/* query.cc: query executor for ferretfx
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

#include "main.h"
#include "query.h"

/* limit on mset size (as given in espec) */
#define MLIMIT 1000 // FIXME: deeply broken

static bool done_query = false;
static om_docid last = 0;

// declared in parsequery.ll
extern void parse_prob(const string&);

list<om_termname> new_terms_list;
static set<om_termname> new_terms;

static vector<om_termname> pluses;
static vector<om_termname> minuses;
static vector<om_termname> normals;

char *fmtstr =
"<TR><TD VALIGN=top><IMG "
"SRC=\"http://www.euroferret.com/fx-gif/score-$score.gif\" "
"ALT=\"$percentage%\" HEIGHT=35 WIDTH=35></TD>\n"
"<TD VALIGN=top><TABLE BORDER=0 CELLPADDING=1><TR><TD BGCOLOR=\"#ccffcc\">"
"<INPUT TYPE=checkbox NAME=R VALUE=$id$if{$relevant, CHECKED}>"
"</TD></TR></TABLE></TD>\n"
"<TD>\n"
"<B><A HREF=\"$url{$field{url}}\">"
"$html{$or{$field{caption},$field{url}}}"
"</A></B><BR>\n"
"$html{$field{sample}}$if{$field{sample},...}"
"<BR>\n"
"<A HREF=\"$url{$field{url}}\">$html{$field{url}}</A><BR>\n"
"<small>Language: <b>$html{$or{$field{language},unknown}}</b>\n"
"Size: <b>"
"$html{$or{$filesize{$field{size}},unknown}}"
"</b>\n"		       
"Last modified: <b>$html{$or{$date{$field{modified}},unknown}}</b>\n"
"<br>$percentage% relevant, matching: <i>$html{$terms}</i></small>\n"
"</TD></TR>\n";

char *pagefmtstr =
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
"<title>Search: $html{$query}</title>\n"
"</head>\n"
"<body bgcolor=white>\n"
"<FORM NAME=P METHOD=GET "
"ACTION=\"$html{$or{$env{SCRIPT_NAME},ompf}}\" TARGET=\"_top\">\n"
"<center>\n"
"<INPUT NAME=P VALUE=\"$query\" SIZE=65>\n"
"<INPUT TYPE=IMAGE VALUE=\"Search\" BORDER=0 WIDTH=56 HEIGHT=56 ALIGN=middle "
"SRC=\"http://www.euroferret.com/fx-gif/find.gif\">\n"
"</center>\n"
"<table><tr><td bgcolor=\"#ccffcc\">\n"
"$topterms\n"
"$if{$topterms,<BR><NOSCRIPT><INPUT TYPE=hidden NAME=ADD VALUE=1></NOSCRIPT>}\n"
"</td></tr></table>\n"
"<table>\n"
"$hitlist\n"
"</table>\n"
//\PAGES.G
"$save\n"
"</FORM>\n"
"</body>\n"
"</html>\n";

// lookup in table (e.g. en -> English)

static void ensure_match();

string raw_prob;
om_docid msize = 0;
map<om_docid, bool> ticked;

string gif_dir = "/fx-gif";

string thou_sep = ",", dec_sep = ".";

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

/* if term is in the database, add it to the term list */
void check_term(const string &name, termtype type) {
    new_terms_list.push_back(name);
    new_terms.insert(name);
    switch (type) {
     case PLUS:
	pluses.push_back(name);
	break;
     case MINUS:
	minuses.push_back(name);
	break;
     case NORMAL:
	normals.push_back(name);
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
    msize = mset.mbound;
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
   if (msize <= hits_per_page) return;

   if (type == 'T') {
      for (page = 0; page < 10; page++) {
	  new_first = page * hits_per_page;

	  if (new_first > msize - 1) break;
	  cout << "<INPUT TYPE=submit NAME=F" << new_first << " VALUE="
	       << page + 1 << ">\n";
      }
   } else {
      long int plh, plw, have_selected_page_gifs;
      // If not specified, don't default plh and plw since the page
      // gifs may not all be the same size
      plh = atoi(option["pagelink_height"].c_str());
      plw = atoi(option["pagelink_width"].c_str());
      have_selected_page_gifs = atoi(option["pagelink_width"].c_str());

      for ( page = 0; page < 10; page++ ) {
	 new_first = page * hits_per_page;

	 if (new_first > msize - 1) break;

	 if (new_first == topdoc) {
	     cout << "<IMG SRC=\"" << gif_dir << "/page-"
		  << page + 1;
	     if (have_selected_page_gifs) cout << 's';
	     cout << ".gif\"";
	 } else {
	     cout << "<INPUT TYPE=image NAME=F" << new_first << " VALUE="
		  << page + 1 << " SRC=\"" << gif_dir << "/page-"
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
		    if (--len && len % 3 == 0) res += thou_sep;
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
print_fmt(const string &fmt)
{
    string res;
    size_t p = 0, q;
    while ((q = fmt.find('$', p)) != string::npos) {
	res += fmt.substr(p, q - p);
	q++;
	if (q >= fmt.size()) break;
	if (fmt[q] == '$') {
	    // $$ -> output a literal $
	    res += '$';
	    p = q + 1;
	    continue;
	}
	string var, arg;
	if (!isalpha(fmt[q])) {
	    string msg = "Unknown $ code in: $" + fmt.substr(q);
	    throw msg;
	}
	p = fmt.find_first_not_of("ABCDEFGHIJKLMNOPQRSTUVWXYZ"
				  "abcdefghijklmnopqrstuvwxyz"
				  "0123456789_", q);
	if (p == string::npos) p = fmt.size();
	var = fmt.substr(q, p - q);
	if (fmt[p] == '{') {
	    p++;
	    q = p;
	    int nest = 1;
	    while (1) {
		p = fmt.find_first_of("{}", p + 1);
		if (p == string::npos) throw "missing }";
		if (fmt[p] == '{') {
		    ++nest;
		} else {
		    if (--nest == 0) break;
		}
	    }
	    arg = fmt.substr(q, p - q);
	    p++;
	}

	bool ok = false;
	string value;
	char tmp[20];
	switch (var[0]) {
	 case 'd':
	    if (var == "date") {
		ok = true;
		value = display_date(print_fmt(arg));
		break;
	    }
	    break;
	 case 'e':
	    if (var == "env") {
		ok = true;
		var = print_fmt(arg);
		char *env = getenv(var.c_str());
		if (env != NULL) value = env;
		break;
	    }
	    break;
	 case 'f':
	    if (var == "field") {
		ok = true;
		var = print_fmt(arg);
		value = field[var];
		break;
	    }
	    if (var == "freqs") {
		ok = true;
		if (!new_terms_list.empty()) {
		    static string val;
		    if (val.size() == 0) {
			list<om_termname>::const_iterator i;
			for (i = new_terms_list.begin();
			     i != new_terms_list.end(); i++) {
			    const char *term = i->c_str();
			    
			    int freq = 42; // FIXME: enquire->get_termfreq(*i);
			    
			    if (i != new_terms_list.begin()) val += ", ";
			    if (strchr(term, ' ')) {
				val = val + "\"" + term + "\":&nbsp;";
			    } else {
				val = val + term + ":&nbsp;";
			    }
			    val += pretty_sprintf("%d", &freq);
			}		    
		    }
		    value = val;
		}
		break;
	    }
	    if (var == "filesize") {
		ok = true;
		// FIXME: rounding?
		// FIXME: for smaller sizes give decimal fractions, e.g. "1.4K"
		int size = atoi(print_fmt(arg).c_str());
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
	    break;
	 case 'h':
	    if (var == "html") {
		ok = true;
	        value = html_escape(print_fmt(arg));
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
		for (om_docid m = topdoc; m <= last; m++)
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
		size_t comma = arg.find(',');
		if (comma == string::npos) throw "missing ,";
		if (print_fmt(arg.substr(0, comma)).size())
		    value = print_fmt(arg.substr(comma + 1));
		ok = true;
		break;
	    }
	    if (var == "ifnot") {
		size_t comma = arg.find(',');
		if (comma == string::npos) throw "missing ,";
		if (print_fmt(arg.substr(0, comma)).size() == 0)
		    value = print_fmt(arg.substr(comma + 1));
		ok = true;
		break;
	    }
	    break;
	 case 'l':
	    if (var == "lastpage") {
		// "true" if last page, empty otherwise
		if (last >= msize - 1) value = "true";
		ok = true;
		break;
	    }
	    break;
	 case 'm':
	    if (var == "msize") {
		// number of matches
		sprintf(tmp, "%u", msize);
		value = tmp;
		ok = true;
		break;
	    }
	    break;
	 case 'o':
	    if (var == "or") {
		while (arg.size()) {
		    size_t comma = arg.find(',');
		    value = print_fmt(arg.substr(0, comma));
		    if (value.size()) break;
		    arg = arg.substr(comma + 1);
	        }
		ok = true;
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
	    break;
	 case 's':
	    if (var == "score") {
		// Score (0 to 10)
		sprintf(tmp, "%u", percent / 10);
		value = tmp;
		ok = true;
		break;
	    }
	    if (var == "save") {
		ok = true;
		char buf[20];
		/*** save DB name **/
		if (db_name != default_db_name)
		    value = value
		    + "<INPUT TYPE=hidden NAME=DB VALUE=\"" + db_name
		    + "\">\n";
		
		/*** save top doc no. ***/
		if (topdoc != 0) {
		    sprintf(buf, "%u", topdoc);
		    value = value
			+ "<INPUT TYPE=hidden NAME=TOPDOC VALUE=" + buf
			+ ">\n";
		}
		
		/*** save maxhits ***/
		if (list_size != 10) {
		    sprintf(buf, "%u", list_size);
		    value = value
			+ "<INPUT TYPE=hidden NAME=MAXHITS VALUE=" + buf
			+ ">\n";
		}
		
		/*** save fmt ***/
//		if (!fmt.empty())
//		    value = value + "<INPUT TYPE=hidden NAME=FMT VALUE=\"" + fmt + "\">\n";
		
		/*** save prob query ***/
		if (!new_terms_list.empty()) {
		    value += "<INPUT TYPE=hidden NAME=OLDP VALUE=\"";
		    list<om_termname>::const_iterator i;
		    for (i = new_terms_list.begin(); i != new_terms_list.end(); i++)
			value += *i + '.';
		    value += "\">\n";
		}
		
		// save ticked documents which don't have checkboxes displayed
		map <om_docid, bool>::const_iterator i;
		for (i = ticked.begin(); i != ticked.end(); i++) {
		    if (i->second) {
			sprintf(buf, "%u", i->first);
			value = value
			    + "<INPUT TYPE=hidden NAME=R VALUE=" + buf + ">\n";
		    }
		}
		break;
	    }
	    break;
	 case 't':
	    if (var == "terms") {
		// list of matching terms
		om_termname_list matching_terms = enquire->get_matching_terms(q0);
		list<om_termname>::const_iterator term = matching_terms.begin();
		bool comma = false;
		while (term != matching_terms.end()) {
		    if (comma)
			value += ' ';
		    else
			comma = true;
		    // quote terms with spaces in
		    if (term->find(' ') != string::npos)
			value += '"' + *term + '"';
		    else
			value += *term;
		    term++;
		}
		ok = true;
		break;
	    }
	    if (var == "topdoc") {
		// first document on current page of hit list
		// (counting from 0) or empty if on first page
		if (topdoc != 0) {
		    sprintf(tmp, "%u", topdoc);
		    value = tmp;
		}
		ok = true;
		break;
	    }
	    if (var == "topterms") {
		ok = true;
		static string val;
		if (val.size() == 0) {
		    ensure_match();
		    
		    // Present a clickable list of expand terms
		    if (msize) {
			OmESet eset;
			ExpandDeciderFerret decider;
			
			if (rset->items.size()) {
			    eset = enquire->get_eset(20, *rset, 0, &decider);
			} else {
			    // invent an rset
			    OmRSet tmp;
			    
			    for (int m = min(4, int(msize) - 1); m >= 0; m--)
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
	        value = percent_encode(print_fmt(arg));
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
	} else if (line != "" && line != "\n") {
	    // FIXME
	    if (field["caption"].size() == 0) field["caption"] = line;
	    field["sample"] += line;
	}
	if (i == string::npos) break;
	i++;
    }

    return print_fmt(fmtstr);
}

#if 0
\$\(GIF_DIR\) {
    cout << gif_dir;
}
\\PAGES.[A-Z] { // T for text, G for graphical
    print_page_links(yytext[7], size);
}
\\STATLINE {
    if (msize == 0 && new_terms_list.empty()) {
	// eat to next newline (or EOF)
	int c;
	do c = yyinput(); while (c != '\n' && c != EOF);
    }
}
\\STAT[02as].*$ {
    int arg[3];
    bool print = false;
    arg[0] = topdoc + 1;
    arg[1] = last + 1;
    arg[2] = msize;
    string text = string(yytext + 6, yyleng - 6);
    /* We're doing:
     * \STAT0 none
     * \STAT2 returning some matches from over n
     * \STATa returning all matches from n
     * \STATs returning some (but not all) matches from n
     * \STATLINE like \HITLINE but enabled when any one of the
     *        \STATx codes fires
     */
    switch (yytext[5]) {
     case '0': /* followed by string */
	if (msize == 0 && !new_terms_list.empty()) cout << text;
	break;
     case '2':
	/* used to be >= - now use an exact compare since MTOTAL
	 * may have given us the full figure.  If MTOTAL works and
	 * we got exactly MLIMIT hits, this will misreport... */
	if (msize == MLIMIT) print = true;
	break;
     case 'a':
	/* used to be < MLIMIT - now use an exact compare since MTOTAL
	 * may have given us the full figure.  If MTOTAL works and
	 * we got exactly MLIMIT hits, this will misreport... */
	/* FIXME: could use Mike Gatford's "1001" trick */
	if (0 < msize && msize != MLIMIT &&
	    (topdoc == 0 && last + 1 == msize)) {	       
	    arg[0] = msize;
	    print = true;
	}
	break;
     case 's':
	/* used to be < MLIMIT - now use an exact compare since MTOTAL
	 * may have given us the full figure.  If MTOTAL works and
	 * we got exactly MLIMIT hits, this will misreport... */
	/* FIXME: could use Mike Gatford's "1001" trick */
	if (0 < msize && msize != MLIMIT &&
	    !(topdoc == 0 && last + 1 == msize)) print = true;
	break;
    }
    if (print) cout << pretty_sprintf(text.c_str(), arg);
}
\\HITLINE {
    if (!msize) {
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
    last = 0;
    string tmp = option["gif_dir"];
    if (tmp != "") gif_dir = tmp;
    cout << print_fmt(pagefmtstr);
}

om_doccount
do_match()
{
    print_query_page("query");
    return msize;
}

// run query if we haven't already
static void
ensure_match()
{			    
    if (done_query) return;
    
    run_query();
    done_query = true;
    if (topdoc > msize) topdoc = 0;
    
    if (topdoc + list_size < msize)
	last = topdoc + list_size - 1;
    else
	last = msize - 1;
}
