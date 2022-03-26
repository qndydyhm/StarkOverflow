#include <stdio.h>
#include "sfmm.h"

int main(int argc, char const *argv[]) {
    sf_set_magic(0x0);


        size_t sz_u = 200, sz_v = 150, sz_w = 50, sz_x = 150, sz_y = 200, sz_z = 250;
	void *u = sf_malloc(sz_u);
        sf_show_heap();
	/* void *v = */ sf_malloc(sz_v);
        sf_show_heap();
	void *w = sf_malloc(sz_w);
        sf_show_heap();
	/* void *x = */ sf_malloc(sz_x);
        sf_show_heap();
    
	void *y = sf_malloc(sz_y);
        sf_show_heap();
	/* void *z = */ sf_malloc(sz_z);
    sf_show_heap();

	sf_free(u);
    sf_show_heap();
	sf_free(w);
    sf_show_heap();
	sf_free(y);

    return EXIT_SUCCESS;
}
