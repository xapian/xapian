#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/****************************************************************************/

static int nEntries = 0;

typedef struct {
    char name[24];
    char val[1000];
} entry;
entry entries[100];

static int entry_compare(const void *a, const void *b) {
   return strcmp( ((entry*)a)->name, ((entry*)b)->name );
}

static void sort( void ) {
   int i;
   char *p;
   for( i = 0; i < nEntries; i++ ) {
      p = strrchr( entries[i].name, '.' );
      /* Trim off .x or .y (since GIF button X gives X.x and X.y */
      /* We only actually need to keep one of these... */
      if (p) {
	 if (!((p[1] == 'x' || p[1] == 'y') && p[2] == '\0') && entries[i].val[0] != '\0') {
	    char newval[1000];
	    strcpy (newval, p);
	    strcat (newval, ".");
	    strcat (newval, entries[i].val);
	    strcpy (entries[i].val, newval);
	 }
	 *p = '\0';
      }
      p = entries[i].name;
      if (strchr( "RFS", *p)) {
	 char *num = ++p;
	 while (isdigit(*p)) p++;
	 if (*p == '\0') {
	    /* convert R1234=<whatever> into R=1234 */
	    strcpy( entries[i].val, num );
	    *num = '\0';
	 }
      }
   }
   /* and now sort the entries */
   qsort( entries, nEntries, sizeof(entry), entry_compare );
}

/****************************************************************************/

static void getword(char *word, char *line, char stop) {
    int x = 0,y;

    for(x=0;((line[x]) && (line[x] != stop));x++)
        word[x] = line[x];

    word[x] = '\0';
    if(line[x]) ++x;
    y=0;

    while((line[y++] = line[x++]));
}

static void makeword(char *res, char *line, char stop) {
    int x = 0,y;
    char *word = res;

    for(x=0;((line[x]) && (line[x] != stop));x++)
        word[x] = line[x];

    word[x] = '\0';
    if(line[x]) ++x;
    y=0;

    while((line[y++] = line[x++]));
    return;
}

static void fmakeword(char *res, FILE *f, char stop, int *cl) {
    int wsize;
    char *word;
    int ll;

    wsize = 1024;
    ll=0;
    word = res;

    while(1) {
        word[ll] = (char)fgetc(f);
        if(ll==wsize) {
            word[ll+1] = '\0';
	    return;
        }
        --(*cl);
        if((word[ll] == stop) || (feof(f)) || (!(*cl))) {
            if(word[ll] != stop) ll++;
            word[ll] = '\0';
            return;
        }
        ++ll;
    }
}

static char x2c(char *what) {
   register char digit, ch;
   ch = what[0];
   digit = (ch & 0xf) + ((ch & 64) ? 9 : 0);
   ch = what[1];
   digit = digit<<4;
   digit |= ( (ch & 0xf) + ((ch & 64) ? 9 : 0) );
   return digit;
}

static void unescape_url( char *url ) {
   url = strchr( url, '%' );
   if (url) {
      char *pstart = url;
      do {
	 char *p;
	 *url++ = x2c( pstart + 1 );
	 pstart += 3;
	 p = strchr( pstart, '%' );
	 if (p == NULL) p = pstart + strlen(pstart) + 1;
#ifdef NO_MEMMOVE
	 bcopy( pstart, url, p - pstart );
#else
	 memmove( url, pstart, p - pstart );
#endif
	 url += (p - pstart);
	 pstart = p;
      } while (*pstart);
   }
}

#if 0
int main() {
   char buf[1024];
   while (!feof(stdin)) {
      fgets(buf,1024,stdin);
      unescape_url(buf);
      puts(buf);
   }
}
#endif

#if 0
void escape_url (char* in, char* out)
{
    char* pin  = in;
    char* pout = out;

    while (*pin) {
	if (strchr ("<>#%{}|\\^~[]`;/?:@=&$\"", *pin)) {
	    sprintf (pout, "%%%X", (unsigned int) *pin);
	    pout += 3;
	}
	else
	    if (*pin == ' ') *pout++ = '+';
	else
	    *pout++ = *pin;

	pin++;
    }
    *pout = '\0';
}

void escape_quotsp (char* in, char* out)
{
    char* pin  = in;
    char* pout = out;

    while (*pin) {
	if (*pin == '\"') {
	    sprintf (pout, "&quot;");
	    pout += 6;
	}
	else if (*pin == '=') {
	    sprintf (pout, "XXX");
	    pout += 3;
	}
	else
	    if (*pin == ' ') *pout++ = '+';
	else
	    *pout++ = *pin;

	pin++;
    }
    *pout = '\0';
}
#endif

static void plustospace(char *str) {
    register int x;

    for(x=0;str[x];x++) if(str[x] == '+') str[x] = ' ';
}

/****************************************************************************/

void decode_test( void ) {
   int l;
   nEntries = 0;
   while (!feof(stdin)) {
      fgets( entries[nEntries].val, 1000, stdin );
      l = strlen( entries[nEntries].val );
      if (l && entries[nEntries].val[l-1] == '\n')
	 entries[nEntries].val[--l] = '\0';
      if (l == 0) break;
      makeword( entries[nEntries].name, entries[nEntries].val, '=' );
      nEntries++;
   }
   sort();
}

#define MAX_CONTENT_LENGTH 990
void decode_post( void ) {
   char *content_length;
   int cl = MAX_CONTENT_LENGTH;
   
   content_length = getenv("CONTENT_LENGTH");
   /* Netscape Fasttrack server for NT doesn't give CONTENT_LENGTH */
   if (content_length) {
      cl = atoi(content_length);
      if (cl > MAX_CONTENT_LENGTH) {
         printf("<h3>query is too long - please try again with fewer words</h3>\n");
	 exit(0);
      }
   }

   nEntries = 0;
   while (cl && (!feof(stdin))) {
      fmakeword( entries[nEntries].val, stdin, '&', &cl );
      plustospace( entries[nEntries].val );
      unescape_url( entries[nEntries].val );
      makeword( entries[nEntries].name, entries[nEntries].val, '=' );
      nEntries++;
   }

   if (content_length == NULL && (!feof(stdin))) {
      printf("<h3>query is too long - please try again with fewer words</h3>\n");
      exit(0);
   }

   sort();
}

void decode_get( void ) {
   char *q_str;
   q_str = getenv("QUERY_STRING");
   if (q_str == NULL) {
      printf("<h3>No query information to decode.</h3>\n");
      exit(0);
   }

   if (strlen(q_str) > 900) {
      printf("<h3>query string is too long - please try again with fewer words and/or filters</h3>\n");
      exit(0);
   }

   nEntries = 0;
   /*** decode URL into fields ***/
   while (q_str[0] != '\0') {
      getword( entries[nEntries].val, q_str, '&' );
      plustospace( entries[nEntries].val );
      unescape_url( entries[nEntries].val );
      getword( entries[nEntries].name, entries[nEntries].val, '=' );
      nEntries++;
   }
   sort();
}

/* Get an entry with a given name */
/* If there are multiple entries, you might get any, but then qsort will have
 * already jumbled them for you */
char *GetEntry( char *pName ) {
   register int a = 0, b = nEntries - 1, c;
   register int r;
   while (a <= b) {
      c = (unsigned)(a + b) / 2;
      r = strcmp( entries[c].name, pName );
      if (r == 0) return entries[c].val;
      if (r < 0)
         a = c + 1;
      else
         b = c - 1;
   }	
   return NULL; /* couldn't find it... */
}

/* Get *first* entry with a given name, ready to iterate if required */
/* NB first could be any as qsort will have jumbled them */
char *FirstEntry( char *pName, int *p_which ) {
   register int a = 0, b = nEntries - 1, c;
   register int r;
   while (a <= b) {
      c = (unsigned)(a + b) / 2;
      r = strcmp( entries[c].name, pName );
      if (r == 0) {
	 /* now find first hit */
	 while ((c > a) && (strcmp( entries[c-1].name, pName) == 0))
	    c--;
	 if (p_which) *p_which = c;
	 return entries[c].val;
      }
      if (r < 0)
         a = c + 1;
      else
         b = c - 1;
   }
   return NULL; /* couldn't find it... */
}

/* Get the next entry with a given name */
char *NextEntry( char *pName, int *p_which ) {
   (*p_which)++;
   if (strcmp( entries[*p_which].name, pName ) == 0)
      return entries[*p_which].val;
   /* no more... */
   return NULL;
}
