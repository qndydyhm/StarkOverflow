#include <stdio.h>
#include "sfmm.h"

int main(int argc, char const *argv[]) {
    sf_set_magic(0x0);

        size_t sz_x = sizeof(int), sz_y = 10, sz_x1 = sizeof(int) * 20;
	void *x = sf_malloc(sz_x);
	/* void *y = */ sf_malloc(sz_y);
	x = sf_realloc(x, sz_x1);

	// cr_assert_not_null(x, "x is NULL!");
	// sf_block *bp = (sf_block *)((char *)x - 16);


    sf_show_heap();

    return EXIT_SUCCESS;
}
