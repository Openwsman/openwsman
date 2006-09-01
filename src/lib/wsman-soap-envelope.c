
#define _GNU_SOURCE
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <glib.h>

#include "wsman-util.h"
#include "wsman-xml-api.h"
#include "wsman-soap.h"
#include "wsman-xml.h"
#include "wsman-xml-serializer.h"

#include "wsman-debug.h"
#include "wsman-faults.h"
#include "wsman-soap-envelope.h"

/**
 * Change Endpoint Reference from request to response format
 * @param dstHeader Destination header
 * @param epr The Endpoint Reference
 */
static void epr_from_request_to_response(WsXmlNodeH dstHeader, WsXmlNodeH epr)
{
    int i;
    WsXmlNodeH child;
    WsXmlNodeH node = !epr ? NULL : ws_xml_get_child(epr, 0, XML_NS_ADDRESSING, WSA_ADDRESS);
    ws_xml_add_child(dstHeader, XML_NS_ADDRESSING, WSA_TO, 
            !node ? WSA_TO_ANONYMOUS : ws_xml_get_node_text(node));

    if ( !epr )
        goto cleanup;

    if ( (node = ws_xml_get_child(epr, 0, XML_NS_ADDRESSING, WSA_REFERENCE_PROPERTIES)) )
    {
        for(i = 0; (child = ws_xml_get_child(node, i, NULL, NULL)) != NULL; i++)
        {
            ws_xml_duplicate_tree(dstHeader, child);
        }
    }

    if ( (node = ws_xml_get_child(epr, 0, XML_NS_ADDRESSING, WSA_REFERENCE_PARAMETERS)) )
    {
        for(i = 0; (child = ws_xml_get_child(node, i, NULL, NULL)) != NULL; i++)
        {
            ws_xml_duplicate_tree(dstHeader, child);
        }
    }
cleanup:
    return;
}

/**
 * Create a response SOAP envelope
 * @param cntx Context
 * @param rqstDoc The XML document of the request
 * @param action the Response action
 * @return Response envelope
 */
WsXmlDocH ws_create_response_envelope(WsContextH cntx, WsXmlDocH rqstDoc, char* action)
{
    SoapH soap = ((WS_CONTEXT*)cntx)->soap;   
    char* soapNs = ws_xml_get_node_name_ns(ws_xml_get_doc_root(rqstDoc));   
    WsXmlDocH doc = ws_xml_create_envelope(soap, soapNs);	
    if ( doc )
    {
        WsXmlNodeH dstHeader = ws_xml_get_soap_header(doc);
        WsXmlNodeH srcHeader = ws_xml_get_soap_header(rqstDoc);
        WsXmlNodeH srcNode = ws_xml_get_child(srcHeader, 0, XML_NS_ADDRESSING, WSA_REPLY_TO);

        epr_from_request_to_response(dstHeader, srcNode);

        if ( action != NULL ) {
            ws_xml_add_child(dstHeader, XML_NS_ADDRESSING, WSA_ACTION, action);
        } else {
            if ( (srcNode = ws_xml_get_child(srcHeader, 0, XML_NS_ADDRESSING, WSA_ACTION)) != NULL )
            {
                if ( (action = ws_xml_get_node_text(srcNode)) != NULL )
                {
                    int len = strlen(action) + sizeof(WSFW_RESPONSE_STR) + 2;
                    char* tmp = (char*)soap_alloc(sizeof(char) * len, 0);
                    if ( tmp )
                    {
                        sprintf(tmp, "%s%s", action, WSFW_RESPONSE_STR);
                        ws_xml_add_child(dstHeader, XML_NS_ADDRESSING, WSA_ACTION, tmp);
                        soap_free(tmp);
                    }
                }
            }
        }

        if ( (srcNode = ws_xml_get_child(srcHeader, 0, XML_NS_ADDRESSING, WSA_MESSAGE_ID)) != NULL )
        {
            ws_xml_add_child(dstHeader, XML_NS_ADDRESSING, WSA_RELATES_TO, ws_xml_get_node_text(srcNode));
        }
    }
    return doc;
}

WsXmlDocH wsman_build_envelope(WsContextH cntx, char* action, char* replyToUri, char* systemUri, char* resourceUri,
        char* toUri, actionOptions options)
{
    WsXmlDocH doc = ws_xml_create_envelope(ws_context_get_runtime(cntx), NULL);

    if ( doc )
    {
        WsXmlNodeH node;
        unsigned long savedMustUnderstand = ws_get_context_ulong_val(cntx, ENFORCE_MUST_UNDERSTAND);
        WsXmlNodeH header = ws_xml_get_soap_header(doc);
        char uuidBuf[100];

        soap_get_uuid(uuidBuf, sizeof(uuidBuf), 0);

        ws_set_context_ulong_val(cntx, ENFORCE_MUST_UNDERSTAND, 1);

        if ( replyToUri == NULL )
            replyToUri = WSA_TO_ANONYMOUS;

        if ( toUri == NULL )
            toUri = WSA_TO_ANONYMOUS;

        if ( action )
            ws_serialize_str(cntx, header, action, XML_NS_ADDRESSING, WSA_ACTION); 

        if ( systemUri )
            ws_serialize_str(cntx, header, systemUri, XML_NS_WS_MAN, WSM_SYSTEM); 

        if ( toUri )
            ws_serialize_str(cntx, header, toUri, XML_NS_ADDRESSING, WSA_TO); 

        if ( resourceUri )
            ws_serialize_str(cntx, header, resourceUri, XML_NS_WS_MAN, WSM_RESOURCE_URI); 



        ws_serialize_str(cntx, header, uuidBuf, XML_NS_ADDRESSING, WSA_MESSAGE_ID);

        if ( options.timeout )
        {
            char buf[20];
            sprintf(buf, "PT%u.%uS", (unsigned int)options.timeout/1000, (unsigned int)options.timeout % 1000);
            ws_serialize_str(cntx, header, buf, XML_NS_WS_MAN, WSM_OPERATION_TIMEOUT);
        }

        if ( options.max_envelope_size)
        {
            ws_serialize_uint32(cntx, header, options.max_envelope_size, XML_NS_WS_MAN, WSM_MAX_ENVELOPE_SIZE); 
        }
        if ( options.fragment)
        {
            ws_serialize_str(cntx, header, options.fragment, XML_NS_WS_MAN, WSM_FRAGMENT_TRANSFER); 
        }

        ws_set_context_ulong_val(cntx, ENFORCE_MUST_UNDERSTAND, savedMustUnderstand);

        node = ws_xml_add_child(header, XML_NS_ADDRESSING, WSA_REPLY_TO, NULL);
        ws_xml_add_child(node, XML_NS_ADDRESSING, WSA_ADDRESS, replyToUri);
    }
    return doc;
}

/**
 * Buid Inbound Envelope
 * @param  fw SOAP Framework handle
 * @param buf Message buffer
 * @return XML document with Envelope
 */
    /*
WsXmlDocH wsman_build_inbound_envelope(SOAP_FW* fw, char *inputBuffer, int inputBufferSize)
{
    WsXmlDocH doc = NULL;   
    char *buf = (char *)soap_alloc( inputBufferSize + 1, 1);
    strncpy (buf, inputBuffer, inputBufferSize);	        

    if ( (doc = ws_xml_read_memory((SoapH)fw, buf, strlen(buf), NULL, 0)) != NULL )   
    {        
        WsmanFaultCodeType fault_code = wsman_is_valid_envelope(fw, doc);
        if  ( wsman_is_duplicate_message_id(fw, doc) &&  fault_code == WSMAN_RC_OK )
        {            
            wsman_debug (WSMAN_DEBUG_LEVEL_ERROR, 
                    "Envelope Discarded: Duplicate MessageID");    
        }   

        if (fault_code != WSMAN_RC_OK)
        {        		        		
            ws_xml_destroy_doc(doc);
            doc = NULL;            
        }        
    } else {
        wsman_debug (WSMAN_DEBUG_LEVEL_ERROR , "Parse Error!");    		
    }
    free(buf);
    return doc;
}
    */



/**
 * Buid Inbound Envelope
 * @param  fw SOAP Framework handle
 * @param buf Message buffer
 * @return XML document with Envelope
 */
WsXmlDocH wsman_build_inbound_envelope(SOAP_FW* fw, WsmanMessage *msg)
{
    WsXmlDocH doc = NULL;   
    if ( (doc = ws_xml_read_memory((SoapH)fw, 
                    (msg->request.body)?msg->request.body:msg->response.body, 
                    (msg->request.length)?msg->request.length:msg->response.length, NULL, 0)) != NULL )   
    {        
        if (wsman_is_identify_request(doc)) {
            wsman_set_message_flags(msg, FLAG_IDENTIFY_REQUEST);
            wsman_is_valid_envelope(msg, doc);
            return doc;
        } 

        wsman_is_valid_envelope(msg, doc);
        if  ( wsman_is_duplicate_message_id(fw, doc) 
                && !wsman_fault_occured(msg) ) 
        {
            wsman_set_fault(msg, 
                    WSA_FAULT_INVALID_MESSAGE_INFORMATION_HEADER, 
                    WSA_FAULT_DETAIL_DUPLICATE_MESSAGE_ID, NULL);
        }
    } else {
        wsman_set_fault(msg, 
                WSA_FAULT_INVALID_MESSAGE_INFORMATION_HEADER, 0, NULL);
        ws_xml_destroy_doc(doc);
        doc = NULL;
    }
    return doc;
}

/**
 * Get SOAP header value
 * @param fw SOAP Framework handle
 * @param doc XML document
 * @param nsUri Namespace URI
 * @param name Header element name
 * @return Header value
 */
char* get_soap_header_value(SOAP_FW* fw, WsXmlDocH doc, char* nsUri, char* name)
{
    char* retVal = NULL;
    WsXmlNodeH node = get_soap_header_element(fw, doc, nsUri, name);

    if ( node != NULL )
        retVal = soap_clone_string(ws_xml_get_node_text(node));

    return retVal;
}


/**
 * Get SOAP Header
 * @param fw SOAP Framework handle
 * @param doc XML document
 * @param nsUri Namespace URI
 * @param name Header element name
 * @return XML node 
 */
WsXmlNodeH get_soap_header_element(SOAP_FW* fw, 
        WsXmlDocH doc, char* nsUri, char* name)
{
    WsXmlNodeH node = ws_xml_get_soap_header(doc);
    if ( node && name ) {
        node = ws_xml_find_in_tree(node, nsUri, name, 1);
    }
    return node;
}


/**
 * Build SOAP Fault
 * @param  fw SOAP Framework handle
 * @param soapNsUri SOAP Namespace URI
 * @param faultNsUri Fault Namespace URI
 * @param code Fault code
 * @param subCode Fault Subcode
 * @param reason Fault Reson
 * @param detail Fault Details
 * @return Fault XML document
 */
WsXmlDocH build_soap_fault(SOAP_FW* fw, char* soapNsUri, char* faultNsUri, char* code,
        char* subCode, char* reason, char* detail)
{
    WsXmlDocH doc;

    if ( faultNsUri == NULL )
        faultNsUri = soapNsUri;

    if ( (doc = ws_xml_create_doc((SoapH)fw, soapNsUri, SOAP_ENVELOPE)) != NULL ) 
    {
        WsXmlNodeH node;
        WsXmlNodeH fault;
        WsXmlNodeH root = ws_xml_get_doc_root(doc);
        //WsXmlNodeH header = WsXmlAddChild(root, soapNsUri, SOAP_HEADER, NULL);
        WsXmlNodeH body = ws_xml_add_child(root, soapNsUri, SOAP_BODY, NULL);

        ws_xml_define_ns(root, soapNsUri, NULL, 0);
        ws_xml_define_ns(root, XML_NS_ADDRESSING, NULL, 0);
        ws_xml_define_ns(root, XML_NS_XML_NAMESPACES, NULL, 0);
        if ( strcmp(soapNsUri, faultNsUri) != 0 ) 
            ws_xml_define_ns(root, faultNsUri, NULL, 0);
        if ( body && (fault = ws_xml_add_child(body, soapNsUri, SOAP_FAULT, NULL)) )
        {
            if ( code != NULL 
                    &&
                    (node = ws_xml_add_child(fault, soapNsUri, SOAP_CODE, NULL)) != NULL )
            {
                ws_xml_add_qname_child(node, soapNsUri, SOAP_VALUE, soapNsUri, code); 

                if ( subCode != NULL
                        && 
                        (node = ws_xml_add_child(node, soapNsUri, SOAP_SUBCODE, NULL)) != NULL )
                {
                    ws_xml_add_qname_child(node, soapNsUri, SOAP_VALUE, faultNsUri, subCode); 
                }
            }

            if ( reason && (node = ws_xml_add_child(fault, soapNsUri, SOAP_REASON, NULL)) )
            {
                node = ws_xml_add_child(node, soapNsUri, SOAP_TEXT, reason);
                ws_xml_add_node_attr(node, XML_NS_XML_NAMESPACES, SOAP_LANG, "en");
            }

            if ( detail )
                ws_xml_add_child(fault, soapNsUri, SOAP_DETAIL, detail);
        }
    }

    return doc;
}




/**
 * Buid SOAP Version Mismtach Fault
 * @param  fw SOAP Framework handle
 * @todo Send fault back
 */     
void build_soap_version_fault(SOAP_FW* fw)
{
    WsXmlDocH fault = build_soap_fault(fw, 
            NULL, 
            XML_NS_SOAP_1_2, 
            "VersionMismatch", 
            NULL, 
            "Version Mismatch", 
            NULL);

    if ( fault != NULL )
    {
        WsXmlNodeH upgrade;
        WsXmlNodeH h = ws_xml_get_soap_header(fault);

        ws_xml_define_ns(ws_xml_get_doc_root(fault), XML_NS_SOAP_1_1, NULL, 0);

        if ( (upgrade = ws_xml_add_child(h, XML_NS_SOAP_1_2, SOAP_UPGRADE, NULL)) )
        {
            WsXmlNodeH node;

            if ( (node = ws_xml_add_child(upgrade, 
                            XML_NS_SOAP_1_2, 
                            SOAP_SUPPORTED_ENVELOPE, 
                            NULL)) )
            {
                ws_xml_add_qname_attr(node, NULL, "qname", XML_NS_SOAP_1_2, SOAP_ENVELOPE);
            }
            if ( (node = ws_xml_add_child(upgrade, 
                            XML_NS_SOAP_1_2, 
                            SOAP_SUPPORTED_ENVELOPE, 
                            NULL)) )
            {
                ws_xml_add_qname_attr(node, NULL, "qname", XML_NS_SOAP_1_1, SOAP_ENVELOPE);
            }
        }


        // FIXME: Send fault
        ws_xml_destroy_doc(fault);
    }
}   




/**
 * Check if Message ID is duplicate
 * @param  fw SOAP Framework handle
 * @param doc XML document
 * @return status
 */
int wsman_is_duplicate_message_id (SOAP_FW* fw, WsXmlDocH doc)
{    
    char* msgId = get_soap_header_value(fw, doc, XML_NS_ADDRESSING, WSA_MESSAGE_ID);

    int retVal = 0;
    if ( msgId ) {
        wsman_debug (WSMAN_DEBUG_LEVEL_DEBUG, "Checking Message ID: %s", msgId);
        DL_Node* node;
        soap_fw_lock(fw);
        node = DL_GetHead(&fw->processedMsgIdList);

        while( node != NULL )
        {
            if ( !strcmp(msgId, (char*)node->dataBuf) ) {              
                wsman_debug (WSMAN_DEBUG_LEVEL_ERROR, "Duplicate Message ID: %s", msgId);                         
                retVal = 1;
                break;
            }
            node = DL_GetNext(node);
        }

        if ( !retVal )
        {
            while( DL_GetCount(&fw->processedMsgIdList) 
                    >= PROCESSED_MSG_ID_MAX_SIZE )
            {
                node = DL_RemoveHead(&fw->processedMsgIdList);
                soap_free(node->dataBuf);
                soap_free(node);
            }

            if ( (node = DL_MakeNode(&fw->processedMsgIdList, NULL)) )
            {
                if ( (node->dataBuf = soap_clone_string(msgId)) == NULL )
                {
                    DL_RemoveNode(node);
                    soap_free(node);
                }
            }
        }
        soap_fw_unlock(fw);
    } else {       
        wsman_debug (WSMAN_DEBUG_LEVEL_ERROR , "No MessageId found");
    }
    free(msgId);

    return retVal;
}


/**
 * Check if Envelope is valid
 * @param  msg Message data
 * @param doc XML document
 * @return void
 */
void wsman_is_valid_envelope(WsmanMessage *msg, WsXmlDocH doc)
{
    WsXmlNodeH root = ws_xml_get_doc_root(doc);

    if ( strcmp(SOAP_ENVELOPE, ws_xml_get_node_local_name(root)) != 0) {
        wsman_set_fault(msg, 
                WSA_FAULT_INVALID_MESSAGE_INFORMATION_HEADER, 0, 
                "No Envelope");
        goto cleanup;
    }
    char* soapNsUri = ws_xml_get_node_name_ns(root);
    if ( strcmp(soapNsUri, XML_NS_SOAP_1_2) != 0 ) {
        wsman_set_fault(msg, SOAP_FAULT_VERSION_MISMATCH, 0, NULL);
        goto cleanup;
    } 

    if (  ws_xml_get_soap_body(doc) == NULL ) {
        wsman_set_fault(msg, 
                WSA_FAULT_INVALID_MESSAGE_INFORMATION_HEADER, 0, "No Body");
        goto cleanup;
    }
cleanup:
    return ;
}





