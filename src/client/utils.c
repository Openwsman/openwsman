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
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>



#ifdef PRETTY_OUTPUT
#include <libxslt/xslt.h>
#include <libxslt/xsltInternals.h>
#include <libxslt/transform.h>
#include <libxslt/xsltutils.h>
#endif

#include "libsoup/soup.h"
#include "libsoup/soup-session.h"



#include "ws_utilities.h"


#include "ws_xml_api.h"
#include "ws_errors.h"
#include "soap_api.h"
#include "xml_api_generic.h"
#include "xml_serializer.h"

#include "wsman.h"


char* wsman_make_action(char* uri, char* opName)
{
    int len = strlen(uri) + strlen(opName) + 2;
    char* ptr = (char*)malloc(len);
    if ( ptr )
        sprintf(ptr, "%s/%s", uri, opName);
    return ptr;
}


/*
void prompt_user (gpointer data, char **username, char **password) {
    user_data *udata = data;
    char *pw;
    char user[21];

    if (!udata->user && udata) {
        printf("User name: ");
        fflush(stdout); 
        fgets(user, 20, stdin);

        if (strchr(user, '\n'))
            (*(strchr(user, '\n'))) = '\0';
        *username = g_strdup_printf ("%s", user);
    } else {
        *username = udata->user; 
    }

    if (!udata->password && udata)
    {
        pw = getpass("Password: ");
        *password = g_strdup_printf ("%s", pw);
    } else {
        *password = udata->password;
    }
}

*/


#ifdef PRETTY_OUTPUT
void text_output(char *body) {
    xsltStylesheetPtr cur = NULL;
    xmlDocPtr res, xmlDoc;
    const char *params[16 + 1];

    cur = xsltParseStylesheetFile(BAD_CAST "/home/nashif/wsman-text.xslt");
    xmlDoc = xmlReadMemory(body,
            strlen(body), 
            NULL, 
            MY_ENCODING, 
            XML_PARSE_NONET | XML_PARSE_NSCLEAN);

    res = xsltApplyStylesheet(cur, xmlDoc, params);
    xsltSaveResultToFile(stdout, xmlDoc, cur);
    xsltFreeStylesheet(cur);
    xmlFreeDoc(xmlDoc);
    xsltCleanupGlobals();
}
#endif
