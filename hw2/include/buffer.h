/*********************/
/* buffer.h          */
/* for Par 3.20      */
/* Copyright 1993 by */
/* Adam M. Costello  */
/*********************/

/* This is ANSI C code. */


/* Note: Those functions declared here which do not use errmsg    */
/* always succeed, provided that they are passed valid arguments. */


#include <stddef.h>


struct buffer;


struct buffer *newbuffer(size_t itemsize);

  /* newbuffer(itemsize) returns a pointer to a new empty struct  */
  /* buffer which holds items of size itemsize. Any struct buffer */
  /* *buf passed to any function declared in this header must     */
  /* have been obtained from this function. itemsize must not be  */
  /* 0. newbuffer() uses errmsg, and returns NULL on failure.     */


void freebuffer(struct buffer *buf);

  /* freebuffer(buf) frees the memory associated     */
  /* with *buf. buf may not be used after this call. */


void clearbuffer(struct buffer *buf);

  /* clearbuffer(buf) removes all items from */
  /* *buf, but does not free any memory.     */


void additem(struct buffer *buf, const void *item);

  /* additem(buf,item) copies *item to the end of *buf. item must point */
  /* to an object of the proper size for *buf. additem() uses errmsg.   */


int numitems(struct buffer *buf);

  /* numitems(buf) returns the number of items in *buf. */


void *copyitems(struct buffer *buf);

  /* copyitems(buf) returns an array of objects of the proper size for    */
  /* *buf, one for each item in *buf, or (void *) 0 if there are no items */
  /* in buf. The elements of the array are copied from the items in *buf, */
  /* in order. The array is allocated with malloc(), so it may be freed   */
  /* with free(). copyitems() uses errmsg, and returns NULL on failure.   */


void *nextitem(struct buffer *buf);

  /* When buf was created by newbuffer, a pointer associated with buf  */
  /* was initialized to point at the first slot in buf. If there is an */
  /* item in this slot, nextitem(buf) advances the pointer to the next */
  /* slot and returns the old value. If there is no item in the slot,  */
  /* nextitem(buf) leaves the pointer where it is and returns NULL.    */


void rewindbuffer(struct buffer *buf);

  /* rewindbuffer(buf) resets the pointer used by  */
  /* nextitem() to point at the first slot in buf. */
