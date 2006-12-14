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


void xml_serializer_free_scalar_mem(XmlSerializationData* data)
{
	TRACE_ENTER;
    if ( XML_IS_PTR(data->elementInfo) ) {
        if (xml_serializer_free(data, *((XML_TYPE_PTR*)data->elementBuf)) )
            *((XML_TYPE_PTR*)data->elementBuf) = NULL;
    }
    TRACE_EXIT;
}


WsXmlNodeH
xml_serializer_add_child(XmlSerializationData* data, char* value)
{
    char* name = (data->name) ? data->name : data->elementInfo->name;
    TRACE_ENTER;
    TRACE_DETAILS("name = %s; value(%p) = %s", name, value, value);
    return ws_xml_add_child(data->xmlNode, data->ns, name, value); 
    TRACE_EXIT;
}


WsXmlNodeH
xml_serializer_get_child(XmlSerializationData* data)
{
    WsXmlNodeH node;
    char* name = (data->name) ? data->name : data->elementInfo->name;
    TRACE_ENTER;
    /*
    printf("name: %s %d\n", name, data->index );
    if (!name) {
        XmlSerializerInfo* elementInfo = (XmlSerializerInfo*)data->elementInfo->extData;
        if (elementInfo)
            node = ws_xml_get_child(elementInfo->xmlNode, elementInfo->index, elementInfo->ns, elementInfo->name);
        else
            node = ws_xml_get_child(data->xmlNode, data->index, data->ns, name);
    } else {
        node = ws_xml_get_child(data->xmlNode, data->index, data->ns, name);
    }
    */
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
	TRACE_EXIT;
    return node;
}


int
do_serialize_uint(XmlSerializationData * data, int valSize)
{
	WsXmlNodeH      child;
	unsigned long   tmp;
	int             retVal = valSize;
	XML_TYPE_PTR    dataPtr;
	TRACE_ENTER;
	dataPtr = make_dst_ptr(data, valSize);
	if (dataPtr == NULL) {
		if ((data->mode == XML_SMODE_DESERIALIZE ||
		     data->mode == XML_SMODE_SERIALIZE)) {
			retVal = WS_ERR_INSUFFICIENT_RESOURCES;
			goto DONE;
		}
	}
	if (data->mode == XML_SMODE_FREE_MEM) {
		xml_serializer_free_scalar_mem(data);
		goto DONE;
	}
	if (data->mode == XML_SMODE_SERIALIZE) {
		TRACE_DETAILS("value size: %d", valSize);
		if (valSize == 1)
			tmp = *((unsigned char *) dataPtr);
		else if (valSize == 2)
			tmp = *((unsigned short *) dataPtr);
		else
			tmp = *((unsigned long *) dataPtr);

		if ((child = xml_serializer_add_child(data, NULL)) == NULL
		    || ws_xml_set_node_ulong(child, tmp) != 0) {
			retVal = WS_ERR_INSUFFICIENT_RESOURCES;
		}
	} else if (data->mode == XML_SMODE_DESERIALIZE) {
		char           *str;

		if (!(child = xml_serializer_get_child(data))) {
			retVal = WS_ERR_XML_NODE_NOT_FOUND;
		} else if ((str = ws_xml_get_node_text(child)) != NULL) {
			tmp = strtoul(str, NULL, 10);
			/*
			 * TBD:		validate that value doesn 't exceed
			 * size
			 */
			if (valSize == 1)
				*((unsigned char *) dataPtr) = (unsigned char) tmp;
			else if (valSize == 2)
				*((unsigned short *) dataPtr) = (unsigned short) tmp;
			else
				*((unsigned long *) dataPtr) = tmp;
		} else
			retVal = WS_ERR_XML_PARSING;
	} else if (data->mode != XML_SMODE_BINARY_SIZE) {
		retVal = WS_ERR_INVALID_PARAMETER;		
	}

DONE:
	TRACE_EXIT;
	return retVal;
}





int 
do_serialize_uint8(XmlSerializationData* data)
{
    return do_serialize_uint(data, sizeof(XML_TYPE_UINT8));
}


int 
do_serialize_uint16(XmlSerializationData* data)
{
    return do_serialize_uint(data, sizeof(XML_TYPE_UINT16));
}

int 
do_serialize_uint32(XmlSerializationData* data)
{
    return do_serialize_uint(data, sizeof(XML_TYPE_UINT32));
}


static int
ws_serilize_string_array(XmlSerializationData * data)
{
    char **Ptr = *((char ***) data->elementBuf);
    char **p;
    int i, count;
    int retVal = sizeof(XML_TYPE_STR);
    WsXmlNodeH      savedXmlNode = data->xmlNode;
    char *savedName = data->name;
    TRACE_ENTER;


    p = Ptr;
    while (*p != NULL) p++;
    count = p - Ptr;

    if ((data->xmlNode = xml_serializer_add_child(data, NULL)) == NULL) {
        retVal = WS_ERR_INSUFFICIENT_RESOURCES;
        goto DONE;
    }
    XML_UNSET_PTR(data->elementInfo);
    data->name = "string";
    for (i = 0; i < count; i++) {
        data->elementBuf = Ptr + i;
        retVal = do_serialize_string(data);
        if (retVal < 0) {
            break;
        }
    }
DONE:
    XML_SET_PTR(data->elementInfo);
    data->xmlNode = savedXmlNode;
    data->name = savedName;
    data->elementBuf = Ptr;
    TRACE_EXIT;
    return retVal;
}


static int
ws_deserilize_string_array(XmlSerializationData * data)
{
    int retVal = sizeof(XML_TYPE_PTR);
    char *savedName = data->name;
    char *savedElName = data->elementInfo->name;
    WsXmlNodeH savedNode = data->xmlNode;
    void *savedElBuf = data->elementBuf;
    int savedIndex = data->index;
    int count = 0;
    int i;
    char **p;

    data->xmlNode = xml_serializer_get_child(data);
    data->index = 0;
    data->name = NULL;
    data->elementInfo->name = NULL;
    while (xml_serializer_get_child(data) != NULL) {
       data->index++;
    }
    count = data->index;
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
    data->name = savedName;
    data->elementInfo->name = savedElName;
    data->xmlNode = savedNode;
    data->elementBuf = savedElBuf;
    data->index = savedIndex;
    TRACE_EXIT;
    return retVal;
}

/* We will need special function DoSerializeArrayOfStringPtrs() */
int
do_serialize_string(XmlSerializationData * data)
{
	WsXmlNodeH      child;
	int             retVal = sizeof(XML_TYPE_STR);
	TRACE_ENTER;
	if (data->mode == XML_SMODE_FREE_MEM) {
		xml_serializer_free_scalar_mem(data);
		goto DONE;
	}

    if (data->mode == XML_SMODE_BINARY_SIZE) {
        // we already have size 
        goto DONE;
    }
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
	
	if (data->mode == XML_SMODE_SERIALIZE) {
		char           *valPtr = *((char **) data->elementBuf);

		if ((child = xml_serializer_add_child(data, valPtr)) == NULL) {
			retVal = WS_ERR_INSUFFICIENT_RESOURCES;
		} else {
			if (ws_xml_get_node_text(child) == NULL) {
				ws_xml_add_node_attr(child, 
                    XML_NS_SCHEMA_INSTANCE, XML_SCHEMA_NIL, "true");
			}
		}
	} else if (data->mode == XML_SMODE_DESERIALIZE) {
		if ((child = xml_serializer_get_child(data)) == NULL) {
			retVal = WS_ERR_XML_NODE_NOT_FOUND;
            goto DONE;
        }

		char           *src = ws_xml_get_node_text(child);
		if (src == NULL || *src == 0) {
            *((XML_TYPE_PTR *) data->elementBuf) = NULL;
            retVal = 0;
            goto DONE;
        }
		char           *dstPtr;
		int             dstSize = 1 + strlen(src);
        dstPtr = (char *)xml_serializer_alloc(data, dstSize, 1);
		if (dstPtr == NULL) {
			retVal = WS_ERR_INSUFFICIENT_RESOURCES;
            goto DONE;
        }

		strncpy(dstPtr, src, dstSize);
		*((XML_TYPE_PTR *) data->elementBuf) = dstPtr;
	} else {
		if (data->mode != XML_SMODE_BINARY_SIZE) {
			retVal = WS_ERR_INVALID_PARAMETER;
		}
	}

DONE:
	TRACE_EXIT;
	return retVal;
}

int 
do_serialize_char_array(XmlSerializationData* data)
{
    WsXmlNodeH child; 
    int count = data->elementInfo->funcMaxCount & SER_FLAGS_MASK;
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
            int dstSize = SER_FLAGS_MASK & data->elementInfo->funcMaxCount;

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


int
do_serialize_bool(XmlSerializationData * data)
{
	int             retVal = sizeof(XML_TYPE_BOOL);
	TRACE_ENTER;
	if (data->mode == XML_SMODE_FREE_MEM) {
		xml_serializer_free_scalar_mem(data);
		goto DONE;
	}
	if (data->mode == XML_SMODE_SERIALIZE) {
		WsXmlNodeH      child;
		
		unsigned long   tmp;
		int valSize = sizeof(int);
		if (valSize == 1)
			tmp = *((unsigned char *) data->elementBuf);
		else if (valSize == 2)
			tmp = *((unsigned short *) data->elementBuf);
		else
			tmp = *((unsigned long *) data->elementBuf);

			
		if ((child = xml_serializer_add_child(data, (tmp==1)?"true":"false")) == NULL) {
			retVal = WS_ERR_INSUFFICIENT_RESOURCES;
		} 
		goto DONE;
	}
	
	if (data->mode == XML_SMODE_DESERIALIZE) {
		WsXmlNodeH      child;
		XML_TYPE_PTR    dataPtr = make_dst_ptr(data, retVal);

		if (dataPtr == NULL)
			retVal = WS_ERR_INSUFFICIENT_RESOURCES;
		else if ((child = xml_serializer_get_child(data)) == NULL)
			retVal = WS_ERR_XML_NODE_NOT_FOUND;
		else {
			int             tmp = -1;
			char           *src = ws_xml_get_node_text(child);
			if (src == NULL || *src == 0) {
				tmp = 1;
			} else {
				if (isdigit(*src)) {
					tmp = atoi(src);
				} else {
					if (strcmp(src, "yes") == 0 || strcmp(src, "true") == 0) {
						tmp = 1;
					} else if (strcmp(src, "no") == 0 || strcmp(src, "false") == 0) {
						tmp = 0;
					}
				}
			}

			if (tmp == 0 || tmp == 1) {
				*((int *) dataPtr) = tmp;
			} else {
				retVal = WS_ERR_XML_PARSING;
			}
		}
	} else {
		if (data->mode != XML_SMODE_BINARY_SIZE) {
			retVal = WS_ERR_INVALID_PARAMETER;
		}
	}
DONE:
	TRACE_EXIT;
	return retVal;
}

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

static int 
calculate_struct_size(XmlSerializationData* data,
	        		  int elementCount, 
        			  XmlSerializerInfo* elementArray)
{
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

int do_serialize_fixed_size_array(XmlSerializationData* data)
{
    int retVal;
    int savedIndex = data->index;
    int count;
	TRACE_ENTER;
	TRACE_DETAILS("index %d", data->index );
    if ( data->index < 0 ) {
        count = -data->index; 
        data->index = 0;
    } else {    	
        count = data->elementInfo->funcMaxCount & SER_FLAGS_MASK;    
    }
    if ( XML_IS_PTR(data->elementInfo) ) {
        retVal = sizeof(XML_TYPE_PTR);
    } else {
        int savedMode = data->mode;
        data->mode = XML_SMODE_BINARY_SIZE;
        retVal = data->elementInfo->proc(data);
        data->mode = savedMode;
    }
	TRACE_DETAILS("count: %d, retVal %d", count, retVal );
    if ( retVal > 0 && count > 0 ) {
        int adjust = get_adjusted_size(retVal);
        retVal = count * adjust;

        if ( data->mode == XML_SMODE_SERIALIZE || 
                data->mode == XML_SMODE_DESERIALIZE || 
                data->mode == XML_SMODE_FREE_MEM ) {
            char* dstPtr = (char*)data->elementBuf;
            int tmp;

            while(retVal > 0 && count--) {            	
                data->elementBuf = dstPtr;

                if ( (tmp = data->elementInfo->proc(data)) < 0 )
                    retVal = tmp;
                else
                    if ( data->mode == XML_SMODE_FREE_MEM 
                            && 
                            XML_IS_PTR(data->elementInfo) ) {
                        data->elementBuf = dstPtr;
                        xml_serializer_free_scalar_mem(data);
                    }
                dstPtr += adjust;
                data->index++;
            }
        }
        else
            if ( data->mode != XML_SMODE_BINARY_SIZE ) {
                retVal = WS_ERR_INVALID_PARAMETER;
            }
    }

    data->index = savedIndex;
    TRACE_DETAILS("retVal %d", retVal );
	TRACE_EXIT;
    return retVal;
}




XmlSerialiseDynamicSizeData* 
make_dyn_size_data(XmlSerializationData* data)
{
    XmlSerializerInfo* elementInfo = (XmlSerializerInfo*)data->elementInfo->extData;
    XmlSerialiseDynamicSizeData* dyn = XML_IS_PTR(data->elementInfo) ?
        NULL : (XmlSerialiseDynamicSizeData*)data->elementBuf;
    int savedIndex = data->index;

	TRACE_ENTER;
    if (dyn == NULL) {
        dyn = xml_serializer_alloc(data,
                sizeof(XmlSerialiseDynamicSizeData), 1);
    }
    if (dyn == NULL) {
        goto DONE;
    }

    data->index = 0;
    while( xml_serializer_get_child(data) != NULL ) {
        data->index++;
    }
    dyn->count = data->index;
    data->index = savedIndex;
    if (dyn->count == 0) {
        goto DONE;
    }

    int size;

    data->mode = XML_SMODE_BINARY_SIZE;
    size = elementInfo->proc(data);
    data->mode = XML_SMODE_DESERIALIZE;

    if (size > 0) {
        size = dyn->count * get_adjusted_size(size);
        dyn->data = xml_serializer_alloc(data, size, 1);
        if (dyn->data == NULL) {
            if (XML_IS_PTR(data->elementInfo)) {
                xml_serializer_free(data, dyn);
            }
            dyn = NULL;
        }
    }

DONE:
	TRACE_EXIT;
    return dyn;
}


int
do_serialize_dyn_size_array(XmlSerializationData * data)
{

	int             retVal = XML_IS_PTR(data->elementInfo) ?
						sizeof(XML_TYPE_PTR) : sizeof(XmlSerialiseDynamicSizeData);
	void           *savedBufPtr = data->elementBuf;
	XmlSerializerInfo *savedElementInfo = data->elementInfo;
	WsXmlNodeH      savedXmlNode = data->xmlNode;
	int             savedIndex = data->index;

	TRACE_ENTER;
	if (data->mode == XML_SMODE_SERIALIZE || data->mode == XML_SMODE_DESERIALIZE ||
	    data->mode == XML_SMODE_FREE_MEM) {
		XmlSerialiseDynamicSizeData *dyn = NULL;

		if (data->mode == XML_SMODE_DESERIALIZE) {
			if ((dyn = make_dyn_size_data(data)) == NULL) {
				retVal = WS_ERR_INSUFFICIENT_RESOURCES;
				//TBD:		? ? ? validation
			}
			TRACE_DETAILS("1");
		} else if (XML_IS_PTR(savedElementInfo)) {
			dyn = (XmlSerialiseDynamicSizeData *) * ((void **) data->elementBuf);
			TRACE_DETAILS("2");
		} else {
			dyn = (XmlSerialiseDynamicSizeData *) data->elementBuf;
			TRACE_DETAILS("3");
		}

		if (retVal >= 0 && dyn) {
			int             tmp;
			TRACE_DETAILS("data available, proceeding with de-/serialization");
			data->elementInfo = (XmlSerializerInfo *) savedElementInfo->extData;

			if (XML_IS_PTR(savedElementInfo))
				*((void **) data->elementBuf) = dyn;

			data->elementBuf = dyn->data;
			data->index = -dyn->count;

			if ((tmp = do_serialize_fixed_size_array(data)) < 0) {
				retVal = tmp;
			} else {
				if (data->mode == XML_SMODE_FREE_MEM &&
				    XML_IS_PTR(savedElementInfo)  ) {
				    	
					data->elementInfo = savedElementInfo;
					data->elementBuf = &dyn->data;
					xml_serializer_free_scalar_mem(data);

					data->elementBuf = savedBufPtr;
					xml_serializer_free_scalar_mem(data);
				}
			}
		}
	} else {
		if (data->mode != XML_SMODE_BINARY_SIZE) {
			retVal = WS_ERR_INVALID_PARAMETER;
		}
	}

	data->elementBuf = savedBufPtr;
	data->elementInfo = savedElementInfo;
	data->xmlNode = savedXmlNode;
	data->index = savedIndex;
	if (retVal > 0) {
		char           *tmpPtr = (char *) data->elementBuf;
		tmpPtr += get_adjusted_size(retVal);
		data->elementBuf = tmpPtr;
	}
	TRACE_EXIT;
	return retVal;
}

int
do_serialize_struct(XmlSerializationData * data)
{
	int             i;
	int             retVal = 0;
	int             elementCount = 0;
	XmlSerializerInfo *elements =
	            (XmlSerializerInfo *) data->elementInfo->extData;
	WsXmlNodeH      savedXmlNode = data->xmlNode;
    int             savedMode = data->mode;

	TRACE_ENTER;
	while (elements[elementCount].proc != NULL)
		elementCount++;

	if (data->mode == XML_SMODE_BINARY_SIZE) {
		retVal = calculate_struct_size(data, elementCount, elements);
		goto DONE;
	}
	
	if (data->mode != XML_SMODE_SERIALIZE &&
	    data->mode != XML_SMODE_DESERIALIZE &&
	    data->mode != XML_SMODE_FREE_MEM &&
	    data->mode != XML_SMODE_BINARY_SIZE) {
            retVal = WS_ERR_INVALID_PARAMETER;
            goto DONE;
    }


	data->mode = XML_SMODE_BINARY_SIZE;
	retVal = calculate_struct_size(data, elementCount, elements);
	data->mode = savedMode;

	if (retVal <= 0) {
        goto DONE;
    }

    if (data->mode == XML_SMODE_FREE_MEM  &&  XML_IS_PTR(data->elementInfo)) {
        if (xml_serializer_free(data, *((XML_TYPE_PTR *) data->elementBuf))) {
                *((XML_TYPE_PTR *) data->elementBuf) = NULL;
        }
	} else if (data->mode == XML_SMODE_DESERIALIZE) {
	   TRACE_DETAILS("Struct deserialize");
	   if ((data->xmlNode = xml_serializer_get_child(data)) == NULL) {
		    retVal = WS_ERR_XML_NODE_NOT_FOUND;
            goto DONE;
        }
	   if (XML_IS_PTR(data->elementInfo)) {
		   XML_TYPE_PTR    ptr = data->elementBuf;
		   if ((ptr = xml_serializer_alloc(data, retVal, 1)) == NULL) {
			   retVal = WS_ERR_INSUFFICIENT_RESOURCES;
               goto DONE;
           }
		   *((XML_TYPE_PTR *) data->elementBuf) = ptr;
        }
	} else if (data->mode == XML_SMODE_SERIALIZE) {
		TRACE_DETAILS("Struct serialize");
		if ((data->xmlNode = xml_serializer_add_child(data, NULL)) == NULL) {
			retVal = WS_ERR_INSUFFICIENT_RESOURCES;
            goto DONE;
        }
    } else {
        retVal = WS_ERR_INVALID_PARAMETER;
        goto DONE;
    }

	int             elementSize;
	char           *dstPtr = (char *) data->elementBuf;
	XmlSerializerInfo *savedElement = data->elementInfo;
	int             savedIndex = data->index;
	char           *savedName = data->name;

	data->name = NULL;
	TRACE_DETAILS("before for loop. Struct = %p", dstPtr);
	for (i = 0; retVal > 0 && i < elementCount; i++) {
		data->elementInfo = &elements[i];
		data->index = 0;

		if ((data->elementInfo->funcMaxCount & data->skipFlag)) {
						data->mode = XML_SMODE_BINARY_SIZE;
		}
		TRACE_DETAILS("before while loop");
		while (data->index < XML_MAX_OCCURS(data->elementInfo)) {
			data->elementBuf = dstPtr;
            TRACE_DETAILS("for %s data->elementBuf = %p",
                    elements[i].name, data->elementBuf);
			if ((elementSize = elements[i].proc(data)) < 0) {
				if (elementSize != WS_ERR_XML_NODE_NOT_FOUND ||
					data->index < XML_MIN_OCCURS(data->elementInfo)) {
					retVal = elementSize;
					break;
				}
				data->mode = XML_SMODE_BINARY_SIZE;
				if ((elementSize = elements[i].proc(data)) < 0) {
					retVal = elementSize;
					break;
				}
			}
			if (XML_IS_PTR(&elements[i])) {
				elementSize = sizeof(XML_TYPE_PTR);
			}
			dstPtr += get_adjusted_size(elementSize);
			data->index++;
		}
		TRACE_DETAILS("after while loop");
		data->mode = savedMode;
	}
	TRACE_DETAILS("after for loop");
	data->elementInfo = savedElement;
	data->index = savedIndex;
	data->name = savedName;

DONE:
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
    XmlSerializationData data;
    TRACE_ENTER;
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

    debug( "About to process data...");
    if (info->proc)
        retVal = info->proc(&data);
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
    int elementCount = 0;
    int size;
    void* retPtr = NULL;
    XmlSerializationData data;
    XmlSerializerInfo* elements = (XmlSerializerInfo*)info->extData;
	TRACE_ENTER;
    initialize_xml_serialization_data(&data, cntx, info, NULL, XML_SMODE_BINARY_SIZE,
            nameNs, elementNs, xmlParent);

    data.index = index;
    data.name = name;

    if ( output )
        data.skipFlag = SER_IN;
    else
        data.skipFlag = SER_OUT;

    while(elements[elementCount].proc != NULL)
        elementCount++;

    if ( (size = calculate_struct_size(&data, elementCount, elements)) > 0 ) {
        data.mode = XML_SMODE_DESERIALIZE;
        if ( (data.elementBuf = xml_serializer_alloc(&data, size, 1)) != NULL ) {
            retPtr = data.elementBuf;
            if ( info->proc(&data) <= 0 ) {
                data.elementBuf = retPtr;
                retPtr = NULL;
                ws_serializer_free_mem(cntx, data.elementBuf, info);
            }
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
    if ( soap != NULL
            &&
            (ptr = (WsSerializerMemEntry*)u_malloc(sizeof(WsSerializerMemEntry) + size)) != NULL )
    {
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
    if ( soap && ptr != NULL ) {
        u_lock(soap);
        node = list_first(soap->WsSerializerAllocList);
        while( node != NULL )
        {
            WsSerializerMemEntry* entry = (WsSerializerMemEntry*)node->list_data;

            if ( entry && entry->cntx == cntx && (!ptr || ptr == entry->buf) )
            {
                // FIXME
                //lnode_destroy (node);
                //list_delete(soap->WsSerializerAllocList, node);
                if ( ptr != NULL )
                    break;
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
