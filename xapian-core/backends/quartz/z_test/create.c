
#include <stdio.h>   /* main etc */
#include "btree.h"


static int readn(char * s)
{   int n = 0;
    int len = strlen(s);
    int i;
    for (i = 0; i < len; i++) n = 10 * n + s[i] - '0';
    return n;
}

int main(int argc, char * argv[])
{
    if (argc > 2) { printf("Syntax: %s [BLOCK_SIZE]\n", argv[0]); exit(1); }
    int n = argc > 1 ? readn(argv[1]) : 1024;
    int res = Btree_create("B/", n);
    if (res == 0)
    printf("B/DB created with block size %d\n", n);
    else printf("error %d\n", res);
    return 0;
}

