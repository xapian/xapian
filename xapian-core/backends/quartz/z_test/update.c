
#include <stdio.h>   /* main etc */

#include "btree.h"

#define true 1
#define false 0

static int max = 0;

static int trim(int i) { return i > max ? max : i; }

static void process_lines(struct Btree * B, FILE * f)
{
    int line_count = 0;
    unsigned char * s = (unsigned char *) malloc(10000);

    while(true)
    {   int i = 0;
        int key_len = 0;
        int j = 0;
        int mode = 0;
        while(true)
        {   int ch = getc(f);
            if (ch == EOF) { free(s); return; }
            if (ch == ' ')
            {   if (i == 0) continue;
                if (j == 0) j = i;
            }
            if (ch == '+' && i == 0) { mode = 1; continue; }
            if (ch == '-' && i == 0) { mode = 2; continue; }
            if (ch == '\n')
            {
                line_count++;
                if (mode == 0)
                {   if (i == 0) break;
                    fprintf(stderr, "No '+' or '-' on line %d\n", line_count);
                    exit(1);
                }
                else
                if (mode == 1)
                {   /*if (i > 0)*/
                        if (j > 0) Btree_add(B, s, trim(j), s + j + 1, i - j - 1);
                             else  Btree_add(B, s, trim(i), s, 0);
                    break;
                }
                else
                {   /*if (i > 0)*/
                        if (j > 0) Btree_delete(B, s, trim(j));
                             else  Btree_delete(B, s, trim(i));
                    break;
                }
            }
            s[i++] = ch;
        }
    }
}

int main(int argc, char * argv[])
{

    unsigned char s[100];
    struct Btree * b = Btree_open_to_write("B/");
    unsigned long revision = b->revision_number;
    max = b->max_key_len;

    // FIXME: any value works as the second argument...
    if (argc == 1 || argc > 3) { printf("Syntax: %s BTREE_FILE [--compact-mode]\n", argv[0]); exit(1); }
    if (argc == 3) { printf("Compact mode\n"); Btree_full_compaction(b, true); }
    {   FILE * f = fopen(argv[1], "r");
        if (f == 0) { fprintf(stderr, "File %s not found\n", argv[1]); exit(1); }

        process_lines(b, f);
        close(f);

    }

    Btree_close(b, revision + 1);

    return 0;
}

