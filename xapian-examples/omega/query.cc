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

// declared in parsepage.ll
extern void print_query_page(const string &, long int, long int);
// declared in parsequery.ll
extern void parse_prob(const string&);

list<om_termname> new_terms_list;
static set<om_termname> new_terms;

static vector<om_termname> pluses;
static vector<om_termname> minuses;
static vector<om_termname> normals;

char *fmtstr =
"<TR><TD VALIGN=top><IMG SRC=\"ÿG\" ALT=\"ÿP\" HEIGHT=35 WIDTH=35></TD>\n"
"<TD VALIGN=top><TABLE BORDER=0 CELLPADDING=1><TR><TD BGCOLOR=\"#ccffcc\">ÿX</TD></TR></TABLE></TD>\n"
"<TD>\n"
"<B><A HREF=\"ÿU\">ÿC</A></B><BR>\n"
"ÿS<BR>\n"
"<A HREF=\"ÿU\">ÿU</A><BR>\n"
"<small>Language: <b>ÿL</b>\n"
"Size: <b>ÿs</b>\n"		       
"Last modified: <b>ÿM</b>\n"
"<br>ÿP relevant, matching: <i>ÿT</i></small>\n"
"</TD></TR>\n";

string raw_prob;
long int msize = -1;
map<om_docid, bool> ticked;

string gif_dir = "/fx-gif";

char thou_sep = ',', dec_sep = '.';

#ifdef FERRET
string ad_keywords;
#endif

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
extern void
run_query(om_doccount first, om_doccount maxhits)
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

    cout << "Running query: maxmsize = " << first + maxhits << "; " << endl;
    // We could use the value of first as first parameter, but we
    // might need to know the first few items on the mset to fake
    // a relevance set for topterms
    mset = enquire->get_mset(0, first + maxhits, rset); // FIXME - set msetcmp to reverse
    msize = mset.mbound;
    cout << "Ran query: msize = " << msize << "; " << endl;
}

long
do_match(long int first_hit, long int list_size)
{
    print_query_page("query", first_hit, list_size);
    return msize;
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
print_page_links(char type, long int hits_per_page, long int topdoc)
{
   int page;
   long int new_first;

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
      // If not specified, plh and plw no longer default to 15
      // since that prevents the user having different sized page gifs
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

void html_escape(const string &str) {
    const char *p = str.c_str();
    while (1) {
	char ch = *p++;
	switch (ch) {
	    case '\0':
	        return;
 	    case '<':
	        cout << "&lt;";
	        continue;
	    case '>':
	        cout << "&gt;";
	        continue;
	    case '&':
	        cout << "&amp;";
	        continue;
	    default:
	        cout << ch;
	}
    }
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

static void display_date(time_t date) {
   if (date == (time_t)-1) {
       cout << "Unknown";
   } else {
       char buf[64];
       struct tm *then;
       then = gmtime(&date);
       strftime(buf, sizeof buf, "%Y-%m-%d", then);
       cout << buf;
   }
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

extern void
print_caption(long int m)
{
    long int q0 = 0;
    int percent;
    long int wt = 0; 
    time_t lastmod = -1;
    string hostname = "localhost"; /* erk */
    string country;
    string country_code = "x";
    string language;
    string language_code = "x";

    wt = static_cast<long int>(mset.items[m].wt);
    q0 = mset.items[m].did;

#if 0 // FIXME: need access to TermList...
    /* get hostname from longest N tag
     * and country from shortest (allowing for uk+) */
    int len = -1, got_plus = 0;
    TermList *terms = database->open_term_list(q0);
    terms->next();
    while (!terms->at_end()) {
	string term = terms->get_termname();
	int length = term.length();
	switch (term[0]) {
	 case 'N':
	    if (length > len && term[length - 1] != '+') {
		hostname = term.substr(1);
		len = length;
	    }
	    if (!got_plus && length - 1 <= 3) {
		country_code = term.substr(1);
		if (term[length - 1] == '+') {
		    got_plus = 1;
		    country_code = term.substr(1, length - 2) + "%2b";
		}
	    }
	    break;
	 case 'L':
	    language_code = term.substr(1);
	    break;
	}
	terms->next();
    }
    delete terms;
#endif

    country = option["BOOL-N" + country_code];
    if (country.empty()) country = country_code;
    language = option["BOOL-L" + language_code];
    if (language.empty()) language = language_code;
   
    percent = percentage((double)wt, mset.max_possible);
    
    string path, sample, caption;
    int port = -1;
    const char *pp;
    unsigned const char *u;
    int size = -1;

    OmDocument doc = enquire->get_doc(q0);
    OmData data = doc.get_data();
    pp = data.value.c_str() + 14;

    u = (const unsigned char *)pp;
    lastmod = (((unsigned char)(u[0] - 33) * 223) + (u[1] - 33)) * 86400;
    pp += 2;
    size = u[2];
    pp++;
       
    if (*pp == '/') {
	char *q;
	/* / means we have a port number like so "/8080/index.html" */
	port = strtol(pp + 1, &q, 10);
	if (q) pp = q;
	pp++;
    }

    const char *find_fe = strchr(pp, '\xfe');
    if (find_fe) {
	path = string(pp, find_fe - pp);
	find_fe++;
	if (*find_fe != '\xfe' && *find_fe != '\0') {
	    const char *q = strchr(find_fe, '\xfe');
	    if (!q) {
		caption = string(find_fe);
	    } else {
		caption = string(find_fe, q - find_fe);
		sample = string(q + 1);
	    }
	}
    }

    char *p, *q;
    p = fmtstr;
    while ((q = strchr(p, '\xff')) != NULL) {
	cout << string(p, q - p);
	switch (q[1]) {
	 case 'C': /* caption */
	    if (!caption.empty()) {
		html_escape(caption);
		break;
	    }
	    /* otherwise fall through... */
	 case 'U': /* url */
	    cout << "http://" << hostname;
	    if (port >= 0) cout << ':' << port;
	    cout << '/' << path;
	    break;
	 case 'H': /* host */
	    cout << hostname;
	    break;
	 case 'd': /* DB name */
	    cout << db_name;
	    break;
	 case 'Q': /* query url */
	    print_query_string(q + 2);
	    break;
	 case 'S': /* sample */
	    if (!sample.empty()) {
		html_escape(sample);
		cout << "...";
	    }
	    break;
	 case 's': /* size */
	    /* decode packed file size */
	    if (size < 33) {
		cout << "Unknown";
	    } else if (size < 132) {
		size -= 32;
		cout << size / 10 << dec_sep << size % 10 << 'K';
	    } else if (size < 222) {
		cout << size - 132 + 10 << 'K';
	    } else if (size < 243) {
		cout << size - 222 + 10 << "0K";
	    } else if (size < 250) {
		cout << '0' << dec_sep << size - 243 + 3 << 'K';
	    } else if (size < 255) {
		cout << size - 249 << 'M';
	    } else {
		cout << ">5M";
	    }
	    break;
	 case 'I': /* document id */
	    cout << q0;
	    break;
	 case 'L': /* language */
	    cout << language;
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
	 case 'W': /* country */
	    cout << country;
	    break;
	 case 'w': /* country code */
	    cout << country_code;
	    break;
	 case 'M': /* last modified */
	    display_date(lastmod);
	    break;
#if 0 // FIXME:
	 case 'V': /* date last visited */
	    display_date(dadate);
	    break;
#endif
	 case 'P':
	    cout << percent << '%';
	    break;
	 case 'T': {
	     om_termname_list matching_terms = enquire->get_matching_terms(q0);
	     list<om_termname>::const_iterator term = matching_terms.begin();
	     bool comma = false;
	     while (term != matching_terms.end()) {
		 if (comma) cout << ' '; else comma = true;
		 /* quote terms with spaces in */
		 if (term->find(' ') != string::npos)
		     cout << '"' << *term << '"';
		 else
		     cout << *term;
		 term++;
	     }
	     break;
	  }
	  case 'G': /* score Gif */
	     cout << "/fx-gif/score-" << percent / 10 << ".gif";
	     break;
	  case 'X': /* relevance checkboX */
	     if (ticked[q0]) {
		 ticked[q0] = false;
		 cout << "<INPUT TYPE=checkbox NAME=R VALUE=" << q0 << " CHECKED>\n";
	     } else {
		 cout << "<INPUT TYPE=checkbox NAME=R VALUE=" << q0 << ">\n";
	     }
	     break;
	  default:
	     cout << '\xff' << q[1];
	     break;
	 }
	 p = q + 2;
     }
     cout << p;
}
