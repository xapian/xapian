/* query.cc: query executor for ferretfx
 *
 * ----START-LICENCE----
 * Copyright 1999 Dialog Corporation
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

vector<om_termname> new_terms;
static vector<om_termname> pluses;
static vector<om_termname> minuses;
static vector<om_termname> normals;

#ifdef META
char *fmtstr = "ÿP\tÿU\tÿC\tÿS\tÿL\tÿW\tÿH\tÿs\tÿM\tÿT\n";
#else
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
#endif

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

map<om_termname, int> matching_map;

int
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
    // Heuristic: If any words have been removed, it's a "fresh query"
    // which means we discard any relevance judgements
    string oldterm;
    const char *pend;
    const char *term;
    unsigned int n_old_terms = 0; // ?

    if (oldp.empty()) return 0;

    term = oldp.c_str();
    // We used to use "word1#word2#" (with trailing #) but some broken old
    // browsers (versions of MSIE) don't quote # in form GET submissions
    // and everything after the # gets taken as an anchor.
    // So now we use "word1.word2." instead.
    
    pend = term;
    while ((pend = strchr(pend, '.')) != NULL) {
	pend++;
	n_old_terms++;
    }
    // short-cut: if the new query has fewer terms, it must be a new one
    if (new_terms.size() < n_old_terms) return 0;
    
    vector<om_termname>::const_iterator i = new_terms.begin();
    int is_old = 1;
    while ((pend = strchr(term, '.')) != NULL) {
	oldterm = string(term, pend - term);
	while (oldterm != *i) {
	    if (++i == new_terms.end()) {
		is_old = 0;
		break;
	    }
	}
	if (!is_old) break;
	term = pend + 1;
    }
    // return:
    // 0 entirely new query
    // 1 unchanged query
    // -1 new query, but based on the old one
    if (is_old && new_terms.size() > n_old_terms) return -1;
    return is_old;
}

static int term_count = 1; // FIXME: ick

/* if term is in the database, add it to the term list */
void check_term(const string &name, termtype type) {
    new_terms.push_back(name);
    switch (type) {
     case PLUS:
	pluses.push_back(name);
	matching_map[name] = term_count++;
	break;
     case MINUS:
	minuses.push_back(name);
	// don't put MINUS terms in map - they won't match...
	break;
     case NORMAL:
	normals.push_back(name);
	matching_map[name] = term_count++;
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
    if (!new_terms.empty()) {
	// now we constuct the query:
	// ((plusterm_1 AND ... AND plusterm_n) ANDMAYBE
	//  (term_1 OR ... OR term_m)) ANDNOT
	// (minusterm_1 OR ... OR minusterm_p)
	if (!pluses.empty()) matcher->add_oplist(MOP_AND, pluses);
	if (!normals.empty()) {
	    matcher->add_oplist(op, normals);
	    if (!pluses.empty()) matcher->add_op(MOP_AND_MAYBE);
	}       
	if (!minuses.empty()) {
	    matcher->add_oplist(MOP_OR, minuses);
	    if (!matcher->add_op(MOP_AND_NOT)) {
		cout << "You must allow at least one of the search terms\n" << endl; // FIXME
		exit(0);
	    }
	}
    }
    int bool_terms = 0;
    // add any boolean terms and AND them together
    // FIXME: should OR those with same prefix...
    map <char, string>::const_iterator i;
    for (i = filter_map.begin(); i != filter_map.end(); i++) {
        matcher->add_term(i->second);
	bool_terms++;
	if (bool_terms) matcher->add_op(MOP_AND);
    }
    if (bool_terms) matcher->add_op(MOP_FILTER);

    doccount mtotal;
    cout << "Running query: maxmsize = " << first + maxhits << "; " << endl;
    // FIXME: use the value of first as first parameter: don't bother sorting
    // first ``first'' items (but don't get given them either, so need to
    // alter code elsewhere to understand this)
    enquire->set_rset(rset);
    enquire->get_mset(mset, 0, first + maxhits); // FIXME - set msetcmp to reverse
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

static void utf8_to_html(const string &str) {
   const unsigned char *p = (const unsigned char *)str.c_str();
   while (1) {
      int ch = *p++;
      if (ch == 0) break;

      // this is some extra magic, not part of utf-8
      switch (ch) {
       case '\b': // was \r but muscat 3.6 swallows that...
	  cout << " / "; // line break in original
	  continue;
       case '\t':
	  cout << " * "; // bullet point in original
	  continue;
       case '\f':
	  cout << "&nbsp;"; // hardspace in original
	  continue;
       case '<':
	  cout << "&lt;";
	  continue;
       case '>':
	  cout << "&gt;";
	  continue;
       case '&':
	  cout << "&amp;";
	  continue;
      }

      /* A byte in the range 0x80-0xbf or 0xf0-0xff isn't valid in this state,
       * (0xf0-0xfd mean values > 0xffff) so if we encounter one, treat it as
       * literal and try to resync so we cope better when fed non-utf-8 data.
       * Similarly we abandon a multibyte sequence if we hit an invalid
       * character */
      if (ch >= 0xc0 && ch < 0xf0) {
	 int ch1 = *p;
	 if ((ch1 & 0xc0) != 0x80) goto resync;
	 
	 if (ch < 0xe0) {
	    /* 2 byte sequence */
	    ch = ((ch & 0x1f) << 6) | (ch1 & 0x3f);
	    p++;
	 } else {
	    /* 3 byte sequence */
	    int ch2 = p[1];
	    if ((ch2 & 0xc0) != 0x80) goto resync;
	    ch = ((ch & 0x1f) << 12) | ((ch1 & 0x3f) << 6) | (ch2 & 0x3f);
	    p += 2;
	 }
	 if (ch > 255) {
	    cout << "&#" << ch << ";";
	    continue;
	 }
      }
      resync:
      cout << char(ch);
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

class MatchingTermCmp {
    public:
	bool operator()(const termname &a, const termname &b) {
	    if(matching_map.find(a) != matching_map.end() &&
	       matching_map.find(b) != matching_map.end()) {
		return matching_map[a] < matching_map[b];
	    }
	    return a < b;
	}
};

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

    wt = (long int)mset[m].wt;
    q0 = mset[m].did;
    
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
    
    country = option["BOOL-N" + country_code];
    if (country.empty()) country = country_code;
    language = option["BOOL-L" + language_code];
    if (language.empty()) language = language_code;
   
    percent = percentage((double)wt, matcher->get_max_weight());
    
    string path, sample, caption;
    int port = -1;
    const char *pp;
    unsigned const char *u;
    int size = -1;
    int dadate = -1;

#if 0 // FIXME
    // get datestamp of DA file this record is in so we can handle
    // different versions of our record format
    if (q0 >= 0) {
	int n = q0 / 10000000;
	if (n < n_dlist) dadate = dlist[n];
    }
#endif

    IRDocument *doc = database->open_document(q0);
    IRData data = doc->get_data();
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
		utf8_to_html(caption);
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
		utf8_to_html(sample);
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
	 case 'V': /* date last visited */
	    display_date(dadate);
	    break;
	 case 'P':
	    cout << percent << '%';
	    break;
	 case 'T': {
	     // Store the matching terms in a vector and then sort by the
	     // value of matching_map[] so that they come back in the same
	     // order as in the query.
	     vector<termname> matching_terms;
	     TermList *terms = database->open_term_list(q0);
	     terms->next();
	     while (!terms->at_end()) {
		 termname term = terms->get_termname();
		 map<termname, int>::const_iterator i = matching_map.find(term);
		 if (i != matching_map.end()) {
		     matching_terms.push_back(term);
		 }
		 terms->next();
	     }
	     delete terms;

	     sort(matching_terms.begin(),
		  matching_terms.end(),
		  MatchingTermCmp());

	     vector<termname>::const_iterator term = matching_terms.begin();
	     bool comma = false;
	     while (term != matching_terms.end()) {
#ifdef META
		 if (comma) cout << ','; else comma = true;
#else
		 if (comma) cout << ' '; else comma = true;
#endif
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
