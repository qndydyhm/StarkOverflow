/*********************/
/* buffer.c          */
/* for Par 3.20      */
/* Copyright 1993 by */
/* Adam M. Costello  */
/*********************/

/* This is ANSI C code. */


/* additem(), copyitems(), and nextitem() rely on the fact that */
/* sizeof (char) is 1. See section A7.4.8 of The C Programming  */
/* Language, Second Edition, by Kerninghan and Ritchie.         */


#include "buffer.h"  /* Makes sure we're consistent with the */
                     /* prototypes. Also includes <stddef.h> */
#include "errmsg.h"

#include <stdlib.h>
#include <string.h>

#undef NULL
#define NULL ((void *) 0)


struct buffer {
  struct block *firstblk, /* The first block.                    */
               *current,  /* The last non-empty block, or        */
                          /* firstblk if all are empty.          */
               *nextblk;  /* The block containing the item to be */
                          /* returned by nextitem(), or NULL.    */
  int nextindex;          /* Index of item in nextblock->items.  */
  size_t itemsize;        /* The size of an item.                */
};

struct block {
  struct block *next;  /* The next block, or NULL if none.              */
  void *items;         /* Storage for the items in this block.          */
  int maxhere,         /* Number of items that fit in *items.           */
      numprevious,     /* Total of numhere for all previous blocks.     */
      numhere;         /* The first numhere slots in *items are filled. */
};


struct buffer *newbuffer(size_t itemsize)
{
  struct buffer *buf;
  struct block *blk;
  void *items;
  int maxhere;

  maxhere = 124 / itemsize;
  if (maxhere < 4) maxhere = 4;

  buf = (struct buffer *) malloc(sizeof (struct buffer));
  blk = (struct block *) malloc(sizeof (struct block));
  items = malloc(maxhere * itemsize);
  if (!buf || !blk || !items) {
    strcpy(errmsg,outofmem);
    goto nberror;
  }

  buf->itemsize = itemsize;
  buf->firstblk = buf->current = buf->nextblk = blk;
  buf->nextindex = 0;
  blk->numprevious = blk->numhere = 0;
  blk->maxhere = maxhere;
  blk->items = items;

  *errmsg = '\0';
  return buf;

  nberror:
  if (buf) free(buf);
  if (blk) free(blk);
  if (items) free(items);
  return NULL;
}


void freebuffer(struct buffer *buf)
{
  struct block *blk, *tmp;

  blk = buf->firstblk;
  while (blk) {
    tmp = blk;
    blk = blk->next;
    if (tmp->items) free(tmp->items);
    free(tmp);
  }

  free(buf);
}


void clearbuffer(struct buffer *buf)
{
  struct block *blk;

  for (blk = buf->firstblk;  blk;  blk = blk->next)
    blk->numhere = 0;

  buf->current = buf->firstblk;
}


void additem(struct buffer *buf, const void *item)
{
  struct block *blk, *new;
  void *items;
  int maxhere;
  size_t itemsize = buf->itemsize;

  blk = buf->current;

  if (blk->numhere == blk->maxhere) {
    new = blk->next;
    if (!new) {
      maxhere = 2 * blk->maxhere;
      new = (struct block * ) malloc(sizeof (struct block));
      items = malloc(maxhere * itemsize);
      if (!new || !items) {
        strcpy(errmsg,outofmem);
        goto aierror;
      }
      blk->next = new;
      new->next = NULL;
      new->maxhere = maxhere;
      new->numprevious = blk->numprevious + blk->numhere;
      new->numhere = 0;
      new->items = items;
    }
    blk = buf->current = new;
  }

  memcpy( ((char *) blk->items) + (blk->numhere * itemsize), item, itemsize );

  ++blk->numhere;

  *errmsg = '\0';
  return;

  aierror:
  if (new) free(new);
  if (items) free(items);
}


int numitems(struct buffer *buf)
{
  struct block *blk = buf->current;
  return blk->numprevious + blk->numhere;
}


void *copyitems(struct buffer *buf)
{
  int n;
  void *r;
  struct block *blk, *b;
  size_t itemsize = buf->itemsize;

  b = buf->current;
  n = b->numprevious + b->numhere;
  if (!n) return NULL;

  r = malloc(n * itemsize);
  if (!r) {
    strcpy(errmsg,outofmem);
    return NULL;
  }

  b = b->next;

  for (blk = buf->firstblk;  blk != b;  blk = blk->next)
    memcpy( ((char *) r) + (blk->numprevious * itemsize),
            blk->items, blk->numhere * itemsize);

  *errmsg = '\0';
  return r;
}


void rewindbuffer(struct buffer *buf)
{
  buf->nextblk = buf->firstblk;
  buf->nextindex = 0;
}


void *nextitem(struct buffer *buf)
{
  void *r;

  if (!buf->nextblk || buf->nextindex >= buf->nextblk->numhere)
    return NULL;

  r = ((char *) buf->nextblk->items) + (buf->nextindex * buf->itemsize);

  if (++buf->nextindex >= buf->nextblk->maxhere) {
    buf->nextblk = buf->nextblk->next;
    buf->nextindex = 0;
  }

  return r;
}
