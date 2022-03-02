/*********************/
/* errmsg.h          */
/* for Par 3.20      */
/* Copyright 1993 by */
/* Adam M. Costello  */
/*********************/

/* This is ANSI C code. */


extern char errmsg[163];

/* Any function which uses errmsg must, before returning, */
/* either set errmsg[0] to '\0' (indicating success), or  */
/* write an error message string into errmsg, (indicating */
/* failure), being careful not to overrun the space.      */


extern const char * const outofmem;  /* "Out of memory.\n" */
