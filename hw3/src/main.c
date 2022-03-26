#include <stdio.h>
#include "sfmm.h"

int main(int argc, char const *argv[]) {
    sf_set_magic(0x0);
	sf_errno = 0;
	size_t sz = sizeof(int);
	int *x = sf_malloc(sz);
    
    *x = 4;

    return EXIT_SUCCESS;
}
