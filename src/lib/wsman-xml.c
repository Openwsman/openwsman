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
 * @author Eugene Yarmosh
 */
#ifdef HAVE_CONFIG_H
#include <wsman_config.h>
#endif

#include <libxml/xmlmemory.h>
#include <libxml/parser.h>

#include "wsman-xml-api.h"
#include "wsman-client-api.h"
#include "wsman-soap.h"
#include "wsman-xml.h"
#include "wsman-xml-binding.h"
#include "wsman-names.h"
/**
 * @defgroup XMLParserGeneric Generic XML Parser Interface
 * @brief Generic XML Parser interface functions
 * 
 * @{
 */

/**
 * Create default namespace prefix
 * @param node XML node
 * @param uri Namespace URI
 * @param buf Text buffer
 * @param bufsize Buffer size
 */
void
ws_xml_make_default_prefix ( WsXmlNodeH node, 
                             char* uri, 
                             char* buf, 
                             int bufsize)
{
    WsXmlDocH doc = xml_parser_get_doc(node);
    WsXmlNsH ns;

    if ( doc != NULL && (ns = ws_xml_find_wk_ns(doc->fw, uri, NULL)) != NULL )
        strncpy(buf, ws_xml_get_ns_prefix(ns), bufsize);
    else
        if ( bufsize >= 12 )
            sprintf(buf, "n%lu", ++doc->prefixIndex);
        else
            buf[0] = 0;
}

static int
is_xml_val_true( char* text )
{
    int retVal = 0;

    if ( text ) {
        char* ptr = text;

        while( isdigit(*ptr) )
            ptr++;

        if ( *ptr ) {
            if ( !strcasecmp(text, "true") || !strcasecmp(text, "yes") )
                retVal = 1;
        } else {
            if ( atoi(text) != 0 )
                retVal = 1;
        }
    }

    return retVal;
}

/**
 * Enumerate namespaces in a node
 * @param node XML node
 * @param callback Namespace Enumeration callback
 * @param data Callback data 
 */
static int 
ns_enum_at_node( WsXmlNodeH node, 
                 WsXmlNsEnumCallback callback, 
                 void* data)
{
    int retVal = 0;

    if ( node )
    {
        int i;
        WsXmlNsH ns;

        for(i = 0; (ns = ws_xml_get_ns(node, i)) != NULL; i++)
        {
            if ( (retVal = callback(node, ns, data)) != 0 )
                break;
        }
    }
    return retVal;
}


static char*
make_qname( WsXmlNodeH node, 
            char* uri, 
            char* name)
{
    char* buf = NULL; 
    if ( name && uri && name )
    {
        int len = 1 + strlen(name);
        WsXmlNsH ns = xml_parser_ns_find(node, uri, NULL, 1, 1); 
        char* prefix = (!ns) ? NULL : ws_xml_get_ns_prefix(ns); 

        if ( prefix != NULL )
            len += 1 + strlen(prefix);

        if ( (buf = u_malloc(len)) != NULL )
        {
            if ( prefix != NULL )
                sprintf(buf, "%s:%s", prefix, name);
            else
                strcpy(buf, name);
        }
    }
    return buf;
}


/**
 * Add QName attribute
 * @param node Parent XML node
 * @param nameNs Child Namespace 
 * @param name Child Name
 * @param valueNs Namespace for value
 * @param value Child Value
 * @return Child XML node
 * @note 
 * if namespaces has been changed after this function is called, itis caller's
 * responsibility to update QName fields accordingly
 */
WsXmlAttrH ws_xml_add_qname_attr(WsXmlNodeH node, 
        char* nameNs,
        char* name,
        char* valueNs,
        char* value)
{
    WsXmlAttrH attr = NULL;

    if ( name && node && valueNs && value )
    {
        char* buf = make_qname(node, valueNs, value);
        if ( buf != NULL )
        {
            attr = ws_xml_add_node_attr(node, nameNs, name, buf);
            u_free(buf);
        }
    }

    return attr;
}

/**
 * Enumerate namespaces
 * @param node XML node
 * @param callback enumeration callback
 * @param data Callback data
 * @param bWalkUpTree Flag FIXME
 * @brief Enumerates all namespaces defined at the node and optionally (if bIncludeParents isn't zero) 
 * walks up the parent chain
 */
void ws_xml_ns_enum(WsXmlNodeH node, 
                    WsXmlNsEnumCallback callback,
                    void* data,
                    int bWalkUpTree) 
{
    while(node)
    {
        if ( ns_enum_at_node(node, callback, data) || !bWalkUpTree )
            break;
        node = ws_xml_get_node_parent(node);
    }
}


/**
 * Get the SOAP operation from the body of the envelope
 * @param doc The XML document containing the envelope
 * @return The XML node with the operation
 *
 */

/*
WsXmlNodeH ws_xml_get_soap_operation(WsXmlDocH doc)
{
    WsXmlNodeH node = NULL;
    WsXmlNodeH body = ws_xml_get_soap_body(doc);

    if ( body )
        node = ws_xml_get_child(body, 0, NULL, NULL);

    return node;
}
*/

/**
 * Create an empty envelope with a <b>Header</b> and a <b>Body</b> 
 * @param soap Soap handler
 * @param soapVersion The SOAP version to be used for creating the envelope
 * @return An XMl document  
 */
WsXmlDocH ws_xml_create_envelope(SoapH soap, char* soapVersion)
{
    WsXmlDocH doc = NULL;

    if ( soapVersion == NULL )
        soapVersion = XML_NS_SOAP_1_2;

    if ( (doc = ws_xml_create_doc(soap, soapVersion, SOAP_ENVELOPE)) != NULL )
    {
        WsXmlNodeH root = ws_xml_get_doc_root(doc);

        if ( root == NULL
                ||
                ws_xml_add_child(root, soapVersion, "Header", NULL) == NULL 
                ||
                ws_xml_add_child(root, soapVersion, "Body", NULL) == NULL )
        {
            ws_xml_destroy_doc(doc);
            doc = NULL;		
        }
    }

    return doc;
}


/**
 * Duplicate an XML document
 * @param dstSoap Destination SOAP handle
 * @param srcDoc the Source document
 * @return The new XML document
 */
WsXmlDocH ws_xml_duplicate_doc(SoapH dstSoap, WsXmlDocH srcDoc)
{
    WsXmlDocH dst = NULL;

    if ( srcDoc )
    {
        WsXmlNodeH srcRoot = ws_xml_get_doc_root(srcDoc);
        if ( srcRoot )
        {
            SoapH soap = dstSoap;
            char* name = ws_xml_get_node_local_name(srcRoot);
            char* nsUri = ws_xml_get_node_name_ns(srcRoot);

            if ( soap == NULL )
                soap = ws_xml_get_doc_soap_handle(srcDoc);

            if ( (dst = ws_xml_create_doc(soap, nsUri, name)) != NULL )
            {
                int i;
                WsXmlNodeH node;
                WsXmlNodeH dstRoot = ws_xml_get_doc_root(dst);

                for(i = 0; (node = ws_xml_get_child(srcRoot, i, NULL, NULL)) != NULL; i++)
                {
                    ws_xml_duplicate_tree(dstRoot, node);
                }
            }
        }
    }
    return dst;
}


/**
 * Duplicate an XML attribute
 * @param dstNode Destination XML node
 * @param srcNode Source Node
 */
void ws_xml_duplicate_attr(WsXmlNodeH dstNode, WsXmlNodeH srcNode)
{
    int i;
    WsXmlAttrH attr;
    for(i = 0; (attr = ws_xml_get_node_attr(srcNode, i)) != NULL; i++)
    {
        ws_xml_add_node_attr(dstNode, 
                ws_xml_get_attr_ns(attr),
                ws_xml_get_attr_name(attr),
                ws_xml_get_attr_value(attr));
    }
}

/**
 * Duplicate children of an XML node
 * @param dstNode Destination XML node
 * @param srcNode Source XML node
 */
int ws_xml_duplicate_children(WsXmlNodeH dstNode, WsXmlNodeH srcNode)
{
    int i;
    WsXmlNodeH child;
    for(i = 0; (child = ws_xml_get_child(srcNode, i, NULL, NULL)) != NULL; i++) {
        ws_xml_duplicate_tree(dstNode, child);
    }
    return i;
}


/**
 * Duplication complete XML tree
 * @param dstNode Destination XML node
 * @param srcNode Source XML node
 */
void ws_xml_duplicate_tree(WsXmlNodeH dstNode, WsXmlNodeH srcNode)
{
    WsXmlNodeH node;
    if (!srcNode || !dstNode) {
        error("NULL arguments: dst = %p; src = %p", dstNode, srcNode);
        return;
    }
    node = ws_xml_add_child(dstNode,
                    ws_xml_get_node_name_ns(srcNode),
                    ws_xml_get_node_local_name(srcNode), NULL);
    if (!node) {
        error("could not add node");
        return;
    }
    ws_xml_duplicate_attr(node, srcNode);
    if (ws_xml_duplicate_children(node, srcNode) == 0) {
        // no children
        ws_xml_set_node_text(node, ws_xml_get_node_text(srcNode));
    }
}


void
ws_xml_copy_node(WsXmlNodeH src, WsXmlNodeH dst) 
{
    if (src && dst )
    {
        xmlNodePtr x = xmlDocCopyNode((xmlNodePtr)src, ((xmlDocPtr)src)->doc , 1 );
        if (x)
            xmlAddChild((xmlNodePtr ) dst, x );
    }
}


int ws_xml_utf8_strlen(char *buf) {
    return xml_parser_utf8_strlen(buf);
}

/**
 * Dump XML docuemnt contents into a Text buffer
 * @param doc XML document
 * @param buf The target buffer
 * @param ptrSize the size of the buffer
 * @param encoding The encoding to be used
 */
void ws_xml_dump_memory_enc(WsXmlDocH doc, char** buf, int* ptrSize, char* encoding)
{
    xml_parser_doc_to_memory(doc, buf, ptrSize, encoding);
}


#if 0

/**
 * Find a text in an XML document
 * @param doc The XML document
 * @param nsUri Namespace URI
 * @param name Node name
 * @return found text 
 */
char* ws_xml_find_text_in_doc(WsXmlDocH doc, char* nsUri, char* name)
{
    WsXmlNodeH root = ws_xml_get_doc_root(doc);
    return ws_xml_find_text_in_tree(root, nsUri, name, 1);
}


/**
 * Find a text in a XML tree
 * @param head The head node of the tree
 * @param nsUri Namespace URI
 * @param name Node name
 * @param bRecursive Recursive flag
 * @return found text 
 */
char* ws_xml_find_text_in_tree(WsXmlNodeH head, char* nsUri, char* name, int bRecursive)
{
    WsXmlNodeH node = head;

    if ( !ws_xml_is_node_qname(head, nsUri, name) )
        node = ws_xml_find_in_tree(head, nsUri, name, bRecursive); 

    if ( node )
        return ws_xml_get_node_text(node);

    return NULL;
}
#endif

/**
 * Free Memory
 * @param ptr Pointer to be freed
 */
void ws_xml_free_memory(void* ptr)
{
    xml_parser_free_memory(ptr);
}

/**
 * Get SOAP handle of an XML document
 * @param doc XML document
 * @return SOAP handle
 */
SoapH ws_xml_get_doc_soap_handle(WsXmlDocH doc)
{
    SoapH soap = NULL;
    if ( doc )
        soap = doc->fw;
    return soap;
}



/**
 * Initialize XML Parser
 * @param soap SOAP handle
 * @param nsData Array with namespace data
 */
int 
ws_xml_parser_initialize( SoapH soap, 
                          WsXmlNsData nsData[])
{
    int retVal = -1;
    WsXmlParserData* parserData = NULL;

    if ( soap != NULL 
            && 
            (parserData = (WsXmlParserData*)u_zalloc(sizeof(WsXmlParserData))) != NULL)
    {
        u_lock(soap);

        xml_parser_initialize(soap);

        if ( (parserData->nsHolder = ws_xml_create_doc(soap, NULL, "NsList")) != NULL )
        {
            WsXmlNodeH node = ws_xml_get_doc_root(parserData->nsHolder);
            retVal = 0;
            soap->parserData = parserData;
            if ( nsData )
            {
                int i;
                for(i = 0; nsData[i].uri != NULL; i++)
                {
                    WsXmlNsData* nsd = &nsData[i];
                    ws_xml_define_ns(node, nsd->uri, nsd->prefix, 0);
                }
            }
        }
        u_unlock(soap);
    }

    if ( retVal != 0 && parserData )
    {
        u_free(parserData);
    }

    return retVal;
}

void 
ws_xml_parser_destroy(SoapH soap)
{
    if (soap && soap->parserData )
    {
        u_lock(soap);
        ws_xml_destroy_doc(((WsXmlParserData*)soap->parserData)->nsHolder);
        xml_parser_destroy(soap);
        u_free(soap->parserData);
        soap->parserData = NULL;
        u_unlock(soap);
    }
}



/**
 * Get SOAP envelope header
 * @param doc XML document (Envelope)
 * @return XML node of the Header
 */
WsXmlNodeH ws_xml_get_soap_header(WsXmlDocH doc)
{
    return ws_xml_get_soap_element(doc, SOAP_HEADER);
}


/**
 * Enumerate Children
 * @param parent XML node parent
 * @param callback Enumeration callback
 * @param data Callback data
 * @param bRecursive Recursive flag
 * @return 
 * 
 */
int 
ws_xml_enum_children(WsXmlNodeH parent, 
        WsXmlEnumCallback callback, 
        void* data, 
        int bRecursive)
{
    int retVal = 0;
    int i;
    WsXmlNodeH child;

    for(i = 0; (child = ws_xml_get_child(parent, i, NULL, NULL)) != NULL; i++)
    {
        if ( (retVal =  ws_xml_enum_tree(child, callback, data, bRecursive)) )
        {
            break;
        }
    }
    return retVal;
}


/**
 * Get count of children
 * @param parent XML Node parent
 * @return Count of children in node
 */
int ws_xml_get_child_count(WsXmlNodeH parent)
{
    int count = 0;
    if ( parent )
        count = xml_parser_get_count(parent, XML_COUNT_NODE, 0);
    return count;
}


/**
 * Enumerate XML tree
 * @param top Top XML node
 * @param callback Emumeration callback
 * @param data Callback data
 * @param bRecursive Recursive flag
 */
int ws_xml_enum_tree(WsXmlNodeH top, WsXmlEnumCallback callback, void* data, int bRecursive)
{
    int retVal = 0;
    if ( top )
    {
        if ( !(retVal = callback(top, data)) && bRecursive )
        {
            retVal = ws_xml_enum_children(top, callback, data, bRecursive);
        }
    }
    return retVal;
}


/**
 * Get node namespace
 * @param node XML node
 * @return Namespace of node
 */
char* ws_xml_get_node_name_ns(WsXmlNodeH node)
{
    char* uri = NULL;
    if ( node )
        uri = xml_parser_node_query(node, XML_NS_URI);

    return uri;
}


/**
 * Get node local name
 * @param node XML node
 * @return Node local name
 */
char* ws_xml_get_node_local_name(WsXmlNodeH node)
{
    char* name = NULL;
    if ( node )
        name = xml_parser_node_query(node, XML_LOCAL_NAME);
    return name;
}


/**
 * Get XML Document root
 * @param doc XML document
 * @return XML root node
 */
WsXmlNodeH ws_xml_get_doc_root(WsXmlDocH doc)
{
    WsXmlNodeH node = NULL;
    if ( doc != NULL )
        node = xml_parser_get_root(doc);
    return node;
}

/**
 * Get Node text
 * @param node XML node
 * @return XML node text
 */
char* ws_xml_get_node_text(WsXmlNodeH node) {
    char* text = NULL;
    if ( node ) {
        text = xml_parser_node_query(node, XML_TEXT_VALUE);
    }
    return text;	
}

/**
 * Read memory buffer into an XMl document
 * @param soap SOAP handler
 * @param buf Text buffer with XML string
 * @param size Buffer size
 * @param encoding Buffer encoding
 * @param options Parser options
 * @return XML document
 */
WsXmlDocH ws_xml_read_memory(
        SoapH soap, 
        char* buf, 
        size_t size, 
        char* encoding, 
        unsigned long options)
{
    return xml_parser_memory_to_doc(soap, buf, size, encoding, options);
}


WsXmlDocH ws_xml_read_file( SoapH soap, char* filename, 
        char* encoding, unsigned long options)
{
    return xml_parser_file_to_doc(soap, filename, encoding, options);
}


/**
 * Create XML document
 * @param soap SOAP handler
 * @param rootNsUri Root Namespace URI
 * @param rootName Root node name
 * @return XML document
 */
WsXmlDocH 
ws_xml_create_doc( SoapH soap, 
                   char* rootNsUri,
                   char* rootName) {
    WsXmlDocH wsDoc = (WsXmlDocH)u_zalloc(sizeof (*wsDoc));
	WsXmlNodeH rootNode;
	WsXmlNsH ns;
    char prefix[12];

    if (wsDoc == NULL) {
        error("No memory");
        return NULL;
    }
    wsDoc->fw = soap;
    if (xml_parser_create_doc(wsDoc, rootName) != 0) {
            error("xml_parser_create_doc failed");
            u_free(wsDoc);
            return NULL;
    }

    if (rootNsUri == NULL) {
        return wsDoc;
    }

    rootNode = ws_xml_get_doc_root((WsXmlDocH)wsDoc);
    

    ws_xml_make_default_prefix(rootNode, rootNsUri, prefix, sizeof (prefix));
    ns = xml_parser_ns_add(rootNode, rootNsUri, prefix);
    if (ns == NULL) {
         error("xml_parser_ns_add failed");
         ws_xml_destroy_doc(wsDoc);
         return NULL;
    }
    ws_xml_set_node_name(rootNode, rootNsUri, NULL);
    return wsDoc;	
}



/**
 * Set node name
 * @param node XML node
 * @param nsUri Namespace URI
 * @param name Node name
 * @return status
 * 
 */
int ws_xml_set_node_name(WsXmlNodeH node, char* nsUri, char* name)
{
    int retVal = -1;

    if ( node && (name || nsUri) )
    {
        if ( name )
            retVal = xml_parser_node_set(node, XML_LOCAL_NAME, name);
        else
            retVal = 0;

        if ( !retVal && nsUri )
            retVal = xml_parser_node_set(node, XML_NS_URI, nsUri);
    }

    return retVal;
}




/**
 * Destroy XML document
 * @param doc XML document
 */
void ws_xml_destroy_doc(WsXmlDocH doc) {
    if (doc) {
        xml_parser_destroy_doc(doc);
        u_free(doc);
    }
}



/**
 * Callback for finding objects in tree
 * @param node XML node
 * @param _data Callback data
 * @return status
 */
static int find_in_tree_callback(WsXmlNodeH node, void* _data)
{
    FindInTreeCallbackData* data = (FindInTreeCallbackData*)_data;
    int retVal = ws_xml_is_node_qname(node, data->ns, data->name);

    if ( retVal )
        data->node = node;

    return retVal;
}

/**
 * Find node in XML tree
 * @param head Head XML node
 * @param nsUri Namespace URI
 * @param localName Node local name
 * @param bRecursive Recursive flag
 * @return Result XML node
 */
WsXmlNodeH ws_xml_find_in_tree(WsXmlNodeH head, char* nsUri, char* localName, int bRecursive)
{
    FindInTreeCallbackData data;

    data.node = NULL;
    data.ns = nsUri;
    data.name = localName;

    ws_xml_enum_tree(head, find_in_tree_callback, &data, bRecursive);

    return data.node;
}


/**
 * Get SOAP body
 * @param doc XML document
 * @return Result XML node
 */
WsXmlNodeH ws_xml_get_soap_body(WsXmlDocH doc)
{
    return ws_xml_get_soap_element(doc, SOAP_BODY);
}



/**
 * Get SOAP element
 * @param doc XML document
 * @param name Node name
 * @return Result XML node
 */
WsXmlNodeH ws_xml_get_soap_element(WsXmlDocH doc, char* name)
{
    WsXmlNodeH node = NULL;
    WsXmlNodeH env = ws_xml_get_soap_envelope(doc);

    if ( env != NULL )
    {
        char* soapUri = ws_xml_get_node_name_ns(env);

        if ( (node = ws_xml_get_child(env, 0, NULL, NULL)) != NULL )
        {
            if ( !ws_xml_is_node_qname(node, soapUri, name) )
            {
                if ( strcmp(name, SOAP_HEADER) != 0 )
                {
                    if ( (node = ws_xml_get_child(env, 1, NULL, NULL)) != NULL )
                    {
                        if ( !ws_xml_is_node_qname(node, soapUri, name) )
                            node = NULL;
                    }
                }
            }
        }
    } 
    return node;
}

/**
 * Get XML child of a node
 * @param parent Parent node
 * @param index Index of the node to be returned
 * @param nsUri Namespace URI
 * @param localName Local name of the node
 * @return Result XML node
 */
WsXmlNodeH
ws_xml_get_child( WsXmlNodeH parent, 
                  int index,
                  char* nsUri,
                  char* localName)
{
    WsXmlNodeH node = NULL;

    if ( parent && index >= 0 )
    {
        if ( nsUri == NULL && localName == NULL )
            node = xml_parser_node_get(parent, index);
        else
        {
            int count = 0;
            node = xml_parser_get_first_child(parent);
            while( node != NULL )
            {
                if ( ws_xml_is_node_qname(node, nsUri, localName) )
                {
                    if ( count == index )
                        break;
                    count++;
                }
                node = xml_parser_get_next_child(node);
            }
        }
    }

    return node;
}

/**
 * Is the XML node a qualified name
 * @param node XML node
 * @param nsUri Namespace URI
 * @param name node name
 * @return Returns 1 if node is QName
 * @brief Shortcats for QName manipulation name can be NULL, in this case just check namespace
 */
int ws_xml_is_node_qname(WsXmlNodeH node, char* nsUri, char* name)
{
    int retVal = 0;
    if ( node )
    {
        char* nodeNsUri = ws_xml_get_node_name_ns(node);
        if (  ( nsUri == nodeNsUri ) ||
                (nsUri != NULL && nodeNsUri != NULL && !strcmp(nodeNsUri, nsUri)) )
        {
            if ( name == NULL || !strcmp(name, ws_xml_get_node_local_name(node)) )
                retVal = 1;
        }
    }
    return retVal;
}



WsXmlNsH
ws_xml_find_wk_ns( SoapH soap, 
                   char* uri, 
                   char* prefix)
{
    WsXmlNsH ns = NULL;
    if (soap)
    {
        WsXmlParserData* data = (WsXmlParserData*)soap->parserData;
        if ( data && data->nsHolder )
        {            
            WsXmlNodeH root = ws_xml_get_doc_root(data->nsHolder);
            ns = ws_xml_find_ns(root, uri, prefix, 0);            
        }
    }
    return ns;
}

/**
 * Get SOAP envelope
 * @param doc XML document
 * @return XML node with envelope
 */
WsXmlNodeH ws_xml_get_soap_envelope(WsXmlDocH doc)
{
    WsXmlNodeH root = ws_xml_get_doc_root(doc);
    if ( ws_xml_is_node_qname(root, XML_NS_SOAP_1_2, SOAP_ENVELOPE) 
            ||
            ws_xml_is_node_qname(root, XML_NS_SOAP_1_1, SOAP_ENVELOPE) )
    {
        return root;
    }
    return NULL;
}


/**
 * Get SOAP Faul
 * @param doc XML document
 * @return XML node with fault, if NULL is returned, then the document is not a fault
 */
/*
WsXmlNodeH ws_xml_get_soap_fault(WsXmlDocH doc)
{
    char* soapUri = ws_xml_get_node_name_ns(ws_xml_get_doc_root(doc));
    WsXmlNodeH node = ws_xml_get_soap_operation(doc);

    if ( node && !ws_xml_is_node_qname(node, soapUri, SOAP_FAULT) )
        node = NULL;

    return node;
}
*/




/**
 * Get Node parent
 * @param node XML node
 * @return Node parent
 */
WsXmlNodeH ws_xml_get_node_parent(WsXmlNodeH node)
{
    WsXmlNodeH parent = NULL;
    if ( node != NULL )
        parent = xml_parser_node_get(node, XML_ELEMENT_PARENT);

    return parent;
}

/**
 * Callback for finding Namespaces
 * @param node XML node
 * @param ns Namespace
 * @param _data Callback Data
 * @return status
 */
static int
ws_xml_find_ns_callback( WsXmlNodeH node, 
                         WsXmlNsH ns, 
                         void* _data)
{
    WsXmlFindNsData* data = (WsXmlFindNsData*)_data;
    char* curUri = ws_xml_get_ns_uri(ns);
    char* curPrefix = ws_xml_get_ns_prefix(ns);
    // debug("uri: %s prefix: %s", curUri, curPrefix );

    if ( (data->nsUri != NULL && !strcmp(curUri, data->nsUri))
            ||
            (data->prefix != NULL && curPrefix != NULL && !strcmp(curPrefix, data->prefix))
            ||
            (data->nsUri == NULL && data->prefix == NULL && curPrefix == NULL)	)
    {
        data->node = node;
        data->ns = ns;
    }

    return (data->ns != NULL);
}


/**
 * Find namespace in an XML node
 * @param node XML node
 * @param nsUri Namespace URI
 * @param prefix Prefix
 * @param bWalkUpTree Flag FIXME
 * @brief
 * Looks up nsUri defined at the node and optionally
 * (if bIncludeParents isn't zero) walks up the parent chain
 * returns prefix for the namespace and node where it defined
 */
WsXmlNsH 
ws_xml_find_ns( WsXmlNodeH node, 
                char* nsUri, 
                char* prefix, 
                int bWalkUpTree)
{
    WsXmlFindNsData data;

    data.node = NULL;
    data.ns = NULL;
    data.nsUri = nsUri;
    data.prefix = prefix;

    if ( (nsUri || prefix) && node )
        ws_xml_ns_enum(node, ws_xml_find_ns_callback, &data, bWalkUpTree);

    return data.ns;
}


char*
ws_xml_get_node_name_ns_prefix(WsXmlNodeH node)
{
    char* prefix = NULL;
    if ( node )
        prefix = xml_parser_node_query(node, XML_NS_PREFIX);
    return prefix;

}

char*
ws_xml_get_node_name_ns_uri(WsXmlNodeH node)
{
    char* uri = NULL;
    if ( node )
        uri = xml_parser_node_query(node, XML_NS_URI);
    return uri;

}


/**
 * Get count of Namespaces
 * @param node XML node
 * @param bWalkUpTree Tree Flag
 * @return Count
 */
int
ws_xml_get_ns_count( WsXmlNodeH node, 
                     int bWalkUpTree)
{
    int count = xml_parser_get_count(node, XML_COUNT_NS, bWalkUpTree);

    return count;
}


/**
 * Get Namespace Prefix
 * @param ns Namespace
 * @return Prefix of Namespace
 */
char* 
ws_xml_get_ns_prefix(WsXmlNsH ns)
{
    if ( ns )
        return xml_parser_ns_query(ns, XML_NS_PREFIX);
    return NULL;
}


/**
 * Get Namespace URI
 * @param ns Namespace
 * @return URI of namespace, NULL of not found 
 */
char*
ws_xml_get_ns_uri(WsXmlNsH ns)
{
    if ( ns )
        return xml_parser_ns_query(ns, XML_NS_URI);
    return NULL;
}



/**
 * Get Namespace from a node
 * @param node XML node
 * @param index Index
 * @return Namespace
 */
WsXmlNsH ws_xml_get_ns(WsXmlNodeH node, int index)
{
    if ( node )
        return xml_parser_ns_get(node, index);
    return NULL;
}




/**
 * Add child to an XML node
 * @param node XML node
 * @param nsUri Namespace URI
 * @param localName local name
 * @param val Value of the node
 * @return New XML node
 */
WsXmlNodeH 
ws_xml_add_child(WsXmlNodeH node, 
                 char* nsUri,
                 char* localName,
                 char* val)
{
    WsXmlNodeH newNode = 
        xml_parser_node_add(node, XML_LAST_CHILD, nsUri, localName, val); 

    return newNode;
}

WsXmlNodeH 
ws_xml_add_empty_child_format(WsXmlNodeH node, char* nsUri, char* format, ...)
{
	WsXmlNodeH newNode;
    va_list args;
    char buf[4096];
    va_start (args, format);
    vsnprintf(buf,4096,format,args);
    va_end(args);
    newNode = 
        xml_parser_node_add(node, XML_LAST_CHILD, nsUri, buf, NULL); 

    return newNode;
}

WsXmlNodeH 
ws_xml_add_child_format(WsXmlNodeH node, char* nsUri, char* localName, char* format, ...)
{
	WsXmlNodeH newNode;
    va_list args;
    char buf[4096];
    va_start (args, format);
    vsnprintf(buf,4096,format,args);
    va_end(args);
    newNode = 
        xml_parser_node_add(node, XML_LAST_CHILD, nsUri, localName, buf); 

    return newNode;
}


/**
 * Check of namespace prefix is OK.
 * @param ns Namespace
 * @param newPrefix New prefix 
 * @param bDefault FIXME
 * @return 1 if Ok, 0 if not
 */
static int 
is_ns_prefix_ok(WsXmlNsH ns, char* newPrefix, int bDefault)
{
    int retVal = 0;
    char* curPrefix = xml_parser_ns_query(ns, XML_NS_PREFIX);

    if ( bDefault )
    {
        if ( curPrefix == NULL )
            retVal = 1;
    }
    else
    {
        if ( (newPrefix == NULL && curPrefix != NULL)
                ||
                (newPrefix && curPrefix && !strcmp(newPrefix, curPrefix)) )
        {
            retVal = 1;
        }
    }

    return retVal;
}




/**
 * Define Namespace
 * @param node XML node
 * @param nsUri Namespace URI
 * @param nsPrefix Namespace Prefix
 * @param bDefault FIXME
 * @return New Namespace
 * @todo if ns is present, it should work as replace, walk through the tree and
 * update QName values and attributes
 */
WsXmlNsH
ws_xml_define_ns(WsXmlNodeH node, char* nsUri, char* nsPrefix, int bDefault)
{
    WsXmlNsH ns = NULL;

    if ( node && nsUri )
    {
        ns = ws_xml_find_ns(node, nsUri, NULL, 0);
        if ( ns == NULL || !is_ns_prefix_ok(ns, nsPrefix, bDefault) )
        {
            char buf[12];
            if ( !bDefault && nsPrefix == NULL )
            {
                ws_xml_make_default_prefix(node, nsUri, buf, sizeof(buf));
                nsPrefix = buf;
            }
            ns = xml_parser_ns_add(node, nsUri, nsPrefix);
        }
    }
    return ns;
}

/**
 * Add QName child
 * @param parent Parent XML node
 * @param nameNs Child Namespace 
 * @param name Child Name
 * @param valueNs Namespace for value
 * @param value Child Value
 * @return Child XML node
 * @note if namespaces has been changed after this function is called, it is caller's
 * responsibility to update QName fields accordingly
 * 
 */
WsXmlNodeH
ws_xml_add_qname_child(WsXmlNodeH parent, 
        char* nameNs,
        char* name,
        char* valueNs,
        char* value)
{
    WsXmlNodeH node = ws_xml_add_child(parent, nameNs, name, NULL);
    if ( node == NULL )
    {
        ws_xml_set_node_qname_val(node, valueNs, value);
    }
    return node;
}


int ws_xml_get_node_attr_count(WsXmlNodeH node)
{
    int count = 0;

    if ( node )
        count = xml_parser_get_count(node, XML_COUNT_ATTR, 0);

    return count;
}


WsXmlAttrH
ws_xml_add_node_attr( WsXmlNodeH node, 
                      char* nsUri, 
                      char* name, 
                      char* value)
{
    WsXmlAttrH attr = NULL;
    if ( node && name )
        attr = xml_parser_attr_add(node, nsUri, name, value);

    return (WsXmlAttrH)attr;
}


void 
ws_xml_remove_node_attr(WsXmlAttrH attr)
{
    if ( attr )
        xml_parser_attr_remove(attr);
}


WsXmlAttrH
ws_xml_get_node_attr(WsXmlNodeH node, int index)
{
    return xml_parser_attr_get(node, index);
}

WsXmlAttrH 
ws_xml_find_node_attr(WsXmlNodeH node, char* attrNs, char* attrName)
{
    WsXmlAttrH attr = NULL;
    if ( node && attrName )
    {
        int i = 0;

        for(i = 0; (attr = ws_xml_get_node_attr(node, i)) != NULL; i++)
        {
            char* curNsUri = ws_xml_get_attr_ns(attr);
            char* curName = ws_xml_get_attr_name(attr);

            if ((attrNs == curNsUri)
                    ||
                    (attrNs != NULL
                     &&
                     curNsUri != NULL
                     &&
                     !strcmp(curNsUri, attrNs)) )
            {
                if ( !strcmp(attrName, curName) )
                    break;
            }
        }
    }

    return attr;
}


unsigned long 
ws_xml_get_node_ulong(WsXmlNodeH node)
{
    unsigned long val = 0;
    char* text = ws_xml_get_node_text(node);

    if ( text )
        val = atoi(text);
    return val;
}


int 
ws_xml_set_node_ulong(WsXmlNodeH node, unsigned long uVal)
{
    int retVal = -1;
    if ( node )
    {
        char buf[12];
        sprintf(buf, "%lu", uVal);
        retVal = ws_xml_set_node_text(node, buf);
    }
    return retVal;
}




char* ws_xml_get_attr_name(WsXmlAttrH attr)
{
    char* name = NULL;
    if ( attr )
        name = xml_parser_attr_query(attr, XML_LOCAL_NAME);
    return name;
}

char* ws_xml_get_attr_ns(WsXmlAttrH attr)
{
    char* nsUri = NULL;

    if ( attr )
        nsUri = xml_parser_attr_query(attr, XML_NS_URI);

    return nsUri;
}

char* ws_xml_get_attr_ns_prefix(WsXmlAttrH attr)
{
    char* prefix = NULL;

    if ( attr )
        prefix = xml_parser_attr_query(attr, XML_NS_PREFIX);

    return prefix;
}


char* ws_xml_get_attr_value(WsXmlAttrH attr)
{
    char* val = NULL;

    if ( attr )
        val = xml_parser_attr_query(attr, XML_TEXT_VALUE);

    return val;
}


char* ws_xml_find_attr_value(WsXmlNodeH node, char* ns, char* attrName)
{
    char* val = NULL;
    WsXmlAttrH attr = ws_xml_find_node_attr(node, ns, attrName);

    if ( attr )
        val =  ws_xml_get_attr_value(attr);

    return val;
}

int ws_xml_find_attr_bool(WsXmlNodeH node, char* ns, char* attrName)
{
    int retVal = 0;
    char* val = ws_xml_find_attr_value(node, ns, attrName);

    if ( val != NULL )
        retVal = is_xml_val_true(val);

    return retVal;
}


unsigned long ws_xml_find_attr_ulong(WsXmlNodeH node, char* ns, char* attrName)
{
    unsigned long retVal = 0;
    char* val = ws_xml_find_attr_value(node, ns, attrName);

    if ( val != NULL )
        retVal = atoi(val);

    return retVal;
}


// if ns is not defined at the node or at any of its parents, it will be defined at the root
// if namespaces has been changed after this function is called, itis caller's
// responsibility to update QName fields accordingly
int ws_xml_set_node_qname_val(WsXmlNodeH node, char* valNsUri, char* valName)
{
    int retVal = -1;
    if ( node && valName && valNsUri )
    {
        char* buf = make_qname(node, valNsUri, valName);

        if ( buf != NULL )
        {
            retVal = ws_xml_set_node_text(node, buf);
            u_free(buf);
        }
    }
    return retVal;
}

WsXmlDocH ws_xml_get_node_doc(WsXmlNodeH node)
{
    WsXmlDocH doc = NULL;

    if ( node != NULL )
        doc = xml_parser_get_doc(node);

    return doc;
}


int ws_xml_set_node_text(WsXmlNodeH node, char* text)
{
    int retVal = -1;

    if ( node )
        retVal = xml_parser_node_set(node, XML_TEXT_VALUE, text);

    return retVal;
}

void ws_xml_set_node_lang(WsXmlNodeH node, char* lang)
{
    xmlNodeSetLang((xmlNodePtr )node, BAD_CAST lang);
}


// Utitlities
/*
int is_root_node(WsXmlNodeH node)
{
    WsXmlNodeH root = ws_xml_get_doc_root(ws_xml_get_node_doc(node));
    return (root == node);
}
*/


void ws_xml_dump_node_tree(FILE* f, WsXmlNodeH node)
{
    WsXmlDocH doc = xml_parser_get_doc(node);
    xml_parser_doc_dump(f, doc);
    return;
}

void ws_xml_dump_memory_node_tree(WsXmlNodeH node, char** buf, int* ptrSize)
{
    WsXmlDocH doc = xml_parser_get_doc(node);
    xml_parser_doc_dump_memory(doc, buf, ptrSize);
    return;
}

void ws_xml_dump_doc(FILE* f, WsXmlDocH doc ) {
    xml_parser_doc_dump(f, doc);
    return;
}


WsXmlNsH 
ws_xml_ns_add( WsXmlNodeH node, 
               char* uri, 
               char* prefix)
{
    return xml_parser_ns_add(node, uri, prefix );
}



int 
check_xpath(WsXmlNodeH node, char *xpath_expr) {
    //return xml_parser_check_xpath(node, xpath_expr);
    return 0;
}


char *
ws_xml_get_xpath_value (WsXmlDocH doc, char *expression)
{    
    return xml_parser_get_xpath_value(doc, expression);
}



WsXmlDocH 
ws_xml_create_doc_by_import( WsXmlNodeH node ) {
        WsXmlDocH wsDoc = (WsXmlDocH)u_zalloc(sizeof (*wsDoc));
	xml_parser_create_doc_by_import( wsDoc,  node );
        return wsDoc;
}





/** @} */
