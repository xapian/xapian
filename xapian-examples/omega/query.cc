/* limit on mset size (as given in espec) */
#define MLIMIT 1000 // FIXME: deeply broken

#include <list>

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
#include "cgiparam.h"
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
static char gif_dir[256] = "/fx-gif";
static long int r_displayed[200];
static long int r_di;
static struct term new_terms[MAXTERMS];
static int n_new_terms;
static long int score_height, score_width;
static int weight_threshold = 0;

char thou_sep = ',', dec_sep = '.';

#ifdef FERRET
static char ad_keywords[MAX_TERM_LEN * 4 + 4] = "";
int n_ad_keywords = 0;

/* from main.c */
extern int dlist[];
extern int n_dlist;
#endif

static string query_string;

int percent_min = 0; /* default to old behaviour */

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
      p = buf = xstrdup(str);
      while ( (p = strchr( p, ' ' )) != NULL )
	 *p = '+';
      p = buf;
   }
   while ( (p_end = strchr( p, '\"' )) != NULL ) {
      len = p_end - p;
      if (len) fwrite( p, 1, len, stdout );
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
	list<termid> pluses;
	list<termid> minuses;
	list<termid> normals;
      
	for (i = 0; i < n_new_terms; i++) {
	    switch (new_terms[i].type) {
	     case PLUS:
		pluses.push_back(new_terms[i].id);
		break;
	     case MINUS:
		minuses.push_back(new_terms[i].id);
		break;
	     case NORMAL:
		normals.push_back(new_terms[i].id);
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
	    matcher->add_oplist(OR, normals);
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
    while (matcher->add_op(OR)) {
	// OR all the probabilistic terms together
	// FIXME: use AND for "matching all"
    }

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

    percent_min = (percent_min > 100) ? 100 : ((percent_min < 0) ? 0 : percent_min);

    /* Trim the M-set to remove matches with negative weights, or those below
     * the percentage threshold.  Only trim if we need to:
     * (hits && not pure boolean && (negative weights || threshold))
     * [note: Muscat returns maxweight 0 for pure boolean queries]
     * NB we also trim if we have plus-ed terms, so that hits without them are
     * removed
     */
    if (weight_threshold || percent_min) {
#if 1 // FIXME
	matcher->set_min_weight_percent(percent_min);
#else
	weight t = maxweight * percent_min / 100;
	if (t > weight_threshold) weight_threshold = t;
	msize = trim_msize(msize, weight_threshold);

	/* if we're using MTOTAL, give up on it now, since we can't binary
	 * chop off the end of the M-set */
	if (oldmsize > MLIMIT) oldmsize = MLIMIT;
#endif
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
       if (fmt && *fmt)
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

#if 0
	 /*** save bool query ***/
	 Give_Muscat ("show q style q");
	 while (!Getfrom_Muscat (&z)) {
	    check_error(&z);
	    if (z.p[0] == 'I' && z.p[3] == 'b')
	       printf ("<INPUT TYPE=hidden NAME=B VALUE=\"%s\">\n", z.p + 5);
	 }
#endif
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
		  printf ("<INPUT TYPE=hidden NAME=R%ld VALUE=1>\n", r );
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
	       printf ("<INPUT TYPE=hidden NAME=R%ld VALUE=1>\n", r);
	 }
      }

      if (which == 'D') {
	 /*** save R-set ***/
	 Give_Muscat ("show docs r0-1000");
/*	 Give_Muscat ("showv (R*) v");*/
	 while (!Getfrom_Muscat (&z)) {
	    if (sscanf (z.p, "I)%ld", &r)) {
	       if (r != docid)
		  printf ("<INPUT TYPE=hidden NAME=R%ld VALUE=1>\n", r);
	    }
	 }
      }

      /*** save date range ***/
      if (which != 'Q') {
	 char date[256];
	 if (get_muscat_string("Date1", date))
	    printf("<INPUT TYPE=hidden NAME=DATE1 VALUE=\"%s\">\n", date);
	 if (get_muscat_string("Date2", date))
	    printf("<INPUT TYPE=hidden NAME=DATE2 VALUE=\"%s\">\n", date);
	 if (get_muscat_string("DaysMinus", date))
	    printf("<INPUT TYPE=hidden NAME=DAYSMINUS VALUE=\"%s\">\n", date);
      }
#endif

      /*** save number of hits per page and relevance threshold ***/
      if (which != 'Q') {
          printf ("<INPUT TYPE=hidden NAME=MAXHITS VALUE=%ld>\n", maxhits );
          printf ("<INPUT TYPE=hidden NAME=THRESHOLD VALUE=%d>\n", percent_min );
      }

      return 4;
   }

   if (which != 'E') { /* Q and D behave somewhat differently: page vs doc */
      char *format;
      char *pc_end;

      if (!strncmp (pc, "PREVOFF", 7)) {
	 format = pc + 7;
	 pc_end = find_format_string( format );
	 if ( which == 'Q' ? (topdoc == 0) : (showdoc == 0) ) {
	    cout << "<img ";
	    fwrite( format, 1, pc_end - format, stdout );
	    cout << ">\n";
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
	      
	      cout << "<INPUT NAME=F" << new_first << ' ';
	      fwrite( format, 1, pc_end - format, stdout );
	      cout << ">\n";
	  }
	  return pc_end - pc;
      }
      
      if (!strncmp (pc, "NEXTOFF", 7)) {
	  format = pc + 7;
	  pc_end = find_format_string( format );
	  if (last >= msize - 1) {
	      cout << "<img ";
	      fwrite( format, 1, pc_end - format, stdout );
	      cout << ">\n";
	  }
	  return pc_end - pc;
      }
      
      if (!strncmp (pc, "NEXT", 4)) {
	  format = pc + 4;
	  pc_end = find_format_string( format );
	  if (last < msize - 1) {
	      cout << "<INPUT NAME=F" << last + 1 << ' ';
	      fwrite( format, 1, pc_end - format, stdout );
	      cout << ">\n";
	  }
	  return pc_end - pc;
      }
   }

   if (which != 'E') {
      /* Olly: was only on D, but useful (at least for debugging) on Q */
      if (!strncmp (pc, "MSIZE", 5)) {
	 printf ("%ld", msize);
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
	  * \STATS for compatibility
	  * \STAT0 none
	  * \STAT1 returning some matches from n
	  * \STAT2 returning some matches from over n
	  * 
	  * instead of \STAT1, there's also:
	  * \STATa returning all matches from n
	  * \STATs returning some (but not all) matches from n
	  */
	 switch (pc[4]) {
	  case 'S': /* old style \STATS */
	    if (msize) {
	       /* used to be >= - now use an exact compare since MTOTAL
		* may have given us the full figure.  If MTOTAL works and
		* we got exactly MLIMIT hits, this will misreport... */
	       if (msize == MLIMIT) {
		  printf("%ld-%ld of over %ld documents matching one or "
			 "more words", topdoc + 1, last + 1, msize);
	       } else {
		  printf("%ld-%ld of %ld documents matching one or more words",
			 topdoc + 1, last + 1, msize);
	       }
	    } else if (have_query) {
	       cout << "No documents found matching these words";
	    }
	    return 5;
	    break;
	  case '0': /* followed by string */
	    if ((msize == 0) && have_query)
		cout << pc + 5;
	    break;
	  case '1':
	    /* used to be < MLIMIT - now use an exact compare since MTOTAL
	     * may have given us the full figure.  If MTOTAL works and
	     * we got exactly MLIMIT hits, this will misreport... */
	    /* FIXME: could use Mike Gatford's "1001" trick */
	    if (0 < msize && msize != MLIMIT) print = 1;
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
#if 0//FIXME
    get_muscat_string ("gif_dir", gif_dir);
    if (get_muscat_string ("no_expand", line)) expand_on = 0;
    doclink_height = get_muscat_int ("doclink_height");
    if (!doclink_height) doclink_height = 25;
    doclink_width = get_muscat_int ("doclink_width");
    if (!doclink_width)  doclink_width = 25;
    score_height = get_muscat_int ("score_height");
    score_width = get_muscat_int ("score_width");
#else
    score_height = 21;
    score_width = 21;
#endif
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
		fwrite (pre, 1, pc - pre, stdout);
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
		    printf("# fields are tab separated, extra fields may be appended in future\n"
			   "first\tlast\ttotal\n"
			   "%ld\t%ld\t%ld\n", first + 1, last + 1, msize);		    
		    cout << "relevance\turl\tcaption\tsample\tlanguage\tcountry\thostname\tsize\tlast modified\tmatching\n";
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
			int fd = open(fmtfile, O_RDONLY);
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
		        fputs("SELECTED", stdout);
		    pc += 10; /* 7 for MAXHITS + 3 for number */
		}

	        /* SA 1997-07-11 - item in threshold selector box */
                else if (!strncmp (pc, "TSELECT", 7)) {
		    if (percent_min == atoi(pc + 7))
		        fputs("SELECTED", stdout);
		    pc += 10; /* 7 for TSELECT + 3 for number */
		}

	        /* SA 1997-07-11 - text entry of threshold */
                else if (!strncmp (pc, "THRESHOLD", 9)) {
		    printf ("%d", percent_min);
		    pc += 9;
		}
		else if (!strncmp (pc, "TOPTERMS", 8)) {
		  if (msize) {
		    /* Olly's expand on query page idea */
		    // int c = 0;
		    // int rel_hack = 0;
#if 1 // FIXME
		      printf("Sorry, we've not implemented relevance feedback yet\n");
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
			      printf("<INPUT TYPE=checkbox NAME=X "
				     "VALUE=\"%s.\" onClick=\"C(this)\">&nbsp;%s. ",
				     z.p + 2, z.p + 2);
			      c++;
			   }
			}
		    }
 		    if (c) {
		       puts("<BR><NOSCRIPT>"
			    "<INPUT TYPE=hidden NAME=ADD VALUE=1>"
			    "</NOSCRIPT>");
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
		   if (strlen(db_name) >= 12) {
		      switch (db_name[8]) {
		       case 'r':
			 if (strcmp(db_name, "ferret.french") == 0) tag = 7542;
			 break;
		       case 'e':
			 if (strcmp(db_name, "ferret.german") == 0) tag = 7543;
			 break;
		       case 't':
			 if (strcmp(db_name, "ferret.italian") == 0) tag = 7584;
			 break;
		       case 'p':
			 if (strcmp(db_name, "ferret.spanish") == 0) tag = 7544;
			 break;
		       case 'w':
			 if (strcmp(db_name, "ferret.swedish") == 0) tag = 7545;
			 break;
		      }
		   }
		   printf("<A HREF=\"http://adforce.imgis.com/"
			  "?adlink|44|%d|%d|1|key=", tag, pageid );
		   fputs( ad_keywords, stdout );
		   printf("\" TARGET=_top><IMG\n"
			  "SRC=\"http://adforce.imgis.com/"
			  "?adserv|44|%d|%d|1|key=", tag, pageid );
		   fputs( ad_keywords, stdout );
		   printf("\" BORDER=0 HEIGHT=60 WIDTH=468 NATURALSIZEFLAG=0 "
			  "ALIGN=BOTTOM "
			  "ALT=\"Intelligent access to over 30 million web pages\""
			  "></A>\n");
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
			       puts( "<B>Individual word frequencies:</B>" );
			   } else {
			       puts( ", " );
			   }
			   if (strchr(term, ' ')) {
			       printf("\"%s\":&nbsp;", term);
			   } else {
			       printf("%s:&nbsp;", term);			       
			   }
			   pretty_printf("%d", &freq);
		       }
		       if (i != 0) puts("<BR>");
		   }
		   pc += 5;
		}
#endif

		else {
		    putchar (*pc++);
		}
		pre = pc;

	    } while ((pc = strchr (pre, '\\')) != NULL);

	    fputs (pre, stdout);
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
      for ( page = 0; page < 10; page++ ) {
	 new_first = page * hits_per_page;

	 if (new_first > msize - 1) break;
	 printf( "<INPUT TYPE=submit NAME=F%ld VALUE=%d>\n", new_first,
		 page + 1 );
      }
   } else {
      long int plh, plw, have_selected_page_gifs;
      /* If not specified, plh and plw no longer default to 15
       * since that prevents the user having different sized page gifs
       */
#if 0
      plh = get_muscat_int("pagelink_height");
      plw = get_muscat_int("pagelink_width");
      have_selected_page_gifs = get_muscat_int("have_selected_page_gifs");
#else
      plh = 0;
      plw = 0;
      have_selected_page_gifs = 0;
#endif

      for ( page = 0; page < 10; page++ ) {
	 new_first = page * hits_per_page;

	 if (new_first > msize - 1) break;

	 if ((new_first == topdoc) && have_selected_page_gifs) {
	     printf( "<INPUT TYPE=image NAME=F%ld VALUE=%d "
		     "SRC=\"%s/page%c%ds.gif\"",
		     new_first, page + 1, gif_dir, dash_chr, page + 1 );
	 }
	 else {
	     printf( "<INPUT TYPE=image NAME=F%ld VALUE=%d "
		     "SRC=\"%s/page%c%d.gif\"",
		     new_first, page + 1, gif_dir, dash_chr, page + 1 );
	 }
	 if (plh) printf( " HEIGHT=%ld", plh );
	 if (plw) printf( " WIDTH=%ld", plw );
	 fputs( " BORDER=0>", stdout );
      }
      putchar ('\n');
   }
}

#ifdef FERRET
static void utf8_to_html(const char *str) {
   const unsigned char *p = (const unsigned char *)str;
   while (1) {
      int ch = *p++;
      if (ch == 0) break;

      /* this is extra magic, not part of utf-8 */
      switch (ch) {
       case '\b': /* was \r but core muscat swallows that... */
	 fputs(" / ", stdout); /* line break in original */
	 continue;
       case '\t':
	 fputs(" * ", stdout); /* bullet point in original */
	 continue;
       case '\f':
	 fputs("&nbsp;", stdout); /* hardspace in original */
	 continue;
       case '<':
	 fputs("&lt;", stdout);
	 continue;
       case '>':
	 fputs("&gt;", stdout);
	 continue;
       case '&':
	 fputs("&amp;", stdout);
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
	    printf("&#%d;",ch);
	    continue;
	 }
      }
      resync:
      putchar(ch);
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
	    fputs(qs, stdout);
	    break;
	 }
	 amp++;
	 while (amp[0] == 'B' && amp[1] == '=' && amp[2] == prefix) {
	    fwrite(qs, amp - qs - 1, 1, stdout);
	    qs = strchr(amp + 3, '&');
	    if (!qs) break;
	    amp = qs + 1;
	 }
      }
   } else {
      fputs(query_string.c_str(), stdout);
   }
}
#endif

static void display_date(time_t date) {
   if (date == (time_t)-1) {
      fputs("Unknown", stdout);
   } else {
      char buf[64];
      struct tm *then;
      then = gmtime(&date);
      strftime(buf, sizeof buf, "%Y-%m-%d", then);
      fputs(buf, stdout);
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
    if (w < weight_threshold) return 1; /** not relevant enough **/
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
       char *path = NULL;
       int port = -1;
       char *caption = NULL;
       char *sample = NULL;
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

       path = xstrdup(pp);
       char *p = strchr(path, '\xfe');
       if (p) {
	  *p = '\0';
	  p++;
	  if (*p != '\xfe' && *p != '\0') caption = p;
	  p = strchr(p, '\xfe');
	  if (p) {
	     *p = '\0';
	     p++;
	     if (*p != '\0') sample = p;
	  }
       }

       {
	  char *p, *q;
	  p = fmtstr;
	  while ((q = strchr(p, '\xff')) != NULL) {
	     fwrite(p, 1, q - p, stdout);
	     switch (q[1]) {
	      case 'C': /* caption */
		if (caption) {
		   utf8_to_html(caption);
		   break;
		}
		/* otherwise fall through... */
	      case 'U': /* url */
		fputs("http://", stdout);
		fputs(hostname, stdout);
		if (port >= 0) printf(":%d", port);
		putchar('/');
		fputs(path, stdout);
		break;
	      case 'H': /* host */
		fputs(hostname, stdout);
		break;
	      case 'd': /* DB name */
		fputs(db_name, stdout);
		break;
	      case 'Q': /* query url */
		print_query_string(q + 2);
		break;
	      case 'S': /* sample */
		if (sample) {
		   utf8_to_html(sample);
		   fputs("...", stdout);
		}
		break;
	      case 's': /* size */
		/* decode packed file size */
		if (size < 33) {
		   fputs("Unknown", stdout);
		} else if (size < 132) {
		   size -= 32;
		   printf("%d%c%dK", size / 10, dec_sep, size % 10);
		} else if (size < 222) {
		   printf("%dK", size - 132 + 10);
		} else if (size < 243) {
		   printf("%d0K", size - 222 + 10);
		} else if (size < 250) {
		   printf("0%c%dM", dec_sep, size - 243 + 3);
		} else if (size < 255) {
		   printf("%dM", size - 249);
		} else {
		   fputs(">5M", stdout);
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
		printf("%d%%", percent);
		break;
	      case 'T': {
#if 0 // FIXME:
		 int comma = 0;		  
		 Give_Muscatf("show qandtof m%ld style q", m);
		 while (!Getfrom_Muscat (&z)) {
		    if (z.length > 6 && z.p[2] == '+' && z.p[3] == 'p') {
		       char *p = z.p + 6;
#ifdef META
		       if (comma) putchar(','); else comma = 1;
#else
		       if (comma) putchar(' '); else comma = 1;
#endif
		       /* quote terms with spaces in */
		       if (strchr(p, ' '))
			  printf("\"%s\"", p);
		       else
			  fputs(p, stdout);
		    }
		 }
#endif
		 break;
	      }
	      case 'G': /* score Gif */
		printf("/fx-gif/score-%d.gif", percent / 10);
		break;
	      case 'X': /* relevance checkboX */
		if (r) {
		   r_displayed[r_di++] = q0;
		   printf("<INPUT TYPE=checkbox NAME=R%ld CHECKED>\n", q0);
		} else {
		   printf("<INPUT TYPE=checkbox NAME=R%ld>\n", q0);
		}
		break;
	      default:
		putchar('\xff');
		putchar(q[1]);
		break;
	     }
	     p = q + 2;
	  }
	  fputs(p, stdout);
       }
       free(path);
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
