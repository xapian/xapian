/* limit on mset size (as given in espec) */
#define MLIMIT 1000 // FIXME: deeply broken

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

#define MAX_TERM_LEN 128

typedef enum { ABSENT = 0, NORMAL, PLUS, MINUS, BOOL_FILTER } termtype;

struct term {
    string termname;
    termid id;
    termtype type;
};
#define MAXTERMS 500

#ifdef META
static char *fmtstr = "ÿP\tÿU\tÿC\tÿS\tÿL\tÿW\tÿH\tÿs\tÿM\tÿT\n";
#else
static char *fmtstr =
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

char raw_prob[4048];
static long int msize = 0;
static double maxweight = -1;
static string gif_dir = "/fx-gif";
static long int r_displayed[200];
static long int r_di;
static struct term new_terms[MAXTERMS];
static int n_new_terms;
static long int score_height, score_width;

char thou_sep = ',', dec_sep = '.';

#ifdef FERRET
static char ad_keywords[MAX_TERM_LEN * 4 + 4] = "";
int n_ad_keywords = 0;

/* from main.c */
extern int dlist[];
extern int n_dlist;
#endif

static string query_string;

matchop op = OR; // default matching mode

static map<termname, int> matching_map;

static void do_adjustm ( void );
static int parse_prob( const char *, struct term * );
static char *find_format_string( char *pc );
static void run_query(void);
static void print_query_page( const char *, long int, long int );
static int print_caption( long int, int );
static void print_page_links( char, long int, long int );
static int get_next_char( const char **p );

/******************************************************************/
/* print string to stdout, with " replaced by &#34; */
static void print_escaping_dquotes( char *str, int spaces ) {
   char *p;
   char *p_end;
   size_t len;
   char *buf = NULL;

   p = str;
   if (spaces) {
      p = buf = strdup(str);
      if (!p) throw "memory";
      while ( (p = strchr( p, ' ' )) != NULL )
	 *p = '+';
      p = buf;
   }
   while ( (p_end = strchr( p, '\"' )) != NULL ) {
      len = p_end - p;
      if (len) cout << string(p, len);
      cout << "&#34;";
      p = p_end + 1;
   }
   cout << p;
   if (buf) free(buf);
}

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
static int is_old_query( const char *oldp ) {
   string oldterm;
   const char *pend;
   const char *term;
   int is_old;
   char oldp_sep = '.';
   int n_old_terms = 0; // ?

   if (!oldp) return 0;

   term = oldp;
   is_old = 0;
   /* note old format is "word1#word2#", with trailing # */
   /* new format uses . instead of # as some old browsers don't quote #
    * in form submissions */
   
   pend = strchr(term, oldp_sep);
   if (!pend) oldp_sep = '#';

#ifdef FERRET
   pend = term;
   while ((pend = strchr(pend, oldp_sep)) != NULL) {
      pend++;
      n_old_terms++;
   }
   /* short-cut: if the new query has fewer terms, it must be a new one */
   if (n_new_terms < n_old_terms) return 0;
#endif

   while ((pend = strchr( term, oldp_sep )) != NULL) {
      size_t len = pend - term;
      /* ignore oversized terms */
      if (len < MAX_TERM_LEN) {
	 int i;
	 oldterm = string(term, len);
	 is_old = 0;
	 for (i = 0; i < n_new_terms; i++ ) {
	    if (oldterm == new_terms[i].termname) {
	       is_old = 1;
	       break;
	    }
	 }
	 if (!is_old) break;
	 term = pend + 1;
      }
   }
#ifdef FERRET
   /* for the ferret, return:
    * 0 entirely new query
    * 1 unchanged query
    * -1 new query, but based on the old one
    */
   if (is_old && n_new_terms > n_old_terms) return -1;
#endif
   return is_old;
}

/**************************************************************/
int set_probabilistic(const char *p, const char *oldp) {
   int i;
   char *q;
   int is_old;

   /* strip leading whitespace */
   while (isspace(*p)) p++;

   strcpy(raw_prob, p);

   /* and strip trailing whitespace */
   q = raw_prob + strlen(raw_prob);
   while (q > raw_prob && isspace(q[-1])) q--;
   *q = '\0';

   n_new_terms = parse_prob(raw_prob, new_terms);

   is_old = is_old_query(oldp);

   /* clear relevance set if query has changed */
   if (!is_old) {
      // FIXME Give_Muscat("delrels r0-*");
   }

    if (n_new_terms) {
	vector<termid> pluses;
	vector<termid> minuses;
	vector<termid> normals;
      
	for (i = 0; i < n_new_terms; i++) {	    
	    switch (new_terms[i].type) {
	     case PLUS:
		pluses.push_back(new_terms[i].id);
		matching_map[new_terms[i].termname] = i;
		break;
	     case MINUS:
		minuses.push_back(new_terms[i].id);
		// don't put MINUS terms in map - they won't match...
		break;
	     case NORMAL:
		normals.push_back(new_terms[i].id);
		matching_map[new_terms[i].termname] = i;
		break;
	     default:
		cout << "ignoring term " << new_terms[i].termname << endl; // FIXME
		break;
	  }
	}

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

static int checked_a_term = 0;

/* if term is in the database, add it to the term list */
static int check_term(struct term *pt, const char *buf, termtype type) {
    checked_a_term = 1;
    termid id = database.term_name_to_id(buf);
    if (!id) return 0;
    pt->id = id;
    pt->termname = buf;
    pt->type = type;
    return 1;
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

static int parse_prob( const char *text, struct term *pTerm ) {
    const char *pC = text;
    char *pTo;
    int size;
    termtype type = NORMAL;
    int got = 0;
    char buf[MAX_TERM_LEN];
    int stem, stem_all;
    int ch;
#ifdef FERRET
    int in_quotes = 0;
    char phrase_buf[MAX_TERM_LEN];
#endif
    StemEn stemmer;

#ifdef FERRET
    stem = 1;
    stem_all = 1;
#else
    stem = !get_muscat_string ("no_stem", buf);
    /* stem capitalised words too -- needed for EuroFerret - Olly 1997-03-19 */
    stem_all = get_muscat_string ("all_stem", buf);
#endif

    ch = get_next_char( &pC );
    while (ch) {	
	if (isalnum (ch)) {
	    bool got_next = false;
	    int do_stem;
#ifdef COLONFILTERS
	    int is_bool = 0;
	    const char *extra_chars = "+&";
#endif /* COLONFILTERS */
	    pTo = buf;
	    size = 0;
	    do_stem = stem;

	    if (in_quotes) do_stem = 0;

	    if (!stem_all && ch >= 'A' && ch <= 'Z') do_stem = 0;

#ifndef COLONFILTERS
	    while (isalnum (ch) || ch == '+' || ch == '&') {
		*pTo++ = tolower(ch);
	        ch = get_next_char( &pC );
		if (++size > MAX_TERM_LEN-3) break;
	    }
#else /* COLONFILTERS */
 	    read_on:
	    while (isalnum(ch) || strchr(extra_chars, ch)) {
		*pTo++ = tolower(ch);
	        ch = get_next_char( &pC );
		if (++size > MAX_TERM_LEN-3) break;
	    }
#endif /* COLONFILTERS */

	    *pTo = '\0';
	   
#ifdef COLONFILTERS
	    if (is_bool) {
	       add_term(buf); /* turn into boolean term */
	       if (ch) ch = get_next_char( &pC ); /* skip unless it's a '\0' */
	       continue;
	    }

	    if (!in_quotes && ch == ':') {
	       /* handle "domain:uk", etc */
	       switch (buf[0]) {
		case 'd': /* domain */
		case 's': /* site */
		  buf[0] = 'N';
		  break;
		case 'l': /* language */
		  buf[0] = 'L';
		  break;
		default:
		  goto bogus_prefix;
	       }
	       pTo = buf + 1;
	       ch = get_next_char( &pC );
	       extra_chars = "+&.-_";
	       is_bool = 1;
	       
	       goto read_on;
	       bogus_prefix:
	    }
#endif /* COLONFILTERS */

	    if (n_ad_keywords < 4) {
	       /* FIXME: && type != ABSENT, or pick 4 top +ve weights later? */
	       if (n_ad_keywords)
		  strcat( ad_keywords, "+" );
/*	       else *ad_keywords = '\0'; */

	       strcat( ad_keywords, buf );
	       n_ad_keywords++;
	    }

	    if (!in_quotes && ch == '.') {
	       got_next = true;
	       ch = get_next_char(&pC);
	       /* ignore a dot if followed by an alphanum e.g. index.html) */
	       if (!isalnum(ch)) do_stem = 0;
	    }

	    /* muscat_stem() writes stemmed word over unstemmed */
	    if (do_stem) {
		string term = stemmer.stem_word(buf);
		strcpy(buf, term.c_str());
	    }

	    if (!in_quotes) {
	       if (check_term(pTerm, buf, type)) {
		  got++;
		  if (got > MAXTERMS) break;
		  pTerm++;
	       }
	       if (ch != '-') {
		   /* Currently we index hyphenated words as multiple terms, so
		    * we probably want to keep same +/- weighting for all
		    * hyphenated */
		   type = NORMAL;
	       }
	    } else {
	       if (in_quotes > 1) {
		  char tmp_buf[MAX_TERM_LEN];
		  int len, len2;
	          len = strlen( phrase_buf );
	          len2 = strlen( buf );
		  if (len+len2+2 <= MAX_TERM_LEN) {
		     strcpy( tmp_buf, phrase_buf );
		     tmp_buf[len] = ' ';
		     strcpy( tmp_buf + len + 1, buf);
		     
		     if (check_term(pTerm, tmp_buf, type)) {
			got++;
			if (got > MAXTERMS) break;
			pTerm++;
		     }
		  }
	       }
	       strcpy( phrase_buf, buf );	       
	    }

	    if (ch == '\"') {
	       if (in_quotes == 1) {
		  /* had a single term in quotes, so add it */
		  if (check_term(pTerm, phrase_buf, type)) {
		     got++;
		     if (got > MAXTERMS) break;
		     pTerm++;
		  }
	       }
	       in_quotes = ! in_quotes;
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
	   if (in_quotes == 2) {
	      /* had a single term in quotes, so add it */
	      if (check_term(pTerm, phrase_buf, type)) {
		 got++;
		 if (got > MAXTERMS) break;
		 pTerm++;
	      }
	   }
	   in_quotes = !in_quotes;
	}
       
        if (ch) ch = get_next_char(&pC); /* skip unless it's a '\0' */
    }

    if (in_quotes == 2) {
       /* had a single term in unterminated quotes, so add it */
       if (check_term(pTerm, phrase_buf, type)) {
	  got++;
/* pointless: if (got <= MAXTERMS) pTerm++; */ 
       }
    }

    return got;
}

// FIXME: multimap for general use?
static map<char, string> filter_map;
/**************************************************************/
void add_bterm(const char *term) {
    filter_map[term[0]] = string(term);
}

/**************************************************************/
static void run_query(void) {
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

    /* Fix problem when there's a boolean and none of the probabilistic terms
     * were in the term list.  Otherwise Muscat throws away all the pterms and
     * then returns all records matching just the boolean query */
    if (n_new_terms == 0 && checked_a_term) {
       maxweight = 0;
       msize = 0;
       return;
    }

    matcher->match();

    maxweight = matcher->get_max_weight();

    msize = matcher->mtotal;
}

/**************************************************************/
static void do_adjustm ( void ) {
#if 0
    long int adjust;
    char sortfield[64];
    if (msize == 0)
    	return; /* SA 13/6/97 -- no point reordering an empty M-set,
                   * & the following adjustm would crash the compiled version */
    if ((adjust = get_muscat_int ("reorder_phrase"))) {
	Give_Muscatf("adjustm phrase %ld", adjust);
	Ignore_Muscat();
    }
    else if ((adjust = get_muscat_int ("reorder_tight"))) {
	Give_Muscatf("adjustm tight %ld", adjust);
	Ignore_Muscat();
    }
    else if ((adjust = get_muscat_int ("reorder_loose"))) {
	Give_Muscatf("adjustm loose %ld", adjust);
	Ignore_Muscat();
    }

    /* reorder alphabetically within weighting bands on some given field */
    if (get_muscat_string( "sort_field", sortfield )) {
        /* only sort top 100 for efficiency -- user probably won't notice if
	 * hits past 100 aren't "properly" sorted, but will a slow response */
        Give_Muscatf("sortm 100 on *%s afterw", sortfield );
        Ignore_Muscat();
    }
#endif
}

/**************************************************************/
long do_match ( long int first_hit, long int list_size) {
    print_query_page ( "query", first_hit, list_size);

    return msize; /* Ol 1997-01-31 return msize for logging code */
}

/* pretty print numbers with thousands separated */
/* NB only handles %ld and %d with no width or flag specifiers... */
static void pretty_printf(const char *p, int *a) {
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
		cout << ch;
		if (--len && len % 3 == 0) cout << thou_sep;
	    }
	    continue;
	 }
      }
      cout << ch;
   }
}

/*******************************************************************/
/* showdoc and docid are only relevant for print_doc_page */
static size_t process_common_codes( int which, char *pc, long int topdoc,
				   long int maxhits, long int last,
				   long int showdoc, long int docid ) {
   if (!strncmp (pc, "GIF_DIR", 7)) {
      cout << gif_dir;
      return 7;
   }

   if (!strncmp (pc, "PROB", 4)) {
      int plus = (pc[4] == '+');
      print_escaping_dquotes(raw_prob, plus);
      return 4 + plus;
   }
   
   /* Ol 1997-01-23 - \SCRIPT_NAME is the pathname fx was invoked with */
    if (!strncmp (pc, "SCRIPT_NAME", 11)) {
	char *p = getenv("SCRIPT_NAME");
	if (p == NULL) /* we're probably in test mode, or the server's crap */
	    p = "fx";
	cout << p;
	return 11;
    }

   if (!strncmp (pc, "TOPDOC", 6)) {
       cout << topdoc;
       return 6;
   }

   else if (!strncmp (pc, "VERSION", 7)) {
       cout << FX_VERSION_STRING << endl;
       return 7;
   }

   if (!strncmp (pc, "SAVE", 4)) {
       long int r, i;
       r = r; // FIXME

       /*** save DB name **/
       if (db_name != default_db_name)
	   cout << "<INPUT TYPE=hidden NAME=DB VALUE=\"" << db_name << "\">\n";

       /*** save top doc no. ***/
       if (topdoc != 0)
	   cout << "<INPUT TYPE=hidden NAME=TOPDOC VALUE=" << topdoc << "\n";
       
       /*** save maxhits ***/
       if (maxhits != 10)
	   cout << "<INPUT TYPE=hidden NAME=MAXHITS VALUE=" << maxhits << ">\n";

       /*** save fmt ***/
       if (fmt.size())
	   cout << "<INPUT TYPE=hidden NAME=FMT VALUE=\"" << fmt << "\">\n";

      if (which == 'Q') {
	 /*** save prob query ***/
	 if (n_new_terms) {
	     cout << "<INPUT TYPE=hidden NAME=OLDP VALUE=\"";
	     for (i = 0; i < n_new_terms; i++) {
		 cout << new_terms[i].termname.c_str() << '.';
	     }
	     cout << "\">\n";
	 }
      }

      if (which != 'Q') {
	 /*** save prob query ***/
	 cout << "<INPUT TYPE=hidden NAME=P VALUE=\"";
	 print_escaping_dquotes(raw_prob, 0);
	 cout << "\">\n";

	 // FIXME: save bool query
      }

      if (which == 'Q') {
#if 0 // FIXME
	 /*** save R-set (not in r_displayed) ***/
	 Give_Muscat ("show docs r0-1000");
	 while (!Getfrom_Muscat (&z)) {
	    if (sscanf (z.p, "I)%ld", &r)) {
	       int print_it = 1;
	       for (i = 0; i < r_di; i++) {
		  if (r == r_displayed[i]) {
		     print_it = 0;
		     break;
		  }
	       }
	       if (print_it)
		    cout << "<INPUT TYPE=hidden NAME=R" << r << "VALUE=1>\n";
	    }
	 }
#endif
      }

#if 0
      if (which == 'E') {
	 /*** save R-set ***/
	 Give_Muscat ("show docs r0-1000");
	 while (!Getfrom_Muscat (&z)) {
	    if (sscanf (z.p, "I)%ld", &r))
		 cout << "<INPUT TYPE=hidden NAME=R" << r << "VALUE=1>\n";
	 }
      }

      if (which == 'D') {
	 /*** save R-set ***/
	 Give_Muscat ("show docs r0-1000");
/*	 Give_Muscat ("showv (R*) v");*/
	 while (!Getfrom_Muscat (&z)) {
	    if (sscanf (z.p, "I)%ld", &r)) {
	       if (r != docid)
		    cout << "<INPUT TYPE=hidden NAME=R" << r << "VALUE=1>\n";
	    }
	 }
      }
#endif

      /*** save number of hits per page and relevance threshold ***/
      if (which != 'Q') {
          cout << "<INPUT TYPE=hidden NAME=MAXHITS VALUE=" << maxhits << ">\n";
	  if (op == AND)
	      cout << "<INPUT TYPE=hidden NAME=MATCHOP VALUE=AND>\n";
      }

      return 4;
   }

   if (which != 'E') { /* Q and D behave somewhat differently: page vs doc */
      char *format;
      char *pc_end;

      if (!strncmp (pc, "PREVOFF", 7)) {
	 format = pc + 7;
	 pc_end = find_format_string( format );
	 if (topdoc == 0) {
	    cout << "<img " << string(format, pc_end - format) << ">\n";
	 }
	 return pc_end - pc;
      }

      if (!strncmp (pc, "PREV", 4)) {
	  format = pc + 4;
	  pc_end = find_format_string( format );
	  if (topdoc > 0) {
	      long int new_first;
	      new_first = topdoc - maxhits;
	      if (new_first < 0) new_first = 0;
	      
	      cout << "<INPUT NAME=F" << new_first << ' '
		   << string(format, pc_end - format) << ">\n";
	  }
	  return pc_end - pc;
      }
      
      if (!strncmp (pc, "NEXTOFF", 7)) {
	  format = pc + 7;
	  pc_end = find_format_string( format );
	  if (last >= msize - 1) {
	      cout << "<img " << string(format, pc_end - format) << ">\n";
	  }
	  return pc_end - pc;
      }
      
      if (!strncmp (pc, "NEXT", 4)) {
	  format = pc + 4;
	  pc_end = find_format_string( format );
	  if (last < msize - 1) {
	      cout << "<INPUT NAME=F" << last + 1 << ' '
		   << string(format, pc_end - format) << ">\n";
	  }
	  return pc_end - pc;
      }
   }

   if (which != 'E') {
      /* Olly: was only on D, but useful (at least for debugging) on Q */
      if (!strncmp (pc, "MSIZE", 5)) {
	  cout << msize;
	  return 5;
      }
   }

   if (which == 'Q') {
      if (!strncmp (pc, "PAGES.", 6)) {
	 print_page_links (pc[6], maxhits, topdoc);
	 return 7;
      }

      if (!strncmp (pc, "STAT", 4)) {
	 int arg[3];
	 int print = 0;
	 arg[0] = topdoc + 1;
	 arg[1] = last + 1;
	 arg[2] = msize;
	 /* We're doing:
	  * \STAT0 none
	  * \STAT2 returning some matches from over n
	  * \STATa returning all matches from n
	  * \STATs returning some (but not all) matches from n
	  */
	 switch (pc[4]) {
	  case '0': /* followed by string */
	    if ((msize == 0) && have_query)
		cout << pc + 5;
	    break;
	  case '2':
	    /* used to be >= - now use an exact compare since MTOTAL
	     * may have given us the full figure.  If MTOTAL works and
	     * we got exactly MLIMIT hits, this will misreport... */
	    if (msize == MLIMIT) print = 1;
	    break;
	  case 'a':
	    /* used to be < MLIMIT - now use an exact compare since MTOTAL
	     * may have given us the full figure.  If MTOTAL works and
	     * we got exactly MLIMIT hits, this will misreport... */
	    /* FIXME: could use Mike Gatford's "1001" trick */
	    if (0 < msize && msize != MLIMIT &&
		(topdoc == 0 && last + 1 == msize)) {	       
	       arg[0] = msize;
	       print = 1;
	    }
	    break;
	  case 's':
	    /* used to be < MLIMIT - now use an exact compare since MTOTAL
	     * may have given us the full figure.  If MTOTAL works and
	     * we got exactly MLIMIT hits, this will misreport... */
	    /* FIXME: could use Mike Gatford's "1001" trick */
	    if (0 < msize && msize != MLIMIT &&
		!(topdoc == 0 && last + 1 == msize)) print = 1;
	    break;
	 }
	 if (print) pretty_printf(pc + 5, arg);
	 return strlen(pc); /* skip rest of line */
      }
   }
   
   return 0;
}

static int
order(const void *a, const void *b)
{
   return strcmp(*(char **)a, *(char **)b);
}

/* generate a sorted picker */
static void
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

/*******************************************************************/
static void print_query_page( const char* page, long int first, long int size) {
    FILE *filep;
    char line[512];
    char *pc;
    char *pre;
    long int last = 0;
    int expand_on = 1;

    string tmp = option["gif_dir"];
    if (tmp != "") gif_dir = tmp;
    if (option["no_expand"] != "") expand_on = 0;
    score_height = atoi(option["score_height"].c_str());
    score_width = atoi(option["score_width"].c_str());
    if ((filep = page_fopen(page)) == NULL) return;

    r_di = 0;

    /*** parse the page ***/
    while (fgets (line, 511, filep)) {
	if ((pc = strchr (line, '\\')) == NULL) {
	   cout << line;
	}
	else {
	    pre = line;
	    do {
	        size_t skip;
		cout << string(pre, pc - pre);
	        pc++;

		skip = process_common_codes( 'Q', pc, first, size, last, 0, 0 );
	        
	        if (skip) {
		   pc += skip;
		   pre = pc;
		   continue;
		}

		if (!strncmp (pc, "HITLINE", 7)) {
		    if (!msize){
			pc += strlen(pc); /* Ignore the rest of this line if no hits */
		    } else {
			pc += 7; /* Just ignore this tag if there *are* hits. */
		    }
		}

		else if (!strncmp (pc, "HITS", 4)) {
		    long int m;
#ifdef META
		    cout << "# fields are tab separated, extra fields may be appended in future\n"
			    "first\tlast\ttotal\n" << first + 1 << '\t'
			 <<  last + 1 << '\t' msize << '\n'
			 << "relevance\turl\tcaption\tsample\tlanguage\tcountry\thostname\tsize\tlast modified\tmatching\n";
#endif
		    {
			char *q;
			int ch;
			
			query_string = "?DB=";
			query_string += db_name;
			query_string += "&P=";
			q = raw_prob;
			while ((ch = *q++) != '\0') {
			   if (ch == '+') {
			      query_string += "%2b";
			   } else if (ch == '"') {
			      query_string += "%22";
			   } else {
			      if (ch == ' ') ch = '+';
			      query_string += ch;
			   }
			}
			/* add any boolean terms */
			map <char, string>::const_iterator i;			 
			for (i = filter_map.begin(); i != filter_map.end(); i++) {
			    query_string += "&B=";
			    query_string += i->first;
			    query_string += i->second;
			}
		     }

#ifndef META
		     {
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
		     }
#endif

		    for (m = first; m <= last; m++) {
		       if (print_caption(m, expand_on)) break;
		    }
		    pc += 4;
		}


	        /* SA 7/4/1997 - item in max hits selector box */
                else if (!strncmp (pc, "MAXHITS", 7)) {
		    if (size == atoi(pc + 7))
		        cout << "SELECTED";
		    pc += 10; /* 7 for MAXHITS + 3 for number */
		}

                else if (!strncmp (pc, "OSELECT", 7)) {
		    if ((op == AND) ^! (pc[7] == 'A'))
		        cout << "SELECTED";
		    pc += 8; /* 7 for TSELECT + 1 for char */
		}

		else if (!strncmp (pc, "TOPTERMS", 8)) {
		  if (msize) {
		    /* Olly's expand on query page idea */
		    // int c = 0;
		    // int rel_hack = 0;
#if 1 // FIXME
		    cout << "Sorry, we've not implemented relevance feedback yet\n";
#else
		    /* see if we have any docs marked as relevant */
		    Give_Muscat( "show docs style w r0" );
		    if (!Getfrom_Muscat(&z) && z.p[0] != 'I') {
		       Ignore_Muscat();
		       Give_Muscat( "rels m0-4" );
		       rel_hack = 1;
		    }
		    Ignore_Muscat();
		    Give_Muscat("expand 20");
		    /* Give_Muscatf("expand %ld", expand_size); */
		    while (!Getfrom_Muscat (&z)) {
		        check_error(&z);
			if (z.p[0] == 'I') {
			   int width = z.length - 2;
			   /* only suggest 4 or more letter words for now to
			    * avoid italian problems !HACK! */
			   if (width > 3) {
			      cout << "<INPUT TYPE=checkbox NAME=X "
				      "VALUE=\"" << z.p+2
				   << ".\" onClick=\"C(this)\">&nbsp;"
				   << z.p + 2 << ". ";
			      c++;
			   }
			}
		    }
 		    if (c) {
			cout << "<BR><NOSCRIPT>"
			        "<INPUT TYPE=hidden NAME=ADD VALUE=1>"
			        "</NOSCRIPT>\n");
		    }

		    /* If we faked a relvance set, clear it again */
		    if (rel_hack) {
		       Give_Muscat("delrels r0-*");
		       Ignore_Muscat();
		    }
#endif
		  }
		  pc += 8;
		}
#ifdef FERRET
		else if (!strncmp (pc, "DOMATCH", 7)) {
		   /* don't rerun query if we ran it earlier */
		   if (maxweight < 0) {
		      matcher->set_max_msize(first + size);
		      run_query();
		      do_adjustm();
		   }

		   if (first > msize) first = 0;
    
		   if (first + size < msize)
		      last = first + size - 1;
		   else
		      last = msize - 1;

		   pc += 7;
		}
	       
	        else if (!strncmp (pc, "FER-WHERE", 9)) {
		   /* ferret countrycode picker */
		   static const char *doms[] = {
		      "ad", "al", "am", "at", "az", "ba", "be", "bg",
		      "by", "ch", "cy", "cz", "de", "dk", "ee", "es",
		      "fi", "fo", "fr", "ge", "gi", "gl",
		      "gr", "hr", "hu", "ie", "is", "it",
		      "li", "lt", "lu", "lv", "mc", "mk", "mt", "mo",
		      "nl", "no", "pl", "pt", "ro", "ru", "se", "si",
		      "sk", "sm", "su", "tr", "ua", "uk+", "va", "yu",
		      NULL
		      /* Now in uk+: "gb", "gg", "im", "je", "uk" */
		   };
		   do_picker('N', doms);
		   pc += 9;
		}
	        else if (!strncmp(pc, "FER-LANG", 8)) {
		   /* ferret language picker */
		   static const char *langs[] = {
		      "cs", /*"cy",*/ "da", "de", "en", "es", "fi", "fr",
		      "is", "it", "nl", "no", "pl", "pt", "sv",
		      NULL
		   };
		   do_picker('L', langs);
		   pc += 8;
		}
	        else if (!strncmp(pc, "FER-AD", 6)) {
		   /* ferret advert link (with keywords) */
		   int pageid = time(NULL) - 894000000;
		   int tag = 7533; /* english */
		   if (db_name.size() >= 12) {
		      switch (db_name[8]) {
		       case 'r':
			 if (db_name == "ferret.french") tag = 7542;
			 break;
		       case 'e':
			 if (db_name == "ferret.german") tag = 7543;
			 break;
		       case 't':
			 if (db_name == "ferret.italian") tag = 7584;
			 break;
		       case 'p':
			 if (db_name == "ferret.spanish") tag = 7544;
			 break;
		       case 'w':
			 if (db_name == "ferret.swedish") tag = 7545;
			 break;
		      }
		   }
		   cout << "<A HREF=\"http://adforce.imgis.com/"
			   "?adlink|44|" << tag << '|' << pageid << "|1|key="
			<< ad_keywords << "\" TARGET=_top><IMG\n"
			   "SRC=\"http://adforce.imgis.com/"
			   "?adserv|44|" << tag << '|' << pageid << "|1|key="
			<< ad_keywords
			<< "\" BORDER=0 HEIGHT=60 WIDTH=468 NATURALSIZEFLAG=0 "
			   "ALIGN=BOTTOM "
			   "ALT=\"Intelligent access to over 30 million web pages\""
			   "></A>\n";
		   pc += 6;
		}
	        else if (!strncmp(pc, "FREQS", 5)) {
		   if (msize) {
		       int i;
		       for (i = 0; i < n_new_terms; i++) {
			   const char *term = new_terms[i].termname.c_str();

			   PostList *pl = database.open_post_list(new_terms[i].id);
			   int freq = pl->get_termfreq();
			   delete pl;

			   if (i == 0) {
			       cout << "<B>Individual word frequencies:</B>\n";
			   } else {
			       cout << ", ";
			   }
			   if (strchr(term, ' ')) {
			       cout << "\"" << term << "\":&nbsp;";
			   } else {
			       cout << term << ":&nbsp;";
			   }
			   pretty_printf("%d", &freq);
		       }
		       if (i != 0) cout << "<BR>";
		   }
		   pc += 5;
		}
#endif

		else {
		    cout << *pc++;
		}
		pre = pc;

	    } while ((pc = strchr (pre, '\\')) != NULL);

	    cout << pre;
	}
    }

    fclose (filep);
}

/******************************************************************/
static void print_page_links( char type, long int hits_per_page,
			      long int topdoc ) {
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
static int print_caption( long int m, int do_expand ) {
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
   
    percent = percentage((double)w, maxweight);

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
		     r_displayed[r_di++] = q0;
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
static char *find_format_string( char *pc ) {
    while (*pc && *pc != '\\' && *pc != '\n' && *pc != '\r')
       pc++;
    return pc;
}
