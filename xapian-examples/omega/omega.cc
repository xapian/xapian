#include "main.h"

int dlist[64];
int n_dlist = 0;

#include <stdio.h>

#include <fstream>

#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include <time.h>

#include <fcntl.h>
#ifdef WIN32
#include <io.h>
#else
#include <unistd.h>
#endif

#include "cgiparam.h"
#include "query.h"

DADatabase database;
Match *matcher;
RSet *rset;

map<string, string> option;

static bool ssi = false;

const string default_db_name = "ompf";

static string map_dbname_to_dir(const string &db_name);

static void make_log_entry(const char *action, long matches);
static void make_query_log_entry(const string &buf);

string db_name;
static string db_dir;
string fmt;
string fmtfile = "t/fmt";

static const string muscat_dir = "/usr/muscat";

static int main2(int argc, char *argv[]);

int main(int argc, char *argv[])
{
    try {
	return main2(argc, argv);
    }
    catch (...) {
	cout << "Caught unknown exception\n";
    }
    return 0;
}

static int main2(int argc, char *argv[])
{
    int n;
    string big_buf;
    long int list_size;
    bool more = false;
    int      is_old;
    long int topdoc = 0;
    char     *method;
    char     *val;
    
    setvbuf(stdout, NULL, _IOLBF, 0);
      
    /* 1997-01-23 added so you can find the version of a given FX easily */
    method = getenv("REQUEST_METHOD");
    if (method == NULL) {
	/* Seems we're running from the command line so print a version number and stop */
	cout << "ompf "FX_VERSION_STRING" (compiled "__DATE__" "__TIME__")\n"
	        "Enter NAME=VALUE lines, end with blank line\n";
    }

    /* Was for XITAMI (used on WebCD), but newer versions behave better */
#ifndef NO_CONTENTTYPE
#ifdef META
    cout << "Content-type: text/plain\n\n";
#else
    cout << "Content-type: text/html\n\n";
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

    /*** get database name ***/
    val = GetEntry("DB");
    if (val != NULL)
	db_name = val;
    else
	db_name = default_db_name;
#ifdef META
    ssi = false;
#else
    /* if we're called from a SSI page, set flag to use query-ssi, etc */
    if (getenv("REDIRECT_QUERY_STRING")) ssi = true;
#endif

    /* Translate DB parameter to path to database directory */
    db_dir = map_dbname_to_dir(db_name);

    if (chdir(db_dir.c_str()) == -1) {
	// if we can't cd there, odds are it's not a database
	cout << "<HTML><HEAD>"
	        "<TITLE>Database '" << db_name << "' not found</TITLE></HEAD>"
	        "<BODY BGCOLOR=white>"
	        "<H3>Database '" << db_name << "' not found "
	        "(or not readable)</H3>\n"
	        "</BODY></HTML>";
	exit(0);
    }

    // read t/vars
    string vars_file = db_dir + "/t/vars";
    std::ifstream in(vars_file.c_str());
    if (!in) {
	cout << "bogus\n";
	exit(0);
    }    
    string line;
    while (!in.eof()) {
	getline(in, line);
	if (line[0] != '\'') continue;
	size_t i = line.find('\'', 1);
	if (i == string::npos) continue;
	string key = line.substr(1, i - 1);
	i++;
	i = line.find('\'', i);
	if (i == string::npos) continue;
	i++;
	size_t j = line.find('\'', i + 1);
	if (j == string::npos) continue;
	option[key] = line.substr(i, j - i);
    }
    in.close();
	
    database.open(db_dir, 0);
    matcher = new Match(&database);
    rset = new RSet(&database);
    matcher->set_rset(rset);
       
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
	      }
	   }
	   fclose(f);
	}
     }
#endif
   
    /* read thousands and decimal separators: e.g. 16<thou>729<dec>8037 */
    
    if (option["dec_sep"].size()) dec_sep = option["dec_sep"][0];
    if (option["thou_sep"].size()) thou_sep = option["thou_sep"][0];
   
    list_size = 0;
    if ((val = GetEntry ("MAXHITS")) != NULL) list_size = atol( val );
    if (list_size <= 10) {
       list_size = 10;
    } else if (list_size >= 1000) {
       list_size = 1000;
    }

    if ((val = GetEntry("MATCHOP")) != NULL) {
	if (strcmp(val, "AND") == 0 || strcmp(val, "and") == 0) op = AND;
    } else if ((val = GetEntry("THRESHOLD")) != NULL) {
	if (atoi(val) == 100) op = AND;
    }

    big_buf = "";

#if 0 // FIXME def FERRET
    if ((val = GetEntry("MORELIKE")) != NULL) {
       int doc = atol(val);
       
       Give_Muscatf("rel %ld", doc);
       Ignore_Muscat();
       Give_Muscat("expand 6");
       while (!Getfrom_Muscat (&z)) {
	  check_error(&z);
	  if (z.p[0] == 'I') {
	     if (more) big_buf += ' ';
	     big_buf += (z.p + 2);
	     big_buf += '.';
	     more = true;
	  }
       }
       Give_Muscat("delrels r0-*");
       Ignore_Muscat();

       if (more) goto got_query_from_morelike;
    }

    if ((val = GetEntry("IDSPISPOPD")) != NULL) {
       int doc = atol(val);

	cout << "<b>Clunk<b> ... <i>god mode engaged!</i><hr>\n"
	        "Raw record #" << doc << ":<br>\n";
      
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
		      cout << "&lt;";
		      break;
		   case '>':
		      cout << "&gt;";
		      break;
		   case '&':
		      cout << "&amp;";
		      break;
		   case '\t':
		      cout << "\\t";
		      break;
		   case '\r':
		      cout << "\\r";
		      break;
		   case '\b':
		      cout << "\\b";
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
	     cout << "<br>\n";
	  }
       }
       
	cout "<hr>\nTerms indexing this record<br>\n"
	     "<table><tr><th>Term</th><th>Freq</th></tr>\n"
	     "<FORM NAME=P METHOD=GET ACTION=\"/\">\n"
	     "<NOSCRIPT><INPUT TYPE=hidden NAME=ADD VALUE=1></NOSCRIPT>\n"
	     "<SCRIPT> <!--\n"
	     "document.write('<INPUT NAME=P VALUE=\"\" SIZE=65>')\n"
	     "// -->\n"
	     "</SCRIPT>\n"
	     "<INPUT ALIGN=middle TYPE=image HEIGHT=56 WIDTH=56 BORDER=0 "
	     "SRC=\"http://www.euroferret.com/fx-gif/find.gif\" "
	     "VALUE=Find>\n";

	cout << "<INPUT TYPE=hidden NAME=DB VALUE=" << db_name << ">\n";

       Give_Muscatf("tof %ld style f", doc);
       while (!Getfrom_Muscat (&z)) {
	  check_error(&z);
	  if (z.p[0] == 'I') {
	      unsigned char *p;
	      int ch;
	      int freq = strtol(z.p + 2, &p, 10);
	     
	      while (*p == ' ') p++;
	     
	      cout << "<tr><td>";
	      if (isupper(*p)) {
		  cout << "<input type=checkbox name=B value=\"" << p << "\">";
	      } else if (strchr(p, ' ')) {
		  cout << "<input type=checkbox name=X onclick=C(this) "
		          "value=\"&quot;" << p << "&quot;\">";
	      } else {
		  cout << "<input type=checkbox name=X onclick=C(this) "
		          "value=\"" << p << ".\">";
	      }
	      cout << " <A HREF=\"/?DB=" << db_name << "&";
	      if (isupper(*p)) {
		  cout << "B=" << p << "\">";
	      } else if (strchr(p, ' ')) {
		  char *q = p;
		  cout << "P=%22";
		  while (*q) {
		      if (*q == ' ')
			  cout << '+';
		      else
			  cout << *q;
		      q++;
		  }
		  cout << "%22\">";
	      } else {
		  char *q = p;
		  cout << "P=";
		  while (*q) {
		      if (*q == '+')
			  cout << "%2b";
		      else if (*q == '&')
			  cout << "%26";
		      else
			  cout << *q;
		      q++;
		  }
		  cout << ".\">";
	      }

	      while ((ch = *p++) != '\0') {
		  switch (ch) {
		   case '<':
		      cout << "&lt;";
		      break;
		   case '>':
		      cout << "&gt;";
		      break;
		   case '&':
		      cout << "&amp;";
		      break;
		   case '\t':
		      cout << "\\t";
		      break;
		   case '\r':
		      cout << "\\r";
		      break;
		   case '\b':
		      cout << "\\b";
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
	      cout << "</A></td><td>" << freq << "</td></tr>\n";
	  }
       }
       cout << "</table>\n";
#if 0
       cout << "<hr>\nExpand terms<br>\n"
	       "<table><tr><th>Term</th><th>Freq</th></tr>\n";
       
       Give_Muscatf("rel %ld", doc);
       Ignore_Muscat();
       Give_Muscat("expand 1000");
       while (!Getfrom_Muscat (&z)) {
	  check_error(&z);
	  if (z.p[0] == 'I') {
	     if (more) big_buf += ' ';
	     big_buf += (z.p + 2);
	     big_buf += '.';
	     more = true;
	  }
       }
       Give_Muscat("delrels r0-*");
       Ignore_Muscat();

       if (more) goto got_query_from_morelike;
#endif
       cout << "<hr>\n";
       exit(0);
    }
#endif
      
    /*** collect the prob fields ***/   
    val = FirstEntry("P", &n);
    while (val) {
	if (*val) {
	    if (more) big_buf += ' ';
	    big_buf += val;
	    more = true;
	}
	val = NextEntry( "P", &n );
    }

    /*** add expand terms ? **/
    if (GetEntry("ADD") != NULL) {
       val = FirstEntry( "X", &n );
       while (val) {
	   if (more) big_buf += ' ';
	   big_buf += val;
	   more = true;
	   val = NextEntry( "X", &n );
       }
    }
   
#if 0 // FIXME def FERRET
   got_query_from_morelike:
#endif

    /*** set Boolean ***/
    val = FirstEntry( "B", &n );
    while (val != NULL) {
       /* we'll definitely get empty B fields from "-ALL-" options */
       if (isalnum(val[0])) add_bterm(val);
       val = NextEntry( "B", &n );
    }

    if ((val = GetEntry("FMT")) != NULL && *val) {
       if (strlen(val) <= 10) {
	  char *p = val;	  
	  while (islower(*p)) p++;
	  if (*p == '\0') {
	     fmt = val;
	     fmtfile = "t/fmt." + fmt;
	  }
       }       
    }
   
    /*** get old prob query (if any) ***/
    val = GetEntry("OLDP");
    is_old = set_probabilistic(big_buf, val?val:"");
    if (!val) is_old = 1; /** not really, but it should work **/

    /* if query has changed, force first page of hits */
    if (is_old < 1) topdoc = 0;

#if 1
    if (is_old != 0) {
	// set up the R-set
	val = FirstEntry("R", &n);
	while (val != NULL) {
	    docid d = atoi(val);
	    if (d) rset->add_document(d);
	    val = NextEntry("R", &n);
	}
    }
#endif

    /*** process commands ***/
    if (1) {
        long matches = do_match(topdoc, list_size);
	if (GetEntry("X")) {
	    make_log_entry("add", matches);
#if 0 // def FERRET
	} else if (GetEntry("MORELIKE")) {
	    make_log_entry("morelike", matches);
#endif
	} else if (big_buf[0]) {
	    make_log_entry("query", matches);
	}
    }
    // Stick a newline on so we can add the line to the logfile with
    // one call to write (which should be atomic)
    big_buf += '\n';
    make_query_log_entry(big_buf);
   
    return 0;
}

static string map_dbname_to_dir(const string &db_name) {
   const char *p = db_name.c_str();
   while ((p = strchr(p, '.')) != NULL) {
      if (p[1] == '.') throw "naughty hacker"; /* FIXME db_name has .. in */
      p += 2;
   }
   string dir = muscat_dir + "/data/";
   dir += db_name;
   return dir;
}

/**************************************************************/

/* support function for opening the template html pages */
extern FILE *page_fopen(const string &page) {
    FILE *fp;
    string fnm;

    fnm = db_dir + "/html/";
    fnm += page;

    if (ssi) fnm += "-ssi";

    fp = fopen(fnm.c_str(), "r");
    if (fp) return fp;

    /* Perhaps the filename was truncated to 8.3 by DOS or ISO-9660 */
    /* No real need to be too clever about this right now ... */
    if (page == "expand_error")
	return page_fopen("expand_e");

    cout << "Couldn't open file \"" << fnm << "\"\n";
    return NULL;
}

/****************************************************************************/

/* Logging code */

static void make_log_entry( const char *action, long matches ) {
   int fd;
   string log_buf = db_dir + "/fx.log";
   fd = open(log_buf.c_str(), O_CREAT|O_APPEND|O_WRONLY, 0644);
       
   if (fd != -1) {
      char log_buf[512];
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
      sprintf( p, "GET /%s/%s\" 200 %li ", db_name.c_str(), action, matches );
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
}

static void make_query_log_entry(const string &buf) {
    string log_buf = db_dir + "/query.log";
    int fd = open(log_buf.c_str(), O_CREAT|O_APPEND|O_WRONLY, 0644);
    if (fd != -1) {
	write(fd, buf.data(), buf.size());
	close(fd);
    }
}
