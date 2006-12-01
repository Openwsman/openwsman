/*
 *   Copyright (c) 2000, 2001, Core SDI S.A., Argentina
 *   All rights reserved
 *
 *   Redistribution and use in source and binary forms, with or without
 *   modification, are permitted provided that the following conditions
 *   are met:
 *
 *   1. Redistributions of source code must retain the above copyright
 *      notice, this list of conditions and the following disclaimer.
 *   2. Redistributions in binary form must reproduce the above copyright
 *      notice, this list of conditions and the following disclaimer in the
 *      documentation and/or other materials provided with the distribution.
 *   3. Neither name of the Core SDI S.A. nor the names of its contributors
 *      may be used to endorse or promote products derived from this software
 *      without specific prior written permission.
 *
 *   THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 *   IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 *   OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 *   IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 *   INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 *   NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *   DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *   THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 *   THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */


#ifndef WIN32           /* Unix */
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>

#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <unistd.h>
#include <stdarg.h>
#include <string.h>

#else                   /* Win32 */
#include <sys/types.h>
#include <sys/stat.h>
#include <conio.h>
#include <winsock2.h>
#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>


#endif /* WIN32 */

#include <u/os.h>

#ifdef WIN32

/*
 * getpass():
 *      Clone of unix getpass(3).
 */
const char *
getpass(const char *prompt)
{
    static char password[_PASSWORD_LEN + 1];
    int         i, ch;

    _cputs(prompt);
    for (i = 0; i < _PASSWORD_LEN; i++) {
        /* Get char from the console, skip function and arrow keys */
        while ( (ch = _getch()) == 0 || ch == 0xE0)
            _getch();

        if (ch == 13) {
            cputs("\n\r");
            break;
        }

        password[i] = (char) ch;
    }

    password[i] = '\0';
    return (password);
}
#endif /* WIN32 */
