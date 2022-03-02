/*********************/
/* reformat.h        */
/* for Par 3.20      */
/* Copyright 1993 by */
/* Adam M. Costello  */
/*********************/

/* This is ANSI C code. */


char **reformat(const char * const *inlines, int width,
                int prefix, int suffix, int hang, int last, int min);

  /* inlines is a NULL-terminated array of pointers to input lines. The     */
  /* other parameters are the variables of the same name as described in    */
  /* "par.doc". reformat(inlines,width,prefix,suffix,hang,last,min) returns */
  /* a NULL-terminated array of pointers to output lines containing the     */
  /* reformatted paragraph, according to the specification in "par.doc".    */
  /* None of the integer parameters may be negative. reformat() uses errmsg */
  /* (see "errmsg.h"), and returns NULL on failure.                         */
