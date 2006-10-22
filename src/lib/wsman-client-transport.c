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
 * @author Anas Nashif
 * @author Vadim Revyakin
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "wsman-client-transport.h"
#include "../client/wsman-client-options.h"
#include "u/libu.h"

char *auth_methods[] = {
     "basic",
     "digest",
     "ntlm",
     NULL,
};

static void
request_usr_pwd(ws_auth_type_t auth,
                char **username,
                char **password);

ws_auth_request_func_t request_func = &request_usr_pwd;


static void
request_usr_pwd(ws_auth_type_t auth,
                char **username,
                char **password)
{
    char *pw;
    char user[21];

    fprintf(stdout,"Authentication failed, please retry\n");
    fprintf(stdout, "%s authentication is used\n",
                ws_client_transport_get_auth_name(auth));
    printf("User name: ");
    fflush(stdout); 
    fgets(user, 20, stdin);

    if (strchr(user, '\n'))
        (*(strchr(user, '\n'))) = '\0';
    *username = u_strdup_printf ("%s", user);

    pw = getpass("Password: ");
    *password = u_strdup_printf ("%s", pw);
}



char *ws_client_transport_get_auth_name(ws_auth_type_t auth)
{
    switch (auth) {
        case WS_NO_AUTH :    return "No Auth";
        case WS_BASIC_AUTH:  return "Basic";
        case WS_DIGEST_AUTH: return "Digest";
        case WS_NTLM_AUTH:   return "NTLM";
    }
    return "Unknown";
}

void ws_client_transport_set_auth_request_func(ws_auth_request_func_t f)
{
    request_func = f;
}



int wsman_is_auth_method(int method)
{
	//FIXME
	char *authentication_method = NULL;
    if (authentication_method == NULL) {
        return 1;
    }
    if (method >= AUTH_MAX) {
        return 0;
    }
    return (!strncasecmp(authentication_method, auth_methods[method],
            strlen(authentication_method)));
}



