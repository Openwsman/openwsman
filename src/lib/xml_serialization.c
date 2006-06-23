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
 * @author Eugene Yarmosh
 */


#include "stdlib.h"
#include "string.h"
#include "stdio.h"
#include <glib.h>

#include <ctype.h>
#include "assert.h"


#include "ws_utilities.h"

#include "ws_xml_api.h"
#include "soap_api.h"
#include "xml_api_generic.h"

#include "ws_dispatcher.h"
#include "ws_errors.h"
#include "xml_serializer.h"
#include "xml_serialization.h"
#include "wsman-debug.h"

// xml_serializer_alloc
void* xml_serializer_alloc(XmlSerializationData* data, int size, int zeroInit)
{
    void* ptr = ws_serializer_alloc(data->cntx, size);

    if ( ptr && zeroInit )
        memset(ptr, 0, size);
    return ptr;
}


// SerializerFree
int xml_serializer_free(XmlSerializationData* data, void* buf)
{
    return ws_serializer_free(data->cntx, buf);
}


// MakeDstPtr
XML_TYPE_PTR make_dst_ptr(XmlSerializationData* data, int size)
{
    void* ptr = data->elementBuf;
    if ( XML_IS_PTR(data->elementInfo) )
    {
        if ( data->mode == XML_SMODE_DESERIALIZE )
        {
            ptr = xml_serializer_alloc(data, size, 0);
            *((XML_TYPE_PTR*)data->elementBuf) = ptr;
        }
        else
            ptr = *((XML_TYPE_PTR*)data->elementBuf);
    }
    return ptr;
}


// SerialiserFreeScalarMem
void xml_serializer_free_scalar_mem(XmlSerializationData* data)
{
    if ( XML_IS_PTR(data->elementInfo) )
    {
        if (xml_serializer_free(data, *((XML_TYPE_PTR*)data->elementBuf)) )
            *((XML_TYPE_PTR*)data->elementBuf) = NULL;
    }
}


//SerialzerAddChild
WsXmlNodeH xml_serializer_add_child(XmlSerializationData* data, char* value)
{
    char* name = (data->name) ? data->name : data->elementInfo->name;		
    return ws_xml_add_child(data->xmlNode, data->ns, name, value); 
}


//SerialzerGetChild
WsXmlNodeH xml_serializer_get_child(XmlSerializationData* data)
{
    char* name = (data->name) ? data->name : data->elementInfo->name;
    WsXmlNodeH node = ws_xml_get_child(data->xmlNode, data->index, data->ns, name);

    if ( g_NameNameAliaseTable )
    {
        int index = 0;
        while( node == NULL && g_NameNameAliaseTable[index].name != NULL )
        {
            if ( !strcmp(g_NameNameAliaseTable[index].name, name) )
                node = ws_xml_get_child(data->xmlNode, 
                        data->index, 
                        data->ns, 
                        g_NameNameAliaseTable[index].aliase);
            index++;
        }
    }

    return node;
}


// DoSerializeUint
int do_serialize_uint(XmlSerializationData* data, int valSize) 
{
    WsXmlNodeH child;
    unsigned long tmp;
    int retVal = valSize;
    XML_TYPE_PTR dataPtr = make_dst_ptr(data, valSize);

    if ( dataPtr == NULL )
    {
        if ( (data->mode == XML_SMODE_DESERIALIZE 
                    ||
                    data->mode == XML_SMODE_SERIALIZE) )
        {
            retVal = WS_ERR_INSUFFICIENT_RESOURCES;
            goto error;
        }
    }

    if ( data->mode == XML_SMODE_FREE_MEM )
    {
        xml_serializer_free_scalar_mem(data);
    }
    else
        if ( data->mode == XML_SMODE_SERIALIZE )
        {
            if ( valSize == 1 )
                tmp = *((unsigned char*)dataPtr);
            else
                if ( valSize == 2 )
                    tmp = *((unsigned short*)dataPtr);
                else
                    tmp = *((unsigned long*)dataPtr);

            if ( (child = xml_serializer_add_child(data, NULL)) == NULL
                    ||
                    ws_xml_set_node_ulong(child, tmp) != 0 )
            {
                retVal = WS_ERR_INSUFFICIENT_RESOURCES;
            }
        }
        else
            if ( data->mode == XML_SMODE_DESERIALIZE )
            {
                char* str;

                if ( !(child = xml_serializer_get_child(data)) )
                {
                    retVal = WS_ERR_XML_NODE_NOT_FOUND;
                }
                else
                    if ( (str = ws_xml_get_node_text(child)) != NULL )
                    {
                        tmp = strtoul(str, NULL, 10);
                        // TBD: validate that value  doesn't exceed size
                        if ( valSize == 1 )
                            *((unsigned char*)dataPtr) = (unsigned char)tmp;
                        else
                            if ( valSize == 2 )
                                *((unsigned short*)dataPtr) = (unsigned short)tmp;
                            else
                                *((unsigned long*)dataPtr) = tmp;
                    }
                    else
                        retVal = WS_ERR_XML_PARSING;
            }
            else
                if ( data->mode != XML_SMODE_BINARY_SIZE )
                {
                    retVal = WS_ERR_INVALID_PARAMETER;
                }
error:
    return retVal;
}





// DoSerializeUint8
int do_serialize_uint8(XmlSerializationData* data)
{
    return do_serialize_uint(data, sizeof(XML_TYPE_UINT8));
}


int do_serialize_uint16(XmlSerializationData* data)
{
    return do_serialize_uint(data, sizeof(XML_TYPE_UINT16));
}

int do_serialize_uint32(XmlSerializationData* data)
{
    return do_serialize_uint(data, sizeof(XML_TYPE_UINT32));
}

// We will need special function DoSerializeArrayOfStringPtrs()
int do_serialize_string(XmlSerializationData* data)
{
    WsXmlNodeH child;
    int retVal = sizeof(XML_TYPE_STR);

    if ( data->mode == XML_SMODE_FREE_MEM )
    {
        xml_serializer_free_scalar_mem(data);
    }
    else
        if ( data->mode == XML_SMODE_SERIALIZE )
        {
            wsman_debug (WSMAN_DEBUG_LEVEL_DEBUG, "Serializing string...");
            char* valPtr = *((char**)data->elementBuf); 

            if ( (child = xml_serializer_add_child(data, valPtr)) == NULL )
            {
                retVal = WS_ERR_INSUFFICIENT_RESOURCES;
            }
            else
            {
                if ( ws_xml_get_node_text(child) == NULL )
                {
                    ws_xml_add_node_attr(child, XML_NS_SCHEMA_INSTANCE, XML_SCHEMA_NIL, "true");
                }
            }
        }
        else
        {
            if ( data->mode == XML_SMODE_DESERIALIZE )
            {
                wsman_debug (WSMAN_DEBUG_LEVEL_DEBUG, "De-serializing string...");
                if ( (child = xml_serializer_get_child(data)) == NULL )
                {
                    retVal = WS_ERR_XML_NODE_NOT_FOUND;
                }
                else
                {
                    char* src = ws_xml_get_node_text(child);
                    wsman_debug (WSMAN_DEBUG_LEVEL_DEBUG, "string: %s", src);

                    if ( src != NULL && *src != 0 )
                    {
                        char* dstPtr;
                        int dstSize = 1 + strlen(src);

                        if ( (dstPtr = (char*)xml_serializer_alloc(data, dstSize, 0)) == NULL )
                            retVal = WS_ERR_INSUFFICIENT_RESOURCES;
                        else
                        {
                            strncpy(dstPtr, src, dstSize);
                            *((XML_TYPE_PTR*)data->elementBuf) = dstPtr;
                            retVal = dstSize;
                        }
                    }
                    else
                    {
                        *((XML_TYPE_PTR*)data->elementBuf) = NULL;
                        retVal = 0;
                    }
                }
            }
            else
            {
                if ( data->mode != XML_SMODE_BINARY_SIZE )
                {
                    retVal = WS_ERR_INVALID_PARAMETER;
                }
            }
        }

    return retVal;
}

int do_serialize_char_array(XmlSerializationData* data)
{
    wsman_debug (WSMAN_DEBUG_LEVEL_DEBUG, "Serializing char array...");
    WsXmlNodeH child;
    int count = data->elementInfo->funcMaxCount & SER_FLAGS_MASK;
    int retVal = sizeof(XML_TYPE_CHAR) * count;

    if ( data->mode == XML_SMODE_SERIALIZE )
    {
        XML_TYPE_CHAR* tmp = (char*)xml_serializer_alloc(data, retVal + sizeof(XML_TYPE_CHAR), 0);
        if ( tmp == NULL )
            retVal = WS_ERR_INSUFFICIENT_RESOURCES;
        else
        {
            memcpy(tmp, data->elementBuf, retVal);
            tmp[count] = 0;

            if ( xml_serializer_add_child(data, tmp) == NULL ) 
            {
                retVal = WS_ERR_INSUFFICIENT_RESOURCES;
            }
            xml_serializer_free(data, tmp);
        }
    }
    else
        if ( data->mode == XML_SMODE_DESERIALIZE )
        {
            if ( (child = xml_serializer_get_child(data)) == NULL ) 
            {
                retVal = WS_ERR_XML_NODE_NOT_FOUND;
            }
            else
            {
                char* src = ws_xml_get_node_text(child);
                char* dstPtr = (char*)data->elementBuf;
                int dstSize = SER_FLAGS_MASK & data->elementInfo->funcMaxCount;

                if ( XML_IS_PTR(data->elementInfo) )
                {
                    dstPtr = xml_serializer_alloc(data, dstSize, 0);
                    *((XML_TYPE_PTR*)data->elementBuf) = dstPtr;
                }

                if ( dstPtr )
                {
                    if ( src )
                    {
                        int len = strlen(src);

                        if ( len < dstSize )
                            strcpy(dstPtr, src);
                        else
                            memcpy(dstPtr, src, dstSize);
                    }
                    else
                        *dstPtr = 0;
                }
                else
                    retVal = WS_ERR_INSUFFICIENT_RESOURCES;
            }
        }
        else
            if ( data->mode == XML_SMODE_FREE_MEM )
            {
                xml_serializer_free_scalar_mem(data);
            }
            else	
                if ( data->mode != XML_SMODE_BINARY_SIZE )
                {
                    retVal = WS_ERR_INVALID_PARAMETER;
                }

    return retVal;
}


int do_serialize_bool(XmlSerializationData* data)
{
    wsman_debug (WSMAN_DEBUG_LEVEL_DEBUG, "Serializing boolean...");
    int retVal = sizeof(XML_TYPE_BOOL);

    if ( data->mode == XML_SMODE_FREE_MEM )
    {
        xml_serializer_free_scalar_mem(data);
    }
    else	
        if ( data->mode == XML_SMODE_SERIALIZE )
        {
            retVal = do_serialize_uint(data, sizeof(int));
        }
        else
            if ( data->mode == XML_SMODE_DESERIALIZE )
            {
                WsXmlNodeH child; 
                XML_TYPE_PTR dataPtr = make_dst_ptr(data, retVal);

                if (dataPtr == NULL )
                    retVal = WS_ERR_INSUFFICIENT_RESOURCES;
                else
                    if ( (child = xml_serializer_get_child(data)) == NULL ) 
                    {
                        retVal = WS_ERR_XML_NODE_NOT_FOUND;
                    }
                    else
                    {
                        int tmp = -1;
                        char* src = ws_xml_get_node_text(child);

                        if ( src == NULL || *src == 0 )
                        {
                            tmp = 1;
                        }
                        else
                            if ( isdigit(*src) )
                            {
                                tmp = atoi(src);
                            }
                            else
                                if ( strcmp(src, "yes") )
                                {
                                    tmp = 1;
                                }
                                else
                                    if ( strcmp(src, "no") )
                                    {
                                        tmp = 0;
                                    }

                        if ( tmp == 0 || tmp == 1 )
                        {
                            *((int*)dataPtr) = tmp;
                        }
                        else
                            retVal = WS_ERR_XML_PARSING;
                    }
            }
            else
                if ( data->mode != XML_SMODE_BINARY_SIZE )
                {
                    retVal = WS_ERR_INVALID_PARAMETER;
                }

    return retVal;
}

//get_adjusted_size
int get_adjusted_size(int baseSize)
{
    int size = baseSize;
    if ( XML_SADJUSTMENT > 1 )
    {
        if ( (baseSize % XML_SADJUSTMENT) != 0 )
            size = (baseSize / XML_SADJUSTMENT + 1) * XML_SADJUSTMENT;
    }
    return size;
}

// CalculateStrucSize
int calculate_struct_size(XmlSerializationData* data,
        int elementCount, 
        XmlSerializerInfo* elementArray)
{
    int totalSize = 0;
    int i;
    XmlSerializerInfo* savedElement = data->elementInfo;

    for(i = 0; totalSize >= 0 && i < elementCount; i++)
    {
        int j;
        XmlSerializerInfo* curElement = &elementArray[i];
        int max = XML_MAX_OCCURS(curElement);
        for(j = 0; j < max; j++ )
        {
            int elementSize;
            if ( XML_IS_PTR(curElement) )
            {
                elementSize = sizeof(XML_TYPE_PTR);
            }
            else
            {
                data->elementInfo = curElement;
                elementSize = curElement->proc(data);
            }
            if ( elementSize < 0 )
            {
                totalSize = elementSize;
                break;
            }

            if ( XML_SADJUSTMENT > 1 )
                elementSize = get_adjusted_size(elementSize);

            totalSize += elementSize;
        }
    }
    data->elementInfo = savedElement;

    return totalSize;
}

// DoSerializeFixedSizeArray
int do_serialize_fixed_size_array(XmlSerializationData* data)
{
    wsman_debug (WSMAN_DEBUG_LEVEL_DEBUG, "Serializing fixed size array...");
    int retVal;
    int savedIndex = data->index;
    int count;

    if ( data->index < 0 )
    {
        count = -data->index; 
        data->index = 0;
    }
    else
        count = data->elementInfo->funcMaxCount & SER_FLAGS_MASK;

    if ( XML_IS_PTR(data->elementInfo) )
        retVal = sizeof(XML_TYPE_PTR);
    else
    {
        int savedMode = data->mode;
        data->mode = XML_SMODE_BINARY_SIZE;
        retVal = data->elementInfo->proc(data);
        data->mode = savedMode;
    }

    if ( retVal > 0 && count > 0 )
    {
        int adjust = get_adjusted_size(retVal);
        retVal = count * adjust;

        if ( data->mode == XML_SMODE_SERIALIZE 
                || 
                data->mode == XML_SMODE_DESERIALIZE
                || 
                data->mode == XML_SMODE_FREE_MEM )
        {
            char* dstPtr = (char*)data->elementBuf;
            int tmp;

            while(retVal > 0 && count--)
            {
                data->elementBuf = dstPtr;

                if ( (tmp = data->elementInfo->proc(data)) < 0 )
                    retVal = tmp;
                else
                    if ( data->mode == XML_SMODE_FREE_MEM 
                            && 
                            XML_IS_PTR(data->elementInfo) )
                    {
                        data->elementBuf = dstPtr;
                        xml_serializer_free_scalar_mem(data);
                    }

                dstPtr += adjust;
                //WsXmlDumpNodeTree(stdout, WsXmlGetDocRoot(WsXmlGetNodeDoc(data->xmlNode)), 1);

                data->index++;
            }
        }
        else
            if ( data->mode != XML_SMODE_BINARY_SIZE )
            {
                retVal = WS_ERR_INVALID_PARAMETER;
            }
    }

    data->index = savedIndex;

    return retVal;
}




//MakeDynSizeData
XmlSerialiseDynamicSizeData* make_dyn_size_data(XmlSerializationData* data)
{
    XmlSerializerInfo* elementInfo = (XmlSerializerInfo*)data->elementInfo->extData;
    XmlSerialiseDynamicSizeData* dyn = XML_IS_PTR(data->elementInfo) ?
        NULL : (XmlSerialiseDynamicSizeData*)data->elementBuf;

    if ( dyn || (dyn = xml_serializer_alloc(data, sizeof(XmlSerialiseDynamicSizeData), 1)) != NULL ) 
    {
        while( xml_serializer_get_child(data) != NULL ) 
        {
            dyn->count++;
        }
        if ( dyn->count )
        {
            int size;

            data->mode = XML_SMODE_BINARY_SIZE;
            size = elementInfo->proc(data);
            data->mode = XML_SMODE_DESERIALIZE;

            if ( size > 0 )
            {
                size = dyn->count * get_adjusted_size(size);
                if ( (dyn->data = xml_serializer_alloc(data, size, 1)) == NULL )
                {
                    if ( XML_IS_PTR(data->elementInfo) )
                        xml_serializer_free(data, dyn);
                    dyn = NULL;
                }
            }
        }
    }

    return dyn;
}


//DoSerializeDynSizeArray
int do_serialize_dyn_size_array(XmlSerializationData* data)
{
    wsman_debug (WSMAN_DEBUG_LEVEL_DEBUG, "Serializing dyn. size array...");
    int retVal = XML_IS_PTR(data->elementInfo) ? 
        sizeof(XML_TYPE_PTR) : sizeof(XmlSerialiseDynamicSizeData);
    void* savedBufPtr = data->elementBuf;
    XmlSerializerInfo* savedElementInfo = data->elementInfo;
    WsXmlNodeH savedXmlNode = data->xmlNode;
    int savedIndex = data->index;

    if ( data->mode == XML_SMODE_SERIALIZE 
            || 
            data->mode == XML_SMODE_DESERIALIZE
            || 
            data->mode == XML_SMODE_FREE_MEM )
    {
        XmlSerialiseDynamicSizeData* dyn = NULL;

        if ( data->mode == XML_SMODE_DESERIALIZE )
        {
            if ( (dyn = make_dyn_size_data(data)) == NULL )
            {
                retVal = WS_ERR_INSUFFICIENT_RESOURCES;
                // TBD: ??? validation
            }
        }
        else
            if ( XML_IS_PTR(savedElementInfo) )
                dyn = (XmlSerialiseDynamicSizeData*)*((void**)data->elementBuf);
            else
                dyn = (XmlSerialiseDynamicSizeData*)data->elementBuf;

        if ( retVal >= 0 && dyn )
        {
            int tmp;
            data->elementInfo = (XmlSerializerInfo*)savedElementInfo->extData;

            if ( XML_IS_PTR(savedElementInfo) )
                *((void**)data->elementBuf) = dyn;

            data->elementBuf = dyn->data;
            data->index = -dyn->count;

            if ( (tmp = do_serialize_fixed_size_array(data)) < 0 )
                retVal = tmp;
            else
                if ( data->mode == XML_SMODE_FREE_MEM 
                        && 
                        XML_IS_PTR(savedElementInfo) )
                {
                    data->elementInfo = savedElementInfo;

                    data->elementBuf = &dyn->data;
                    xml_serializer_free_scalar_mem(data);

                    data->elementBuf = savedBufPtr;
                    xml_serializer_free_scalar_mem(data);
                }
        }
    }
    else
        if ( data->mode != XML_SMODE_BINARY_SIZE )
        {
            retVal = WS_ERR_INVALID_PARAMETER;
        }

    data->elementBuf = savedBufPtr;
    data->elementInfo = savedElementInfo;
    data->xmlNode = savedXmlNode;
    data->index = savedIndex;
    if ( retVal > 0 )
    {
        char* tmpPtr = (char*)data->elementBuf;
        tmpPtr += get_adjusted_size(retVal);
        data->elementBuf = tmpPtr;
    }
    return retVal;
}

// DoSerializeStruct
int do_serialize_struct(XmlSerializationData* data)
{
    wsman_debug (WSMAN_DEBUG_LEVEL_DEBUG, "Serializing Struct...");
    int i;
    int retVal = 0;
    int elementCount = 0;
    XmlSerializerInfo* elements = 
        (XmlSerializerInfo*)data->elementInfo->extData;
    WsXmlNodeH savedXmlNode = data->xmlNode;

    while(elements[elementCount].proc != NULL)
        elementCount++;

    if ( data->mode == XML_SMODE_BINARY_SIZE )
    {
        retVal = calculate_struct_size(data, elementCount, elements);
    }
    else
    {
        if ( data->mode == XML_SMODE_SERIALIZE 
                || 
                data->mode == XML_SMODE_DESERIALIZE
                || 
                data->mode == XML_SMODE_FREE_MEM 
                ||
                data->mode == XML_SMODE_BINARY_SIZE )
        {
            int savedMode = data->mode;

            data->mode = XML_SMODE_BINARY_SIZE;
            retVal = calculate_struct_size(data, elementCount, elements);
            data->mode = savedMode;

            if ( retVal > 0 )
            {
                if ( data->mode == XML_SMODE_DESERIALIZE )
                {
                    if ( (data->xmlNode = xml_serializer_get_child(data)) == NULL )
                    {
                        retVal = WS_ERR_XML_NODE_NOT_FOUND;
                    }
                    else
                        if ( XML_IS_PTR(data->elementInfo) )
                        {
                            XML_TYPE_PTR ptr = data->elementBuf;
                            if ( (ptr = xml_serializer_alloc(data, retVal, 1)) == NULL )
                                retVal = WS_ERR_INSUFFICIENT_RESOURCES;
                            else
                                *((XML_TYPE_PTR*)data->elementBuf) = ptr;
                        }
                }
                else
                    if ( data->mode == XML_SMODE_SERIALIZE )
                    {
                        if ( (data->xmlNode = xml_serializer_add_child(data, NULL)) == NULL )
                        {
                            retVal = WS_ERR_INSUFFICIENT_RESOURCES;
                        }
                    }

                if ( retVal > 0 )
                {
                    int elementSize;
                    char* dstPtr = (char*)data->elementBuf;
                    XmlSerializerInfo* savedElement = data->elementInfo;
                    int savedIndex = data->index;
                    char* savedName = data->name;

                    data->name = NULL;

                    for(i = 0; retVal > 0 && i < elementCount; i++)
                    {
                        data->elementInfo = &elements[i];
                        data->index = 0;				

                        if ( (data->elementInfo->funcMaxCount & data->skipFlag) )
                        {
                            data->mode = XML_SMODE_BINARY_SIZE;
                        }

                        while(data->index < XML_MAX_OCCURS(data->elementInfo))
                        {
                            data->elementBuf = dstPtr;

                            if ( (elementSize = elements[i].proc(data)) < 0 )
                            {
                                if ( elementSize != WS_ERR_XML_NODE_NOT_FOUND
                                        ||
                                        data->index < XML_MIN_OCCURS(data->elementInfo) )
                                {
                                    retVal = elementSize;
                                    break;
                                }
                                data->mode = XML_SMODE_BINARY_SIZE;
                                if ( (elementSize = elements[i].proc(data)) < 0 )
                                {
                                    retVal = elementSize;
                                    break;
                                }
                            }

                            if ( XML_IS_PTR(&elements[i]) )
                                elementSize = sizeof(XML_TYPE_PTR);

                            dstPtr += get_adjusted_size(elementSize);

                            data->index++;
                        }
                        data->mode = savedMode;

                    }
                    data->elementInfo = savedElement;
                    data->index = savedIndex;
                    data->name = savedName;
                }
            }

            if ( data->mode == XML_SMODE_FREE_MEM 
                    && 
                    XML_IS_PTR(data->elementInfo) )
            {
                if ( xml_serializer_free(data, *((XML_TYPE_PTR*)data->elementBuf)) )
                    *((XML_TYPE_PTR*)data->elementBuf) = NULL;
            }
        }
        else
            retVal = WS_ERR_INVALID_PARAMETER;
    }
    data->xmlNode = savedXmlNode;

    return retVal;
}


//InitializeXmlSerializationData
void initialize_xml_serialization_data(
        XmlSerializationData* data,
        WsContextH cntx,
        XmlSerializerInfo* elementInfo,
        XML_TYPE_PTR dataBuf,
        int mode,
        char* nameNs,
        char* ns,
        WsXmlNodeH xmlNode)
{
    wsman_debug (WSMAN_DEBUG_LEVEL_DEBUG, "Initialize XML Serialization..."); 

    memset(data, 0, sizeof(XmlSerializationData));
    data->cntx = cntx;
    data->elementInfo = elementInfo;
    data->elementBuf = dataBuf;
    data->mode = mode;
    data->ns = ns;
    data->nameNs = nameNs;
    data->xmlNode = xmlNode;

    wsman_debug (WSMAN_DEBUG_LEVEL_DEBUG, "Finished initializing XML Serialization..."); 
    return;
}



// WsSerialize
int ws_serialize(WsContextH cntx, 
        WsXmlNodeH xmlNode, 
        XML_TYPE_PTR dataPtr, 
        XmlSerializerInfo* info,
        char* name,
        char* nameNs,
        char* elementNs,
        int output)
{
    int retVal = WS_ERR_INSUFFICIENT_RESOURCES;
    XmlSerializationData data;
    initialize_xml_serialization_data(&data,
            cntx,
            info, 
            dataPtr, 
            XML_SMODE_SERIALIZE, 
            nameNs,
            elementNs,
            xmlNode);
    data.name = name;
    if ( output )
        data.skipFlag = SER_IN;
    else
        data.skipFlag = SER_OUT;

    wsman_debug (WSMAN_DEBUG_LEVEL_DEBUG, "About to process data...");
    if (info->proc)
        retVal = info->proc(&data);

    return retVal;
}


int ws_serializer_free_mem(WsContextH cntx, XML_TYPE_PTR buf, XmlSerializerInfo* info)
{
    wsman_debug (WSMAN_DEBUG_LEVEL_DEBUG, "Freeing serializer memory...");
    int retVal;
    XmlSerializationData data;
    initialize_xml_serialization_data(&data, 
            cntx,
            info, 
            buf, 
            XML_SMODE_FREE_MEM, 
            NULL, NULL,
            NULL);

    if ( (retVal = info->proc(&data)) >= 0 )
    {
        xml_serializer_free(&data, buf);		
    }

    return retVal;
}


//WsDeSerialize
void* ws_deserialize(WsContextH cntx, 
        WsXmlNodeH xmlParent, 
        XmlSerializerInfo* info,
        char* name,
        char* nameNs,
        char* elementNs,
        int index,
        int output)
{
    int elementCount = 0;
    int size;
    void* retPtr = NULL;
    XmlSerializationData data;
    XmlSerializerInfo* elements = (XmlSerializerInfo*)info->extData;

    initialize_xml_serialization_data(&data, 
            cntx,
            info, 
            NULL, 
            XML_SMODE_BINARY_SIZE,
            nameNs,
            elementNs,
            xmlParent);

    data.index = index;
    data.name = name;

    if ( output )
        data.skipFlag = SER_IN;
    else
        data.skipFlag = SER_OUT;

    while(elements[elementCount].proc != NULL)
        elementCount++;

    if ( (size = calculate_struct_size(&data, elementCount, elements)) > 0 )
    {
        data.mode = XML_SMODE_DESERIALIZE;
        if ( (data.elementBuf = xml_serializer_alloc(&data, size, 1)) != NULL )
        {
            retPtr = data.elementBuf;
            if ( info->proc(&data) <= 0 )
            {
                data.elementBuf = retPtr;
                retPtr = NULL;
                ws_serializer_free_mem(cntx, data.elementBuf, info);
            }
        }
    }
    return retPtr;
}

void enforce_mustunderstand_if_needed(WsContextH cntx, WsXmlNodeH node)
{
    if ( node && ws_get_context_ulong_val(cntx, ENFORCE_MUST_UNDERSTAND) )
    {
        WsXmlDocH doc = ws_xml_get_node_doc(node);
        char* ns = ws_xml_get_node_name_ns(ws_xml_get_doc_root(doc));
        ws_xml_add_node_attr(node, ns, SOAP_MUST_UNDERSTAND, "true");
    }
}

int ws_serialize_str(WsContextH cntx, WsXmlNodeH parent, char* str, 
        char* nameNs, char* name)
{
    WsXmlNodeH node = ws_xml_add_child(parent, nameNs, name, str);
    enforce_mustunderstand_if_needed(cntx, node);
    return (node == NULL);
}


int ws_serialize_uint32(WsContextH cntx, 
        WsXmlNodeH parent, 
        unsigned long val, 
        char* nameNs,
        char* name)
{
    WsXmlNodeH node = ws_xml_add_child(parent, nameNs, name, NULL);
    if ( node )
    {
        ws_xml_set_node_ulong(node, val);
        enforce_mustunderstand_if_needed(cntx, node);
    }

    return (node == NULL);
}


// WsDeSerializeStr
char* ws_deserialize_str(WsContextH cntx, 
        WsXmlNodeH parent, 
        int index, 
        char* nameNs, 
        char* name)
{
    char* str = NULL;
    WsXmlNodeH node = ws_xml_get_child(parent, index, nameNs, name);

    if ( node )
    {
        str = ws_xml_get_node_text(node);
        if ( cntx && str )
        {
            int len = strlen(str) + 1;
            char* tmp = str;
            if ( (str = ws_serializer_alloc(cntx, len * sizeof(char))) )
                strcpy(str, tmp);
        }
    }

    return str;
}



// WsDeSerializeUint32
unsigned long ws_deserialize_uint32(WsContextH cntx, 
        WsXmlNodeH parent, 
        int index,
        char* nameNs, 
        char* name)
{
    unsigned long val = 0;
    WsXmlNodeH node = ws_xml_get_child(parent, index, nameNs, name);

    if ( node )
    {
        val = ws_xml_get_node_ulong(node);
    }

    return val;

}


void* ws_serializer_alloc(WsContextH cntx, int size)
{
    SoapH soap = ws_context_get_runtime(cntx);
    WsSerializerMemEntry* ptr = NULL;

    if ( soap != NULL
            &&
            (ptr = (WsSerializerMemEntry*)soap_alloc(sizeof(WsSerializerMemEntry) + size, 0)) != NULL )
    {
        ptr->cntx = cntx;
        soap_fw_lock(soap);
        if ( DL_MakeNode(&((SOAP_FW*)soap)->WsSerializerAllocList, ptr) == NULL )
        {
            soap_free(ptr);
            ptr = NULL;
        }
        soap_fw_unlock(soap);
    }

    return ptr->buf;
}


// DoSerializerFree
int do_serializer_free(WsContextH cntx, void* ptr)
{
    DL_Node* node = NULL;
    SoapH soap = ws_context_get_runtime(cntx);

    if ( soap && ptr != NULL )
    {
        soap_fw_lock(soap);

        node = DL_GetHead(&((SOAP_FW*)soap)->WsSerializerAllocList);

        while( node != NULL )
        {
            WsSerializerMemEntry* entry = (WsSerializerMemEntry*)node->dataBuf;

            if ( entry && entry->cntx == cntx && (!ptr || ptr == entry->buf) )
            {
                DL_FreeNode(node);
                if ( ptr != NULL )
                    break;
            }

            node = DL_GetNext(node);
        }

        soap_fw_unlock(soap);
    }

    return (node != NULL);
}

// WsSerializerFree
int ws_serializer_free(WsContextH cntx, void* ptr)
{
    int retVal = 0;

    if ( ptr != NULL )
        retVal =  do_serializer_free(cntx, ptr);

    return retVal;
}




// WsSerializerFreeAll
void ws_serializer_free_all(WsContextH cntx)
{
    do_serializer_free(cntx, NULL);
}
