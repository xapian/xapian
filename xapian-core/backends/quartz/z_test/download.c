
#include <stdio.h>   /* main etc */
#include "btree.h"
#include "om/omerror.h"

int main(int argc, char * argv[])
{
    try {
	Btree btree;
	if (!btree.open_to_read("B/"))
	    return 1;

	AutoPtr<Bcursor> BC = Bcursor_create(&btree);
	struct Btree_item * item = Btree_item_create();

	int i;

	BC->find_key((byte *)"", 0);

	while(BC->get_key(item))
	{
	    BC->get_tag(item);

	    if (item->key_len != 0)
	    {
		printf("+");
		for (i = 0; i < item->key_len; i++) printf("%c", item->key[i]);

		printf(" ");
		for (i = 0; i < item->tag_len; i++) printf("%c", item->tag[i]);

		printf("\n");
	    }
	}

	Btree_item_lose(item);
    } catch(OmError & e) {
    }
    return 0;
}


