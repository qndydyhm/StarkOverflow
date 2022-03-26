#include <stdio.h>
#include "sfmm.h"

int main(int argc, char const *argv[]) {
    sf_set_magic(0x0);

        size_t sz_x = sizeof(int) * 20, sz_y = sizeof(int) * 16;
	void *x = sf_malloc(sz_x);
	sf_realloc(x, sz_y);



    sf_show_heap();

    return EXIT_SUCCESS;
}
