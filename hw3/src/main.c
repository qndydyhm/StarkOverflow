#include <stdio.h>
#include "sfmm.h"

int main(int argc, char const *argv[]) {
    sf_set_magic(0x0);


	sf_malloc(4032);
    sf_show_heap();

    return EXIT_SUCCESS;
}
