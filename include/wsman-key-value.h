
/*******************************************************************************
 * Copyright (C) 2015 SUSE Linux GmbH. All rights reserved.
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
 * @author Klaus Kämpf
 */

#ifndef WSMAN_TYPES_INTERNAL_H_
#define WSMAN_TYPES_INTERNAL_H_


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

struct epr_struct;

/* key/value pair
 * to represent either a value or an epr_t
 */
typedef struct {
  char *key;
  int type; /* 0: char*, else epr_t* */
  union {
    char *text;
    struct epr_struct *epr;
  } v;
} key_value_t;

/* if kv is non-NULL, it's pre-allocated (part of array) */
key_value_t *key_value_create(const char *key, const char *text, const struct epr_struct *epr, key_value_t *prealloc);
void key_value_copy(const key_value_t *from, key_value_t *to);
/* if part_of_array is non-zero, only release key/value, not element itself */
void key_value_destroy(key_value_t *, int part_of_array);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
