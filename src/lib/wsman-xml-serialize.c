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

#ifdef HAVE_CONFIG_H
#include <wsman_config.h>
#endif

#include "stdlib.h"
#include "string.h"
#include "stdio.h"

#include <ctype.h>
#include "assert.h"


#include "u/libu.h"

#include "wsman-xml-api.h"
#include "wsman-soap.h"
#include "wsman-xml.h"

#include "wsman-dispatcher.h"
#include "wsman-errors.h"
#include "wsman-xml-serializer.h"
#include "wsman-xml-serialize.h"
#include "wsman-soap-envelope.h"

NameAliase* g_NameNameAliaseTable;

struct __WsSerializerMemEntry
{
    WsContextH cntx;
    char buf[1];
};
typedef struct __WsSerializerMemEntry WsSerializerMemEntry;


struct __dummy
{
    char __x;
};

#define XML_SADJUSTMENT     (sizeof(struct __dummy))




void* 
xml_serializer_alloc(XmlSerializationData* data, int size, int zeroInit)
{
    void* ptr = ws_serializer_alloc(data->cntx, size);
	TRACE_ENTER;
    if ( ptr && zeroInit )
        memset(ptr, 0, size);
    TRACE_EXIT;
    return ptr;
}


int 
xml_serializer_free(XmlSerializationData* data, void* buf)
{
    return ws_serializer_free(data->cntx, buf);
}

#if 0
static XML_TYPE_PTR 
make_dst_ptr(XmlSerializationData* data, int size)
{
    void* ptr = data->elementBuf;
    TRACE_ENTER;
    if ( XML_IS_PTR(data->elementInfo) ) {
        if ( data->mode == XML_SMODE_DESERIALIZE ) {
            ptr = xml_serializer_alloc(data, size, 0);
            *((XML_TYPE_PTR*)data->elementBuf) = ptr;
        }
        else
            ptr = *((XML_TYPE_PTR*)data->elementBuf);
    }
    TRACE_EXIT;
    return ptr;
}

#endif

#if 0
void xml_serializer_free_scalar_mem(XmlSerializationData* data)
{
	TRACE_ENTER;
    if ( XML_IS_PTR(data->elementInfo) ) {
        if (xml_serializer_free(data, *((XML_TYPE_PTR*)data->elementBuf)) )
            *((XML_TYPE_PTR*)data->elementBuf) = NULL;
    }
    TRACE_EXIT;
}

#endif

WsXmlNodeH
xml_serializer_add_child(XmlSerializationData* data, char* value)
{
    char* name = (data->name) ? data->name : data->elementInfo->name;
    WsXmlNodeH node;
    TRACE_ENTER;
    debug("data->name = %s; data->elementInfo->name = %s",
                data->name, data->elementInfo->name);
    debug("name = %s; value(%p) = %s", name, value, value);
    node = ws_xml_add_child(data->xmlNode, data->ns, name, value); 
    TRACE_EXIT;
    return node;
}



WsXmlNodeH
xml_serializer_get_child(XmlSerializationData* data)
{
    WsXmlNodeH node;
    char* name = (data->name) ? data->name : data->elementInfo->name;
    TRACE_ENTER;
    debug("name = %s; elname = %s", data->name,
                                    data->elementInfo->name);
    debug("name = %s in %s [%d]", name,
                    ws_xml_get_node_local_name(data->xmlNode),
                    data->index);
    node = ws_xml_get_child(data->xmlNode, data->index, data->ns, name);

    if ( g_NameNameAliaseTable ) {
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
    debug("returned %p; %s",
                node, node?ws_xml_get_node_local_name(node):"");
    TRACE_EXIT;
    return node;
}


static int
do_serialize_uint(XmlSerializationData * data, int valSize)
{
    WsXmlNodeH      child;
    unsigned long   tmp;
    int             retVal = 0;

    TRACE_ENTER;
    debug("handle %d UINT%d %s;", DATA_COUNT(data),
        8 * valSize, data->elementInfo->name);
    if (data->mode == XML_SMODE_FREE_MEM) {
        goto DONE;
    }
    if ((data->mode != XML_SMODE_DESERIALIZE &&
             data->mode != XML_SMODE_SERIALIZE)) {
        retVal = WS_ERR_INVALID_PARAMETER;
        goto DONE;
    }

    for (data->index = 0; data->index < DATA_COUNT(data); data->index++) {
         debug("%s[%d] = %p", data->elementInfo->name,
                            data->index, data->elementBuf);

        if (data->mode == XML_SMODE_SERIALIZE) {
        debug("value size: %d", valSize);
            if (valSize == 1)
                tmp = *((XML_TYPE_UINT8 *)data->elementBuf);
            else if (valSize == 2)
                tmp = *((XML_TYPE_UINT16 *)data->elementBuf);
            else if (valSize == 4)
                tmp = *((XML_TYPE_UINT32 *)data->elementBuf);
            else {
                retVal = WS_ERR_INVALID_PARAMETER;
                goto DONE;
            }

            if (((child = xml_serializer_add_child(data, NULL)) == NULL) ||
                            (ws_xml_set_node_ulong(child, tmp)) != 0) {
                retVal = WS_ERR_INSUFFICIENT_RESOURCES;
                goto DONE;
            }
        }
        if (data->mode == XML_SMODE_DESERIALIZE) {
             char  *str;

            if ((child = xml_serializer_get_child(data)) == NULL) {
                // just move data->elementBuf
                data->elementBuf = (char *)data->elementBuf + DATA_SIZE(data);
                continue;
            }
            if ((str = ws_xml_get_node_text(child)) == NULL) {
                retVal = WS_ERR_XML_PARSING;
                goto DONE;
            }
            tmp = strtoul(str, NULL, 10);
            /*
            * TBD:		validate that value doesn 't exceed
            * size
            */
            if (valSize == 1) {
                *((XML_TYPE_UINT8 *)data->elementBuf) = (XML_TYPE_UINT8)tmp;
            } else if (valSize == 2) {
                *((XML_TYPE_UINT16 *)data->elementBuf) = (XML_TYPE_UINT16)tmp;
            } else if (valSize == 4) {
                *((XML_TYPE_UINT32 *)data->elementBuf) = (XML_TYPE_UINT32)tmp;
            } else {
                retVal = WS_ERR_INVALID_PARAMETER;
                goto DONE;
            }
        }
        data->elementBuf = (char *)data->elementBuf + DATA_SIZE(data);
    }

DONE:
    TRACE_EXIT;
    return retVal;
}





int 
do_serialize_uint8(XmlSerializationData* data)
{
    int retVal = DATA_ALL_SIZE(data);
    if ((char *)data->elementBuf + retVal > data->stopper) {
        return WS_ERR_INVALID_PARAMETER;
    }
    if (DATA_MUST_BE_SKIPPED(data)) {
        data->elementBuf = (char *)data->elementBuf + retVal;
        return retVal;
    }
    int tmp = do_serialize_uint(data, sizeof(XML_TYPE_UINT8));
    if (tmp < 0) {
        retVal = tmp;
    }
    return retVal;
}


int 
do_serialize_uint16(XmlSerializationData* data)
{
    typedef struct {
        XML_TYPE_UINT8 a;
        XML_TYPE_UINT16 b;
    } dummy;
    size_t al = (char *)&(((dummy *)NULL)->b) - (char *)NULL;
    size_t pad = (unsigned long)data->elementBuf % al;

    if (pad) {
        pad = al - pad;
    }
    int retVal = pad + DATA_ALL_SIZE(data);
    if ((char *)data->elementBuf + retVal > data->stopper) {
        return WS_ERR_INVALID_PARAMETER;
    }
    if (DATA_MUST_BE_SKIPPED(data)) {
        data->elementBuf = (char *)data->elementBuf + retVal;
        return retVal;
    }
    data->elementBuf = (char *)data->elementBuf + pad;
    int tmp = do_serialize_uint(data, sizeof(XML_TYPE_UINT16));
    if (tmp < 0) {
        retVal = tmp;
    }
    return retVal;
}

int 
do_serialize_uint32(XmlSerializationData* data)
{
   typedef struct {
        XML_TYPE_UINT8 a;
        XML_TYPE_UINT32 b;
    } dummy;
    size_t al = (char *)&(((dummy *)NULL)->b) - (char *)NULL;
    size_t pad = (unsigned long)data->elementBuf % al;
    if (pad) {
        pad = al - pad;
    }
    int retVal = pad + DATA_ALL_SIZE(data);
    if ((char *)data->elementBuf + retVal > data->stopper) {
        return WS_ERR_INVALID_PARAMETER;
    }
    if (DATA_MUST_BE_SKIPPED(data)) {
        data->elementBuf = (char *)data->elementBuf + retVal;
        return retVal;
    }
    data->elementBuf = (char *)data->elementBuf + pad;
    int tmp = do_serialize_uint(data, sizeof(XML_TYPE_UINT32));
    if (retVal < 0) {
        retVal = tmp;
    }
    return retVal;
}

#if 0
static int
ws_serilize_string_array(XmlSerializationData * data)
{
    char **Ptr = *((char ***) data->elementBuf);
    char **p;
    int i, count;
    int retVal = sizeof(XML_TYPE_STR);
 //   WsXmlNodeH      savedXmlNode = data->xmlNode;
    char *savedName = data->name;
    TRACE_ENTER;


    p = Ptr;
    while (*p != NULL) p++;
    count = p - Ptr;
/*
    if ((data->xmlNode = xml_serializer_add_child(data, NULL)) == NULL) {
        retVal = WS_ERR_INSUFFICIENT_RESOURCES;
        goto DONE;
    }
*/
    XML_UNSET_PTR(data->elementInfo);
//    data->name = "string";
    for (i = 0; i < count; i++) {
        data->elementBuf = Ptr + i;
        retVal = do_serialize_string(data);
        if (retVal < 0) {
            break;
        }
    }
//DONE:
    XML_SET_PTR(data->elementInfo);
//    data->xmlNode = savedXmlNode;
    data->name = savedName;
    data->elementBuf = Ptr;
    TRACE_EXIT;
    return retVal;
}


static int
ws_deserilize_string_array(XmlSerializationData * data)
{
    int retVal = sizeof(XML_TYPE_PTR);
//    char *savedName = data->name;
//    char *savedElName = data->elementInfo->name;
//    WsXmlNodeH savedNode = data->xmlNode;
    void *savedElBuf = data->elementBuf;
    int savedIndex = data->index;
    int count = 0;
    int i;
    char **p;

//    data->xmlNode = xml_serializer_get_child(data);
    data->index = 0;
//    data->name = NULL;
//    data->elementInfo->name = NULL;
    while (xml_serializer_get_child(data) != NULL) {
       data->index++;
    }
    count = data->index;
    data->index = 0;
    p = xml_serializer_alloc(data, sizeof(XML_TYPE_STR) * (count + 1), 1);
    if (p == NULL) {
        retVal = WS_ERR_INSUFFICIENT_RESOURCES;
        goto DONE;
    }

    XML_UNSET_PTR(data->elementInfo);
    data->index = 0;
    while (data->index < count) {
        data->elementBuf = p + data->index;
        i = do_serialize_string(data);
        if (i < 0) {
            retVal = i;
            goto DONE;
        }
        data->index++;
    }

    *((XML_TYPE_PTR *)savedElBuf) = p;

DONE:
    XML_SET_PTR(data->elementInfo);
//    data->name = savedName;
 //   data->elementInfo->name = savedElName;
//    data->xmlNode = savedNode;
    data->elementBuf = savedElBuf;
    data->index = savedIndex;
    TRACE_EXIT;
    return retVal;
}

#endif

/* We will need special function DoSerializeArrayOfStringPtrs() */
int
do_serialize_string(XmlSerializationData * data)
{
    WsXmlNodeH      child;
    int             retVal = DATA_ALL_SIZE(data);

    typedef struct {
        XML_TYPE_UINT8 a;
        XML_TYPE_STR b;
    } dummy;

    TRACE_ENTER;
    debug("handle %d strings %s = %p", DATA_COUNT(data),
                    data->elementInfo->name, data->elementBuf);
    size_t al = (char *)&(((dummy *)NULL)->b) - (char *)NULL;
    size_t pad = (unsigned long)data->elementBuf % al;
    if (pad) {
        pad = al - pad;
    }
    retVal += pad;
    if ((char *)data->elementBuf + retVal > data->stopper) {
        return WS_ERR_INVALID_PARAMETER;
    }
    if (DATA_MUST_BE_SKIPPED(data)) {
        data->elementBuf = (char *)data->elementBuf + retVal;
        return retVal;
    }
    data->elementBuf = (char *)data->elementBuf + pad;
    debug("adjusted elementBuf = %p", data->elementBuf);

#if 0
    if (XML_IS_PTR(data->elementInfo)) {
        if (data->mode == XML_SMODE_SERIALIZE) {
            retVal = ws_serilize_string_array(data);
        } else if (data->mode == XML_SMODE_DESERIALIZE) {
            retVal = ws_deserilize_string_array(data);
        } else {
            retVal = WS_ERR_INVALID_PARAMETER;
        }
        goto DONE;
    }
#endif
    for (data->index = 0; data->index < DATA_COUNT(data); data->index++) {
        if (data->mode == XML_SMODE_FREE_MEM) {
             xml_serializer_free(data, data->elementBuf);
            *((XML_TYPE_STR *)data->elementBuf) = NULL;
        } else if (data->mode == XML_SMODE_SERIALIZE) {
            char *valPtr = *((char **)data->elementBuf);
            child = xml_serializer_add_child(data, valPtr);
            if (child == NULL) {
                debug("xml_serializer_add_child failed.");
                retVal = WS_ERR_INSUFFICIENT_RESOURCES;
                goto DONE;
            }
            if (ws_xml_get_node_text(child) == NULL) {
                ws_xml_add_node_attr(child, 
                     XML_NS_SCHEMA_INSTANCE, XML_SCHEMA_NIL, "true");
            }
        } else if (data->mode == XML_SMODE_DESERIALIZE) {
            if ((child = xml_serializer_get_child(data)) == NULL) {
                // no such node. just move pointer
                data->elementBuf = (char *)data->elementBuf + DATA_SIZE(data);
                continue;
            }

            char *src = ws_xml_get_node_text(child);
            if (src != NULL || *src != 0) {
                char   *dstPtr;
                int    dstSize = 1 + strlen(src);
                dstPtr = (char *)xml_serializer_alloc(data, dstSize, 1);
                if (dstPtr == NULL) {
                    retVal = WS_ERR_INSUFFICIENT_RESOURCES;
                    goto DONE;
                }
                strncpy(dstPtr, src, dstSize);
                *((XML_TYPE_PTR *)data->elementBuf) = dstPtr;
            }
	    } else {
            retVal = WS_ERR_INVALID_PARAMETER;
            goto DONE;
        }
        data->elementBuf = (char *)data->elementBuf + DATA_SIZE(data);
    }

DONE:
    TRACE_EXIT;
    return retVal;
}

#if 0
int 
do_serialize_char_array(XmlSerializationData* data)
{
    WsXmlNodeH child; 
    int count = XML_MAX_OCCURS(data->elementInfo);
    int retVal = sizeof(XML_TYPE_CHAR) * count;
    TRACE_ENTER;
    if ( data->mode == XML_SMODE_SERIALIZE ) {
        XML_TYPE_CHAR* tmp = (char*)xml_serializer_alloc(
                     data, retVal + sizeof(XML_TYPE_CHAR), 0);
        if ( tmp == NULL )
            retVal = WS_ERR_INSUFFICIENT_RESOURCES;
        else {
            memcpy(tmp, data->elementBuf, retVal);
            tmp[count] = 0;

            if ( xml_serializer_add_child(data, tmp) == NULL ) 
            {
                retVal = WS_ERR_INSUFFICIENT_RESOURCES;
            }
            xml_serializer_free(data, tmp);
        }
    } else if ( data->mode == XML_SMODE_DESERIALIZE ) {
        if ( (child = xml_serializer_get_child(data)) == NULL ) {
            retVal = WS_ERR_XML_NODE_NOT_FOUND;
        } else {
            char* src = ws_xml_get_node_text(child);
            char* dstPtr = (char*)data->elementBuf;
            int dstSize = XML_MAX_OCCURS(data->elementInfo);

            if ( XML_IS_PTR(data->elementInfo) )
            {
                dstPtr = xml_serializer_alloc(data, dstSize, 0);
                *((XML_TYPE_PTR*)data->elementBuf) = dstPtr;
            }

            if ( dstPtr ) {
                if ( src ) {
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
    } else if ( data->mode == XML_SMODE_FREE_MEM ) {
            xml_serializer_free_scalar_mem(data);
    } else if ( data->mode != XML_SMODE_BINARY_SIZE ) {
            retVal = WS_ERR_INVALID_PARAMETER;
    }
    TRACE_EXIT;
    return retVal;
}

#endif


int
do_serialize_bool(XmlSerializationData * data)
{
    int             retVal = DATA_ALL_SIZE(data);
    typedef struct {
        XML_TYPE_UINT8 a;
        XML_TYPE_BOOL b;
    } dummy;

    TRACE_ENTER;
    debug("handle %d booleans %s; ptr = %p", DATA_COUNT(data),
                    data->elementInfo->name, data->elementBuf);
    size_t al = (char *)&(((dummy *)NULL)->b) - (char *)NULL;
    size_t pad = (unsigned long)data->elementBuf % al;
    if (pad) {
        pad = al - pad;
    }
    retVal += pad;
    if ((char *)data->elementBuf + retVal > data->stopper) {
        return WS_ERR_INVALID_PARAMETER;
    }
    if (DATA_MUST_BE_SKIPPED(data) ||
        data->mode == XML_SMODE_FREE_MEM) {
        data->elementBuf = (char *)data->elementBuf + retVal;
        return retVal;
    }
    data->elementBuf = (char *)data->elementBuf + pad;
    debug("adjusted elementBuf = %p", data->elementBuf);

    for (data->index = 0; data->index < DATA_COUNT(data); data->index++) {
        debug("%s[%d] = %p", data->elementInfo->name, data->index,
                        data->elementBuf); 
        if (data->mode == XML_SMODE_SERIALIZE) {
            WsXmlNodeH      child;
            XML_TYPE_BOOL   tmp;

            tmp = *((XML_TYPE_BOOL *) data->elementBuf);	
            if ((child = xml_serializer_add_child(data,
                         (tmp==0) ? "false" : "true")) == NULL) {
                retVal = WS_ERR_INSUFFICIENT_RESOURCES;
                goto DONE;
            }
        } else if (data->mode == XML_SMODE_DESERIALIZE) {
            WsXmlNodeH      child;
            XML_TYPE_PTR    dataPtr = data->elementBuf;

            if ((child = xml_serializer_get_child(data)) == NULL) {
                // no data. default is false
                data->elementBuf = (char *)data->elementBuf + DATA_SIZE(data);
                continue;
            }
            XML_TYPE_BOOL  tmp = -1;
            char           *src = ws_xml_get_node_text(child);
            if (src == NULL || *src == 0) {
                tmp = 1;
            } else {
                if (isdigit(*src)) {
                    tmp = atoi(src);
                } else {
                    if (strcmp(src, "yes") == 0 || strcmp(src, "true") == 0) {
                        tmp = 1;
                    } else if (strcmp(src, "no") == 0 ||
                                        strcmp(src, "false") == 0) {
                        tmp = 0;
                    }
                }
            }

            if (tmp == 0 || tmp == 1) {
                *((XML_TYPE_BOOL *) dataPtr) = tmp;
            } else {
                retVal = WS_ERR_XML_PARSING;
                goto DONE;
            }
        } else {
            retVal = WS_ERR_INVALID_PARAMETER;
            goto DONE;
        }
        data->elementBuf = (char *)data->elementBuf + DATA_SIZE(data);
    }
DONE:
    TRACE_EXIT;
    return retVal;
}

#if 0
static int 
get_adjusted_size(int baseSize)
{
    int size = baseSize;
    if ( XML_SADJUSTMENT > 1 )
    {
        if ( (baseSize % XML_SADJUSTMENT) != 0 )
            size = (baseSize / XML_SADJUSTMENT + 1) * XML_SADJUSTMENT;
    }
    return size;
}

#endif
#if 0

static int 
calculate_struct_size(XmlSerializationData* data,
	        		  int elementCount, 
        			  XmlSerializerInfo* elementArray)
{
    return DATA_SIZE(data);
    int totalSize = 0;
    int i;
    XmlSerializerInfo* savedElement = data->elementInfo;
    void *savedElementBuf = data->elementBuf;
	TRACE_ENTER;
    for(i = 0; totalSize >= 0 && i < elementCount; i++) {
        int j;
        XmlSerializerInfo* curElement = &elementArray[i];
        int max = XML_MAX_OCCURS(curElement);
        for(j = 0; j < max; j++ ) {
            int elementSize;
            if ( XML_IS_PTR(curElement) ) {
                elementSize = sizeof(XML_TYPE_PTR);
            } else {
                data->elementInfo = curElement;
                elementSize = curElement->proc(data);
            }
            if ( elementSize < 0 ) {
                totalSize = elementSize;
                break;
            }

            if ( XML_SADJUSTMENT > 1 )
                elementSize = get_adjusted_size(elementSize);

            totalSize += elementSize;
        }
    }
    data->elementInfo = savedElement;
    data->elementBuf = savedElementBuf;
	TRACE_EXIT;
    return totalSize;
}
#endif

#if 0
static int
ws_serialize_fixed_size_array(
            XmlSerializationData* data,
            char *name,
            int count)
{
    int retVal;
    TRACE_ENTER;
    debug("name = %s, count = %d, index %d", name, count, data->index);

    retVal = 0;

    if (count == 0) {
        debug("count == 0");
        goto DONE;
    }
    debug("count = %d; name = <%s>; elname = <%s>",
                    count, data->name, data->elementInfo->name);

    char *savedElName = data->elementInfo->name;
    data->elementInfo->name = name;
    int savedIndex = data->index;
    size_t savedCount = DATA_COUNT(data);

    DATA_SET_COUNT(data, count);
    retVal = data->elementInfo->proc(data);
    DATA_SET_COUNT(data, savedCount);
    data->index = savedIndex;
    data->elementInfo->name = savedElName;

DONE:
    debug("retVal %d; index = %d", retVal, data->index);
	TRACE_EXIT;
    return retVal;
}

#endif


XmlSerialiseDynamicSizeData* 
make_dyn_size_data(XmlSerializationData* data)
{
    XmlSerialiseDynamicSizeData* dyn =
                (XmlSerialiseDynamicSizeData*)data->elementBuf;
    TRACE_ENTER;

    debug("name = <%s>, elname = <%s>",
            data->name, data->elementInfo->name);
    int savedIndex = data->index;
    data->index = 0;
    while (xml_serializer_get_child(data) != NULL ) {
        data->index++;
    }
    dyn->count = data->index;
    data->index = savedIndex;

    debug("count = %d", dyn->count);
    if (dyn->count == 0) {
        goto DONE;
    }

    int size = DATA_SIZE(data) * dyn->count;
    dyn->data = xml_serializer_alloc(data, size, 1);
    if (dyn->data == NULL) {
        dyn = NULL;
    }

DONE:
	TRACE_EXIT;
    return dyn;
}


int
do_serialize_dyn_size_array(XmlSerializationData * data)
{
    typedef struct {
        char a;
        XmlSerialiseDynamicSizeData b;
    } dummy;
    size_t al = (char *)&(((dummy *)NULL)->b) - (char *)NULL;
    size_t pad = (unsigned long)data->elementBuf % al;
    TRACE_ENTER;
    debug("Dyn size array %s; ptr = %p", data->elementInfo->name,
                        data->elementBuf);

    if (pad) {
        pad = al - pad;
    }
    int  retVal = sizeof(XmlSerialiseDynamicSizeData) + pad;
    if ((char *)data->elementBuf + retVal > data->stopper) {
        return WS_ERR_INVALID_PARAMETER;
    }
    if (DATA_MUST_BE_SKIPPED(data)) {
        data->elementBuf = (char *)data->elementBuf + retVal;
        return retVal;
    }
    data->elementBuf = (char *)data->elementBuf + pad;
    debug("adjusted elementBuf = %p", data->elementBuf);

    void *savedBufPtr = data->elementBuf;
    XmlSerializerInfo *savedElementInfo = data->elementInfo;

    if (data->mode != XML_SMODE_SERIALIZE &&
               data->mode != XML_SMODE_DESERIALIZE &&
               data->mode != XML_SMODE_FREE_MEM) {
        retVal = WS_ERR_INVALID_PARAMETER;
        goto DONE;
    }

    XmlSerialiseDynamicSizeData *dyn = NULL;

    if (data->mode == XML_SMODE_DESERIALIZE) {
        if ((dyn = make_dyn_size_data(data)) == NULL) {
            retVal = WS_ERR_INSUFFICIENT_RESOURCES;
                //TBD:      ? ? ? validation
            goto DONE;
        }
    } else {
        dyn = (XmlSerialiseDynamicSizeData *) data->elementBuf;
    }

    if ((data->mode == XML_SMODE_FREE_MEM) && dyn->data) {
        xml_serializer_free(data, dyn->data);
        goto DONE;
    }

    if (dyn->count == 0) {
        // no dynamic data. nothing to do
        goto DONE;
    }

    int   tmp;
    XmlSerializerInfo myinfo;
    int savedIndex = data->index;
    char *savedStopper = data->stopper;
    memcpy(&myinfo, savedElementInfo->extData, sizeof(XmlSerializerInfo));
    myinfo.count = dyn->count;
    myinfo.name = data->elementInfo->name;

    data->stopper = (char *)dyn->data + DATA_SIZE(data) * dyn->count;
    data->elementInfo = &myinfo;
    data->elementBuf = dyn->data;
    data->index = 0;

    debug("dyn = %p, dyn->data = %p", dyn, dyn->data);
    tmp = data->elementInfo->proc(data);

    data->index = savedIndex;
    data->elementBuf = savedBufPtr;
    data->elementInfo = savedElementInfo;
    data->stopper = savedStopper;
    if (tmp < 0) {
        dyn->count = 0;
        retVal = tmp;
        goto DONE;
    }
DONE:
    data->elementBuf = (char *)data->elementBuf +
                sizeof (XmlSerialiseDynamicSizeData);
    TRACE_EXIT;
    return retVal;
}




int
do_serialize_struct(XmlSerializationData * data)
{
    int retVal = 0;
    int             i;
    int             elementCount = 0;
    XmlSerializerInfo *elements =
               (XmlSerializerInfo *) data->elementInfo->extData;
    WsXmlNodeH      savedXmlNode = data->xmlNode;
    XmlSerializerInfo *savedElement = data->elementInfo;
    int             savedMode = data->mode;
    int savedIndex = data->index;
    void *savedStopper = data->stopper;
    TRACE_ENTER;
    struct dm {
        void *a;
        short b;
    };
    typedef struct {
        char a;
        struct dm b;
    } dummy;

    TRACE_ENTER;
    debug("handle %d structure %s ptr = %p", DATA_COUNT(data),
                    data->elementInfo->name, data->elementBuf);
    if (data->mode != XML_SMODE_SERIALIZE &&
        data->mode != XML_SMODE_DESERIALIZE &&
        data->mode != XML_SMODE_FREE_MEM) {
            retVal = WS_ERR_INVALID_PARAMETER;
            debug("Incorrect data->mode = %d", data->mode);
            goto DONE;
    }
    size_t al = (char *)&(((dummy *)NULL)->b) - (char *)NULL;
    size_t pad = (unsigned long)data->elementBuf % al;
    if (pad) {
        pad = al - pad;
    }
    retVal = pad + XML_IS_HEAD(savedElement) ?
                           DATA_SIZE(data) :DATA_ALL_SIZE(data);
    if ((char *)data->elementBuf + retVal > data->stopper) {
        return WS_ERR_INVALID_PARAMETER;
    }
    if (DATA_MUST_BE_SKIPPED(data)) {
        data->elementBuf = (char *)data->elementBuf + retVal;
        return retVal;
    }
    data->elementBuf = (char *)data->elementBuf + pad;
    debug("adjusted ptr= %p", data->elementBuf);

    while (elements[elementCount].proc != NULL) {
        elementCount++;
    }

    size_t count = DATA_COUNT(data);
    size_t struct_size = DATA_SIZE(data);
    if (XML_IS_HEAD(savedElement)) {
        count = data->index + 1;
    } else {
        data->index = 0;
    }

    int savedLocalIndex;
    char *savedLocalElementBuf;
    for (; data->index < count; data->index++) {
        savedLocalIndex = data->index;
        savedLocalElementBuf = (char *)data->elementBuf;
        data->stopper = savedLocalElementBuf + DATA_SIZE(data);
        debug("%s[%d] = %p", data->elementInfo->name,
                data->index, data->elementBuf);
        if (data->mode == XML_SMODE_SERIALIZE) {
            data->xmlNode = xml_serializer_add_child(data, NULL);
            if (data->xmlNode == NULL) {
                retVal = WS_ERR_INSUFFICIENT_RESOURCES;
                goto DONE;
            }
        } else {
            if ((data->xmlNode = xml_serializer_get_child(data)) == NULL) {
                if (XML_IS_HEAD(savedElement)) {
                    // first structure must be
                    retVal = WS_ERR_INVALID_PARAMETER;
                    goto DONE;
                }
                // no data, skip
                goto CONTINUE;
            }
        }

        int tmp = 0;

        debug("before for loop. Struct %s = %p",
                        savedElement->name, data->elementBuf);

        for (i = 0; retVal > 0 && i < elementCount; i++) {
            data->elementInfo = &elements[i];
            debug("handle %d elements %s of struct %s",
                DATA_COUNT(data), data->elementInfo->name,
                savedElement->name);
            if (XML_IS_SKIP(data->elementInfo)) {
                data->mode = XML_SMODE_SKIP;
            }

            debug("handle %s element; dstPtr = %p",
                    data->elementInfo->name, data->elementBuf);
            tmp = data->elementInfo->proc(data);

            if (tmp < 0) {
                retVal = tmp;
                goto DONE;
            }
        }
CONTINUE:
        data->elementInfo = savedElement;
        data->elementBuf = savedLocalElementBuf + struct_size;
        data->index = savedLocalIndex;
        data->mode = savedMode;
        data->xmlNode = savedXmlNode;
    }
    debug("after for loop");
DONE:
    data->stopper = savedStopper;
    data->elementInfo = savedElement;
    data->index = savedIndex;
    data->mode = savedMode;
    data->xmlNode = savedXmlNode;
    TRACE_EXIT;
    return retVal;
}



static void 
initialize_xml_serialization_data(
        XmlSerializationData* data,
        WsContextH cntx,
        XmlSerializerInfo* elementInfo,
        XML_TYPE_PTR dataBuf,
        int mode,
        char* nameNs,
        char* ns,
        WsXmlNodeH xmlNode)
{
    debug( "Initialize XML Serialization..."); 
    TRACE_ENTER;
    memset(data, 0, sizeof(XmlSerializationData));
    data->cntx = cntx;
    data->elementInfo = elementInfo;
    data->elementBuf = dataBuf;
    data->mode = mode;
    data->ns = ns;
    data->nameNs = nameNs;
    data->xmlNode = xmlNode;

    debug( "Finished initializing XML Serialization..."); 
    TRACE_EXIT;
    return;
}



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
    XmlSerializerInfo myinfo;
    XmlSerializationData data;

    TRACE_ENTER;
    if (info->proc == NULL) {
        debug("info->proc == NULL");
        goto DONE;
    }
    memcpy(&myinfo, info, sizeof (XmlSerializerInfo));
    if (name) {
        myinfo.name = name;
    }
    myinfo.flags |= SER_HEAD;
    initialize_xml_serialization_data(&data,
            cntx,
            &myinfo,
            dataPtr,
            XML_SMODE_SERIALIZE, 
            nameNs,
            elementNs,
            xmlNode);

    data.stopper = (char *)dataPtr + myinfo.size;
    if (output) {
        data.skipFlag = SER_IN;
    } else {
        data.skipFlag = SER_OUT;
    }
    retVal = myinfo.proc(&data);

DONE:
    TRACE_EXIT;
    return retVal;
}


int ws_serializer_free_mem(WsContextH cntx, XML_TYPE_PTR buf, XmlSerializerInfo* info)
{
    int retVal;
    XmlSerializationData data;
    TRACE_ENTER;
    initialize_xml_serialization_data(&data, 
            cntx,
            info, 
            buf, 
            XML_SMODE_FREE_MEM, 
            NULL, NULL,
            NULL);

    if ( (retVal = info->proc(&data)) >= 0 ) {
        xml_serializer_free(&data, buf);		
    }
    TRACE_EXIT;
    return retVal;
}


void*
ws_deserialize(WsContextH cntx,
               WsXmlNodeH xmlParent,
               XmlSerializerInfo* info,
               char* name,
               char* nameNs,
               char* elementNs,
               int index,
               int output)
{
    int size;
    void* retPtr = NULL;
    XmlSerializationData data;
    XmlSerializerInfo myinfo;

    TRACE_ENTER;
    memcpy(&myinfo, info, sizeof (XmlSerializerInfo));
    if (name) {
        myinfo.name = name;
    }
    myinfo.flags |= SER_HEAD;
    initialize_xml_serialization_data(&data, cntx,  &myinfo, NULL,
                         XML_SMODE_BINARY_SIZE, nameNs, elementNs, xmlParent);

    data.index = index;

    if (output) {
        data.skipFlag = SER_IN;
    } else {
        data.skipFlag = SER_OUT;
    }

    size = myinfo.size;
    data.mode = XML_SMODE_DESERIALIZE;
    if ((data.elementBuf = xml_serializer_alloc(&data, size, 1)) != NULL ) {
        retPtr = data.elementBuf;
        data.stopper = (char *)retPtr + size;
        if (myinfo.proc(&data) <= 0) {
            data.elementBuf = retPtr;
            retPtr = NULL;
            ws_serializer_free_mem(cntx, data.elementBuf, &myinfo);
        }
    }
    TRACE_EXIT;
    return retPtr;
}

void enforce_mustunderstand_if_needed(WsContextH cntx, WsXmlNodeH node)
{
    if ( node && ws_get_context_ulong_val(cntx, ENFORCE_MUST_UNDERSTAND) ) {
        WsXmlDocH doc = ws_xml_get_node_doc(node);
        char* ns = ws_xml_get_node_name_ns(ws_xml_get_doc_root(doc));
        ws_xml_add_node_attr(node, ns, SOAP_MUST_UNDERSTAND, "true");
    }
}

int
ws_serialize_str(WsContextH cntx, WsXmlNodeH parent, char* str, 
        char* nameNs, char* name)
{
    WsXmlNodeH node;
    TRACE_ENTER;
    node = ws_xml_add_child(parent, nameNs, name, str);
    enforce_mustunderstand_if_needed(cntx, node);
    TRACE_EXIT;
    return (node == NULL);
}


int ws_serialize_uint32(WsContextH cntx, WsXmlNodeH parent, unsigned long val, 
        char* nameNs, char* name)
{
    WsXmlNodeH node = ws_xml_add_child(parent, nameNs, name, NULL);
    TRACE_ENTER;
    if ( node ) {
        ws_xml_set_node_ulong(node, val);
        enforce_mustunderstand_if_needed(cntx, node);
    }
	TRACE_EXIT;
    return (node == NULL);
}


char* ws_deserialize_str(WsContextH cntx, WsXmlNodeH parent, int index, 
        char* nameNs, char* name)
{
    char* str = NULL;
    WsXmlNodeH node = ws_xml_get_child(parent, index, nameNs, name);
    TRACE_ENTER;
    if ( node ) {
        str = ws_xml_get_node_text(node);
        if ( cntx && str ) {
            int len = strlen(str) + 1;
            char* tmp = str;
            if ( (str = ws_serializer_alloc(cntx, len * sizeof(char))) )
                strcpy(str, tmp);
        }
    }
    TRACE_EXIT;
    return str;
}

unsigned long ws_deserialize_uint32(WsContextH cntx, 
        WsXmlNodeH parent, int index, char* nameNs, char* name)
{
    unsigned long val = 0;
    WsXmlNodeH node = ws_xml_get_child(parent, index, nameNs, name);
    TRACE_ENTER;
    if ( node ) {
        val = ws_xml_get_node_ulong(node);
    }
    TRACE_EXIT;
    return val;
}


void* ws_serializer_alloc(WsContextH cntx, int size)
{
    SoapH soap = ws_context_get_runtime(cntx);
    WsSerializerMemEntry* ptr = NULL;
    TRACE_ENTER;
    if (soap != NULL &&
            (ptr = (WsSerializerMemEntry*)u_malloc(
                sizeof(WsSerializerMemEntry) + size)) != NULL) {
        lnode_t *node;
        ptr->cntx = cntx;
        u_lock(soap);
        if ( ( node = lnode_create(ptr)) == NULL ) {
            u_free(ptr);
            ptr = NULL;
        } else {
            list_append(soap->WsSerializerAllocList, node );
        }
        u_unlock(soap);
    }
    TRACE_EXIT;
    return ptr->buf;
}


static int 
do_serializer_free(WsContextH cntx,
                   void* ptr)
{
    lnode_t* node = NULL;
    SoapH soap = ws_context_get_runtime(cntx);
    TRACE_ENTER;
    if (soap && ptr != NULL) {
        u_lock(soap);
        node = list_first(soap->WsSerializerAllocList);
        while (node != NULL) {
            WsSerializerMemEntry* entry = (WsSerializerMemEntry*)node->list_data;

            if (entry && entry->cntx == cntx && (!ptr || ptr == entry->buf)) {
                // FIXME
                //lnode_destroy (node);
                //list_delete(soap->WsSerializerAllocList, node);
                if (ptr != NULL) {
                    break;
                }
            }
            node = list_next(soap->WsSerializerAllocList, node);
        }
        // list_destroy(((env_t*)soap)->WsSerializerAllocList);
        u_unlock(soap);
    }
    TRACE_EXIT;
    return (node != NULL);
}

int
ws_serializer_free( WsContextH cntx, 
                    void* ptr)
{
    int retVal = 0;
    TRACE_ENTER;
    if ( ptr != NULL )
        retVal =  do_serializer_free(cntx, ptr);
    TRACE_EXIT;
    return retVal;
}

void
ws_serializer_free_all(WsContextH cntx)
{
    TRACE_ENTER;
    do_serializer_free(cntx, NULL);
    TRACE_EXIT;
}
