#include <stdio.h>
#include "sfmm.h"

int main(int argc, char const *argv[]) {
    sf_set_magic(0x0);

        size_t sz_x = sizeof(double) * 8, sz_y = sizeof(int);
	void *x = sf_malloc(sz_x);
	sf_realloc(x, sz_y);

	// cr_assert_not_null(x, "x is NULL!");
	// sf_block *bp = (sf_block *)((char *)x - 16);



    sf_show_heap();

    return EXIT_SUCCESS;
}
