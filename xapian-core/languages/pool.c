
#include <stdio.h>
#include <string.h>

#include "pool.h"

#define true 1
#define false 0

static void merge(int n, char * p, char * q, char * r, char * l, int k,
                  int (*f)(char *, char *))
  /* repeatedly merges n-byte sequences of items of pitch k from [p] with
     [q] to [r] using comparison routine f. l is a limit for q. */
{  char * q0 = q;
   if (q0 > l) { memmove(r, p, l-p); return; }
   while (p < q0)
   {  char * pl = n+p;
      char * ql = n+q;
      if (ql > l) ql = l;
      while(true)
      {  if (p >= pl) { memmove(r, q, ql-q); r += ql-q; q = ql; break; }
         if (q >= ql) { memmove(r, p, pl-p); r += pl-p; p = pl; break; }
         if (f(p, q)) { memmove(r, p, k); p += k; }
                else { memmove(r, q, k); q += k; }
         r += k;
      }
   }
   memmove(r, q, l-q);
}

static void sort(char * p, int c, int l, int k,
                 int (*f)(char *, char *))
{   /* sorts items of pitch k starting at [p+c]: limit l. k divides
       l-c. f(p, q) is true <=> [p] before [q] */
    char * q = malloc(l-c);
    int j = k;
    int w = l-c;
    while (j < w)
    {   int cycle;
        for (cycle = 1; cycle <= 2; cycle++)
        {   int h = (w+j-1) / j / 2 * j; /* half way */
            if (cycle == 1) merge(j, p+c, p+c+h, q, p+l, k, f);
                       else merge(j, q, q+h, p+c, q+w, k, f);
            j *= 2;
        }
    }
    free(q);
}

struct pool_entry {

    char * translation;
    char * pointer;
    int length;

};

static void print_entry(struct pool_entry * p)
    {
       { int j; for (j=0;j<p->length;j++) fprintf(stderr, "%c", (p->pointer)[j]); }
       fprintf(stderr, " --> %s\n", p->translation);
    }

/*
static void print_pool(struct pool * p)
{   int i;
    int size = p->size;
    struct pool_entry * q = p->entries;
    fprintf(stderr, "\nPool:\n");
    for (i = 0; i < size; i++) print_entry(q+i);
}
*/

static int compare(char * char_p, char * char_q)
{   struct pool_entry * p = (struct pool_entry *) char_p;
    struct pool_entry * q = (struct pool_entry *) char_q;
    if (p->length == q->length) return memcmp(p->pointer, q->pointer, p->length) < 0;
    return p->length < q->length;
}

static int count_slashes(char * s[])
{   int slash_count = 0;
    int i;
    for (i = 1; s[i] != 0; i += 2)
    {   char * p = s[i];
        int j = 0;
        while (p[j] != 0) if (p[j++] == '/') slash_count++;
    }
    return slash_count;
}

extern struct pool * create_pool(char * s[])
{   int size = count_slashes(s);
    struct pool_entry * z = (struct pool_entry *) malloc(size * sizeof(struct pool_entry));
    struct pool_entry * q = z;
    int i;
    for (i = 1; s[i] != 0; i += 2)
    {   char * p = s[i];
        int j = 0;
        int j0 = 0;
        while(true)
        {   if (p[j] == 0)
            {  if (j0 != j) { fprintf(stderr, "%s lacks final '/'\n", p); exit(1); }
               break;
            }
            if (p[j] == '/')
            {
                q->translation = s[i-1];
                q->pointer = p+j0; q->length = j-j0;
                q++;
                j0 = j+1;
            }
            j++;
        }
    }
    sort((char *) z, 0, size * sizeof(struct pool_entry), sizeof(struct pool_entry), compare);

    /* now validate the contents */

    for (i = 1; i < size; i++)
    {   struct pool_entry * p = z+i;
        struct pool_entry * q = z+i-1;
        if (p->length == q->length && memcmp(p->pointer, q->pointer, p->length) == 0)
        {   fprintf(stderr, "warning: "); print_entry(p);
            fprintf(stderr, " and "); print_entry(q);
        }
    }

    {   struct pool * p = (struct pool *) malloc(sizeof(struct pool));
        p->entries = z;
        p->size = size;
        return p;
    }
}

static int compare_to_pool(int length, char * s, int length_p, char * s_p)
{   if (length != length_p) return length-length_p;
    return memcmp(s, s_p, length);
}

extern char * search_pool(struct pool * p, int length, char * s)
{   int i = 0;
    int j = p->size;
    struct pool_entry * q = p->entries;
    if (j == 0) return 0;  /* empty pool */
    if (compare_to_pool(length, s, q->length, q->pointer) < 0) return 0;
    while(true)
    {
        int h = (i+j)/2;
        int diff = compare_to_pool(length, s, (q+h)->length, (q+h)->pointer);
        if (diff == 0) return (q+h)->translation;
        if (j-i <= 1) return 0;
        if (diff < 0) j = h; else i = h;
    }
}

extern void free_pool(struct pool * p)
{   free(p->entries);
    free(p);
}

