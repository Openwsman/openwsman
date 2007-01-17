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
 * @author Vadim Revyakin
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


WsXmlNodeH
xml_serializer_add_child(XmlSerializationData* data, char* value)
{
    char* name = (data->name) ? data->name : data->elementInfo->name;
    char *ns   = (data->elementInfo->ns) ? data->elementInfo->ns : data->ns;
    WsXmlNodeH node;
    TRACE_ENTER;
    debug("data->name = %s; data->elementInfo->name = %s",
                data->name, data->elementInfo->name);
    debug("name = %s; value(%p) = %s", name, value, value);
    node = ws_xml_add_child(data->xmlNode, ns, name, value); 
    TRACE_EXIT;
    return node;
}



WsXmlNodeH
xml_serializer_get_child(XmlSerializationData* data)
{
    WsXmlNodeH node;
    char* name = (data->name) ? data->name : data->elementInfo->name;
    char *ns   = (data->elementInfo->ns) ? data->elementInfo->ns : data->ns;
    TRACE_ENTER;
    debug("name = %s; elname = %s", data->name,
                                    data->elementInfo->name);
    debug("name = %s:%s in %s [%d]", ns, name,
                    ws_xml_get_node_local_name(data->xmlNode),
                    data->index);
    node = ws_xml_get_child(data->xmlNode, data->index, ns, name);

    if ( g_NameNameAliaseTable ) {
        int index = 0;
        while( node == NULL && g_NameNameAliaseTable[index].name != NULL )
        {
            if ( !strcmp(g_NameNameAliaseTable[index].name, name) )
                node = ws_xml_get_child(data->xmlNode, 
                        data->index, 
                        ns, 
                        g_NameNameAliaseTable[index].aliase);
            index++;
        }
    }
    debug("returned %p; %s",
                node, node?ws_xml_get_node_local_name(node):"");
    TRACE_EXIT;
    return node;
}

static size_t
get_struct_align(void)
{
   typedef struct {
        XML_TYPE_UINT8 a;
        struct {char x; void *y;} b;
    } dummy;
    return (char *)&(((dummy *)NULL)->b) - (char *)NULL;
}


static int
handle_attrs(struct __XmlSerializationData* data,
             WsXmlNodeH node,
             size_t sz)
{
    int ret = 0;
    char *savedBufPtr = DATA_BUF(data);

    TRACE_ENTER;
    debug("node name = %s", ws_xml_get_node_local_name(node));
    if (!XML_IS_ATTRS(data->elementInfo)) {
        debug("No attrs");
        goto DONE;
    }

    typedef struct {
        XML_TYPE_UINT8 a;
        XML_NODE_ATTR * b;
    } dummy;

    DATA_BUF(data) = DATA_BUF(data) + sz;
    size_t al = (char *)&(((dummy *)NULL)->b) - (char *)NULL;
    size_t pad = (unsigned long)DATA_BUF(data) % al;
    if (pad) {
        pad = al - pad;
    }
    debug("initial DATABUF = %p", DATA_BUF(data));
    DATA_BUF(data) = DATA_BUF(data) + pad;
    debug("alligned databuf = %p; pad = 0x%x", DATA_BUF(data), pad);

    if (data->mode == XML_SMODE_FREE_MEM) {
            // XXXXX   free memory 
        goto DONE;
    }
    if (data->mode == XML_SMODE_SERIALIZE) {
        XML_NODE_ATTR *attrs = *((XML_NODE_ATTR **)DATA_BUF(data));
        debug("attrs = %p", attrs);
        while (attrs) {
            debug("add attr. %s:%s = %s",
                    attrs->ns, attrs->name, attrs->value);
            if (ws_xml_add_node_attr(node,
                    attrs->ns, attrs->name, attrs->value) == NULL) {
                error("could not add attr. %s:%s = %s",
                    attrs->ns, attrs->name, attrs->value);
                ret = 1;
                goto DONE;
            }
            attrs = attrs->next;
        }
        goto DONE;
    }
    // XML_SMODE_DESERIALIZE
    int i;
    XML_NODE_ATTR **attrsp = (XML_NODE_ATTR **)DATA_BUF(data);
    XML_NODE_ATTR *attr;
    WsXmlAttrH xmlattr;
    char   *src, *dstPtr;
    int    dstSize;
    *attrsp = NULL;
    for (i = 0; i < ws_xml_get_node_attr_count(node); i++) {
        attr = xml_serializer_alloc(data, sizeof (XML_NODE_ATTR), 1);
        if (attr == NULL) {
            error("no memory");
            ret = 1;
            goto DONE;
        }
        xmlattr = ws_xml_get_node_attr(node, i);
        if (xmlattr == NULL) {
            error("could not get attr %d", i);
            ret = 1;
            goto DONE;
        }
        src = ws_xml_get_attr_ns(xmlattr);
        if (!(src == NULL || *src == 0)) {
            dstSize = 1 + strlen(src);
            dstPtr = (char *)xml_serializer_alloc(data, dstSize, 1);
            if (dstPtr == NULL) {
                error("no memory");
                ret = 1;
                goto DONE;
            }
            strncpy(dstPtr, src, dstSize);
            attr->ns = dstPtr;
        }
        src = ws_xml_get_attr_name(xmlattr);
        if (!(src == NULL || *src == 0)) {
            dstSize = 1 + strlen(src);
            dstPtr = (char *)xml_serializer_alloc(data, dstSize, 1);
            if (dstPtr == NULL) {
                error("no memory");
                ret = 1;
                goto DONE;
            }
            strncpy(dstPtr, src, dstSize);
            attr->name = dstPtr;
        }
        src = ws_xml_get_attr_value(xmlattr);
        if (!(src == NULL || *src == 0)) {
            dstSize = 1 + strlen(src);
            dstPtr = (char *)xml_serializer_alloc(data, dstSize, 1);
            if (dstPtr == NULL) {
                error("no memory");
                ret = 1;
                goto DONE;
            }
            strncpy(dstPtr, src, dstSize);
            attr->value = dstPtr;
        }
        attr->next = *attrsp;
        *attrsp = attr;
    }
DONE:
    DATA_BUF(data) = savedBufPtr;
    TRACE_EXIT;
    return ret;
}


static int
do_serialize_uint(XmlSerializationData * data, int valSize)
{
    WsXmlNodeH      child = NULL;
    XML_TYPE_UINT32   tmp;
    int             retVal = DATA_ALL_SIZE(data);

    TRACE_ENTER;
    debug("handle %d UINT%d %s;", DATA_COUNT(data),
        8 * valSize, data->elementInfo->name);
    if (DATA_BUF(data) + retVal > data->stopper) {
        error("size of %d structures %s exceeds stopper (%p > %p)",
                DATA_COUNT(data), DATA_ELNAME(data),
                DATA_BUF(data) + retVal, data->stopper);
        return WS_ERR_INVALID_PARAMETER;
    }
    if (DATA_MUST_BE_SKIPPED(data)) {
        DATA_BUF(data) = DATA_BUF(data) + retVal;
        return retVal;
    }
    if (data->mode == XML_SMODE_FREE_MEM) {
        goto DONE;
    }
    if ((data->mode != XML_SMODE_DESERIALIZE &&
             data->mode != XML_SMODE_SERIALIZE)) {
        error("invalid mode %d", data->mode);
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
                error("unsupported uint size + %d", 8 * valSize);
                retVal = WS_ERR_INVALID_PARAMETER;
                goto DONE;
            }

            if (((child = xml_serializer_add_child(data, NULL)) == NULL) ||
                            (ws_xml_set_node_ulong(child, tmp)) != 0) {
                debug("could not add child %s[%d]",
                             DATA_ELNAME(data), data->index);
                retVal = WS_ERR_INSUFFICIENT_RESOURCES;
                goto DONE;
            }
        }
        if (data->mode == XML_SMODE_DESERIALIZE) {
             char  *str;

            if ((child = xml_serializer_get_child(data)) == NULL) {
                error("not enough (%d < %d) instances of element %s",
                       data->index, DATA_COUNT(data), DATA_ELNAME(data));
                retVal = WS_ERR_XML_PARSING;
                goto DONE;
            }
            if ((str = ws_xml_get_node_text(child)) == NULL) {
                error("No text of node %s[%d]",
                             DATA_ELNAME(data), data->index);
                retVal = WS_ERR_XML_PARSING;
                goto DONE;
            }
            while (isspace(str[0])) str++;
            char *end;
            errno = 0;
            if (str[0] == '-' || str[0] == 0) {
                error("negative or abcent value = %s", str);
                retVal = WS_ERR_XML_PARSING;
                goto DONE;
            }
            tmp = strtoull(str, &end, 10);
            if (errno) {
                error("strtoul(%s) failed; errno = #d", str, errno);
                retVal = WS_ERR_XML_PARSING;
                goto DONE;
            }
            while (isspace(end[0])) end++;
            if (*end != 0) {
                error("wrong token = %s.", str);
                retVal = WS_ERR_XML_PARSING;
                goto DONE;
            }
            if (valSize == 1) {
                if (tmp > 255) {
                    error("not uint8 = %ld", tmp);
                    retVal = WS_ERR_XML_PARSING;
                    goto DONE;
                }
                *((XML_TYPE_UINT8 *)data->elementBuf) = (XML_TYPE_UINT8)tmp;
            } else if (valSize == 2) {
                if ( tmp > 65535) {
                    error("not uint16 = %ld", tmp);
                    retVal = WS_ERR_XML_PARSING;
                    goto DONE;
                }
                *((XML_TYPE_UINT16 *)data->elementBuf) = (XML_TYPE_UINT16)tmp;
            } else if (valSize == 4) {
                *((XML_TYPE_UINT32 *)data->elementBuf) = (XML_TYPE_UINT32)tmp;
            } else {
                error("unsupported uint size + %d", 8 * valSize);
                retVal = WS_ERR_INVALID_PARAMETER;
                goto DONE;
            }
        }
        handle_attrs(data, child, valSize);
        DATA_BUF(data) = DATA_BUF(data) + DATA_SIZE(data);
    }
    if ((data->mode == XML_SMODE_DESERIALIZE) &&
                            xml_serializer_get_child(data)) {
        error("too many (%d > %d) instances of element %s", data->index,
                        DATA_COUNT(data), DATA_ELNAME(data));
        retVal = WS_ERR_XML_PARSING;
        goto DONE;
    }
DONE:
    TRACE_EXIT;
    return retVal;
}



int 
do_serialize_uint8(XmlSerializationData* data)
{
    if (XML_IS_ATTRS(data->elementInfo)) {
        size_t al = get_struct_align();
        size_t pad = (unsigned long)DATA_BUF(data) % al;
        if (pad) {
            pad = al - pad;
        }
        DATA_BUF(data) = DATA_BUF(data) + pad;
    }

    return  do_serialize_uint(data, sizeof(XML_TYPE_UINT8));
}


int 
do_serialize_uint16(XmlSerializationData* data)
{
    typedef struct {
        XML_TYPE_UINT8 a;
        XML_TYPE_UINT16 b;
    } dummy;
    size_t al;
    if (XML_IS_ATTRS(data->elementInfo)) {
        al = get_struct_align();
    } else {
        al = (char *)&(((dummy *)NULL)->b) - (char *)NULL;
    }
    size_t pad = (unsigned long)DATA_BUF(data) % al;

    if (pad) {
        pad = al - pad;
    }

    DATA_BUF(data) = DATA_BUF(data) + pad;
    int retVal = do_serialize_uint(data, sizeof (XML_TYPE_UINT16));
    if (retVal >= 0) {
        retVal += pad;
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
    size_t al;
    if (XML_IS_ATTRS(data->elementInfo)) {
        al = get_struct_align();
    } else {
        al = (char *)&(((dummy *)NULL)->b) - (char *)NULL;
    }
    size_t pad = (unsigned long)data->elementBuf % al;
    if (pad) {
        pad = al - pad;
    }

    DATA_BUF(data) = DATA_BUF(data) + pad;
    int retVal = do_serialize_uint(data, sizeof (XML_TYPE_UINT32));
    if (retVal >= 0) {
        retVal += pad;
    }
    return retVal;
}


int
do_serialize_string(XmlSerializationData * data)
{
    WsXmlNodeH      child = NULL;
    int             retVal = DATA_ALL_SIZE(data);

    typedef struct {
        XML_TYPE_UINT8 a;
        XML_TYPE_STR b;
    } dummy;

    TRACE_ENTER;
    debug("handle %d strings %s = %p", DATA_COUNT(data),
                    data->elementInfo->name, data->elementBuf);
    size_t al;
    if (XML_IS_ATTRS(data->elementInfo)) {
        al = get_struct_align();
    } else {
        al = (char *)&(((dummy *)NULL)->b) - (char *)NULL;
    }
    size_t pad = (unsigned long)DATA_BUF(data) % al;
    if (pad) {
        pad = al - pad;
    }
    retVal += pad;
    if (DATA_BUF(data) + retVal > data->stopper) {
        error("stopper: %p > %p",
                   DATA_BUF(data) + retVal, data->stopper);
        return WS_ERR_INVALID_PARAMETER;
    }
    if (DATA_MUST_BE_SKIPPED(data)) {
        data->elementBuf = DATA_BUF(data) + retVal;
        return retVal;
    }
    DATA_BUF(data) = DATA_BUF(data) + pad;
    debug("adjusted elementBuf = %p", data->elementBuf);

    for (data->index = 0; data->index < DATA_COUNT(data); data->index++) {
        if (data->mode == XML_SMODE_FREE_MEM) {
             xml_serializer_free(data, DATA_BUF(data));
            *(XML_TYPE_STR *)DATA_BUF(data) = NULL;
        } else if (data->mode == XML_SMODE_SERIALIZE) {
            char *valPtr = *((char **)DATA_BUF(data));
            child = xml_serializer_add_child(data, valPtr);
            if (child == NULL) {
                error("xml_serializer_add_child failed.");
                retVal = WS_ERR_INSUFFICIENT_RESOURCES;
                goto DONE;
            }
            if (ws_xml_get_node_text(child) == NULL) {
                ws_xml_add_node_attr(child, 
                     XML_NS_SCHEMA_INSTANCE, XML_SCHEMA_NIL, "true");
            }
        } else if (data->mode == XML_SMODE_DESERIALIZE) {
            if ((child = xml_serializer_get_child(data)) == NULL) {
                error("not enough (%d < %d) instances of element %s",
                       data->index, DATA_COUNT(data), DATA_ELNAME(data));
                retVal = WS_ERR_XML_PARSING;
                goto DONE;
            }

            char *src = ws_xml_get_node_text(child);
            if (src != NULL || *src != 0) {
                char   *dstPtr;
                int    dstSize = 1 + strlen(src);
                dstPtr = (char *)xml_serializer_alloc(data, dstSize, 1);
                if (dstPtr == NULL) {
                    error("no memory");
                    retVal = WS_ERR_INSUFFICIENT_RESOURCES;
                    goto DONE;
                }
                strncpy(dstPtr, src, dstSize);
                *((XML_TYPE_PTR *)DATA_BUF(data)) = dstPtr;
            }
	    } else {
            error("invalid mode");
            retVal = WS_ERR_INVALID_PARAMETER;
            goto DONE;
        }
        handle_attrs(data, child, sizeof (XML_TYPE_STR));
        DATA_BUF(data) = DATA_BUF(data) + DATA_SIZE(data);
    }
    if ((data->mode == XML_SMODE_DESERIALIZE) &&
                            xml_serializer_get_child(data)) {
        error("too many (%d > %d) instances of element %s", data->index,
                        DATA_COUNT(data), DATA_ELNAME(data));
        retVal = WS_ERR_XML_PARSING;
        goto DONE;
    }
DONE:
    TRACE_EXIT;
    return retVal;
}



int
do_serialize_bool(XmlSerializationData * data)
{
    int  retVal = DATA_ALL_SIZE(data);
    typedef struct {
        XML_TYPE_UINT8 a;
        XML_TYPE_BOOL b;
    } dummy;

    TRACE_ENTER;
    debug("handle %d booleans %s; ptr = %p", DATA_COUNT(data),
                    DATA_ELNAME(data), DATA_BUF(data));
    size_t al;
    if (XML_IS_ATTRS(data->elementInfo)) {
        al = get_struct_align();
    } else {
        al = (char *)&(((dummy *)NULL)->b) - (char *)NULL;
    }
    size_t pad = (unsigned long)DATA_BUF(data) % al;
    if (pad) {
        pad = al - pad;
    }
    retVal += pad;
    if (DATA_BUF(data) + retVal > data->stopper) {
        return WS_ERR_INVALID_PARAMETER;
    }
    if (DATA_MUST_BE_SKIPPED(data) ||
        data->mode == XML_SMODE_FREE_MEM) {
        data->elementBuf = DATA_BUF(data) + retVal;
        return retVal;
    }
    DATA_BUF(data) = DATA_BUF(data) + pad;
    debug("adjusted elementBuf = %p", data->elementBuf);

    WsXmlNodeH      child = NULL;
    for (data->index = 0; data->index < DATA_COUNT(data); data->index++) {
        debug("%s[%d] = %p", DATA_ELNAME(data), data->index, DATA_BUF(data));
        if (data->mode == XML_SMODE_SERIALIZE) {
            XML_TYPE_BOOL   tmp;

            tmp = *((XML_TYPE_BOOL *)DATA_BUF(data));	
            if ((child = xml_serializer_add_child(data,
                         (tmp==0) ? "false" : "true")) == NULL) {
                retVal = WS_ERR_INSUFFICIENT_RESOURCES;
                goto DONE;
            }
        } else if (data->mode == XML_SMODE_DESERIALIZE) {
            XML_TYPE_PTR    dataPtr = (XML_TYPE_PTR)DATA_BUF(data);

            if ((child = xml_serializer_get_child(data)) == NULL) {
                error("not enough (%d < %d) instances of element %s",
                       data->index, DATA_COUNT(data), DATA_ELNAME(data));
                retVal = WS_ERR_XML_PARSING;
                goto DONE;
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
        handle_attrs(data, child, sizeof (XML_TYPE_BOOL));
        DATA_BUF(data) = DATA_BUF(data) + DATA_SIZE(data);
    }
    if ((data->mode == XML_SMODE_DESERIALIZE) &&
                            xml_serializer_get_child(data)) {
        error("too many (%d > %d) instances of element %s", data->index,
                        DATA_COUNT(data), DATA_ELNAME(data));
        retVal = WS_ERR_XML_PARSING;
        goto DONE;
    }
DONE:
    TRACE_EXIT;
    return retVal;
}



static XmlSerialiseDynamicSizeData* 
make_dyn_size_data(XmlSerializationData* data, int *retValp)
{
    XmlSerialiseDynamicSizeData* dyn =
                (XmlSerialiseDynamicSizeData*)data->elementBuf;
    TRACE_ENTER;

    debug("name = <%s>, elname = <%s>", data->name, DATA_ELNAME(data));
    int savedIndex = data->index;
    data->index = 0;
    while (xml_serializer_get_child(data) != NULL ) {
        data->index++;
    }
    dyn->count = data->index;
    data->index = savedIndex;
    if (dyn->count < DATA_MIN_COUNT(data)) {
        error("not enough (%d < %d) elements %s", dyn->count,
               DATA_MIN_COUNT(data), DATA_ELNAME(data)); 
        *retValp = WS_ERR_XML_PARSING;
        dyn = NULL;
        goto DONE;
    }
    if ((DATA_MAX_COUNT(data) > 0) && dyn->count > DATA_MAX_COUNT(data)) {
        error("too many (%d > %d) elements %s", dyn->count,
               DATA_MAX_COUNT(data), DATA_ELNAME(data)); 
        *retValp = WS_ERR_XML_PARSING;
        dyn = NULL;
        goto DONE;
    }
    debug("count = %d of %d sizes", dyn->count, DATA_SIZE(data));
    if (dyn->count == 0) {
        goto DONE;
    }

    int size = DATA_SIZE(data) * dyn->count;
    dyn->data = xml_serializer_alloc(data, size, 1);
    if (dyn->data == NULL) {
        error("no memory");
        *retValp = WS_ERR_INSUFFICIENT_RESOURCES;
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
    int  retVal = DATA_SIZE(data) + pad;
    if (DATA_BUF(data) + retVal > data->stopper) {
        return WS_ERR_INVALID_PARAMETER;
    }
    if (DATA_MUST_BE_SKIPPED(data)) {
        DATA_BUF(data) = DATA_BUF(data) + retVal;
        return retVal;
    }
    DATA_BUF(data) = DATA_BUF(data) + pad;
    debug("adjusted elementBuf = %p", data->elementBuf);

    char *savedBufPtr = DATA_BUF(data);
    XmlSerializerInfo *savedElementInfo = data->elementInfo;

    if (data->mode != XML_SMODE_SERIALIZE &&
               data->mode != XML_SMODE_DESERIALIZE &&
               data->mode != XML_SMODE_FREE_MEM) {
        retVal = WS_ERR_INVALID_PARAMETER;
        goto DONE;
    }

    XmlSerialiseDynamicSizeData *dyn = NULL;

    if (data->mode == XML_SMODE_DESERIALIZE) {
        if ((dyn = make_dyn_size_data(data, &retVal)) == NULL) {
            goto DONE;
        }
    } else {
        dyn = (XmlSerialiseDynamicSizeData *) data->elementBuf;
        if (data->mode == XML_SMODE_SERIALIZE) {
            if (dyn->count < DATA_MIN_COUNT(data)) {
                error("not enough (%d < %d) elements %s", dyn->count,
                        DATA_MIN_COUNT(data), DATA_ELNAME(data)); 
                retVal = WS_ERR_XML_PARSING;
                goto DONE;
            }
            if ((DATA_MAX_COUNT(data) > 0) && dyn->count > DATA_MAX_COUNT(data)) {
                error("too many (%d > %d) elements %s", dyn->count,
                        DATA_MAX_COUNT(data), DATA_ELNAME(data)); 
                retVal = WS_ERR_XML_PARSING;
                goto DONE;
            }
        }
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
    myinfo.ns  = data->elementInfo->ns;

    data->stopper = (char *)dyn->data + DATA_SIZE(data) * dyn->count;
    data->elementInfo = &myinfo;
    DATA_BUF(data) = dyn->data;
    data->index = 0;

    debug("dyn = %p, dyn->data = %p + 0x%x",
                  dyn, dyn->data, DATA_SIZE(data) * dyn->count);
    tmp = data->elementInfo->proc(data);

    data->index = savedIndex;
    DATA_BUF(data) = savedBufPtr;
    data->elementInfo = savedElementInfo;
    data->stopper = savedStopper;
    if (tmp < 0 && data->mode != XML_SMODE_FREE_MEM) {
        retVal = tmp;
        goto DONE;
    }
    if ((data->mode == XML_SMODE_FREE_MEM) && dyn->data) {
        xml_serializer_free(data, dyn->data);
        goto DONE;
    }
DONE:
    DATA_BUF(data) = DATA_BUF(data) + DATA_SIZE(data);
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

    debug("handle %d structure %s ptr = %p", DATA_COUNT(data),
                    data->elementInfo->name, data->elementBuf);
    if (data->mode != XML_SMODE_SERIALIZE &&
        data->mode != XML_SMODE_DESERIALIZE &&
        data->mode != XML_SMODE_FREE_MEM) {
            retVal = WS_ERR_INVALID_PARAMETER;
            debug("Incorrect data->mode = %d", data->mode);
            goto DONE;
    }
    size_t al = get_struct_align();
    size_t pad = (unsigned long)DATA_BUF(data) % al;
    if (pad) {
        pad = al - pad;
    }
    retVal = pad + XML_IS_HEAD(savedElement) ?
                           DATA_SIZE(data) :DATA_ALL_SIZE(data);
    if ((char *)DATA_BUF(data) + retVal > data->stopper) {
        error("size of %d structures %s exceeds stopper (%p > %p)",
                DATA_COUNT(data), DATA_ELNAME(data),
                (char *)DATA_BUF(data) + retVal, data->stopper);
        return WS_ERR_INVALID_PARAMETER;
    }
    if (DATA_MUST_BE_SKIPPED(data)) {
        debug(" %d elements %s skipped", DATA_COUNT(data), DATA_ELNAME(data));
        DATA_BUF(data) = DATA_BUF(data) + retVal;
        return retVal;
    }
    DATA_BUF(data) = DATA_BUF(data) + pad;
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
    WsXmlNodeH child;
    for (; data->index < count; data->index++) {
        child = NULL;
        savedLocalIndex = data->index;
        savedLocalElementBuf = DATA_BUF(data);
        data->stopper = savedLocalElementBuf + DATA_SIZE(data);
        debug("%s[%d] = %p", DATA_ELNAME(data), data->index, DATA_BUF(data));
        if (data->mode == XML_SMODE_SERIALIZE) {
            child = xml_serializer_add_child(data, NULL);
            data->xmlNode = child;
            if (data->xmlNode == NULL) {
                error("cant add child");
                retVal = WS_ERR_INSUFFICIENT_RESOURCES;
                goto DONE;
            }
        } else {
            child = xml_serializer_get_child(data);
            if (child == NULL) {
                error("not enough (%d < %d) instances of element %s",
                       data->index, DATA_COUNT(data), DATA_ELNAME(data));
                retVal = WS_ERR_XML_PARSING;
                goto DONE;
            }
        }

        int tmp = 0;

        debug("before for loop. Struct %s = %p",
                        savedElement->name, DATA_BUF(data));

        for (i = 0; retVal > 0 && i < elementCount; i++) {
            data->elementInfo = &elements[i];
            debug("handle %d elements %s of struct %s. dstPtr = %p",
                DATA_COUNT(data), DATA_ELNAME(data),
                savedElement->name, DATA_BUF(data));
            if (XML_IS_SKIP(data->elementInfo)) {
                data->mode = XML_SMODE_SKIP;
            }

            tmp = data->elementInfo->proc(data);

            if (tmp < 0) {
                error("handling element %s failed = %d", DATA_ELNAME(data), tmp);
                retVal = tmp;
                goto DONE;
            }
        }
        data->elementInfo = savedElement;
        data->index = savedLocalIndex;
        data->mode = savedMode;
        data->xmlNode = savedXmlNode;
        data->elementInfo = savedElement;
        handle_attrs(data, child, 0);
        data->elementBuf = savedLocalElementBuf + struct_size;
    }
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
        error("info->proc == NULL");
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
