
#include <stdio.h>   /* main etc */
#include "btree.h"

int main(int argc, char * argv[])
{
    if (argc > 2) // FIXME: complain
    Btree_check("B/", argc == 1 ? "" : argv[1]);

    return 0;
}

