// declared in parsepage.y
extern void print_query_page(const char *, long int, long int);

#include <vector>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include <time.h>

#include <sys/stat.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>

#include "main.h"
#include "query.h"

typedef enum { /*ABSENT = 0,*/ NORMAL, PLUS, MINUS /*, BOOL_FILTER*/ } termtype;

vector<termname> new_terms;
static vector<termname> pluses;
static vector<termname> minuses;
static vector<termname> normals;

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
map<docid, bool> r_displayed;

string gif_dir = "/fx-gif";

char thou_sep = ',', dec_sep = '.';

#ifdef FERRET
string ad_keywords;
static int n_ad_keywords = 0;
#endif

string query_string;

matchop op = OR; // default matching mode

static map<termname, int> matching_map;

static void parse_prob(const string&);
static char *find_format_string(char *pc);
static int get_next_char(const char **p);

/**************************************************************/
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

/**************************************************************/
/* Check new query against the previous one */
/* Heuristic: If any words have been removed, it's a "fresh query"
 * so we should clear the R-set */
static int is_old_query(const string &oldp) {
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
   /* short-cut: if the new query has fewer terms, it must be a new one */
   if (new_terms.size() < n_old_terms) return 0;

   vector<termname>::const_iterator i = new_terms.begin();
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
   /* for the ferret, return:
    * 0 entirely new query
    * 1 unchanged query
    * -1 new query, but based on the old one
    */
   if (is_old && new_terms.size() > n_old_terms) return -1;
   return is_old;
}

/**************************************************************/
int set_probabilistic(const string &newp, const string &oldp) {
   const char *p = newp.c_str();
   int is_old;

   /* strip leading whitespace */
   while (isspace(*p)) p++;
   /* and strip trailing whitespace */
   size_t len = strlen(p);
   while (len && isspace(p[len - 1])) len--;
   
   raw_prob = string(p, len);

   parse_prob(raw_prob);

   is_old = is_old_query(oldp);

   /* clear relevance set if query has changed */
   if (!is_old) {
      // FIXME Give_Muscat("delrels r0-*");
   }

    if (!new_terms.empty()) {
	// now we constuct the query:
	// ((plusterm_1 AND ... AND plusterm_n) ANDMAYBE
	//  (term_1 OR ... OR term_m)) ANDNOT
	// (minusterm_1 OR ... OR minusterm_p)
	if (!pluses.empty()) matcher->add_oplist(AND, pluses);
	if (!normals.empty()) {
	    matcher->add_oplist(op, normals);
	    if (!pluses.empty()) matcher->add_op(AND_MAYBE);
	}       
	if (!minuses.empty()) {
	    matcher->add_oplist(OR, minuses);
	    if (!matcher->add_op(AND_NOT)) {
		cout << "Don't be so negative\n" << endl;
		exit(0);
	    }
	}
    }
   
    return is_old;
}

/**************************************************************/

static int term_count = 1; // FIXME: ick

/* if term is in the database, add it to the term list */
static void check_term(string name, termtype type) {
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

/**************************************************************/
/* transliterate accented characters in step with what the indexers do */
static int get_next_char( const char **p ) {
   static int cache = 0;
   int ch;
   if (cache) {
      ch = cache;
      cache = 0;
      return ch;
   }
   ch = (int)(unsigned char)(*(*p)++);
   switch (ch) {
#include "symboltab.h"
   }
   return ch;
}

static void
parse_prob(const string &text)
{
    const char *pC = text.c_str();
    termtype type = NORMAL;
    string buf;
    int stem, stem_all;
    int ch;
#ifdef FERRET
    int in_quotes = 0;
    string phrase_buf;
#endif
    StemEn stemmer;

#ifdef FERRET
    stem = 1;
    stem_all = 1;
#else
    stem = !atoi(option["no_stem"].c_str());
    /* stem capitalised words too -- needed for EuroFerret - Olly 1997-03-19 */
    stem_all = atoi(option["all_stem"].c_str());
#endif

    ch = get_next_char(&pC);
    while (ch) {	
	if (isalnum(ch)) {
	    bool got_next = false;
	    int do_stem;
	    // FIXME: allow domain:uk in query...
	    // don't allow + and & in term then ?
	    // or allow +&.-_ ?
	    // domain/site/language/host ?
	    do_stem = stem;

	    if (in_quotes) do_stem = 0;

	    if (!stem_all && ch >= 'A' && ch <= 'Z') do_stem = 0;

            buf = "";
	    while (isalnum(ch) || ch == '+' || ch == '&') {
		buf += tolower(ch);
	        ch = get_next_char(&pC);
	    }

	    if (n_ad_keywords < 4) {
	       /* FIXME: && type != ABSENT, or pick 4 top +ve weights later? */
	       if (n_ad_keywords)
		  ad_keywords += '+';

	       ad_keywords += buf;
	       n_ad_keywords++;
	    }

	    if (!in_quotes && ch == '.') {
	       got_next = true;
	       ch = get_next_char(&pC);
	       /* ignore a dot if followed by an alphanum e.g. index.html) */
	       if (!isalnum(ch)) do_stem = 0;
	    }

	    if (do_stem) buf = stemmer.stem_word(buf);

	    if (!in_quotes) {
		check_term(buf, type);
		/* Currently we index hyphenated words as multiple terms, so
		 * we probably want to keep same +/- weighting for all
		 * hyphenated */
		if (ch != '-') type = NORMAL;
	    } else {
		if (in_quotes > 1) {
		    string tmp_buf;
		    tmp_buf = phrase_buf;
		    tmp_buf += ' ';
		    tmp_buf += buf;
		    check_term(tmp_buf, type);
		}
		phrase_buf = buf;
	    }

	    if (ch == '\"') {
		/* had a single term in quotes, so add it */
		if (in_quotes == 1) check_term(phrase_buf, type);
		in_quotes = !in_quotes;
		if (!in_quotes) type = NORMAL; /* reset +/- */
	    } else {
		if (in_quotes) in_quotes++;
	    }
	    if (got_next) continue;
	} else if (ch == '+') {
	    type = PLUS;
	} else if (ch == '-') {
	    type = MINUS;
	} else if (ch == '\"') {
	    /* had a single term in quotes, so add it */
	    if (in_quotes == 2) check_term(phrase_buf, type);
	    in_quotes = !in_quotes;
	}
       
        if (ch) ch = get_next_char(&pC); /* skip unless it's a '\0' */
    }

    /* had a single term in unterminated quotes, so add it */
    if (in_quotes == 2) check_term(phrase_buf, type);
}

// FIXME: multimap for general use?
map<char, string> filter_map;
/**************************************************************/
void add_bterm(const string &term) {
    filter_map[term[0]] = term;
}

/**************************************************************/
extern void
run_query(void)
{
    int bool_terms = 0;
    // add any boolean terms and AND them together
    // FIXME: should OR those with same prefix...
    map <char, string>::const_iterator i;
    for (i = filter_map.begin(); i != filter_map.end(); i++) {
        matcher->add_term(i->second);
	bool_terms++;
	if (bool_terms) matcher->add_op(AND);
    }
    if (bool_terms) matcher->add_op(FILTER);

    matcher->match();

    msize = matcher->mtotal;
}

/**************************************************************/
long do_match ( long int first_hit, long int list_size) {
    print_query_page ( "query", first_hit, list_size);

    return msize; /* Ol 1997-01-31 return msize for logging code */
}

static int
order(const void *a, const void *b)
{
   return strcmp(*(char **)a, *(char **)b);
}

/* generate a sorted picker */
extern void
do_picker(char prefix, const char **opts)
{
    const char **p;
    char **q;
    char buf[16] = "BOOL-X";
    int do_current = 0;
    char current[256];
    char txtbuf[16384];
    char *t;
    int len;
    char *picker[256];
    
    map <char, string>::const_iterator i = filter_map.find(prefix);
    if (i != filter_map.end() && i->second.length() > 1) {
	i->second.copy(current, 256, 1);
	current[i->second.length() - 1] = '\0';
	do_current = 1;
    }

    cout << "<SELECT NAME=B>\n<OPTION VALUE=\"\"";

    if (!do_current) cout << " SELECTED";

    cout << '>';

    buf[5] = prefix;

    string tmp = option[buf];
    if (tmp.size())
	cout << tmp;
    else
	cout << "-Any-";

   t = txtbuf;
   q = picker;
   for (p = opts; *p; p++) {
      strcpy(buf+6, *p);
      string trans = option[buf];
      len = trans.size();
      if (len) {
	  trans.copy(t, 16384); // FIXME
      } else {
	  if (prefix == 'N')
	      sprintf(t, ".%s", *p);
	  else 
	      sprintf(t, "[%s]", *p);
	  len = strlen(t);
      }
      *q++ = t;
      t += len;
      if (do_current && !strcmp(*p, current)) {
	 *t++ = '\n';
	 do_current = 0;
      }
      *t++ = '\t';		      
      strcpy(t, *p);
      t += strlen(t) + 1;
   }
   
   *q = NULL;
   
   qsort(picker, q - picker, sizeof(char*), order);

   if (do_current) {
      sprintf(t, "%s\n\t%s", current, current);
      *q++ = t;
      *q = NULL;
   }

   for (q = picker; *q; q++) {
      char *s, *t;
      s = *q;
      t = strchr(s, '\t');
      if (!t) continue;
      *t++ = '\0';
      cout << "\n<OPTION VALUE=" << prefix << t;
      if (t[-2] == '\n') cout << " SELECTED";
      cout << '>' << s;
   }
   
   cout << "</SELECT>\n";
}

/******************************************************************/
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
      /* If not specified, plh and plw no longer default to 15
       * since that prevents the user having different sized page gifs
       */
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

#ifdef FERRET
static void utf8_to_html(const string &str) {
   const unsigned char *p = (const unsigned char *)str.c_str();
   while (1) {
      int ch = *p++;
      if (ch == 0) break;

      /* this is extra magic, not part of utf-8 */
      switch (ch) {
       case '\b': /* was \r but core muscat swallows that... */
	 cout << " / "; /* line break in original */
	 continue;
       case '\t':
	 cout << " * "; /* bullet point in original */
	 continue;
       case '\f':
	 cout << "&nbsp;"; /* hardspace in original */
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
      int prefix = 0;
      const char *amp, *qs;
      qs = query_string.c_str(); // FIXME: use string methods
			 
      prefix = after[3];
      amp = qs;
      while (qs) {
	 amp = strchr(amp, '&');
	 if (!amp) {
	     cout << qs;
	     break;
	 }
	 amp++;
	 while (amp[0] == 'B' && amp[1] == '=' && amp[2] == prefix) {
	    cout << string(qs, amp - qs - 1);
	    qs = strchr(amp + 3, '&');
	    if (!qs) break;
	    amp = qs + 1;
	 }
      }
   } else {
       cout << query_string;
   }
}
#endif

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
/***********************************************************************/
extern int
print_caption(long int m)
{
    long int q0 = 0, r = 0;
    int percent;
    long int w = 0; 
    time_t lastmod = -1;
    char hostname[256] = "localhost"; /* erk */
    string country = "x";
    char country_code[8] = "x";
    string language = "x";
    char language_code[3] = "x";

    w = (int)matcher->mset[m].w;
    q0 = matcher->mset[m].id;
    
    /* get hostname from longest N tag
     * and country from shortest (allowing for uk+) */
     {  int len = -1, got_plus = 0;
	TermList *terms = database.open_term_list(q0);
	terms->next();
	while (!terms->at_end()) {
	    const char *term = database.term_id_to_name(terms->get_termid()).c_str();
	    int length = strlen(term);
	    switch (term[0]) {
	     case 'N':
		if (length > len && term[length - 1] != '+') {
		    strcpy(hostname, term + 1);
		    len = length;
		}
		if (!got_plus && length - 1 <= 3) {
		    strcpy(country_code, term + 1);
		    if (term[length - 1] == '+') {
			got_plus = 1;
			strcpy(country_code + 2, "%2b");
		    }
		}
		break;
	     case 'L':
		strcpy(language_code, term + 1);
		break;
	    }
	    terms->next();
	}
	delete terms;
     }
    
    {
	char buf[16] = "BOOL-N";
	strcpy(buf + 6, country_code);
	string tmp = buf;
	country = option[tmp];
	if (!country.size()) country = country_code;
    }
    {
	char buf[16] = "BOOL-L";
	buf[6] = language_code[0];
	buf[7] = language_code[1];
	buf[8] = '\0';
	language = option[buf];
	if (!language.size()) language = language_code;
    }
   
    percent = percentage((double)w, matcher->get_max_weight());

    {
       string path, sample, caption;
       int port = -1;
       const char *pp;
       unsigned const char *u;
       int size = -1;
       int dadate = -1;

       /* get datestamp of DA file this record is in so we can handle
	* different versions of our record format */
       if (q0 >= 0) {
	  int n = q0 / 10000000;
	  if (n < n_dlist) dadate = dlist[n];
       }

       IRDocument *doc = database.open_document(q0);
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

       const char *p = strchr(pp, '\xfe');
       if (p) {
	   path = string(pp, p - pp);
	   p++;
	   if (*p != '\xfe' && *p != '\0') {
	       const char *q = strchr(p, '\xfe');
	       if (!q) {
		   caption = string(p);
	       } else {
		   caption = string(p, q - p);
		   sample = string(q + 1);
	       }
	   }
       }

       {
	  char *p, *q;
	  p = fmtstr;
	  while ((q = strchr(p, '\xff')) != NULL) {
	     cout << string(p, q - p);
	     switch (q[1]) {
	      case 'C': /* caption */
		if (caption.size()) {
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
		 if (sample.size()) {
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
		 if (strcmp(language_code, "x") == 0) {
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
		  bool comma = false;
		  // FIXME: in general we should store the matching terms
		  // in a vector and then sort by the value of matching_map[]
		  // In the DA case the way termid-s are invented means we
		  // don't actually need to do this...
		  TermList *terms = database.open_term_list(q0);
		  terms->next();		  
		  while (!terms->at_end()) {
		      termname term = database.term_id_to_name(terms->get_termid());
		      map<termname, int>::iterator i = matching_map.find(term);
		      if (i != matching_map.end()) {
#ifdef META
			  if (comma) cout << ','; else comma = true;
#else
			  if (comma) cout << ' '; else comma = true;
#endif
			  /* quote terms with spaces in */
			  if (term.find(' ') != string::npos)
			      cout << '"' << term << '"';
			  else
			      cout << term;
		      }
		      terms->next();
		  }
		  delete terms;
		  break;
	      }
	      case 'G': /* score Gif */
		 cout << "/fx-gif/score-" << percent / 10 << ".gif";
		 break;
	      case 'X': /* relevance checkboX */
		 if (r) {
		     r_displayed[q0] = true;
		     cout << "<INPUT TYPE=checkbox NAME=R" << q0 << " CHECKED>\n";
		 } else {
		     cout << "<INPUT TYPE=checkbox NAME=R" << q0 << ">\n";
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
    }

    return 0;
}

/***********************************************************************/
/* modified from get_format_string -- no longer copies to a buffer */
static char *find_format_string(char *pc) {
    while (*pc && *pc != '\\' && *pc != '\n' && *pc != '\r')
       pc++;
    return pc;
}
