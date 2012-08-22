/*
 *  Copyright (c) 2006 Dell, Inc.
 *  by Praveen K Paladugu <praveen_paladugu@dell.com>
 *  Licensed under the GNU General Public license, version 2.
 */


#include "wsman_config.h"

#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "ctype.h"


#include "u/libu.h"

#include "wsman-xml-api.h"
#include "wsman-soap.h"
#include "wsman-xml.h"
#include "wsman-client-api.h"
#include "wsman-xml-serializer.h"

#include <wsman-client-transport.h>
#include <wsman-debug.h>

#include "wsman-soap-envelope.h"
#include "wsman-soap-message.h"

#include "redirect.h"


int wsman_get_transport_timeout(WsContextH cntx, WsXmlDocH doc);
int check_response_code(WsManClient *cl, WsXmlDocH doc);

//DEBUG
static void xml_print( WsXmlDocH doc);



int Redirect_Get_EP( SoapOpH op,
                void* appData,
                void *opaqueData )

{
			
    WsmanMessage *msg = wsman_get_msg_from_op(op);
    SoapH soap = soap_get_op_soap(op);
    WsXmlDocH in_doc = soap_get_op_doc(op, 1);	
    WsContextH cntx = ws_create_ep_context(soap, in_doc);
    WsManClient *cl=NULL; 



    debug ("Test Get Endpoint Called"); 
    cl = setup_redirect_client(cntx, msg->auth_data.username, msg->auth_data.password );

    wsman_send_request(cl,cntx->indoc);

    soap_set_op_doc(op, 
		ws_xml_duplicate_doc(wsmc_build_envelope_from_response(cl)), 
		0);
    wsmc_release(cl);

return 0;
}

int Redirect_Put_EP(WsContextH cntx)
{
    debug( "Put Endpoint Called"); 
}


int Redirect_Enumerate_EP(WsContextH cntx, 
			WsEnumerateInfo* enumInfo,
			WsmanStatus *status, void *opaqueData)
{

    WsXmlDocH  r_header=NULL, r_node=NULL, r_body=NULL, r_response=NULL, r_opt=NULL;
    char *resource_uri, *remote_enumContext;
    int op; 
    WsManClient *cl=NULL;

//The redirected Enumeration request must have RequestTotalItemsCountEstimate enabled

    r_header = ws_xml_get_soap_header(cntx->indoc);
    if ( (r_node = ws_xml_get_child(r_header,0,XML_NS_WS_MAN, WSM_REQUEST_TOTAL )) == NULL )
	    ws_xml_add_child(r_header, XML_NS_WS_MAN, WSM_REQUEST_TOTAL, NULL);     


    cl = setup_redirect_client(cntx,  enumInfo->auth_data.username, enumInfo->auth_data.password);


//Set the enumInfo flags based on the indoc. This is required while handling the response in wsenum_eunmerate_stub
    r_body=ws_xml_get_soap_body(cntx->indoc);
    if ( ( r_node = ws_xml_get_child(r_body ,0, XML_NS_ENUMERATION, WSENUM_ENUMERATE )) != NULL )
    {
	    if ( (r_opt = ws_xml_get_child(r_node,0,XML_NS_WS_MAN,WSM_OPTIMIZE_ENUM )) != NULL )
		    enumInfo->flags |= WSMAN_ENUMINFO_OPT ;

    }
    wsman_send_request(cl,cntx->indoc);


    r_response = ws_xml_duplicate_doc(wsmc_build_envelope_from_response(cl));


//Get the Estimated Total No.of Items from the response.
    r_header=ws_xml_get_soap_header(r_response);
    r_node=ws_xml_get_child(r_header,0,XML_NS_WS_MAN, WSM_TOTAL_ESTIMATE );
    enumInfo->totalItems=(!r_node) ? 0: atoi(ws_xml_get_node_text(r_node));


//Get the remote context
    remote_enumContext = wsmc_get_enum_context(r_response);



//Set the pullResultPtr only if some Enum Items are returned, in optimized mode.
    r_body= ws_xml_get_soap_header(r_response);
 
    if ((r_node = ws_xml_get_child(r_body,0,XML_NS_WS_MAN, WSENUM_ITEMS )) != NULL ){

	enumInfo->pullResultPtr = r_response; 

	if( strlen(remote_enumContext) != 0 )
	    strncpy(enumInfo->enumId, remote_enumContext, strlen(remote_enumContext)+1);
    
	else  // If all the instances are returned, the context will be NULL
	    enumInfo->enumId[0]='\0';
	    
    
    }
    else{
	    
	    strncpy(enumInfo->enumId, remote_enumContext, strlen(remote_enumContext)+1);
	    ws_xml_destroy_doc(r_response);
	
    }
    
    wsmc_release(cl);


    debug ("The context on the remote host=%s", remote_enumContext);
    
    return 0;

}


int Redirect_Release_EP(WsContextH cntx,
                        WsEnumerateInfo* enumInfo,
                        WsmanStatus *status, void *opaqueData)
{


    WsManClient *cl=NULL;
    WsXmlDocH response=NULL;


    cl = setup_redirect_client(cntx,  enumInfo->auth_data.username, enumInfo->auth_data.password);
    
    wsman_send_request(cl,cntx->indoc);
    debug ("***Release:: request sent:::");
    xml_print(cntx->indoc);

    response=wsmc_build_envelope_from_response(cl);
    debug ("*****Release: response received::");
    xml_print(response);

    return check_response_code(cl, response);
}

int Redirect_Pull_EP(WsContextH cntx, WsEnumerateInfo* enumInfo,
			WsmanStatus *status, void *opaqueData)
{
    WsXmlDocH doc=NULL,response=NULL;
    WsManClient *cl=NULL;




    cl = setup_redirect_client( cntx, enumInfo->auth_data.username, enumInfo->auth_data.password);


    wsman_send_request(cl,cntx->indoc);


    response = ws_xml_duplicate_doc(wsmc_build_envelope_from_response(cl));
    
   
     if ( ! wsman_is_fault_envelope(response))
	enumInfo->pullResultPtr = response;
    
    else{
	enumInfo->pullResultPtr = NULL;
	wsman_get_fault_status_from_doc (response, status);
    }

    wsmc_release(cl);


    return 0;
}





static void xml_print( WsXmlDocH doc)
{
        char *filename="/tmp/tmp";
        FILE *f;


        if (filename) {
                f = fopen(filename, "w");
                if (f == NULL) {
                        error("Could not open file for writing");
                        return;
                }
        }
        ws_xml_dump_node_tree(stdout, ws_xml_get_doc_root(doc));
                fclose(f);
        return;

}

int check_response_code(WsManClient *cl, WsXmlDocH doc)
{

	//Check the http response codes and also for Wsman Faults.     
                   if (wsmc_get_response_code(cl) != 200
                                        && wsmc_get_response_code(cl) != 400
                                        && wsmc_get_response_code(cl) != 500) {
                                return 1;
                        }

		if (doc != NULL)
		{
			WsXmlNodeH body = ws_xml_get_soap_body(doc);
			if (body && ws_xml_get_child(body, 0, XML_NS_SOAP_1_2, SOAP_FAULT))
				return 1;
		}


	return 0;
}	
