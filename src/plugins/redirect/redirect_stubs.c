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
#include "wsman-dispatcher.h"

#include "redirect.h"


WsXmlDocH redirect_generate_fault( WsXmlDocH in_doc, WsManClient *cl);
char* redirect_fault_msg (char * last_error_string);

//DEBUG
static void xml_print( WsXmlDocH doc);

int Redirect_transfer_action ( SoapOpH op,
                void* appData,
                void *opaqueData)
{
    //Same function to be called for Get, Put, Create, Delete Actions
    WsmanMessage *msg = wsman_get_msg_from_op(op);
    SoapH soap = soap_get_op_soap(op);
    WsXmlDocH in_doc = soap_get_op_doc(op, 1);	
    WsContextH cntx = ws_create_ep_context(soap, in_doc);
    WsManClient *cl=NULL;
    WsXmlDocH response=NULL;



    debug ("Test Get Endpoint Called"); 
    cl = setup_redirect_client(cntx, msg->auth_data.username, msg->auth_data.password );

    wsman_send_request(cl,cntx->indoc);


    if (wsmc_get_last_error(cl) != WS_LASTERR_OK ){
	//CURL/ HTTP errors	
	soap_set_op_doc(op, 
	    redirect_generate_fault( cntx->indoc , cl), 
	    0);

	    return 1;
    }


    response = wsmc_build_envelope_from_response(cl);
  
    soap_set_op_doc(op, 
		ws_xml_duplicate_doc(response), 0);

    wsmc_release(cl);

    return 0;
}


int Redirect_Get_EP( SoapOpH op,
                void* appData,
                void *opaqueData)

{
	return Redirect_transfer_action(op,appData, opaqueData);

}

int Redirect_Create_EP( SoapOpH op,
                void* appData,
                void *opaqueData)

{
	return Redirect_transfer_action(op,appData, opaqueData);

}

int Redirect_Delete_EP( SoapOpH op,
                void* appData,
                void *opaqueData)

{
	return Redirect_transfer_action(op,appData, opaqueData);

}



int Redirect_Put_EP( SoapOpH op,
                void* appData,
                void *opaqueData)
{
	return Redirect_transfer_action(op,appData, opaqueData);
}


int Redirect_Enumerate_EP(WsContextH cntx, 
			WsEnumerateInfo* enumInfo,
			WsmanStatus *status, void *opaqueData)
{

    WsXmlNodeH r_header=NULL, r_node=NULL, r_body=NULL, r_opt=NULL;
    WsXmlDocH r_response=NULL;
    char *resource_uri, *remote_enumContext=NULL;
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

    if (wsmc_get_last_error(cl) != WS_LASTERR_OK ){
	//CURL or HTTP errors
	enumInfo->pullResultPtr = NULL;
	status->fault_code = WSMAN_INTERNAL_ERROR;
	status->fault_detail_code = 0;
	status->fault_msg = redirect_fault_msg( wsman_transport_get_last_error_string(  wsmc_get_last_error(cl) )  );
            return 1;
    }




    r_response = wsmc_build_envelope_from_response(cl);

 
    if (  wsman_is_fault_envelope(r_response)){
        enumInfo->pullResultPtr = NULL;
        wsman_get_fault_status_from_doc(r_response, status);
	return 1;
    }
 


    //Get the Estimated Total No.of Items from the response.
    r_header=ws_xml_get_soap_header(r_response);
    r_node=ws_xml_get_child(r_header,0,XML_NS_WS_MAN, WSM_TOTAL_ESTIMATE );
    enumInfo->totalItems=(!r_node) ? 0: atoi(ws_xml_get_node_text(r_node));


    //Get the remote context
    remote_enumContext = wsmc_get_enum_context(r_response);



    //Set the pullResultPtr only if some Enum Items are returned, in optimized mode.
    r_body= ws_xml_get_soap_body(r_response);

    if (  (r_node = ws_xml_get_child(r_body,0,XML_NS_ENUMERATION, WSENUM_ENUMERATE_RESP )) != NULL && 
			( ws_xml_get_child(r_node,0,XML_NS_WS_MAN,WSENUM_ITEMS) != NULL)  )
    
    {

	enumInfo->pullResultPtr = r_response; 

	if( strlen(remote_enumContext) != 0 )
	    strncpy(enumInfo->enumId, remote_enumContext, strlen(remote_enumContext)+1);
    
	else  // If all the instances are returned, the context will be NULL
	    enumInfo->enumId[0]='\0';
	    
    }
    
    else{
	    //If not items are returned, set the context and return. 
	    strncpy(enumInfo->enumId, remote_enumContext, strlen(remote_enumContext)+1);
	    ws_xml_destroy_doc(r_response);
	
    }
    
    wsmc_release(cl);
    if (remote_enumContext != NULL)
	free(remote_enumContext);
    
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
    if (wsmc_get_last_error(cl) != WS_LASTERR_OK ){
	//just return for now, as the release_stub is not handling the status codes.			
	return 1;
    }	

    response=wsmc_build_envelope_from_response(cl);

    //The status value is not used in the release stub. So, just return, if fault or not.
    return wsman_is_fault_envelope(response);
}

int Redirect_Pull_EP(WsContextH cntx, WsEnumerateInfo* enumInfo,
			WsmanStatus *status, void *opaqueData)
{
    WsXmlDocH doc=NULL,response=NULL;
    WsManClient *cl=NULL;
    int retVal=0;




    cl = setup_redirect_client( cntx, enumInfo->auth_data.username, enumInfo->auth_data.password);


    wsman_send_request(cl,cntx->indoc);

    if (wsmc_get_last_error(cl) != WS_LASTERR_OK ){
        //CURL or HTTP errors
        enumInfo->pullResultPtr = NULL;
        status->fault_code = WSMAN_INTERNAL_ERROR;
        status->fault_detail_code = 0;
        status->fault_msg = redirect_fault_msg( wsman_transport_get_last_error_string(  wsmc_get_last_error(cl) )  );
            return 1;
    }


    response = wsmc_build_envelope_from_response(cl);

   
    if ( ! wsman_is_fault_envelope(response) )
	    enumInfo->pullResultPtr = response;
    
    else{
	    //If there a fault, return the status code.
	    enumInfo->pullResultPtr = NULL;
	    wsman_get_fault_status_from_doc (response, status);
	    retVal=1;
    }

    wsmc_release(cl);

    return retVal;
}



int Redirect_Custom_EP( SoapOpH op,
		void* appData,
		void *opaqueData )
{

    //By passing the stubs. Called from process_inbound_operation
   
    //Fix the wsa:To element in the forwarded request?? Nothing is verifying the URL in this element anyway. 
	return Redirect_transfer_action(op,appData, opaqueData);

    
    
}


static void xml_print( WsXmlDocH doc)
{

        ws_xml_dump_node_tree(stdout, ws_xml_get_doc_root(doc));
        return;

}

WsXmlDocH redirect_generate_fault( WsXmlDocH in_doc, WsManClient *cl)
{
    
    WS_LASTERR_Code last_error =wsmc_get_last_error(cl);
    char *last_error_string = wsman_transport_get_last_error_string(last_error);
    
     
    return wsman_generate_fault(in_doc, WSMAN_INTERNAL_ERROR, 0,
	redirect_fault_msg(last_error_string) );


}

char* redirect_fault_msg (char * last_error_string)
{
    char *prepend_string = "Redirect Plugin: ";
    char* fault_msg =calloc(1,strlen(prepend_string)+strlen(last_error_string)+2);

    strncpy(fault_msg, prepend_string, strlen(prepend_string));
    strncat(fault_msg, last_error_string, strlen(last_error_string));

    return fault_msg;

}
	
