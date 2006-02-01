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
#include "wsman.h"

#include "parse_uri.h"

int wsman_transfer_get (
        char *url,
        WsClientContextH *ctx,
        char *resourceUri
        ) 
{
    int j = 0;
    int retVal = 0;
    struct pair_t *_query = NULL;
    char *action = wsman_make_action(XML_NS_TRANSFER, TRANSFER_GET);
    WsXmlDocH respDoc = NULL;


    SoupUri *_resourceUri = soup_uri_new(resourceUri);

    if (_resourceUri->query != NULL) {
        _query = parse_query(_resourceUri->query, '&');
        _resourceUri->query = NULL;
    }
    resourceUri = soup_uri_to_string(_resourceUri, FALSE);

    WsXmlDocH rqstDoc = wsman_build_envelope(ctx->wscntx,
            action,
            WSA_TO_ANONYMOUS,
            NULL,
            resourceUri,
            url,
            60000,
            50000);

    if ( rqstDoc )
    {
        if (_query != NULL) {
            for(j = 0; _query[j].name; j++)
            {
                wsman_set_selector(ctx->wscntx, rqstDoc, _query[j].name, _query[j].value);
            }
        }
        respDoc = _ws_send_get_response(ctx,rqstDoc,url);
        
        if (respDoc) 
        {
        		ws_xml_dump_node_tree(stdout, ws_xml_get_doc_root(respDoc), 1);
     		printf("\n");
        }
        else 
        {
        		retVal = 1;
        }
       
    }
    soup_uri_free(_resourceUri);
    return retVal;
}






int wsman_transfer_put(char* url, WsClientContextH *ctx, 
			char *resourceUri, 
			WsProperties *properties)
{
	int retVal = 0;
    int j = 0;
    struct pair_t *_query = NULL;	
    char* action = wsman_make_action(XML_NS_TRANSFER, TRANSFER_PUT);

  	SoupUri *_resourceUri = soup_uri_new(resourceUri);

    if (_resourceUri->query != NULL) {
        _query = parse_query(_resourceUri->query, '&');
        _resourceUri->query = NULL;
    }
    resourceUri = soup_uri_to_string(_resourceUri, FALSE);     
    
    
    
    WsXmlDocH rqstDoc = wsman_build_envelope(ctx->wscntx, 
            action, 
            WSA_TO_ANONYMOUS, 
            NULL, 
            resourceUri,
            url,
            60000,
            50000); 
       
    
    if ( rqstDoc )
    {
	/*
        WsXmlNodeH body = ws_xml_get_soap_body(rqstDoc);
        WsXmlNodeH node;
	*/
        WsXmlDocH respDoc;
        
        if (_query != NULL) {
            for(j = 0; _query[j].name; j++)
            {
                wsman_set_selector(ctx->wscntx, rqstDoc, _query[j].name, _query[j].value);
            }
        }
        /*
        node = ws_xml_add_child(body, XML_NS_CIM_V2_9, opName, NULL);
        ws_xml_add_child(node, XML_NS_CIM_V2_9, "LowerThresholdNonCritical", val);
        */
      

        if ( (respDoc = _ws_send_get_response(ctx, rqstDoc, url)) )
        {
           
            ws_xml_dump_node_tree(stdout, ws_xml_get_doc_root(respDoc), 1);
            printf("\n");
            ws_xml_destroy_doc(respDoc);
        }
        else
            printf("WsSendGetResponse failed\n");

        ws_xml_destroy_doc(rqstDoc);
    }
    else
        printf("WsManBuildEnvelope failed\n");
    free(action);
    return retVal;
}




