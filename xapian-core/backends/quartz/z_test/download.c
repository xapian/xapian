
#include <stdio.h>   /* main etc */
#include "btree.h"

#define true 1
#define unless(E) if(!(E))

int main(int argc, char * argv[])
{
    struct Btree * B = Btree_open_to_read("B/");
    struct Bcursor * BC = Bcursor_create(B);
    struct Btree_item * item = Btree_item_create();

    int i;

    Bcursor_find_key(BC, "", 0);

    while(true)
    {
        unless (Bcursor_get_key(BC, item)) break;
        Bcursor_get_tag(BC, item);
        if (item->key_len != 0)
        {   printf("+");
            for (i = 0; i < item->key_len; i++) printf("%c", item->key[i]);

            printf(" ");
            for (i = 0; i < item->tag_len; i++) printf("%c", item->tag[i]);

            printf("\n");
        }
    }

    Btree_item_lose(item);
    Bcursor_lose(BC);
    Btree_quit(B);
    return 0;
}


