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

#define _GNU_SOURCE
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

#include "u/libu.h"
#include "wsman-xml-api.h"
#include "wsman-soap.h"
#include "wsman-xml.h"
#include "wsman-xml-serializer.h"

#include "wsman-faults.h"
#include "wsman-soap-envelope.h"


/**
 * Change Endpoint Reference from request to response format
 * @param dstHeader Destination header
 * @param epr The Endpoint Reference
 */
static void
epr_from_request_to_response( WsXmlNodeH dstHeader,
                              WsXmlNodeH epr)
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
WsXmlDocH
ws_create_response_envelope( WsContextH cntx, 
                             WsXmlDocH rqstDoc, 
                             char* action)
{
  SoapH soap = ((WS_CONTEXT*)cntx)->soap;   
  char* soapNs = ws_xml_get_node_name_ns(ws_xml_get_doc_root(rqstDoc));   
  WsXmlDocH doc = ws_xml_create_envelope(soap, soapNs);	
  if ( wsman_is_identify_request(rqstDoc) ) {
    return doc;
  } 
  else if ( doc ) 
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
          char* tmp = (char*)u_malloc(sizeof(char) * len);
          if ( tmp )
          {
            sprintf(tmp, "%s%s", action, WSFW_RESPONSE_STR);
            ws_xml_add_child(dstHeader, XML_NS_ADDRESSING, WSA_ACTION, tmp);
            u_free(tmp);
          }
        }
      }
    }

    if ( (srcNode = ws_xml_get_child(srcHeader, 0, 
                                     XML_NS_ADDRESSING, WSA_MESSAGE_ID)) != NULL )
    {
      ws_xml_add_child(dstHeader, XML_NS_ADDRESSING, 
                       WSA_RELATES_TO, ws_xml_get_node_text(srcNode));
    }
  }
  return doc;
}


/**
 * Buid Inbound Envelope
 * @param  fw SOAP Framework handle
 * @param buf Message buffer
 * @return XML document with Envelope
 */
WsXmlDocH
wsman_build_inbound_envelope(SoapH soap, WsmanMessage *msg)
{
  WsXmlDocH doc = NULL;
  if ( (doc = ws_xml_read_memory(soap,
             (msg->request.body) ? msg->request.body : msg->response.body,
             (msg->request.length) ? msg->request.length :
                                     msg->response.length, NULL, 0)) != NULL) {
    if (wsman_is_identify_request(doc)) {
      wsman_set_message_flags(msg, FLAG_IDENTIFY_REQUEST);
      wsman_is_valid_envelope(msg, doc);
      return doc;
    } 

    wsman_is_valid_envelope(msg, doc);
    if  ( wsman_is_duplicate_message_id(soap, doc) &&
                               !wsman_fault_occured(msg)) {
      wsman_set_fault(msg,
                      WSA_INVALID_MESSAGE_INFORMATION_HEADER, 
                      WSA_DETAIL_DUPLICATE_MESSAGE_ID, NULL);
    }
  } else {
    wsman_set_fault(msg, 
                    WSA_INVALID_MESSAGE_INFORMATION_HEADER, 0, NULL);
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
char*
get_soap_header_value(SoapH soap, 
                       WsXmlDocH doc,
                       char* nsUri,
                       char* name)
{
  char* retVal = NULL;
  WsXmlNodeH node = get_soap_header_element(soap, doc, nsUri, name);

  if ( node != NULL )
    retVal = u_str_clone(ws_xml_get_node_text(node));

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
WsXmlNodeH get_soap_header_element(SoapH soap,
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
WsXmlDocH build_soap_fault(SoapH soap, char* soapNsUri, char* faultNsUri, char* code,
                           char* subCode, char* reason, char* detail)
{
  WsXmlDocH doc;

  if ( faultNsUri == NULL )
    faultNsUri = soapNsUri;

  if ( (doc = ws_xml_create_doc(soap, soapNsUri, SOAP_ENVELOPE)) != NULL ) {
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
void build_soap_version_fault(SoapH soap)
{
  WsXmlDocH fault = build_soap_fault(soap, NULL, XML_NS_SOAP_1_2,
                                     "VersionMismatch", NULL,
                                     "Version Mismatch", NULL);

  if (fault != NULL) {
    WsXmlNodeH upgrade;
    WsXmlNodeH h = ws_xml_get_soap_header(fault);

    ws_xml_define_ns(ws_xml_get_doc_root(fault), XML_NS_SOAP_1_1, NULL, 0);

    upgrade = ws_xml_add_child(h, XML_NS_SOAP_1_2, SOAP_UPGRADE, NULL);
    if (upgrade) {
      WsXmlNodeH node;

      if ( (node = ws_xml_add_child(upgrade, 
                                    XML_NS_SOAP_1_2, 
                                    SOAP_SUPPORTED_ENVELOPE, 
                                    NULL)) )
      {
        ws_xml_add_qname_attr(node, NULL, "qname", XML_NS_SOAP_1_2, SOAP_ENVELOPE);
      }
      node = ws_xml_add_child(upgrade, XML_NS_SOAP_1_2, 
                              SOAP_SUPPORTED_ENVELOPE, NULL);
      if (node) {
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
int wsman_is_duplicate_message_id (SoapH soap, WsXmlDocH doc)
{
  char* msgId = get_soap_header_value(soap, doc, XML_NS_ADDRESSING, WSA_MESSAGE_ID);

  int retVal = 0;
  if (msgId) {
    debug( "Checking Message ID: %s", msgId);
    u_lock(soap);
    lnode_t *node = list_first(soap->processedMsgIdList);

    while( node != NULL ) {
      if ( !strcmp(msgId, (char*)node->list_data) ) {
        debug( "Duplicate Message ID: %s", msgId);
        retVal = 1;
        break;
      }
      node = list_next(soap->processedMsgIdList, node);
    }

    if (!retVal) {
      while (list_count(soap->processedMsgIdList) >=
                                  PROCESSED_MSG_ID_MAX_SIZE) {
        node = list_del_first(soap->processedMsgIdList);
        u_free(node->list_data);
        u_free(node);
      }

      node = lnode_create( NULL);
      if (node) {
        node->list_data = u_str_clone(msgId);
        if (node->list_data == NULL) {
          u_free(node);
        } else {
          list_append(soap->processedMsgIdList, node);
        }
      }
    }
    u_unlock(soap);
  } else {       
    debug( "No MessageId found");
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
                    WSA_INVALID_MESSAGE_INFORMATION_HEADER, 0, 
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
                    WSA_INVALID_MESSAGE_INFORMATION_HEADER, 0, "No Body");
    goto cleanup;
  }
 cleanup:
  return ;
}



/**
 * Create a Fault
 * @param cntx WS Context
 * @param rqstDoc Request document (Envelope)
 * @param code Fault code
 * @param subCodeNs Namespace of sub code
 * @param subCode Sub code
 * @param lang Language for Reason section
 * @param reason Fault Reason
 * @param addDetailProc Callback for details
 * @param addDetailProcData Pointer to callback data
 * @return XML document of the fault
 */
WsXmlDocH 
wsman_create_fault_envelope(WsContextH cntx,
                            WsXmlDocH rqstDoc,
                            char* code,
                            char* subCodeNs,
                            char* subCode,
                            char* lang,
                            char* reason,
                            char* faultDetail)
{
  WsXmlDocH doc = NULL;

  if ( rqstDoc ) {
    doc = ws_create_response_envelope(cntx, rqstDoc, WSA_ACTION_FAULT); 
  } else {
    SoapH soap = ((WS_CONTEXT*)cntx)->soap;
    doc = ws_xml_create_envelope(soap, NULL);
  }

  if ( doc == NULL )
  {
    return NULL;
  }
  char uuidBuf[50];
  WsXmlNodeH header = ws_xml_get_soap_header(doc);
  WsXmlNodeH body = ws_xml_get_soap_body(doc);
  char* soapNs = ws_xml_get_node_name_ns(body);
  WsXmlNodeH fault = ws_xml_add_child(body, soapNs, SOAP_FAULT, NULL);
  WsXmlNodeH codeNode = ws_xml_add_child(fault, soapNs, SOAP_CODE, NULL);
  WsXmlNodeH node = ws_xml_add_child(codeNode, soapNs, SOAP_VALUE, NULL);

  ws_xml_set_node_qname_val(node, soapNs, code);

  if ( subCode )
  {
    node = ws_xml_add_child(codeNode, soapNs, SOAP_SUBCODE, NULL);
    node = ws_xml_add_child(node, soapNs, SOAP_VALUE, NULL);
    if ( subCodeNs )
      ws_xml_set_node_qname_val(node, subCodeNs, subCode);
    else
      ws_xml_set_node_text(node, subCode);
  }

  if ( reason ) 
  {
    node = ws_xml_add_child(fault, soapNs, SOAP_REASON, NULL);
    node = ws_xml_add_child(node, soapNs, SOAP_TEXT, NULL);
    ws_xml_set_node_text(node, reason);            
    ws_xml_set_node_lang(node, !lang ? "en" : lang );
  }

  if (faultDetail) {
    WsXmlNodeH d = ws_xml_add_child(fault, soapNs, SOAP_DETAIL, NULL);
    node = ws_xml_add_child_format(d, XML_NS_WS_MAN, 
                                   SOAP_FAULT_DETAIL, "%s/%s", XML_NS_WSMAN_FAULT_DETAIL, faultDetail);        
  }


  generate_uuid(uuidBuf, sizeof(uuidBuf), 0);
  ws_xml_add_child(header, XML_NS_ADDRESSING, WSA_MESSAGE_ID, uuidBuf);


  return doc;
}




char*
wsman_get_enum_mode( WsContextH cntx, 
                     WsXmlDocH doc) 
{
  char *enum_mode = NULL;
  if ( doc == NULL )
    doc = ws_get_context_xml_doc_val(cntx, WSFW_INDOC);

  if ( doc )
  {
    WsXmlNodeH node = ws_xml_get_soap_body(doc);
    if ( node && (node = ws_xml_get_child(node, 0, XML_NS_ENUMERATION, WSENUM_ENUMERATE)) )
    {
      WsXmlNodeH opt = ws_xml_get_child(node, 0, XML_NS_WS_MAN, WSM_ENUM_MODE);
      if ( opt ) 
      {
        char *text = ws_xml_get_node_text(opt);
        if (text != NULL)
          enum_mode = text;
      }
    }
  } 
  return enum_mode;
}

void
wsman_set_enum_mode( char *enum_mode,
                     WsEnumerateInfo *enumInfo) 
{
  if (strcmp(enum_mode, WSM_ENUM_EPR) == 0 )
    enumInfo->flags |= FLAG_ENUMERATION_ENUM_EPR;
  else if (strcmp(enum_mode, WSM_ENUM_OBJ_AND_EPR) == 0 )
    enumInfo->flags |= FLAG_ENUMERATION_ENUM_OBJ_AND_EPR;

  return;
}


void
wsman_set_polymorph_mode( WsContextH cntx,
                          WsXmlDocH doc, WsEnumerateInfo *enumInfo)
{
 
  if ( doc == NULL )
    doc = ws_get_context_xml_doc_val(cntx, WSFW_INDOC);

  if ( doc ) {
    WsXmlNodeH node = ws_xml_get_soap_body(doc);

    if ( node && (node = ws_xml_get_child(node, 0, XML_NS_ENUMERATION, WSENUM_ENUMERATE)) )
    {
      WsXmlNodeH opt = ws_xml_get_child(node, 0, XML_NS_CIM_BINDING,  WSMB_POLYMORPHISM_MODE);
      if ( opt ) {              
        char *mode = ws_xml_get_node_text(opt);
        if (strcmp(mode,  WSMB_EXCLUDE_SUBCLASS_PROP) == 0 )
           enumInfo->flags |= FLAG_ExcludeSubClassProperties;
        else if (strcmp(mode,   WSMB_INCLUDE_SUBCLASS_PROP) == 0 )
         enumInfo->flags |= FLAG_IncludeSubClassProperties;
        else if (strcmp(mode,   WSMB_NONE) == 0 )
          enumInfo->flags |= FLAG_POLYMORPHISM_NONE;
      } else {
        enumInfo->flags |= FLAG_IncludeSubClassProperties;
        return;
      }
    }
  } 
 
}



int
wsman_is_optimization( WsContextH cntx,
                       WsXmlDocH doc)
{
  int max_elements = 0;
  if ( doc == NULL )
    doc = ws_get_context_xml_doc_val(cntx, WSFW_INDOC);

  if ( doc ) {
    WsXmlNodeH node = ws_xml_get_soap_body(doc);

    if ( node && (node = ws_xml_get_child(node, 0, XML_NS_ENUMERATION, WSENUM_ENUMERATE)) )
    {
      WsXmlNodeH opt = ws_xml_get_child(node, 0, XML_NS_WS_MAN, WSM_OPTIMIZE_ENUM);
      if ( opt ) {
        WsXmlNodeH max = ws_xml_get_child(node, 0, XML_NS_WS_MAN, WSM_MAX_ELEMENTS);
        if (max) {
          char *text = ws_xml_get_node_text(max);
          if (text != NULL)
            max_elements = atoi(text);
        } else {
          max_elements = 1;
        }
      }
    }
  } 
  return max_elements;
}

int
wsen_get_max_elements( WsContextH cntx,
                       WsXmlDocH doc)
{
  int max_elements = 0;
  if ( doc == NULL )
    doc = ws_get_context_xml_doc_val(cntx, WSFW_INDOC);

  if ( doc ) {
    WsXmlNodeH node = ws_xml_get_soap_body(doc);

    if ( node && (node = ws_xml_get_child(node, 0, XML_NS_ENUMERATION, WSENUM_PULL)) )
    {
      node = ws_xml_get_child(node, 0, XML_NS_ENUMERATION, WSENUM_MAX_ELEMENTS);
      if ( node ) {
        char *text = ws_xml_get_node_text(node);
        if (text != NULL)
          max_elements = atoi(text);
      }
    }
  } else {
    return 0;
  }
  return max_elements;

}


char*
wsman_get_method_name ( WsContextH cntx ) 
{
  char *m = ws_addressing_get_action(cntx, NULL);
  char *method = u_strdup(strrchr(m, '/') + 1);
  return method;
}



char*
wsman_get_class_name ( WsContextH cntx ) 
{
  //char *r = NULL;
  char *resource_uri = wsman_get_resource_uri(cntx, NULL);
  //wsman_remove_query_string(resourceUri, &r);	
  char *className = u_strdup(strrchr(resource_uri, '/') + 1);
  return className;
}

char*
wsman_get_resource_uri( WsContextH cntx, 
                        WsXmlDocH doc )
{
  char* val = NULL;

  if ( doc == NULL )
    doc = ws_get_context_xml_doc_val(cntx, WSFW_INDOC);

  if ( doc )
  {
    WsXmlNodeH header = ws_xml_get_soap_header(doc);
    WsXmlNodeH node = ws_xml_get_child(header, 0, XML_NS_WS_MAN, WSM_RESOURCE_URI);
    val = (!node) ? NULL : ws_xml_get_node_text(node);
  }
  return val;
}


char*
wsman_get_system_uri( WsContextH cntx,
                      WsXmlDocH doc)
{
  char *val = 0;
	
  WsXmlNodeH header = ws_xml_get_soap_header(doc);
  WsXmlNodeH node = ws_xml_get_child(header, 0, XML_NS_WS_MAN, WSM_SYSTEM);
  val = (!node) ? NULL : ws_xml_get_node_text(node);
  return val;
}





hash_t*
wsman_get_method_args ( WsContextH cntx, 
                        char *resource_uri ) 
{
  char *input = NULL;
  hash_t *h = hash_create(HASHCOUNT_T_MAX, 0, 0);
  WsXmlDocH doc = ws_get_context_xml_doc_val(cntx, WSFW_INDOC);
  if ( doc ) {
    WsXmlNodeH body = ws_xml_get_soap_body(doc);
    char *mn = wsman_get_method_name (cntx);
    input = u_strdup_printf("%s_INPUT", mn );
    WsXmlNodeH in_node = ws_xml_get_child(body, 0, resource_uri, input);
    if (in_node) {
      debug("INPUT found");
      WsXmlNodeH arg;
      int index = 0;
      while( (arg = ws_xml_get_child(in_node, index++, NULL, NULL)) )
      {
        char *key = ws_xml_get_node_local_name(arg);
        debug( "Argument: %s=%s", key, ws_xml_get_node_text(arg));
        if (!hash_alloc_insert(h, key, ws_xml_get_node_text(arg))) {
          error("hash_alloc_insert failed");
        }
      }
    }
    u_free(mn);
    u_free(input);
  } else {
    error("xml document is null");
  }
  if (!hash_isempty(h))
    return h;

  hash_destroy(h);
  return NULL;
}

hash_t*
wsman_get_selector_list( WsContextH cntx,
                         WsXmlDocH doc)
{
  hash_t *h = hash_create(HASHCOUNT_T_MAX, 0, 0);
  if ( doc == NULL )
    doc = ws_get_context_xml_doc_val(cntx, WSFW_INDOC);

  if ( doc )
  {
    WsXmlNodeH header = ws_xml_get_soap_header(doc);
    WsXmlNodeH node = ws_xml_get_child(header, 0, XML_NS_WS_MAN, WSM_SELECTOR_SET);
    if ( node )
    {
      WsXmlNodeH selector;
      int index = 0;
      while( (selector = ws_xml_get_child(node, index++, XML_NS_WS_MAN, WSM_SELECTOR)) )
      {
        char* attrVal = ws_xml_find_attr_value(selector, XML_NS_WS_MAN, WSM_NAME);
        if ( attrVal == NULL )
          attrVal = ws_xml_find_attr_value(selector, NULL, WSM_NAME);

        debug( "Selector: %s=%s", attrVal, ws_xml_get_node_text(selector));
        if ( attrVal )
        {
          if (!hash_alloc_insert(h, attrVal, ws_xml_get_node_text(selector))) {
            error("hash_alloc_insert failed");
          }
        }
      }
    }
  } else {
    error( "doc is null");
  }
  if (!hash_isempty(h))
    return h;

  hash_destroy(h);
  return NULL;
}

char*
wsman_get_selector( WsContextH cntx,
                    WsXmlDocH doc,
                    char* name, 
                    int index)
{
  char* val = NULL;
  if ( doc == NULL )
    doc = ws_get_context_xml_doc_val(cntx, WSFW_INDOC);
  if ( doc ) {
    WsXmlNodeH header = ws_xml_get_soap_header(doc);
    WsXmlNodeH node = ws_xml_get_child(header, index, XML_NS_WS_MAN, WSM_SELECTOR_SET);

    if ( node )
    {
      WsXmlNodeH selector;
      int index = 0;

      while( (selector = ws_xml_get_child(node, index++, XML_NS_WS_MAN, WSM_SELECTOR)) )
      {
        char* attrVal = ws_xml_find_attr_value(selector, XML_NS_WS_MAN, WSM_NAME);
        if ( attrVal == NULL )
          attrVal = ws_xml_find_attr_value(selector, NULL, WSM_NAME);

        if ( attrVal && !strcmp(attrVal, name) )
        {
          val = ws_xml_get_node_text(selector);
          break;
        }
      }
    }
  }
  debug( "Selector value for %s: %s", name, val );
  return val;
}

char*
ws_addressing_get_action( WsContextH cntx, 
                          WsXmlDocH doc)
{
  char *val = NULL;
  if ( doc == NULL )
    doc = ws_get_context_xml_doc_val(cntx, WSFW_INDOC);
  if ( doc ) {
    WsXmlNodeH header = ws_xml_get_soap_header(doc);
    WsXmlNodeH node = ws_xml_get_child(header, 0, XML_NS_ADDRESSING, WSA_ACTION);
    val = (!node) ? NULL : ws_xml_get_node_text(node);
  }

  return val;
}

void
wsman_add_selector( WsXmlNodeH baseNode,
                    char* name,
                    char* val)
{
  WsXmlNodeH selector = NULL;
  WsXmlNodeH set = ws_xml_get_child(baseNode, 0, XML_NS_WS_MAN, WSM_SELECTOR_SET);

  if ( set || (set = ws_xml_add_child(baseNode, XML_NS_WS_MAN, WSM_SELECTOR_SET, NULL)) )
  {
    if ( (selector = ws_xml_add_child(set, XML_NS_WS_MAN, WSM_SELECTOR, val)) )
    {
      ws_xml_add_node_attr(selector, NULL,  WSM_NAME, name);
    }
  }
  return;
}



void 
wsman_set_estimated_total(WsXmlDocH in_doc, 
                          WsXmlDocH out_doc, 
                          WsEnumerateInfo *enumInfo) 
{
  WsXmlNodeH header = ws_xml_get_soap_header( in_doc);
  if (ws_xml_get_child(header,0 , 
                       XML_NS_WS_MAN, 
                       WSM_REQUEST_TOTAL) != NULL) {
    if (out_doc) {
      WsXmlNodeH response_header = ws_xml_get_soap_header( out_doc);
      if (enumInfo->totalItems >= 0 )
        ws_xml_add_child_format(response_header, 
                                XML_NS_WS_MAN, 
                                WSM_TOTAL_ESTIMATE, "%d", enumInfo->totalItems);
    }
  }
  return;
}

int 
wsman_is_identify_request(WsXmlDocH doc) {

  WsXmlNodeH node = ws_xml_get_soap_body(doc);
  node = ws_xml_get_child(node, 0, XML_NS_WSMAN_ID, WSMID_IDENTIFY);
  if (node)
    return 1;
  else
    return 0;
}




