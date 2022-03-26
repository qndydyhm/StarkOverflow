#include <stdio.h>
#include "sfmm.h"

int main(int argc, char const *argv[]) {
    sf_set_magic(0x0);

        size_t sz_u = 200, sz_v = 150, sz_w = 50, sz_x = 150, sz_y = 200, sz_z = 250;
	void *u = sf_malloc(sz_u);
	/* void *v = */ sf_malloc(sz_v);
	void *w = sf_malloc(sz_w);
	/* void *x = */ sf_malloc(sz_x);
	void *y = sf_malloc(sz_y);
	/* void *z = */ sf_malloc(sz_z);

	sf_free(u);
	sf_free(w);
	sf_free(y);

    int i = 3;
	sf_block *bp = sf_free_list_heads[i].body.links.next;
    printf("Wrong first block in free list %d: (found=%p, exp=%p, u=%p, w=%p)\n",
                     i, &bp->header, (char *)y - 8, (char *)u - 8, (char *)w - 8);
    sf_show_heap();


    sf_show_heap();

    return EXIT_SUCCESS;
}
