#include <stdio.h>
#include "sfmm.h"

int main(int argc, char const *argv[])
{
    sf_malloc(200);
    sf_malloc(250);
    sf_malloc(300);
    sf_show_heap();
    return EXIT_SUCCESS;
}
