
/*******************************************************************************
 * Copyright (C) 2004-2006 Intel Corp. All rights reserved.
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
 * @author Vadim Revyakin
 */

#ifdef HAVE_CONFIG_H
#include <wsman_config.h>
#endif

#include <string.h>
#include <stdlib.h>
#ifdef HAVE_CRYPT_H
#include <crypt.h>
#endif
#include "u/libu.h"
#include <stdio.h>
#include <unistd.h>


int initialize(void *arg);
int authorize(char *username, const char *password);

static char *filename = NULL;

int initialize(void *arg) {
    FILE *fp;

    if (arg == NULL) {
        debug("No password file");
        return 1;
    }
    filename = (char *)arg;
    debug("Basic File authentication uses password file: %s", filename);

    fp = fopen(filename, "r");
    if (fp == NULL) {
        debug("Could not fopen password file %s (current dir %s)", filename, get_current_dir_name());
        return 1;
    }
    fclose(fp);
    return 0;
}

/*
 * constant-time comparison to prevent brute-force attacks on authorize()
 *
 * returns zero only if s1 and s2 are bit-wise identical for the first len characters.
 */
static int
safe_cmp(unsigned const char *s1, unsigned const char *s2, size_t len)
{
  size_t i = 0;
  unsigned char result = 0;

  while (i++ < len) {
    result |= *s1++ ^ *s2++;
  }
  return result;
}

int
authorize(char *username, const char *password)
{
        int             authorized = 0;
        char            l[256], u[65], passwd[129];
        char *newpw = NULL ;
        size_t username_l;
        size_t min_len;

        debug( "Checking basic for user: %s; password XXXXX",
                            username);


        if ((username == NULL) || (password == NULL)) {
                debug("No username (%p) or password (XXXXX)",
                    username);
                return 0;
        }
        username_l = strlen(username);
        FILE *fp = fopen(filename, "r");
        if (!fp) {
            debug( "Couldn't open basic passwd file %s",
                        filename);
            return 0;
        }

        while (fgets(l, sizeof(l), fp) != NULL) {
                if (sscanf(l, "%64[^:]:%128s", u, passwd) != 2)
                    continue;       /* Ignore malformed lines */
                debug( "user: %s,  passwd: XXXX", u);
                min_len = strlen(u);
                if (username_l < min_len) {
                  min_len = username_l;
                }
                if (!safe_cmp((unsigned char *)username, (unsigned char *)u, min_len)) {
                        size_t newpw_l;
                        min_len = strlen(passwd);
                        newpw = crypt(password, passwd);
                        newpw_l = strlen(newpw);
                        if (newpw_l < min_len) {
                          min_len = newpw_l;
                        }
                        debug( "user: %s,  passwd: XXXXX", u );
                        authorized = ( safe_cmp ((unsigned char *)newpw, (unsigned char *)passwd, min_len) == 0 );
                    break;
                }
       }

       fclose(fp);

       return authorized;
}
