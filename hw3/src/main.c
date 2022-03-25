#include <stdio.h>
#include "sfmm.h"

int main(int argc, char const *argv[]) {
    sf_set_magic(0x0);
    double* ptr = sf_malloc(sizeof(double));

    *ptr = 114514;

    printf("%f\n", *ptr);

    sf_free(ptr);

    sf_show_heap();
    sf_show_free_lists();

    return EXIT_SUCCESS;
}
