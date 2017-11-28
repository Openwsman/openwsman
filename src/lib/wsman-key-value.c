/*******************************************************************************
 * Copyright (C) SUSE Linux GmbH. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *  - Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 *  - Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 *  - Neither the name of Intel Corp. nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL Intel Corp. OR THE CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *******************************************************************************/

/**
 * @author Klaus Kämpf, SUSE Linux
 */

#ifdef HAVE_CONFIG_H
#include <wsman_config.h>
#endif

#define _GNU_SOURCE
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <assert.h>

#include "u/libu.h"
#include "wsman-types.h"
#include "wsman-key-value.h"
#include "wsman-epr.h"

key_value_t *
key_value_create(const char *key, const char *text, const epr_t *epr, key_value_t *prealloc)
{
  if (prealloc == NULL)
    prealloc = (key_value_t *)u_malloc(sizeof(key_value_t));
  if (!prealloc) {
    debug("u_malloc() failed in key_value_create\n");
    return NULL;
  }
  if (key) /* might be NULL if only value is stored */
    prealloc->key = u_strdup(key);
  else
    prealloc->key = NULL;
  if (text) {
    prealloc->type = 0;
    prealloc->v.text = u_strdup(text);
  }
  else {
    prealloc->type = 1;
    prealloc->v.epr = epr_copy(epr);
  }
  return prealloc;
}

void
key_value_copy(const key_value_t *from, key_value_t *to)
{
  if (from->key)
    to->key = u_strdup(from->key);
  else
    to->key = NULL;
  to->type = from->type;
  if (from->type == 0)
    to->v.text = u_strdup(from->v.text);
  else
    to->v.epr = epr_copy(from->v.epr);
}

void
key_value_destroy(key_value_t *kv, int part_of_array)
{
  u_free(kv->key);
  if (kv->type == 0)
    u_free(kv->v.text);
  else
    epr_destroy(kv->v.epr);

  if (part_of_array == 0) {
    u_free(kv);
  }
}
