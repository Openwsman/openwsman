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
 * @author Haihao Xiang
 * @author Anas Nashif
 * @author Eugene Yarmosh
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "libsoup/soup.h"
#include "libsoup/soup-session.h"


#include "ws_utilities.h"


#include "ws_xml_api.h"
#include "ws_errors.h"
#include "soap_api.h"
#include "xml_api_generic.h"
#include "xml_serializer.h"
#include "wsman-client.h"
#include "wsman.h"



#define PRIVATE_CATCH "Catch"

int  wsman_private_catch(         
        WsClientContextH *ctx,
        char *resourceUri
        ) 
{
		int retVal = 0;
        char *action = wsman_make_action(XML_NS_DOEM_TEST, PRIVATE_CATCH);
        WsXmlDocH respdoc = NULL;
        WsXmlDocH rqstdoc = wsman_build_envelope(ctx->wscntx, 
                                             action, 
                                             WSA_TO_ANONYMOUS, 
                                             NULL, 
                                             resourceUri,
                                             ctx->url,
                                             60000,
                                             50000); 
        if ( rqstdoc != NULL ) {
                ws_xml_add_child(ws_xml_get_soap_body(rqstdoc), 
                                                   XML_NS_DOEM_TEST, 
                                                   PRIVATE_CATCH, 
                                                   NULL);
        }


        respdoc = ws_send_get_response(ctx->wscntx, rqstdoc, 60000);

        if (respdoc)
        {
        		ws_xml_dump_node_tree(stdout, ws_xml_get_doc_root(respdoc), 1);
        		printf("\n");
        }
        	else 
        		retVal = 1;

        
        return retVal;
}
