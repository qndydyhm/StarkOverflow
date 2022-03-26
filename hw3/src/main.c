#include <stdio.h>
#include "sfmm.h"

int main(int argc, char const *argv[])
{
    sf_malloc(sizeof(int));
	sf_free(((void *)(intptr_t)sf_mem_end()) - 2 * sizeof(sf_header));
    return EXIT_SUCCESS;
}
