#include <stdio.h>
#include "sfmm.h"

int main(int argc, char const *argv[])
{
    size_t sz_x = sizeof(int), sz_y = 10, sz_x1 = sizeof(int) * 20;
	void *x = sf_malloc(sz_x);
	/* void *y = */ sf_malloc(sz_y);
	sf_realloc(x, sz_x1);

    sf_show_heap();
    return EXIT_SUCCESS;
}
