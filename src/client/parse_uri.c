/* 
 * CGI query parse sample
 * Copyright (C) 2000, 2001, Just Another Perl User <AyuMoe@JAPU.ORG>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR (JUST ANOTHER PERL USER)
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE
 * AUTHOR OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "parse_uri.h"


#define MAX_QUERY_KEYS 100



struct pair_t * parse_query(const char *string, int separator)
{
  int i;
  char *p;
  char *query_string;
  size_t query_len;
  struct pair_t *query;

  if(!string)
    return NULL;

  if((query = (struct pair_t *)malloc(sizeof(struct pair_t)*MAX_QUERY_KEYS))
     == NULL)
  {
    fprintf(stderr, "Could not malloc\n");
    exit (1);
  }

  query_len = strlen(string);
  if((query_string = malloc(query_len + 1)) == NULL)
  {
    fprintf(stderr, "Could not malloc\n");
    exit (1);
  }

  memcpy(query_string, string, query_len + 1);

  for(i = 0, p = query_string; i < MAX_QUERY_KEYS && *p; i++, p++) {
    while(*p == ' ') p++;

    query[i].name = p;
    while(*p != '=' && *p != separator && *p) p++;
    if(*p != '=') {
      *p = 0;
      query[i].value = NULL;
      continue;
    }
    *p++ = 0;
    
    query[i].value = p;
    while(*p != separator && *p) p++;
    *p = 0;
  }

  query[i].name = NULL;

  for(i = 0; query[i].name; i++) {
    uri_unescape(query[i].name);
    uri_unescape(query[i].value);
  }

  return query;
}

int xdigit_to_int(int c)
{
  c = tolower(c);

  if(isdigit(c))
    return c - '0';
  else if(isxdigit(c))
    return c - 'a';
  else
    return -1;
}


void uri_unescape(char *ptr)
{
  char *nptr = ptr;
  int c, d;

  while(*ptr) {
    if(*ptr == '%') {
      if((c = xdigit_to_int(*++ptr)) != -1 &&
     (d = xdigit_to_int(*++ptr)) != -1) {
    *nptr++ = (c << 4) | d;
    ptr++;
      }
    } else if(*ptr == '+') {
      *nptr++ = ' ';
      ptr++;
    } else {
      *nptr++ = *ptr++;
    }
  }
  *nptr = 0;
  return;
}

