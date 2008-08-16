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



#include <stdio.h>
#include "u/base64.h"



static const char ETable[]="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
static char DTable[256];

static void
init_DTable(void)
{
	static int inited = 0;
	int i;

	if (inited) {
		return;
	}

	for (i = 0; i < 255; i++) {
		DTable[i] = 0x80;
	}
	for (i = 'A'; i <= 'I'; i++) {
		DTable[i] = 0 + (i - 'A');
	}
	for (i = 'J'; i <= 'R'; i++) {
		DTable[i] = 9 + (i - 'J');
	}
	for (i = 'S'; i <= 'Z'; i++) {
		DTable[i] = 18 + (i - 'S');
	}
	for (i = 'a'; i <= 'i'; i++) {
		DTable[i] = 26 + (i - 'a');
	}
	for (i = 'j'; i <= 'r'; i++) {
		DTable[i] = 35 + (i - 'j');
	}
	for (i = 's'; i <= 'z'; i++) {
		DTable[i] = 44 + (i - 's');
	}
	for (i = '0'; i <= '9'; i++) {
		DTable[i] = 52 + (i - '0');
	}
	DTable['+'] = 62;
	DTable['/'] = 63;
	DTable['='] = 0;

	inited = 1;
}

void
ws_base64_encode(const char *from, int len, char *to)
{
	int i;
	size_t n;
	char	in[3] = {0};
	char	out[4];
	const char *s = from;

	while (1) {
		//	n = (s - from + len >= 3) ? 3 : s - from + len;
		n = (len >= 3) ? 3 : len;
		if (n == 0) {
			break;
		}
		in[0] = *s;
		if (n > 1) in[1] = *(s + 1);
		if (n > 2) in[2] = *(s + 2);
		out[0] = ETable[(in[0] >> 2) & 0x3f];
		out[1] = ETable[((in[0] & 3) << 4) | ((in[1] >> 4) & 0x0f )];
		out[2] = ETable[((in[1] & 0xF) << 2) | ((in[2] >> 6 ) & 0x03) ];
		out[3] = ETable[in[2] & 0x3F];
		if (n < 3) {
			out[3] = '=';
		}
		if (n < 2) {
			out[2] = '=';
		}
		for (i = 0; i < 4; i++) {
			*to++ = out[i];
		}
		s += n;
		len -= n;
	}
	*to = 0;
}


int
ws_base64_decode(const char *from, int len, char *to, int to_len)
{
	int i, j;
	int n = 0;
	char a[4], b[4], o[3];
	const char *s = from;
	char c;

	init_DTable();
	if (len % 4 != 0) {
		// Length must be multiple
		return 0;
	}

	if ((len / 4 * 3) > to_len)
		return 0;

	while (1) {
		if ((s - from) == len) {
			break;
		}
		for (i = 0; i < 4; i++) {
			c = *(s + i);
			if (DTable[(unsigned int)c] & 0x80) {
				// Not printable input character
				return 0;
			}
			a[i] = c;
			b[i] = DTable[(unsigned int)c];
		}
		o[0] = (b[0] << 2) | (b[1] >> 4);
		o[1] = (b[1] << 4) | (b[2] >> 2);
		o[2] = (b[2] << 6) | b[3];
		i = a[2] == '=' ? 1 : (a[3] == '=' ? 2 : 3);
		for (j = 0; j < i; j ++) {
			*(to + n + j) = o[j];
		}
		n += i;
		s += 4;
	}

	return n;

}

