/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * This code implements the MD5 message-digest algorithm.
 * The algorithm is due to Ron Rivest.  This code was
 * written by Colin Plumb in 1993, no copyright is claimed.
 * This code is in the public domain; do with it what you wish.
 *
 * Equivalent code is available from RSA Data Security, Inc.
 * This code has been tested against that, and is equivalent,
 * except that you don't need to include two pages of legalese
 * with every copy.
 *
 * To compute the message digest of a chunk of bytes, declare an
 * WsmanMD5Context structure, pass it to wsman_md5_init, call soup_md5_update as
 * needed on buffers full of bytes, and then call wsman_md5_Final, which
 * will fill a supplied 16-byte array with the digest.
 */

#include "wsman-md5-utils.h"
#include <string.h>

static void wsman_md5_transform (guint32 buf[4], const guint32 in[16]);

/*
 * Note: this code is harmless on little-endian machines.
 */
static void 
byte_reverse (guchar *buf, guint32 longs)
{
	guint32 t;
	do {
		t = (guint32) ((guint32) buf[3] << 8 | buf[2]) << 16 |
			((guint32) buf[1] << 8 | buf[0]);
		*(guint32 *) buf = t;
		buf += 4;
	} while (--longs);
}

/**
 * wsman_md5_init: Initialise an md5 context object
 * @ctx: md5 context 
 * 
 * Initialise an md5 buffer. 
 *
 **/
void 
wsman_md5_init (WsmanMD5Context *ctx)
{
	ctx->buf[0] = 0x67452301;
	ctx->buf[1] = 0xefcdab89;
	ctx->buf[2] = 0x98badcfe;
	ctx->buf[3] = 0x10325476;
	
	ctx->bits[0] = 0;
	ctx->bits[1] = 0;
	
	ctx->doByteReverse = (G_BYTE_ORDER == G_BIG_ENDIAN);
}



/**
 * wsman_md5_update: add a buffer to md5 hash computation
 * @ctx: conetxt object used for md5 computaion
 * @buf: buffer to add
 * @len: buffer length
 * 
 * Update context to reflect the concatenation of another buffer full
 * of bytes. Use this to progressively construct an md5 hash.
 **/
void 
wsman_md5_update (WsmanMD5Context *ctx, gconstpointer buf, gsize len)
{
	const char *cbuf = buf;
	guint32 t;
	
	/* Update bitcount */
	
	t = ctx->bits[0];
	if ((ctx->bits[0] = t + ((guint32) len << 3)) < t)
		ctx->bits[1]++;		/* Carry from low to high */
	ctx->bits[1] += len >> 29;
	
	t = (t >> 3) & 0x3f;	/* Bytes already in shsInfo->data */
	
	/* Handle any leading odd-sized chunks */
	
	if (t) {
		guchar *p = (guchar *) ctx->in + t;
		
		t = 64 - t;
		if (len < t) {
			memcpy (p, cbuf, len);
			return;
		}
		memcpy (p, cbuf, t);
		if (ctx->doByteReverse)
			byte_reverse (ctx->in, 16);
		wsman_md5_transform (ctx->buf, (guint32 *) ctx->in);
		cbuf += t;
		len -= t;
	}
	/* Process data in 64-byte chunks */
	
	while (len >= 64) {
		memcpy (ctx->in, cbuf, 64);
		if (ctx->doByteReverse)
			byte_reverse (ctx->in, 16);
		wsman_md5_transform (ctx->buf, (guint32 *) ctx->in);
		cbuf += 64;
		len -= 64;
	}
	
	/* Handle any remaining bytes of data. */
	
	memcpy (ctx->in, cbuf, len);
}

/*
 * Final wrapup - pad to 64-byte boundary with the bit pattern 
 * 1 0* (64-bit count of bits processed, MSB-first)
 */
/**
 * wsman_md5_final: copy the final md5 hash to a bufer
 * @digest: 16 bytes buffer
 * @ctx: context containing the calculated md5
 * 
 * Performs the final md5 transformation on the context, and
 * then copies the resulting md5 hash to a buffer
 **/
void 
wsman_md5_final (WsmanMD5Context *ctx, guchar digest[16])
{
	guint32 count;
	guchar *p;
	
	/* Compute number of bytes mod 64 */
	count = (ctx->bits[0] >> 3) & 0x3F;
	
	/* Set the first char of padding to 0x80.  This is safe since there is
	   always at least one byte free */
	p = ctx->in + count;
	*p++ = 0x80;
	
	/* Bytes of padding needed to make 64 bytes */
	count = 64 - 1 - count;
	
	/* Pad out to 56 mod 64 */
	if (count < 8) {
		/* Two lots of padding:  Pad the first block to 64 bytes */
		memset (p, 0, count);
		if (ctx->doByteReverse)
			byte_reverse (ctx->in, 16);
		wsman_md5_transform (ctx->buf, (guint32 *) ctx->in);
		
		/* Now fill the next block with 56 bytes */
		memset (ctx->in, 0, 56);
	} else {
		/* Pad block to 56 bytes */
		memset (p, 0, count - 8);
	}
	if (ctx->doByteReverse)
		byte_reverse (ctx->in, 14);
	
	/* Append length in bits and transform */
	((guint32 *) ctx->in)[14] = ctx->bits[0];
	((guint32 *) ctx->in)[15] = ctx->bits[1];
	
	wsman_md5_transform (ctx->buf, (guint32 *) ctx->in);
	if (ctx->doByteReverse)
		byte_reverse ((guchar *) ctx->buf, 4);
	memcpy (digest, ctx->buf, 16);
}



/**
 * wsman_md5_final_hex: copy the final md5 hash to a bufer
 * @digest: 33 bytes buffer (32 hex digits plus NUL)
 * @ctx: context containing the calculated md5
 * 
 * As wsman_md5_final(), but copies the final md5 hash
 * to a buffer as a NUL-terminated hexadecimal string
 **/
void 
wsman_md5_final_hex (WsmanMD5Context *ctx, char hex_digest[33])
{
	static const char hexdigits[16] =  {
		'0', '1', '2', '3', '4', '5', '6', '7',
		'8', '9', 'a', 'b', 'c', 'd', 'e', 'f'
	};
	guchar digest[16];
	int p;

	wsman_md5_final (ctx, digest);

	hex_digest[32] = 0;
	for (p = 15; p >= 0; p--) {
		guchar b = digest[p];
		hex_digest[p * 2 + 1] = hexdigits[ (b & 0x0F ) ];
		hex_digest[p * 2] = hexdigits[ (b & 0xF0 ) >> 4 ];
	}
}


/* The four core functions - F1 is optimized somewhat */

/* #define F1(x, y, z) (x & y | ~x & z) */
#define F1(x, y, z) (z ^ (x & (y ^ z)))
#define F2(x, y, z) F1(z, x, y)
#define F3(x, y, z) (x ^ y ^ z)
#define F4(x, y, z) (y ^ (x | ~z))

/* This is the central step in the MD5 algorithm. */
#define MD5STEP(f, w, x, y, z, data, s) \
	( w += f(x, y, z) + data,  w = w<<s | w>>(32-s),  w += x )

/*
 * The core of the MD5 algorithm, this alters an existing MD5 hash to
 * reflect the addition of 16 longwords of new data.  wsman_md5_Update blocks
 * the data and converts bytes into longwords for this routine.
 */
static void 
wsman_md5_transform (guint32 buf[4], const guint32 in[16])
{
	register guint32 a, b, c, d;
	
	a = buf[0];
	b = buf[1];
	c = buf[2];
	d = buf[3];
	
	MD5STEP (F1, a, b, c, d, in[0] + 0xd76aa478, 7);
	MD5STEP (F1, d, a, b, c, in[1] + 0xe8c7b756, 12);
	MD5STEP (F1, c, d, a, b, in[2] + 0x242070db, 17);
	MD5STEP (F1, b, c, d, a, in[3] + 0xc1bdceee, 22);
	MD5STEP (F1, a, b, c, d, in[4] + 0xf57c0faf, 7);
	MD5STEP (F1, d, a, b, c, in[5] + 0x4787c62a, 12);
	MD5STEP (F1, c, d, a, b, in[6] + 0xa8304613, 17);
	MD5STEP (F1, b, c, d, a, in[7] + 0xfd469501, 22);
	MD5STEP (F1, a, b, c, d, in[8] + 0x698098d8, 7);
	MD5STEP (F1, d, a, b, c, in[9] + 0x8b44f7af, 12);
	MD5STEP (F1, c, d, a, b, in[10] + 0xffff5bb1, 17);
	MD5STEP (F1, b, c, d, a, in[11] + 0x895cd7be, 22);
	MD5STEP (F1, a, b, c, d, in[12] + 0x6b901122, 7);
	MD5STEP (F1, d, a, b, c, in[13] + 0xfd987193, 12);
	MD5STEP (F1, c, d, a, b, in[14] + 0xa679438e, 17);
	MD5STEP (F1, b, c, d, a, in[15] + 0x49b40821, 22);
	
	MD5STEP (F2, a, b, c, d, in[1] + 0xf61e2562, 5);
	MD5STEP (F2, d, a, b, c, in[6] + 0xc040b340, 9);
	MD5STEP (F2, c, d, a, b, in[11] + 0x265e5a51, 14);
	MD5STEP (F2, b, c, d, a, in[0] + 0xe9b6c7aa, 20);
	MD5STEP (F2, a, b, c, d, in[5] + 0xd62f105d, 5);
	MD5STEP (F2, d, a, b, c, in[10] + 0x02441453, 9);
	MD5STEP (F2, c, d, a, b, in[15] + 0xd8a1e681, 14);
	MD5STEP (F2, b, c, d, a, in[4] + 0xe7d3fbc8, 20);
	MD5STEP (F2, a, b, c, d, in[9] + 0x21e1cde6, 5);
	MD5STEP (F2, d, a, b, c, in[14] + 0xc33707d6, 9);
	MD5STEP (F2, c, d, a, b, in[3] + 0xf4d50d87, 14);
	MD5STEP (F2, b, c, d, a, in[8] + 0x455a14ed, 20);
	MD5STEP (F2, a, b, c, d, in[13] + 0xa9e3e905, 5);
	MD5STEP (F2, d, a, b, c, in[2] + 0xfcefa3f8, 9);
	MD5STEP (F2, c, d, a, b, in[7] + 0x676f02d9, 14);
	MD5STEP (F2, b, c, d, a, in[12] + 0x8d2a4c8a, 20);
	
	MD5STEP (F3, a, b, c, d, in[5] + 0xfffa3942, 4);
	MD5STEP (F3, d, a, b, c, in[8] + 0x8771f681, 11);
	MD5STEP (F3, c, d, a, b, in[11] + 0x6d9d6122, 16);
	MD5STEP (F3, b, c, d, a, in[14] + 0xfde5380c, 23);
	MD5STEP (F3, a, b, c, d, in[1] + 0xa4beea44, 4);
	MD5STEP (F3, d, a, b, c, in[4] + 0x4bdecfa9, 11);
	MD5STEP (F3, c, d, a, b, in[7] + 0xf6bb4b60, 16);
	MD5STEP (F3, b, c, d, a, in[10] + 0xbebfbc70, 23);
	MD5STEP (F3, a, b, c, d, in[13] + 0x289b7ec6, 4);
	MD5STEP (F3, d, a, b, c, in[0] + 0xeaa127fa, 11);
	MD5STEP (F3, c, d, a, b, in[3] + 0xd4ef3085, 16);
	MD5STEP (F3, b, c, d, a, in[6] + 0x04881d05, 23);
	MD5STEP (F3, a, b, c, d, in[9] + 0xd9d4d039, 4);
	MD5STEP (F3, d, a, b, c, in[12] + 0xe6db99e5, 11);
	MD5STEP (F3, c, d, a, b, in[15] + 0x1fa27cf8, 16);
	MD5STEP (F3, b, c, d, a, in[2] + 0xc4ac5665, 23);
	
	MD5STEP (F4, a, b, c, d, in[0] + 0xf4292244, 6);
	MD5STEP (F4, d, a, b, c, in[7] + 0x432aff97, 10);
	MD5STEP (F4, c, d, a, b, in[14] + 0xab9423a7, 15);
	MD5STEP (F4, b, c, d, a, in[5] + 0xfc93a039, 21);
	MD5STEP (F4, a, b, c, d, in[12] + 0x655b59c3, 6);
	MD5STEP (F4, d, a, b, c, in[3] + 0x8f0ccc92, 10);
	MD5STEP (F4, c, d, a, b, in[10] + 0xffeff47d, 15);
	MD5STEP (F4, b, c, d, a, in[1] + 0x85845dd1, 21);
	MD5STEP (F4, a, b, c, d, in[8] + 0x6fa87e4f, 6);
	MD5STEP (F4, d, a, b, c, in[15] + 0xfe2ce6e0, 10);
	MD5STEP (F4, c, d, a, b, in[6] + 0xa3014314, 15);
	MD5STEP (F4, b, c, d, a, in[13] + 0x4e0811a1, 21);
	MD5STEP (F4, a, b, c, d, in[4] + 0xf7537e82, 6);
	MD5STEP (F4, d, a, b, c, in[11] + 0xbd3af235, 10);
	MD5STEP (F4, c, d, a, b, in[2] + 0x2ad7d2bb, 15);
	MD5STEP (F4, b, c, d, a, in[9] + 0xeb86d391, 21);
	
	buf[0] += a;
	buf[1] += b;
	buf[2] += c;
	buf[3] += d;
}

