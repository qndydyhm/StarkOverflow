#include <stdio.h>
#include "sfmm.h"

int main(int argc, char const *argv[]) {
    sf_set_magic(0x0);
    	size_t sz_x = 8, sz_y = 200, sz_z = 1;
	/* void *x = */ sf_malloc(sz_x);
	void *y = sf_malloc(sz_y);
	/* void *z = */ sf_malloc(sz_z);

	sf_free(y);

    return EXIT_SUCCESS;
}
