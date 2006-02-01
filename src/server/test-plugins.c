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
 
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <glib.h>
#include <gmodule.h>


#include "ws_utilities.h"
#include "ws_xml_api.h"
#include "soap_api.h"

#include "xml_api_generic.h"
#include "xml_serializer.h"
#include "ws_dispatcher.h"

#include "wsmand-listener.h"
#include "wsmand-plugins.h"



int
main (int argc, char **argv)
{
	GList *node;
	GList *list=NULL;
	
	GList *node2;
	DL_List list2;
	
	
	WsManListenerH *listener = (WsManListenerH *)g_malloc0(sizeof(WsManListenerH) );
	
	wsman_plugins_load(listener);
        memset(&list2, 0, sizeof(DL_List));

	node = listener->plugins;	
	while (node) {
		
		WsManPlugin *p = (WsManPlugin *)node->data;		
		WsDispatchInterfaceInfo *g_Interface = (WsDispatchInterfaceInfo *)malloc(sizeof(WsDispatchInterfaceInfo));
		p->get_endpoints(p->p_handle, g_Interface );
		
		g_return_val_if_fail(g_Interface != NULL , 1 );			
		list = g_list_append(list, g_Interface);		

		DL_MakeNode(&list2, g_Interface);
		
		node = g_list_next (node);
	}
		
	g_return_val_if_fail(list != NULL , 1 );
	
	printf("GList\n");
	node2 = list;
	while (node2) {
		WsDispatchInterfaceInfo *i = (WsDispatchInterfaceInfo *)node2->data;
		printf("uri: %s\n", i->wsmanResourceUri);		
		node2 = g_list_next (node2);				
	}
	printf("DL_List\n");
	//DL_Node dl_node;
	
	DL_Node *dl_node = DL_GetHead(&list2);
	while (dl_node != NULL) {
		WsDispatchInterfaceInfo *i = (WsDispatchInterfaceInfo *)dl_node->dataBuf;
                if (i->wsmanResourceUri)
                        printf("uri: %s\n", i->wsmanResourceUri);	
                else
                        printf("warning: resource URI is NULL\n");
		dl_node = DL_GetNext(dl_node);
	}
	
	printf("count GList: %u\n",  g_list_length (list));
	printf("count DL_List: %u\n",  DL_GetCount(&list2));
	
	
	wsman_plugins_unload(listener);
	return 0;	
}


