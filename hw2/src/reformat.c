/*********************/
/* reformat.c        */
/* for Par 3.20      */
/* Copyright 1993 by */
/* Adam M. Costello  */
/*********************/

/* This is ANSI C code. */


#include "reformat.h"  /* Makes sure we're consistent with the prototype. */
#include "buffer.h"    /* Also includes <stddef.h>.                       */
#include "errmsg.h"

#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#undef NULL
#define NULL ((void *) 0)


struct word {
  const char *chrs;       /* Pointer to the characters in the word */
                          /* (NOT terminated by '\0').             */
  struct word *prev,      /* Pointer to previous word.             */
              *next,      /* Pointer to next word.                 */
                          /* Supposing this word were the first... */
              *nextline;  /*   Pointer to first word in next line. */
  int linelen,            /*   Length of the first line.           */
      score,              /*   Value of objective function.        */
      length;             /* Length of this word.                  */
};


static int choosebreaks(
  struct word *head, struct word *tail, int L, int last, int min
)
/* Chooses linebreaks in a list of struct words according to */
/* the policy in "par.doc" (L is <L>, last is <last>, and    */
/* min is <min>). head must point to a dummy word, and tail  */
/* must point to the last word. Returns <newL>. Uses errmsg. */
{
  struct word *w1, *w2;
  int linelen, shortest, newL, score, minlen, diff, sumsqdiff;
  const char * const impossibility =
    "Impossibility #%d has occurred. Please report it.\n";

/* Determine maximum length of the shortest line: */

  /* Initialize words that could fit on the last line: */

  for (w1 = tail,  linelen = w1->length;
       w1 != head && linelen <= L;
       w1 = w1->prev,  linelen += 1 + w1->length) {
    w1->nextline = NULL;
    w1->score = last ? linelen : L;
  }

  /* Then choose line breaks: */

  for ( ;  w1 != head;  w1 = w1->prev) {
    w1->score = -1;
    for (linelen = w1->length,  w2 = w1->next;
         linelen <= L;
         linelen += 1 + w2->length,  w2 = w2->next) {
      shortest = linelen <= w2->score ? linelen : w2->score;
      if (shortest > w1->score) {
        w1->nextline = w2;
        w1->score = shortest;
      }
    }
    if (w1->score < 0) {
      sprintf(errmsg,impossibility,1);
      return 0;
    }
  }

  shortest = head->next ? head->next->score : L;

  if (!min)
    newL = L;
  else {

  /* Determine the minimum possible longest line: */

    for (w1 = tail;  w1 != head;  w1 = w1->prev) {
      w1->score = L + 1;
      for (linelen = w1->length, w2 = w1->next;
           linelen < w1->score;
           linelen += 1 + w2->length, w2 = w2->next) {
        if (w2) {
          score = w2->score;
          minlen = shortest;
        }
        else {
          score = 0;
          minlen = last ? shortest : 0;
        }
        if (linelen >= minlen) {
          newL = linelen >= score ? linelen : score;
          if (newL < w1->score) {
            w1->nextline = w2;
            w1->score = newL;
          }
        }
        if (!w2) break;
      }
    }

    newL = head->next ? head->next->score : 0;
    if (newL > L) {
      sprintf(errmsg,impossibility,2);
      return 0;
    }
  }

/* Minimize the sum of the squares of the differences */
/* between newL and the lengths of the lines:         */

  for (w1 = tail;  w1 != head;  w1 = w1->prev) {
    w1->score = -1;
    for (linelen = w1->length,  w2 = w1->next;
         linelen <= newL;
         linelen += 1 + w2->length,  w2 = w2->next) {
      diff = newL - linelen;
      minlen = shortest;
      if (w2)
        score = w2->score;
      else {
        score = 0;
        if (!last) diff = minlen = 0;
      }
      if (linelen >= minlen  &&  score >= 0) {
        sumsqdiff = score + diff * diff;
        if (w1->score < 0  ||  sumsqdiff <= w1->score) {
          w1->nextline = w2;
          w1->score = sumsqdiff;
          w1->linelen = linelen;
        }
      }
      if (!w2) break;
    }
  }

  if (head->next && head->next->score < 0) {
    sprintf(errmsg,impossibility,3);
    return 0;
  }

  *errmsg = '\0';
  return newL;
}


char **reformat(const char * const *inlines, int width,
                int prefix, int suffix, int hang, int last, int min)
{
  int numin, numout, affix, L, linelen, newL;
  const char * const *line, **suffixes = NULL, **suf, *end, *p1, *p2;
  char *q1, *q2, **outlines;
  struct word dummy, *head, *tail, *w1, *w2;
  struct buffer *pbuf = NULL;

/* Initialization: */

  *errmsg = '\0';
  dummy.next = dummy.prev = NULL;
  head = tail = &dummy;

/* Count the input lines: */

  for (line = inlines;  *line;  ++line);
  numin = line - inlines;

/* Allocate space for pointers to the suffixes: */

  if (numin) {
    suffixes = malloc(numin * sizeof (const char *));
    if (!suffixes) {
      strcpy(errmsg,outofmem);
      goto rfcleanup;
    }
  }

/* Set the pointers to the suffixes, and create the words: */

  affix = prefix + suffix;
  L = width - prefix - suffix;

  for (line = inlines, suf = suffixes;  *line;  ++line, ++suf) {
    for (end = *line;  *end;  ++end);
    if (end - *line < affix) {
      sprintf(errmsg,
              "Line %d shorter than <prefix> + <suffix> = %d + %d = %d\n",
              line - inlines + 1, prefix, suffix, affix);
      goto rfcleanup;
    }
    end -= suffix;
    *suf = end;
    p1 = *line + prefix;
    for (;;) {
      while (p1 < end && isspace(*p1)) ++p1;
      if (p1 == end) break;
      p2 = p1;
      while (p2 < end && !isspace(*p2)) ++p2;
      if (p2 - p1 > L) p2 = p1 + L;
      w1 = malloc(sizeof (struct word));
      if (!w1) {
        strcpy(errmsg,outofmem);
        goto rfcleanup;
      }
      w1->next = NULL;
      w1->prev = tail;
      tail = tail->next = w1;
      w1->chrs = p1;
      w1->length = p2 - p1;
      p1 = p2;
    }
  }

/* Expand first word if preceeded only by spaces: */

  w1 = head->next;
  if (w1) {
    p1 = *inlines + prefix;
    for (p2 = p1;  isspace(*p2);  ++p2);
    if (w1->chrs == p2) {
      w1->chrs = p1;
      w1->length += p2 - p1;
    }
  }

/* Choose line breaks according to policy in "par.doc": */

  newL = choosebreaks(head,tail,L,last,min);
  if (*errmsg) goto rfcleanup;

/* Construct the lines: */

  pbuf = newbuffer(sizeof (char *));
  if (*errmsg) goto rfcleanup;

  numout = 0;
  w1 = head->next;
  while (numout < hang || w1) {
    linelen = suffix ? newL + affix :
                  w1 ? w1->linelen + prefix :
                       prefix;
    q1 = malloc((linelen + 1) * sizeof (char));
    if (!q1) {
      strcpy(errmsg,outofmem);
      goto rfcleanup;
    }
    additem(pbuf, &q1);
    if (*errmsg) goto rfcleanup;
    ++numout;
    q2 = q1 + prefix;
    if      (numout <= numin) memcpy(q1, inlines[numout - 1], prefix);
    else if (numin > hang)    memcpy(q1, inlines[numin - 1], prefix);
    else                      while (q1 < q2) *q1++ = ' ';
    q1 = q2;
    if (w1)
      for (w2 = w1;  ; ) {
        memcpy(q1, w2->chrs, w2->length);
        q1 += w2->length;
        w2 = w2->next;
        if (w2 == w1->nextline) break;
        *q1++ = ' ';
      }
    q2 += linelen - affix;
    while (q1 < q2) *q1++ = ' ';
    q2 = q1 + suffix;
    if      (numout <= numin) memcpy(q1, suffixes[numout - 1], suffix);
    else if (numin)           memcpy(q1, suffixes[numin - 1], suffix);
    else                      while(q1 < q2) *q1++ = ' ';
    *q2 = '\0';
    if (w1) w1 = w1->nextline;
  }

  q1 = NULL;
  additem(pbuf, &q1);
  if (*errmsg) goto rfcleanup;

  outlines = copyitems(pbuf);

rfcleanup:

  if (suffixes) free(suffixes);

  while (tail != head) {
    tail = tail->prev;
    free(tail->next);
  }

  if (pbuf) {
    if (!outlines)
      for (;;) {
        outlines = nextitem(pbuf);
        if (!outlines) break;
        free(*outlines);
      }
    freebuffer(pbuf);
  }

  return outlines;
}
