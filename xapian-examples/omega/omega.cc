int dlist[64];
int n_dlist = 0;

#include <stdio.h>
#define EXIT(x) exit((x))

#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>

#ifdef SYSTEM5
extern char *sys_errlist[];
#endif
#ifdef SUNOS
extern char *sys_errlist[];
#endif

#include <time.h>

#ifdef FERRET
#define LOGGING 1
#define logging 1
#endif

#ifdef LOGGING
#include <fcntl.h>
#ifdef WIN32 /* KJM 30/7/1997 - added <io.h> for use in LOGGING on NT (read, write and open) */
#include <io.h>
#else
#include <unistd.h>
#endif
#endif

#define MAXBQ 255

#include "cgiparam.h"
#include "main.h"
#include "query.h"

DADatabase database;
Match *matcher;

#ifdef FERRET
static int ssi=0;
#endif

static int map_dbname_to_dir( char *db_dir, size_t len, const char *db_name );

static void make_log_entry( const char *action, long matches );
static void make_query_log_entry( const char *buf, size_t length );

static void do_easter_egg( void );

char *db_name;
static char db_dir[256];
char dash_chr = '-'; /* renamed from dash_char to avoid odd BCPL problem -- Olly 1997-03-19 */
char *fmt = NULL;
char *fmtfile = "t/fmt";

static char muscat_dir[256] = "/usr/muscat";

int have_query; /* use to trap the "no query" case more reliably */

#ifdef WEBCD
#include <direct.h> /* KJM 17/7/1997 _getdrive */
static char CDDriveLetter; /* KJM 17/7/1997 */
#endif

/* defined in query.c */
extern char str_scoreline[];

int main(int argc, char *argv[]) {
    int n;
    char     big_buf[4048];
    long int list_size;
    char*    to = big_buf;
    int      more = 0;
    int      is_old;
    long int topdoc = 0;
    char     *method;
    char     *val;
    int	boolean_present = 0;

    /* log query details */
     {
	extern char **environ;
	char *p = big_buf;
	int len = 4047;
	char **penv;
	int fd;
	fd = open("/tmp/ferenv.log", O_CREAT|O_APPEND|O_WRONLY, 0644);
       
	if (fd != -1) {
	   for (penv = environ; *penv; penv++) {
	      const char *e = *penv;
	      int l = strchr(e, '=') - e;

	      switch (l) {
	       case 4:
		 if (memcmp(e, "PATH", 4) == 0) continue;
		 break;
	       case 9:
		 if (memcmp(e, "USER_NAME", 9) == 0) continue;
		 break;
	       case 10:
		 if (memcmp(e, "DATE_LOCAL", 10) == 0) continue;
		 break;
	       case 11:
		 if (memcmp(e, "REMOTE_PORT", 11) == 0) continue;
		 if (memcmp(e, "REQUEST_URI", 11) == 0) continue;
		 if (memcmp(e, "SCRIPT_NAME", 11) == 0) continue;
		 if (memcmp(e, "SERVER_NAME", 11) == 0) continue;
		 if (memcmp(e, "SERVER_PORT", 11) == 0) continue;
		 break;
	       case 12:
		 if (memcmp(e, "DOCUMENT_URI", 12) == 0) continue;
		 if (memcmp(e, "REDIRECT_URL", 12) == 0) continue;
		 if (memcmp(e, "SERVER_ADMIN", 12) == 0) continue;
		 break;
	       case 13:
		 if (memcmp(e, "DOCUMENT_NAME", 13) == 0) continue;
		 if (memcmp(e, "DOCUMENT_ROOT", 13) == 0) continue;
		 if (memcmp(e, "LAST_MODIFIED", 13) == 0) continue;
		 break;
	       case 14:
		 if (memcmp(e, "REQUEST_METHOD", 14) == 0) continue;
		 break;
	       case 15:
		 if (memcmp(e, "REDIRECT_STATUS", 15) == 0) continue;
		 if (memcmp(e, "SCRIPT_FILENAME", 15) == 0) continue;
		 if (memcmp(e, "SERVER_PROTOCOL", 15) == 0) continue;
		 if (memcmp(e, "SERVER_SOFTWARE", 15) == 0) continue;
		 break;
	       case 16:
		 if (memcmp(e, "SERVER_SIGNATURE", 16) == 0) continue;
		 break;
	       case 17:
		 if (memcmp(e, "GATEWAY_INTERFACE", 17) == 0) continue;
		 break;
	       case 18:
		 if (memcmp(e, "DOCUMENT_PATH_INFO", 18) == 0) continue;
		 break;
	       case 21:
		 if (memcmp(e, "REDIRECT_QUERY_STRING", 21) == 0) continue;
		 break;
	       case 22:
		 if (memcmp(e, "QUERY_STRING_UNESCAPED", 22) == 0) continue;
		 break;
	      }
	      l = strlen(e);

	      if (l + 1 > len) break;
	      memcpy(p, e, l);
	      len -= l + 1;
	      p += l;
	      strcpy(p, "\n");
	      p++;
	   }
	   
	   if (len) {
	      strcpy(p, "\n");
	      p++;
	   }
	   write(fd, big_buf, p - big_buf);
	   close(fd);
	}
     }
   
    setvbuf(stdout, NULL, _IOLBF, 0);
      
    have_query = 0;

    /* 1997-01-23 added so you can find the version of a given FX easily */
    method = getenv("REQUEST_METHOD");
    if (method == NULL) {
       /* Seems we're running from the command line so print a version number and stop */
       puts( "FX "FX_VERSION_STRING" (Actually compiled "__DATE__" "__TIME__")\n"
	     "Options:"
/* this list probably needs pruning */
#ifdef SHAREWARE
	    " FREEWARE"
#endif
#ifdef CHEAPWARE
	    " 6DBLIMIT"
#endif
#ifdef LOGGING
	    " LOGGING"
#endif
#ifdef BIGENDER
	    " BIGENDER"
#endif
#ifdef TRACINGFULL
	    " TRACINGFULL"
#endif
#ifdef UNIX
	    " UNIX"
#endif
#ifdef OS2
	    " OS2"
#endif
#ifdef WIN32
	    " WIN32"
#endif
#ifdef SUNOS
	    " SUNOS"
#endif
#ifdef DOS
	    " DOS"
#endif
#ifdef SOLARIS2
	    " SOLARIS2"
#endif
#ifdef DEBUGMUS
	    " DEBUGMUS"
#endif
#ifdef COMPILED
	    " COMPILED"
# ifndef CMAIN
	    " (newstyle)"
# endif
#endif
#ifdef NO_CONTENTTYPE
/* Stops the output of 'content-type: text/html\n\n' */
	    " NO_CONTENTTYPE"
#endif
#ifdef WEBCD /* Constrains FX to look at one database name only ('1') */
             /* and sets DB_LIMIT to a given number which should be */
	     /* changed for each CD. This is not used at the moment, */
	     /* but should be in the future. */
	    " WEBCD"
#endif
	    );

       puts("Enter NAME=VALUE lines, end with blank line");
    }

    /* Was for XITAMI (used on WebCD), but newer versions behave better */
#ifndef NO_CONTENTTYPE
#ifdef META
    printf("Content-type: text/plain\n\n");
#else
    printf("Content-type: text/html\n\n");
#endif
#endif
    
    if (method == NULL)
        decode_test();
    else if (*method == 'P')
        decode_post();
    else
        decode_get();

    /*** first doc to display ? ***/
    if ((val = GetEntry("F")) != NULL) {
       topdoc = atol(val);
    } else if ((val = GetEntry("TOPDOC")) != NULL) {
       /*** top doc displayed (for expand & show) ***/
       topdoc = atol(val);
    }

    /* FIXME - ick */
    if (topdoc >= 1000) {
       puts("Sorry, only the first 1000 matches are accessible at present");
       exit(0);
    }

    /*** get database name ***/
    db_name = GetEntry("DB");
    /* if no DB specified, use "ferret" if exe leafname matches "ferret*" */
    if (db_name == NULL) db_name = "ferret";
#ifdef META
    ssi = 0;
#else
    /* if we're called from a SSI page, set flag to use query-ssi, etc */
    if (getenv("REDIRECT_QUERY_STRING")) ssi = 1;
#endif

    /* Translate DB parameter to path to database directory */
    if (!map_dbname_to_dir( db_dir, sizeof(db_dir), db_name )) {
       /* no such database */
       EXIT(0); /* was EXIT(1); but that makes some servers swallow our output -- Olly 1997-03-19 */
    }

    if (chdir(db_dir) == -1) {
       /* if we can't cd there, odds are it's not a database */
       printf( "<HTML><HEAD><TITLE>Database '%s' not found</TITLE></HEAD>"
	       "<BODY BGCOLOR=white><H3>Database '%s' not found (or not readable)</H3>\n"
	       "</BODY></HTML>", db_name, db_name );
       goto muscat_error;
    }

    database.open(db_dir, 0);
    matcher = new Match(&database);       
       
#if 0 //def FERRET
     {
	FILE *f;
	f = fopen("t/dlist", "r");
	if (f) {
	   while (!feof(f)) {
	      char dlistbuf[256];
	      if (!fgets(dlistbuf, 256, f)) break;
	      /* da recs /netapp/ferret-data/data-912508010/R terms /netapp/ferret-data/data-912508010/T */
	      if (strncmp(dlistbuf, "da recs ", 8) == 0) {
		 char *p = strstr(dlistbuf + 8, "/data-");
		 if (p) dlist[n_dlist++] = atoi(p + 6);
		 /* printf("<!-- %s %d -->\n", dlistbuf, dlist[n_dlist-1]); */
	      }
	   }
	   fclose( f );
	}
     }
#endif
   
    // FIXME
#if 0
    /* read thousands and decimal separators: e.g. 16<thou>729<dec>8037 */
    
    if (0) {
	char buf[16];
	if (get_muscat_string("dec_sep", buf)) dec_sep = buf[0];
	if (get_muscat_string("thou_sep", buf)) thou_sep = buf[0];
    }
   
    /* allow score line below each match to be translated */
    if (!get_muscat_string( "translate_scoreline", str_scoreline )) {
       /* check for old style translate_score and translate_matching */
       char *p = str_scoreline;
       if (!get_muscat_string( "translate_score", p ))
	  strcpy( p, "Score" );
       p += strlen(p);
       strcpy( p, ": %d%%, " );
       p += strlen(p);
       if (!get_muscat_string( "translate_matching", p ))
	  strcpy( p, "Matching" );
       strcat(p, ":");
    }

    /* allow use of '_' instead of '-' since ISO-9660 CDROMs don't allow '-' */
    dash_chr = ( get_muscat_int("cdrom_friendly") ? '_' : '-' );
#else
    dash_chr = '-';
#endif

    list_size = 0;
    if ((val = GetEntry ("MAXHITS")) != NULL) list_size = atol( val );
    if (list_size <= 10) {
       list_size = 10;
    } else if (list_size >= 1000) {
       list_size = 1000;
    }

    if ((val = GetEntry ("THRESHOLD")) != NULL) {
	percent_min = atoi(val);
    }

    *to = '\0';

#if 0 // FIXME def FERRET
    if ((val = GetEntry("MORELIKE")) != NULL) {
       int doc = atol(val);
       
       Give_Muscatf("rel %ld", doc);
       Ignore_Muscat();
       Give_Muscat("expand 6");
       while (!Getfrom_Muscat (&z)) {
	  check_error(&z);
	  if (z.p[0] == 'I') {
	     if (more) *to++ = ' ';
	     strcpy(to, z.p + 2);	     
	     to += strlen (to);
	     *to++ = '.';
	     *to = '\0';
	     more = 1;
	  }
       }
       Give_Muscat("delrels r0-*");
       Ignore_Muscat();

       if (more) {
	  have_query = 1;
	  goto got_query_from_morelike;
       }
    }

    if ((val = GetEntry("IDSPISPOPD")) != NULL) {
       int doc = atol(val);

       printf("<b>Clunk<b> ... <i>god mode engaged!</i><hr>\n"
	      "Raw record #%d:<br>\n", doc);
      
       Give_Muscat("set p to d d (a) x s + g0;");
       Ignore_Muscat();
       Give_Muscat("pspec !p");
       Ignore_Muscat();
       Give_Muscatf("dprint %ld", doc);
       while (!Getfrom_Muscat (&z)) {
	  check_error(&z);
	  if (z.p[1] == ')' || z.p[1] == ']') {
	     unsigned char *p = z.p + 2;
	     int ch;
	     while ((ch = *p++) != '\0') {
		switch (ch) {
		 case '<':
		   fputs("&lt;", stdout);
		   break;
		 case '>':
		   fputs("&gt;", stdout);
		   break;
		 case '&':
		   fputs("&amp;", stdout);
		   break;
		 case '\t':
		   fputs("\\t", stdout);
		   break;
		 case '\r':
		   fputs("\\r", stdout);
		   break;
		 case '\b':
		   fputs("\\b", stdout);
		   break;		   
		 default:
		   if (ch < 32 || ch >= 127) {
		      printf("\\x%02x", ch);
		   } else {
		      putchar(ch);
		   }
		   break;
		}
	     }
	     fputs("<br>\n", stdout);
	  }
       }
       
       fputs("<hr>\nTerms indexing this record<br>\n"
	     "<table><tr><th>Term</th><th>Freq</th></tr>\n"
	     "<FORM NAME=P METHOD=GET ACTION=\"/\">\n"
	     "<NOSCRIPT><INPUT TYPE=hidden NAME=ADD VALUE=1></NOSCRIPT>\n"
	     "<SCRIPT> <!--\n"
	     "document.write('<INPUT NAME=P VALUE=\"\" SIZE=65>')\n"
	     "// -->\n"
	     "</SCRIPT>\n"
	     "<INPUT ALIGN=middle TYPE=image HEIGHT=56 WIDTH=56 BORDER=0 "
	     "SRC=\"http://www.euroferret.com/fx-gif/find.gif\" "
	     "VALUE=Find>\n",
	     stdout);

       printf("<INPUT TYPE=hidden NAME=DB VALUE=%s>\n", db_name);

       Give_Muscatf("tof %ld style f", doc);
       while (!Getfrom_Muscat (&z)) {
	  check_error(&z);
	  if (z.p[0] == 'I') {
	     unsigned char *p;
	     int ch;
	     int freq = strtol(z.p + 2, &p, 10);
	     
	     while (*p == ' ') p++;
	     
	     fputs("<tr><td>", stdout);
	     if (isupper(*p)) {
		printf("<input type=checkbox name=B value=\"%s\">", p);
	     } else if (strchr(p, ' ')) {
		printf("<input type=checkbox name=X onclick=C(this) "
		       "value=\"&quot;%s&quot;\">", p);
	     } else {
		printf("<input type=checkbox name=X onclick=C(this) "
		       "value=\"%s.\">", p);
	     }
	     printf(" <A HREF=\"/?DB=%s&", db_name);
	     if (isupper(*p)) {
		printf("B=%s\">", p);
	     } else if (strchr(p, ' ')) {
		char *q = p;
		fputs("P=%22", stdout);
		while (*q) {
		   if (*q == ' ')
		      putchar('+');
		   else
		      putchar(*q);
		   q++;
		}
		fputs("%22\">", stdout);
	     } else {
		char *q = p;
		fputs("P=", stdout);
		while (*q) {
		   if (*q == '+')
		      fputs("%2b", stdout);
		   else if (*q == '&')
		      fputs("%26", stdout);
		   else
		      putchar(*q);
		   q++;
		}
		fputs(".\">", stdout);
	     }

	     while ((ch = *p++) != '\0') {
		switch (ch) {
		 case '<':
		   fputs("&lt;", stdout);
		   break;
		 case '>':
		   fputs("&gt;", stdout);
		   break;
		 case '&':
		   fputs("&amp;", stdout);
		   break;
		 case '\t':
		   fputs("\\t", stdout);
		   break;
		 case '\r':
		   fputs("\\r", stdout);
		   break;
		 case '\b':
		   fputs("\\b", stdout);
		   break;		   
		 default:
		   if (ch < 32 || ch >= 127) {
		      printf("\\x%02x", ch);
		   } else {
		      putchar(ch);
		   }
		   break;
		}
	     }
	     printf("</A></td><td>%d</td></tr>\n", freq);
	  }
       }
       fputs("</table>\n", stdout);
#if 0
       fputs("<hr>\nExpand terms<br>\n"
	     "<table><tr><th>Term</th><th>Freq</th></tr>\n", stdout);
       
       Give_Muscatf("rel %ld", doc);
       Ignore_Muscat();
       Give_Muscat("expand 1000");
       while (!Getfrom_Muscat (&z)) {
	  check_error(&z);
	  if (z.p[0] == 'I') {
	     if (more) *to++ = ' ';
	     strcpy(to, z.p + 2);	     
	     to += strlen (to);
	     *to++ = '.';
	     *to = '\0';
	     more = 1;
	  }
       }
       Give_Muscat("delrels r0-*");
       Ignore_Muscat();

       if (more) {
	  have_query = 1;
	  goto got_query_from_morelike;
       }       
#endif
       fputs("<hr>\n", stdout);
       exit(0);
    }
#endif
      
    /*** collect the prob fields ***/   
    val = FirstEntry( "P", &n );
    while (val) {
       if (more) *to++ = ' ';
       /** Easter Egg! **/
       if (!strcmp (val, "Rosebud?")) do_easter_egg ();
       strcpy (to, val);
       more = strlen (val);
       to += more;
       val = NextEntry( "P", &n );
       have_query = 1;
    }

    /*** add expand terms ? **/
    if (GetEntry("ADD") != NULL) {
       val = FirstEntry( "X", &n );
       if (val) have_query = 1;
       while (val) {
	  if (more) *to++ = ' ';
	  strcpy (to, val);
	  to += strlen (to);
	  more = 1;
	  val = NextEntry( "X", &n );
       }
    }
   
#ifdef FERRET
   got_query_from_morelike:
#endif

    /*** set Boolean ***/
    val = FirstEntry( "B", &n );
    while (val != NULL) {
       /* we'll definitely get empty B fields from "-ALL-" options */
       if (isalnum(val[0])) {
	  add_bterm(val);
	  boolean_present = 1;
       }
       have_query = 1;
       val = NextEntry( "B", &n );
    }

#if 0
    /* set R-set now, so that term weights don't change - otherwise
     * it's a nightmare trying to work out weights for plus/minus */
    val = FirstEntry("R", &n);
    while (val != NULL) {
       Give_Muscatf("rel %s", val);
       Ignore_Muscat();
       val = NextEntry("R", &n);
    }
#endif

    if ((val = GetEntry("FMT")) != NULL && *val) {
       static char fmtbuf[20] = "t/fmt.";
       if (strlen(val) <= 10) {
	  char *p, *q;
	  int ch;
	  p = fmtbuf + 6;
	  q = val;	  
	  while ((ch = *q++)) {
	     if (ch < 'a' || ch > 'z') break;
	     *p++ = ch;
	  }
	  if (ch == 0) {
	     *p = '\0';
	     fmt = val;
	     fmtfile = fmtbuf;
	  }
       }       
    }
   
    {
       /*** get old prob query (if any) ***/
       val = GetEntry("OLDP");
       is_old = set_probabilistic(big_buf, val);
       if (!val) is_old = 1; /** not really, but it should work **/
    }

    /* if query has changed, force first page of hits */
    if (is_old < 1) topdoc = 0;

    /*** process commands ***/
    if (1) {
        long matches = do_match( topdoc, list_size );
        if (logging) {
	   if (GetEntry("X")) {
	      make_log_entry("add", matches);
#ifdef FERRET
	   } else if (GetEntry("MORELIKE")) {
	      make_log_entry("morelike", matches);
#endif
	   } else if (big_buf[0]) {
	      make_log_entry("query", matches);
	   }
	}
    }
    /* temporarily stick a newline on so we can add the line to the    */
    /* logfile with one call to write (which seems to be atomic)       */
    *to = '\n';
    if (logging) make_query_log_entry( big_buf, to-big_buf+1 );
    *to = '\0';
   
    return 0;

muscat_error:
    return 0; /* was exit(1); but that makes some servers swallow our output -- Olly 1997-03-19 */
}

/**************************************************************/
static void do_easter_egg( void ) {
   puts("<CENTER><FONT SIZE=\"+2\" COLOR=\"#666600\">\n"
	"Muscat FX Explorer designed by Tom Mortimer.<BR>\n"
	"Coded by Tom, Graham Simms, Kev Metcalfe,<BR>\n"
	"Simon Arrowsmith, Olly Betts, Jon Thackray.<BR>\n"
	"Graphics by Phil \"The Blue Voice\" Holmes.<BR>\n"
	"Muscat Search Engine by Dr Martin Porter.<BR>\n"
	"Cake by Charis Beynon.<BR>\n"
	"20 November 1997.\n"
	"</FONT></CENTER>" );
}

static int map_dbname_to_dir( char *db_dir, size_t len, const char *db_name ) {
   const char *p;
   if (strlen(muscat_dir) + strlen(db_name) + 7 >= len)
      return 0; /* db_name too long */

   p = db_name;
   while ((p = strchr( p, '.' )) != NULL) {
      if (p[1] == '.') return 0; /* db_name has .. in */
      p += 2;
   }
   sprintf( db_dir, "%s/data/%s", muscat_dir, db_name );
   return 1; /* OK */
}

/**************************************************************/

/* support function for opening the template html pages */
extern FILE *page_fopen( const char *page ) {
   FILE *fp;
   char fnm[256];

   sprintf( fnm, "%s/html/%s", db_dir, page );

   if (ssi) strcat(fnm, "-ssi" );

   fp = fopen( fnm, "r" );
   if (fp) return fp;

   /* Perhaps the filename was truncated to 8.3 by DOS or ISO-9660 */
   /* No real need to be too clever about this right now ... */
   if (strcmp( page, "expand_error" ) == 0)
      return page_fopen( "expand_e" );
   if (strcmp( page, "muscat_error") == 0)
      return page_fopen( "muscat_e" );
   if (strcmp( page, "muscat_e") != 0) {
      /* Don't print the next line if we're trying to report a muscat error
       * - just use some default html so old database work nicely */
      printf ("error (%s) opening file (%s)\n", sys_errlist[errno], fnm);
   }
   return NULL;
}

/****************************************************************************/

/* Logging code */

static void make_log_entry( const char *action, long matches ) {
#ifdef LOGGING
   int fd;
   char log_buf[512];

   sprintf( log_buf, "%s/fx.log", db_dir );
   fd = open( log_buf, O_CREAT|O_APPEND|O_WRONLY, 0644 );
       
   if (fd != -1) {
      /* (remote host) (remote logname from identd) (remote user from auth) (time)  */
      /* \"(first line of request)\" (last status of request) (bytes sent)  */
      /* " - - [01/Jan/1997:09:07:22 +0000] \"GET /path HTTP/1.0\" 200 12345\n";*/
      char *p, *var;
      time_t t;

      t = time(NULL);
      var = getenv("REMOTE_HOST");
      if (var == NULL) {
	 var = getenv("REMOTE_ADDR");
	 if (var == NULL) var = "-";
      }
      strcpy( log_buf, var );
      p = log_buf + strlen( log_buf );
      strftime( p, log_buf+sizeof(log_buf)-p, " - - [%d/%b/%Y:%H:%M:%S +0000] \"", gmtime(&t) );
      p += strlen( p );
      sprintf( p, "GET /%s/%s\" 200 %li ", db_name, action, matches );
      p += strlen( p );
      var = getenv( "HTTP_REFERER" );
      if (var != NULL) {
	 *p++ = '\"';
	 strcpy( p, var );
	 strcat( p, "\"\n" );
      } else {
	 strcpy( p, "-\n" );
      }
      write( fd, log_buf, strlen(log_buf) );
      close( fd );
   }
#endif
}

static void make_query_log_entry( const char *buf, size_t length ) {
#ifdef LOGGING
   int fd;
   char log_buf[512];
   sprintf( log_buf, "%s/query.log", db_dir );
   fd = open( log_buf, O_CREAT|O_APPEND|O_WRONLY, 0644 );
   if (fd != -1) {
      write( fd, buf, length );
      close( fd );
   }
#endif
}
