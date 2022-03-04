/*********************/
/* errmsg.c          */
/* for Par 3.20      */
/* Copyright 1993 by */
/* Adam M. Costello  */
/*********************/

/* This is ANSI C code. */


#include "errmsg.h"  /* Makes sure we're consistent with the declarations. */
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>

static char *errmsg = NULL;

void set_error(char *msg)
{
    errmsg = strdup(msg);
}

int is_error() {
    return errmsg ? 1 : 0;
}

int report_error(FILE *file) {
    if (is_error())
    {
        return fputs(errmsg, file) < 0 ? 1 : 0;
    }
    return 0;
}

void clear_error() {
    if (is_error())
    {
        free(errmsg);
        errmsg = NULL;
    }
}
static char *outofmem = "Out of memory.\n";
