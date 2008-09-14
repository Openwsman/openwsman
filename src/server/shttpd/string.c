/*
 * Copyright (c) 2004-2005 Sergey Lyubka <valenok@gmail.com>
 * All rights reserved
 *
 * "THE BEER-WARE LICENSE" (Revision 42):
 * Sergey Lyubka wrote this file.  As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return.
 */

#include "shttpd_defs.h"

void
my_strlcpy(register char *dst, register const char *src, size_t n)
{
       for (; *src != '\0' && n > 1; n--)
               *dst++ = *src++;
       *dst = '\0';
}

/*
 * Verify that given file has certain extension
 */
int
match_extension(const char *path, const char *ext_list)
{
	size_t		len, path_len;

	path_len = strlen(path);

	FOR_EACH_WORD_IN_LIST(ext_list, len)
		if (len < path_len &&
		    !strncasecmp(path + path_len - len, ext_list, len))
			return (1);

	return (0);
}
