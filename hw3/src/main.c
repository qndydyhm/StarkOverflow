#include <stdio.h>
#include "sfmm.h"

int main(int argc, char const *argv[])
{
    size_t sz_x = sizeof(int);
    void *x1 = sf_malloc(sz_x);
    void *x2 = sf_malloc(sz_x);
    void *x3 = sf_malloc(sz_x);
    void *x4 = sf_malloc(sz_x);
    void *x5 = sf_malloc(sz_x);

    sf_show_heap();

    sf_free(x1);
    sf_free(x2);
    sf_free(x3);
    sf_free(x4);
    sf_free(x5);
    

    void *x6 = sf_malloc(sz_x);
    sf_show_heap();
    sf_free(x6);

    sf_show_heap();
    return EXIT_SUCCESS;
}
