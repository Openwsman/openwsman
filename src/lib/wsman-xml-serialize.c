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

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <ctype.h>
#include <assert.h>

#if defined (__SVR4) && defined (__sun)
#include <strings.h>
#endif

#include <u/libu.h>
#include <math.h>

#include "wsman-xml-api.h"
#include "wsman-soap.h"
#include "wsman-xml.h"

#include "wsman-dispatcher.h"
#include "wsman-xml-serializer.h"
#include "wsman-xml-serialize.h"
#include "wsman-soap-envelope.h"

struct __WsSerializerMemEntry {
	char buf[0];
};

typedef struct __WsSerializerMemEntry WsSerializerMemEntry;

WsSerializerContextH ws_serializer_init()
{
	WsSerializerContextH serializercntx = NULL;
	serializercntx = u_malloc(sizeof(struct __WsSerializerContext));
	if(serializercntx == NULL) return NULL;
	serializercntx->WsSerializerAllocList = list_create(LISTCOUNT_T_MAX);
	if(serializercntx->WsSerializerAllocList == NULL) {
		u_free(serializercntx);
		return NULL;
	}
	u_init_lock(serializercntx);
	return serializercntx;
}

int ws_serializer_cleanup(WsSerializerContextH serctx)
{
	if(serctx && serctx->WsSerializerAllocList) {
		ws_serializer_free_all(serctx);
		list_destroy(serctx->WsSerializerAllocList);
		u_free(serctx);
	}
	return 0;
}

void *xml_serializer_alloc(XmlSerializationData * data, int size,
			   int zeroInit)
{
	void *ptr = ws_serializer_alloc(data->serctx, size);
	TRACE_ENTER;
	if (ptr && zeroInit)
		memset(ptr, 0, size);
	TRACE_EXIT;
	return ptr;
}


int xml_serializer_free(XmlSerializationData * data, void *buf)
{
	return ws_serializer_free(data->serctx, buf);
}


WsXmlNodeH
xml_serializer_add_child(XmlSerializationData * data, char *value)
{
	const char *name = data->elementInfo->name;
	const char *ns = data->elementInfo->ns;
	WsXmlNodeH node;

	TRACE_ENTER;
	debug("name = %s; value(%p) = %s", name ? name : "NULL", value, value ? value : "NULL");
	node = ws_xml_add_child(data->xmlNode, ns, name, value);
	TRACE_EXIT;
	return node;
}



WsXmlNodeH xml_serializer_get_child(XmlSerializationData * data)
{
	WsXmlNodeH node;
	const char *name = data->elementInfo->name;
	const char *ns = data->elementInfo->ns;

	TRACE_ENTER;
	debug("name = %s:%s in %s [%d]", ns, name,
	      ws_xml_get_node_local_name(data->xmlNode), data->index);
	node = ws_xml_get_child(data->xmlNode, data->index, ns, name);
#if 0
	if (g_NameNameAliaseTable) {
		int index = 0;
		while (node == NULL &&
				g_NameNameAliaseTable[index].name != NULL) {
			if (!strcmp(g_NameNameAliaseTable[index].name, name))
			       	node = ws_xml_get_child(data->xmlNode,
						     data->index, ns,
						     g_NameNameAliaseTable
						     [index].aliase);
			index++;
		}
	}
#endif
	debug("returned %p; %s",
	      node, node ? ws_xml_get_node_local_name(node) : "");
	TRACE_EXIT;
	return node;
}

static int get_struct_align(void)
{
	typedef struct {
		XML_TYPE_UINT8 a;
		struct {
			char x;
			void *y;
		} b;
	} dummy;
	return (char *) &(((dummy *) NULL)->b) - (char *) NULL;
}


static int
handle_attrs(struct __XmlSerializationData *data,
	     WsXmlNodeH node, size_t sz)
{
	typedef struct {
		XML_TYPE_UINT8 a;
		XML_NODE_ATTR *b;
	} dummy;

	int ret = 0;
	char *savedBufPtr = DATA_BUF(data);
	size_t al;
	size_t pad;
	int i;
	XML_NODE_ATTR **attrsp;
	XML_NODE_ATTR *attr;
	WsXmlAttrH xmlattr;
	char *src, *dstPtr;
	int dstSize;
	TRACE_ENTER;
	debug("node name = %s", ws_xml_get_node_local_name(node));
	if (!XML_IS_ATTRS(data->elementInfo)) {
		debug("No attrs");
		goto DONE;
	}


	DATA_BUF(data) = DATA_BUF(data) + sz;
	al = (char *) &(((dummy *) NULL)->b) - (char *) NULL;
	pad = (size_t) ((PTRTOINT) DATA_BUF(data) % al);
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
		XML_NODE_ATTR *attrs =
		    *((XML_NODE_ATTR **) DATA_BUF(data));
		debug("attrs = %p", attrs);
		while (attrs) {
			debug("add attr. %s:%s = %s",
			      attrs->ns, attrs->name, attrs->value);
			if (ws_xml_add_node_attr(node,
						 attrs->ns, attrs->name,
						 attrs->value) == NULL) {
				error("could not add attr. %s:%s = %s",
				      attrs->ns, attrs->name,
				      attrs->value);
				ret = 1;
				goto DONE;
			}
			attrs = attrs->next;
		}
		goto DONE;
	}
	// XML_SMODE_DESERIALIZE
	attrsp = (XML_NODE_ATTR **) DATA_BUF(data);
	*attrsp = NULL;
	for (i = 0; i < ws_xml_get_node_attr_count(node); i++) {
		attr =
		    xml_serializer_alloc(data, sizeof(XML_NODE_ATTR), 1);
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
			dstSize = 1 + (int )strlen(src);
			dstPtr =
			    (char *) xml_serializer_alloc(data, dstSize,
							  1);
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
			dstSize = 1 + (int )strlen(src);
			dstPtr =
			    (char *) xml_serializer_alloc(data, dstSize,
							  1);
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
			dstSize = 1 + (int )strlen(src);
			dstPtr =
			    (char *) xml_serializer_alloc(data, dstSize,
							  1);
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

static int do_serialize_int(XmlSerializationData * data, int valSize)
{
	WsXmlNodeH child = NULL;
	XML_TYPE_INT64 tmp;
	int retVal = DATA_ALL_SIZE(data);
	char *end;
	char *str;

	TRACE_ENTER;
	debug("handle %d INT%d %s;", DATA_COUNT(data),
	      8 * valSize, data->elementInfo->name);
	if (DATA_BUF(data) + retVal > data->stopper) {
		error("size of %d structures %s exceeds stopper (%p > %p)",
		      DATA_COUNT(data), DATA_ELNAME(data),
		      DATA_BUF(data) + retVal, data->stopper);
		retVal = WS_ERR_INVALID_PARAMETER;
		goto DONE;
	}
	if (DATA_MUST_BE_SKIPPED(data) || data->mode == XML_SMODE_FREE_MEM) {
		DATA_BUF(data) = DATA_BUF(data) + retVal;
		goto DONE;
	}
	if ((data->mode != XML_SMODE_DESERIALIZE &&
	     data->mode != XML_SMODE_SERIALIZE)) {
		error("invalid mode %d", data->mode);
		retVal = WS_ERR_INVALID_PARAMETER;
		goto DONE;
	}

	for (data->index = 0; data->index < DATA_COUNT(data);
	     data->index++) {
		debug("%s[%d] = %p", data->elementInfo->name, data->index,
		      data->elementBuf);

		if (data->mode == XML_SMODE_SERIALIZE) {
			debug("value size: %d", valSize);
			if (valSize == 1)
				tmp =
				    *((XML_TYPE_INT8 *) data->elementBuf);
			else if (valSize == 2)
				tmp = *((XML_TYPE_INT16 *) data->
				      elementBuf);
			else if (valSize == 4)
				tmp = *((XML_TYPE_INT32 *) data->
				      elementBuf);
			else if (valSize == 8)
				tmp = *((XML_TYPE_INT64 *) data->
				      elementBuf);
			else {
				error("unsupported uint size + %d",
						8 * valSize);
				retVal = WS_ERR_INVALID_PARAMETER;
				goto DONE;
			}

			if (((child = xml_serializer_add_child(data,
						       NULL)) == NULL)
			    || (ws_xml_set_node_long(child, tmp)) != 0) {
				debug("could not add child %s[%d]",
				      DATA_ELNAME(data), data->index);
				retVal = WS_ERR_INSUFFICIENT_RESOURCES;
				goto DONE;
			}
		}
		if (data->mode == XML_SMODE_DESERIALIZE) {
			if ((child = xml_serializer_get_child(data)) == NULL) {
				error ("not enough (%d < %d) instances of element %s",
				     data->index, DATA_COUNT(data),
				     DATA_ELNAME(data));
				retVal = WS_ERR_XML_PARSING;
				goto DONE;
			}
			if ((str = ws_xml_get_node_text(child)) == NULL) {
				error("No text of node %s[%d]",
				      DATA_ELNAME(data), data->index);
				retVal = WS_ERR_XML_PARSING;
				goto DONE;
			}
			while (isspace(str[0]))
				str++;
			errno = 0;
			if (str[0] == 0) {
				if (ws_xml_find_attr_bool(child,
						     XML_NS_SCHEMA_INSTANCE,
						     XML_SCHEMA_NIL)) {
					goto ATTR;
				}
				error("absent value = %s", str);
				retVal = WS_ERR_XML_PARSING;
				goto DONE;
			}
			tmp = strtoll(str, &end, 10);
			if (errno) {
				error("strtoul(%s) failed; errno = %d",
				      str, errno);
				retVal = WS_ERR_XML_PARSING;
				goto DONE;
			}
			while (isspace(end[0]))
				end++;
			if (*end != 0) {
				error("wrong token = %s.", str);
				retVal = WS_ERR_XML_PARSING;
				goto DONE;
			}
			if (valSize == 1) {
				if (tmp > 127 || tmp < -128) {
					error("not int8 = %ld", tmp);
					retVal = WS_ERR_XML_PARSING;
					goto DONE;
				}
				*((XML_TYPE_INT8 *) data->elementBuf) =
				    (XML_TYPE_INT8) tmp;
			} else if (valSize == 2) {
				if (tmp > 32767 || tmp < -32768) {
					error("not int16 = %ld", tmp);
					retVal = WS_ERR_XML_PARSING;
					goto DONE;
				}
				*((XML_TYPE_INT16 *) data->elementBuf) =
				    (XML_TYPE_INT16) tmp;
			} else if (valSize == 4) {
				*((XML_TYPE_INT32 *) data->elementBuf) =
				    (XML_TYPE_INT32) tmp;
			} else if (valSize == 8) {
				*((XML_TYPE_INT64 *) data->elementBuf) =
				    (XML_TYPE_INT64) tmp;
			} else {
				error("unsupported int size + %d",
				      8 * valSize);
				retVal = WS_ERR_INVALID_PARAMETER;
				goto DONE;
			}
		}
ATTR:
		handle_attrs(data, child, valSize);
		DATA_BUF(data) = DATA_BUF(data) + DATA_SIZE(data);
	}
	if ((data->mode == XML_SMODE_DESERIALIZE) &&
	    xml_serializer_get_child(data)) {
		error("too many (%d > %d) instances of element %s",
		      data->index, DATA_COUNT(data), DATA_ELNAME(data));
		retVal = WS_ERR_XML_PARSING;
		goto DONE;
	}
DONE:
	TRACE_EXIT;
	return retVal;
}

static int do_serialize_uint(XmlSerializationData * data, int valSize)
{
	WsXmlNodeH child = NULL;
	XML_TYPE_UINT64 tmp;
	int retVal = DATA_ALL_SIZE(data);
	char *end;
	char *str;

	TRACE_ENTER;
	debug("handle %d UINT%d %s;", DATA_COUNT(data),
	      8 * valSize, data->elementInfo->name);
	if (DATA_BUF(data) + retVal > data->stopper) {
		error("size of %d structures %s exceeds stopper (%p > %p)",
		      DATA_COUNT(data), DATA_ELNAME(data),
		      DATA_BUF(data) + retVal, data->stopper);
		retVal = WS_ERR_INVALID_PARAMETER;
		goto DONE;
	}
	if (DATA_MUST_BE_SKIPPED(data) || data->mode == XML_SMODE_FREE_MEM) {
		DATA_BUF(data) = DATA_BUF(data) + retVal;
		goto DONE;
	}
	if ((data->mode != XML_SMODE_DESERIALIZE &&
	     data->mode != XML_SMODE_SERIALIZE)) {
		error("invalid mode %d", data->mode);
		retVal = WS_ERR_INVALID_PARAMETER;
		goto DONE;
	}

	for (data->index = 0; data->index < DATA_COUNT(data);
	     data->index++) {
		debug("%s[%d] = %p", data->elementInfo->name, data->index,
		      data->elementBuf);

		if (data->mode == XML_SMODE_SERIALIZE) {
			debug("value size: %d", valSize);
			if (valSize == 1)
				tmp =
				    *((XML_TYPE_UINT8 *) data->elementBuf);
			else if (valSize == 2)
				tmp = *((XML_TYPE_UINT16 *) data->
				      elementBuf);
			else if (valSize == 4)
				tmp = *((XML_TYPE_UINT32 *) data->
				      elementBuf);
			else if (valSize == 8)
				tmp = *((XML_TYPE_UINT64 *) data->
				      elementBuf);
			else {
				error("unsupported uint size + %d",
						8 * valSize);
				retVal = WS_ERR_INVALID_PARAMETER;
				goto DONE;
			}

			if (((child = xml_serializer_add_child(data,
						       NULL)) == NULL)
			    || (ws_xml_set_node_ulong(child, tmp)) != 0) {
				debug("could not add child %s[%d]",
				      DATA_ELNAME(data), data->index);
				retVal = WS_ERR_INSUFFICIENT_RESOURCES;
				goto DONE;
			}
		}
		if (data->mode == XML_SMODE_DESERIALIZE) {
			if ((child = xml_serializer_get_child(data)) == NULL) {
				error ("not enough (%d < %d) instances of element %s",
				     data->index, DATA_COUNT(data),
				     DATA_ELNAME(data));
				retVal = WS_ERR_XML_PARSING;
				goto DONE;
			}
			if ((str = ws_xml_get_node_text(child)) == NULL) {
				error("No text of node %s[%d]",
				      DATA_ELNAME(data), data->index);
				retVal = WS_ERR_XML_PARSING;
				goto DONE;
			}
			while (isspace(str[0]))
				str++;
			errno = 0;
			if (str[0] == '-' || str[0] == 0) {
				if (ws_xml_find_attr_bool(child,
						     XML_NS_SCHEMA_INSTANCE,
						     XML_SCHEMA_NIL)) {
					goto ATTR;
				}
				error("absent value = %s", str);
				retVal = WS_ERR_XML_PARSING;
				goto DONE;
			}
			tmp = strtoull(str, &end, 10);
			if (errno) {
				error("strtoul(%s) failed; errno = %d",
				      str, errno);
				retVal = WS_ERR_XML_PARSING;
				goto DONE;
			}
			while (isspace(end[0]))
				end++;
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
				*((XML_TYPE_UINT8 *) data->elementBuf) =
				    (XML_TYPE_UINT8) tmp;
			} else if (valSize == 2) {
				if (tmp > 65535) {
					error("not uint16 = %ld", tmp);
					retVal = WS_ERR_XML_PARSING;
					goto DONE;
				}
				*((XML_TYPE_UINT16 *) data->elementBuf) =
				    (XML_TYPE_UINT16) tmp;
			} else if (valSize == 4) {
				*((XML_TYPE_UINT32 *) data->elementBuf) =
				    (XML_TYPE_UINT32) tmp;
			} else if (valSize == 8) {
				*((XML_TYPE_UINT64 *) data->elementBuf) =
				    (XML_TYPE_UINT64) tmp;
			} else {
				error("unsupported uint size + %d",
				      8 * valSize);
				retVal = WS_ERR_INVALID_PARAMETER;
				goto DONE;
			}
		}
ATTR:
		handle_attrs(data, child, valSize);
		DATA_BUF(data) = DATA_BUF(data) + DATA_SIZE(data);
	}
	if ((data->mode == XML_SMODE_DESERIALIZE) &&
	    xml_serializer_get_child(data)) {
		error("too many (%d > %d) instances of element %s",
		      data->index, DATA_COUNT(data), DATA_ELNAME(data));
		retVal = WS_ERR_XML_PARSING;
		goto DONE;
	}
DONE:
	TRACE_EXIT;
	return retVal;
}



int do_serialize_uint8(XmlSerializationData * data)
{
	if (XML_IS_ATTRS(data->elementInfo)) {
		size_t al = get_struct_align();
		size_t pad = (size_t) ((PTRTOINT) DATA_BUF(data) % al);
		if (pad) {
			pad = al - pad;
		}
		DATA_BUF(data) = DATA_BUF(data) + pad;
	}

	return do_serialize_uint(data, sizeof(XML_TYPE_UINT8));
}


int do_serialize_uint16(XmlSerializationData * data)
{
	typedef struct {
		XML_TYPE_UINT8 a;
		XML_TYPE_UINT16 b;
	} dummy;
	size_t al;
	size_t pad;
	int retVal;
	if (XML_IS_ATTRS(data->elementInfo)) {
		al = get_struct_align();
	} else {
		al = (char *) &(((dummy *) NULL)->b) - (char *) NULL;
	}
	pad = (size_t) ((PTRTOINT) DATA_BUF(data) % al);

	if (pad) {
		pad = al - pad;
	}

	DATA_BUF(data) = DATA_BUF(data) + pad;
	retVal = do_serialize_uint(data, sizeof(XML_TYPE_UINT16));
	if (retVal >= 0) {
		retVal += pad;
	}
	return retVal;
}

int do_serialize_uint32(XmlSerializationData * data)
{
	typedef struct {
		XML_TYPE_UINT8 a;
		XML_TYPE_UINT32 b;
	} dummy;
	size_t al;
	size_t pad;
	int retVal;
	if (XML_IS_ATTRS(data->elementInfo)) {
		al = get_struct_align();
	} else {
		al = (char *) &(((dummy *) NULL)->b) - (char *) NULL;
	}
	pad = (size_t) ((PTRTOINT) DATA_BUF(data) % al);
	if (pad) {
		pad = al - pad;
	}

	DATA_BUF(data) = DATA_BUF(data) + pad;
	retVal = do_serialize_uint(data, sizeof(XML_TYPE_UINT32));
	if (retVal >= 0) {
		retVal += pad;
	}
	return retVal;
}

int do_serialize_uint64(XmlSerializationData * data)
{
	typedef struct {
		XML_TYPE_UINT8 a;
		XML_TYPE_UINT64 b;
	} dummy;
	size_t al;
	size_t pad;
	int retVal;
	if (XML_IS_ATTRS(data->elementInfo)) {
		al = get_struct_align();
	} else {
		al = (char *) &(((dummy *) NULL)->b) - (char *) NULL;
	}
        pad = (size_t) ((PTRTOINT) DATA_BUF(data) % al);
	if (pad) {
		pad = al - pad;
	}

	DATA_BUF(data) = DATA_BUF(data) + pad;
	retVal = do_serialize_uint(data, sizeof(XML_TYPE_UINT64));
	if (retVal >= 0) {
		retVal += pad;
	}
	return retVal;
}

int do_serialize_int8(struct __XmlSerializationData* data)
{
	if (XML_IS_ATTRS(data->elementInfo)) {
		size_t al = get_struct_align();
		size_t pad = (size_t) ((PTRTOINT) DATA_BUF(data) % al);
		if (pad) {
			pad = al - pad;
		}
		DATA_BUF(data) = DATA_BUF(data) + pad;
	}

	return do_serialize_int(data, sizeof(XML_TYPE_INT8));
}
int do_serialize_int16(struct __XmlSerializationData* data)
{
	typedef struct {
		XML_TYPE_UINT8 a;
		XML_TYPE_UINT16 b;
	} dummy;
	size_t al;
	size_t pad;
	int retVal;
	if (XML_IS_ATTRS(data->elementInfo)) {
		al = get_struct_align();
	} else {
		al = (char *) &(((dummy *) NULL)->b) - (char *) NULL;
	}
	pad = (size_t) ((PTRTOINT) DATA_BUF(data) % al);

	if (pad) {
		pad = al - pad;
	}

	DATA_BUF(data) = DATA_BUF(data) + pad;
	retVal = do_serialize_int(data, sizeof(XML_TYPE_INT16));
	if (retVal >= 0) {
		retVal += pad;
	}
	return retVal;
}
int do_serialize_int32(struct __XmlSerializationData* data)
{
	typedef struct {
		XML_TYPE_UINT8 a;
		XML_TYPE_UINT32 b;
	} dummy;
	size_t al;
	size_t pad;
	int retVal;
	if (XML_IS_ATTRS(data->elementInfo)) {
		al = get_struct_align();
	} else {
		al = (char *) &(((dummy *) NULL)->b) - (char *) NULL;
	}
	pad = (size_t) ((PTRTOINT) DATA_BUF(data) % al);
	if (pad) {
		pad = al - pad;
	}

	DATA_BUF(data) = DATA_BUF(data) + pad;
	retVal = do_serialize_int(data, sizeof(XML_TYPE_INT32));
	if (retVal >= 0) {
		retVal += pad;
	}
	return retVal;
}
int do_serialize_int64(struct __XmlSerializationData* data)
{
	typedef struct {
		XML_TYPE_UINT8 a;
		XML_TYPE_UINT64 b;
	} dummy;
	size_t al;
	size_t pad;
	int retVal;
	if (XML_IS_ATTRS(data->elementInfo)) {
		al = get_struct_align();
	} else {
		al = (char *) &(((dummy *) NULL)->b) - (char *) NULL;
	}
        pad = (size_t) ((PTRTOINT) DATA_BUF(data) % al);
	if (pad) {
		pad = al - pad;
	}

	DATA_BUF(data) = DATA_BUF(data) + pad;
	retVal = do_serialize_int(data, sizeof(XML_TYPE_INT64));
	if (retVal >= 0) {
		retVal += pad;
	}
	return retVal;
}

static int do_serialize_real(XmlSerializationData * data, int valSize)
{
	WsXmlNodeH child = NULL;
	XML_TYPE_REAL64 tmp;
	int retVal = DATA_ALL_SIZE(data);
	char *end;
	char *str;

	TRACE_ENTER;
	debug("handle %d REAL%d %s;", DATA_COUNT(data),
	      8 * valSize, data->elementInfo->name);
	if (DATA_BUF(data) + retVal > data->stopper) {
		error("size of %d structures %s exceeds stopper (%p > %p)",
		      DATA_COUNT(data), DATA_ELNAME(data),
		      DATA_BUF(data) + retVal, data->stopper);
		retVal = WS_ERR_INVALID_PARAMETER;
		goto DONE;
	}
	if (DATA_MUST_BE_SKIPPED(data) || data->mode == XML_SMODE_FREE_MEM) {
		DATA_BUF(data) = DATA_BUF(data) + retVal;
		goto DONE;
	}
	if ((data->mode != XML_SMODE_DESERIALIZE &&
	     data->mode != XML_SMODE_SERIALIZE)) {
		error("invalid mode %d", data->mode);
		retVal = WS_ERR_INVALID_PARAMETER;
		goto DONE;
	}

	for (data->index = 0; data->index < DATA_COUNT(data);
	     data->index++) {
		debug("%s[%d] = %p", data->elementInfo->name, data->index,
		      data->elementBuf);

		if (data->mode == XML_SMODE_SERIALIZE) {
			debug("value size: %d", valSize);
			if (valSize == 4)
				tmp = *((XML_TYPE_REAL32 *) data->
				      elementBuf);
			else if (valSize == 8)
				tmp = *((XML_TYPE_REAL64 *) data->
				      elementBuf);
			else {
				error("unsupported real size + %d",
						8 * valSize);
				retVal = WS_ERR_INVALID_PARAMETER;
				goto DONE;
			}

			if (((child = xml_serializer_add_child(data,
						       NULL)) == NULL)
			    || (ws_xml_set_node_real(child, tmp)) != 0) {
				debug("could not add child %s[%d]",
				      DATA_ELNAME(data), data->index);
				retVal = WS_ERR_INSUFFICIENT_RESOURCES;
				goto DONE;
			}
		}
		if (data->mode == XML_SMODE_DESERIALIZE) {
			if ((child = xml_serializer_get_child(data)) == NULL) {
				error ("not enough (%d < %d) instances of element %s",
				     data->index, DATA_COUNT(data),
				     DATA_ELNAME(data));
				retVal = WS_ERR_XML_PARSING;
				goto DONE;
			}
			if ((str = ws_xml_get_node_text(child)) == NULL) {
				error("No text of node %s[%d]",
				      DATA_ELNAME(data), data->index);
				retVal = WS_ERR_XML_PARSING;
				goto DONE;
			}
			while (isspace(str[0]))
				str++;
			errno = 0;
			if (str[0] == 0) {
				if (ws_xml_find_attr_bool(child,
						     XML_NS_SCHEMA_INSTANCE,
						     XML_SCHEMA_NIL)) {
					goto ATTR;
				}
				error("absent value = %s", str);
				retVal = WS_ERR_XML_PARSING;
				goto DONE;
			}
			tmp = strtod(str, &end);
			if (errno) {
				error("strtod(%s) failed; errno = %d",
				      str, errno);
				retVal = WS_ERR_XML_PARSING;
				goto DONE;
			}
			while (isspace(end[0]))
				end++;
			if (*end != 0) {
				error("wrong token = %s.", str);
				retVal = WS_ERR_XML_PARSING;
				goto DONE;
			}
			if (valSize == 4) {
				*((XML_TYPE_REAL32 *) data->elementBuf) =
				    (XML_TYPE_REAL32) tmp;
			} else if (valSize == 8) {
				*((XML_TYPE_REAL64 *) data->elementBuf) =
				    (XML_TYPE_REAL64) tmp;
			} else {
				error("unsupported REAL size + %d",
				      8 * valSize);
				retVal = WS_ERR_INVALID_PARAMETER;
				goto DONE;
			}
		}
ATTR:
		handle_attrs(data, child, valSize);
		DATA_BUF(data) = DATA_BUF(data) + DATA_SIZE(data);
	}
	if ((data->mode == XML_SMODE_DESERIALIZE) &&
	    xml_serializer_get_child(data)) {
		error("too many (%d > %d) instances of element %s",
		      data->index, DATA_COUNT(data), DATA_ELNAME(data));
		retVal = WS_ERR_XML_PARSING;
		goto DONE;
	}
DONE:
	TRACE_EXIT;
	return retVal;
}

int do_serialize_real32(struct __XmlSerializationData* data)
{
	typedef struct {
		XML_TYPE_UINT8 a;
		XML_TYPE_REAL32 b;
	} dummy;
	size_t al;
	size_t pad;
	int retVal;
	if (XML_IS_ATTRS(data->elementInfo)) {
		al = get_struct_align();
	} else {
		al = (char *) &(((dummy *) NULL)->b) - (char *) NULL;
	}
	pad = (size_t) ((PTRTOINT) DATA_BUF(data) % al);
	if (pad) {
		pad = al - pad;
	}

	DATA_BUF(data) = DATA_BUF(data) + pad;
	retVal = do_serialize_real(data, sizeof(XML_TYPE_REAL32));
	if (retVal >= 0) {
		retVal += pad;
	}
	return retVal;
}
int do_serialize_real64(struct __XmlSerializationData* data)
{
	typedef struct {
		XML_TYPE_UINT8 a;
		XML_TYPE_REAL64 b;
	} dummy;
	size_t al;
	size_t pad;
	int retVal;
	if (XML_IS_ATTRS(data->elementInfo)) {
		al = get_struct_align();
	} else {
		al = (char *) &(((dummy *) NULL)->b) - (char *) NULL;
	}
        pad = (size_t) ((PTRTOINT) DATA_BUF(data) % al);
	if (pad) {
		pad = al - pad;
	}
	DATA_BUF(data) = DATA_BUF(data) + pad;
	retVal = do_serialize_real(data, sizeof(XML_TYPE_REAL64));
	if (retVal >= 0) {
		retVal += pad;
	}
	return retVal;
}

int do_serialize_string(XmlSerializationData * data)
{
	WsXmlNodeH child = NULL;
	int retVal = DATA_ALL_SIZE(data);
	size_t al;
	size_t pad;
	typedef struct {
		XML_TYPE_UINT8 a;
		XML_TYPE_STR b;
	} dummy;

	TRACE_ENTER;
	debug("handle %d strings %s = %p", DATA_COUNT(data),
	      data->elementInfo->name, data->elementBuf);
	if (XML_IS_ATTRS(data->elementInfo)) {
		al = get_struct_align();
	} else {
		al = (char *) &(((dummy *) NULL)->b) - (char *) NULL;
	}
	pad = (size_t) ((PTRTOINT) DATA_BUF(data) % al);
	if (pad) {
		pad = al - pad;
	}
	retVal += pad;
	if (DATA_BUF(data) + retVal > data->stopper) {
		error("stopper: %p > %p",
		      DATA_BUF(data) + retVal, data->stopper);
		retVal = WS_ERR_INVALID_PARAMETER;
		goto DONE;
	}
	if (DATA_MUST_BE_SKIPPED(data)) {
		data->elementBuf = DATA_BUF(data) + retVal;
		goto DONE;
	}
	DATA_BUF(data) = DATA_BUF(data) + pad;
	debug("adjusted elementBuf = %p", data->elementBuf);

	for (data->index = 0; data->index < DATA_COUNT(data);
	     data->index++) {
		if (data->mode == XML_SMODE_FREE_MEM) {
			xml_serializer_free(data, *(XML_TYPE_STR *) DATA_BUF(data));
			*(XML_TYPE_STR *) DATA_BUF(data) = NULL;
		} else if (data->mode == XML_SMODE_SERIALIZE) {
			char *valPtr = *((char **) DATA_BUF(data));
			child = xml_serializer_add_child(data, valPtr);
			if (child == NULL) {
				error("xml_serializer_add_child failed.");
				retVal = WS_ERR_INSUFFICIENT_RESOURCES;
				goto DONE;
			}
			if (ws_xml_get_node_text(child) == NULL) {
				ws_xml_add_node_attr(child,
						     XML_NS_SCHEMA_INSTANCE,
						     XML_SCHEMA_NIL,
						     "true");
			}
		} else if (data->mode == XML_SMODE_DESERIALIZE) {
			char *src;
			if ((child =
			     xml_serializer_get_child(data)) == NULL) {
				error
				    ("not enough (%d < %d) instances of element %s",
				     data->index, DATA_COUNT(data),
				     DATA_ELNAME(data));
				retVal = WS_ERR_XML_PARSING;
				goto DONE;
			}

			src = ws_xml_get_node_text(child);
			if (src != NULL && *src != 0) {
				char *dstPtr;
				int dstSize = 1 + strlen(src);
				dstPtr =
				    (char *) xml_serializer_alloc(data,
								  dstSize,
								  1);
				if (dstPtr == NULL) {
					error("no memory");
					retVal =
					    WS_ERR_INSUFFICIENT_RESOURCES;
					goto DONE;
				}
				strncpy(dstPtr, src, dstSize);
				*((XML_TYPE_PTR *) DATA_BUF(data)) =
				    dstPtr;
			}
		} else {
			error("invalid mode");
			retVal = WS_ERR_INVALID_PARAMETER;
			goto DONE;
		}
		handle_attrs(data, child, sizeof(XML_TYPE_STR));
		DATA_BUF(data) = DATA_BUF(data) + DATA_SIZE(data);
	}
	if ((data->mode == XML_SMODE_DESERIALIZE) &&
	    xml_serializer_get_child(data)) {
		error("too many (%d > %d) instances of element %s",
		      data->index, DATA_COUNT(data), DATA_ELNAME(data));
		retVal = WS_ERR_XML_PARSING;
		goto DONE;
	}
      DONE:
	TRACE_EXIT;
	return retVal;
}



int do_serialize_bool(XmlSerializationData * data)
{
	int retVal = DATA_ALL_SIZE(data);
	typedef struct {
		XML_TYPE_UINT8 a;
		XML_TYPE_BOOL b;
	} dummy;
	size_t al;
	size_t pad;
	WsXmlNodeH child = NULL;

	TRACE_ENTER;
	debug("handle %d booleans %s; ptr = %p", DATA_COUNT(data),
	      DATA_ELNAME(data), DATA_BUF(data));
	if (XML_IS_ATTRS(data->elementInfo)) {
		al = get_struct_align();
	} else {
		al = (char *) &(((dummy *) NULL)->b) - (char *) NULL;
	}
	pad = (size_t) ((PTRTOINT) DATA_BUF(data) % al);
	if (pad) {
		pad = al - pad;
	}
	retVal += pad;
	if (DATA_BUF(data) + retVal > data->stopper) {
		retVal = WS_ERR_INVALID_PARAMETER;
		goto DONE;
	}
	if (DATA_MUST_BE_SKIPPED(data) || data->mode == XML_SMODE_FREE_MEM) {
		data->elementBuf = DATA_BUF(data) + retVal;
		goto DONE;
	}
	DATA_BUF(data) = DATA_BUF(data) + pad;
	debug("adjusted elementBuf = %p", data->elementBuf);

	for (data->index = 0; data->index < DATA_COUNT(data);
	     data->index++) {
		debug("%s[%d] = %p", DATA_ELNAME(data), data->index,
		      DATA_BUF(data));
		if (data->mode == XML_SMODE_SERIALIZE) {
			XML_TYPE_BOOL tmp;
			tmp = *((XML_TYPE_BOOL *) DATA_BUF(data));
			if ((child = xml_serializer_add_child(data,
			      (tmp == 0) ? "false" : "true")) == NULL) {
				retVal = WS_ERR_INSUFFICIENT_RESOURCES;
				goto DONE;
			}
		} else if (data->mode == XML_SMODE_DESERIALIZE) {
			XML_TYPE_PTR dataPtr =
			    (XML_TYPE_PTR) DATA_BUF(data);
			XML_TYPE_BOOL tmp = -1;
			char *src;
			if ((child =
			     xml_serializer_get_child(data)) == NULL) {
				error("not enough (%d < %d) instances of element %s",
				     data->index, DATA_COUNT(data), DATA_ELNAME(data));
				retVal = WS_ERR_XML_PARSING;
				goto DONE;
			}
			src = ws_xml_get_node_text(child);
			if (src == NULL || *src == 0) {
				tmp = 1;
			} else {
				if (isdigit(*src)) {
					tmp = atoi(src);
				} else {
					if (strcmp(src, "yes") == 0 ||
						strcmp(src, "true") == 0) {
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
		handle_attrs(data, child, sizeof(XML_TYPE_BOOL));
		DATA_BUF(data) = DATA_BUF(data) + DATA_SIZE(data);
	}
	if ((data->mode == XML_SMODE_DESERIALIZE) &&
	    xml_serializer_get_child(data)) {
		error("too many (%d > %d) instances of element %s",
		      data->index, DATA_COUNT(data), DATA_ELNAME(data));
		retVal = WS_ERR_XML_PARSING;
		goto DONE;
	}
      DONE:
	TRACE_EXIT;
	return retVal;
}



static XmlSerialiseDynamicSizeData *make_dyn_size_data(XmlSerializationData
						       * data,
						       int * retValp)
{
	XmlSerialiseDynamicSizeData *dyn =
	    (XmlSerialiseDynamicSizeData *) data->elementBuf;
	int savedIndex = data->index;
	int size;

	TRACE_ENTER;
	data->index = 0;
	while (xml_serializer_get_child(data) != NULL) {
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
	if ((DATA_MAX_COUNT(data) > 0)
	    && dyn->count > DATA_MAX_COUNT(data)) {
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

	size = DATA_SIZE(data) * dyn->count;
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


int do_serialize_dyn_size_array(XmlSerializationData * data)
{
	typedef struct {
		char a;
		XmlSerialiseDynamicSizeData b;
	} dummy;
	size_t al = (char *) &(((dummy *) NULL)->b) - (char *) NULL;
	size_t pad = (size_t) ((PTRTOINT) DATA_BUF(data) % al);
	int retVal;
	char *savedBufPtr;
	XmlSerializerInfo *savedElementInfo;
	XmlSerialiseDynamicSizeData *dyn = NULL;
	int tmp;
	XmlSerializerInfo myinfo;
	int savedIndex;
	char *savedStopper;
	TRACE_ENTER;
	debug("Dyn size array %s; ptr = %p", data->elementInfo->name,
	      data->elementBuf);

	if (pad)
		pad = al - pad;

	retVal = DATA_SIZE(data) + pad;
	if (DATA_BUF(data) + retVal > data->stopper) {
		retVal = WS_ERR_INVALID_PARAMETER;
		goto DONE;
	}
	if (DATA_MUST_BE_SKIPPED(data)) {
		DATA_BUF(data) = DATA_BUF(data) + retVal;
		goto DONE;
	}
	DATA_BUF(data) = DATA_BUF(data) + pad;
	debug("adjusted elementBuf = %p", data->elementBuf);

	savedBufPtr = DATA_BUF(data);
	savedElementInfo = data->elementInfo;

	if (data->mode != XML_SMODE_SERIALIZE &&
	    data->mode != XML_SMODE_DESERIALIZE &&
	    data->mode != XML_SMODE_FREE_MEM) {
		retVal = WS_ERR_INVALID_PARAMETER;
		goto DONE;
	}

	if (data->mode == XML_SMODE_DESERIALIZE) {
		if ((dyn = make_dyn_size_data(data, &retVal)) == NULL) {
			goto DONE;
		}
	} else {
		dyn = (XmlSerialiseDynamicSizeData *) data->elementBuf;
		if (data->mode == XML_SMODE_SERIALIZE) {
			if (dyn->count < DATA_MIN_COUNT(data)) {
				error("not enough (%d < %d) elements %s",
				      dyn->count, DATA_MIN_COUNT(data),
				      DATA_ELNAME(data));
				retVal = WS_ERR_XML_PARSING;
				goto DONE;
			}
			if ((DATA_MAX_COUNT(data) > 0)
			    && dyn->count > DATA_MAX_COUNT(data)) {
				error("too many (%d > %d) elements %s",
				      dyn->count, DATA_MAX_COUNT(data),
				      DATA_ELNAME(data));
				retVal = WS_ERR_XML_PARSING;
				goto DONE;
			}
		}
	}

	if (dyn->count == 0) {
		// no dynamic data. nothing to do
		goto DONE;
	}

	savedIndex = data->index;
	savedStopper = data->stopper;
	memcpy(&myinfo, savedElementInfo->extData,
	       sizeof(XmlSerializerInfo));
	myinfo.count = dyn->count;
	myinfo.name = data->elementInfo->name;
	myinfo.ns = data->elementInfo->ns;

	data->stopper = (char *) dyn->data + DATA_SIZE(data) * dyn->count;
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




int do_serialize_struct(XmlSerializationData * data)
{
	int retVal = 0;
	int i;
	int elementCount = 0;
	XmlSerializerInfo *elements =
	    (XmlSerializerInfo *) data->elementInfo->extData;
	WsXmlNodeH savedXmlNode = data->xmlNode;
	XmlSerializerInfo *savedElement = data->elementInfo;
	int savedMode = data->mode;
	int savedIndex = data->index;
	void *savedStopper = data->stopper;
	size_t al = get_struct_align();
	size_t pad = (size_t) ((PTRTOINT) DATA_BUF(data) % al);
	size_t count;
	size_t struct_size;
	int savedLocalIndex;
	char *savedLocalElementBuf;
	WsXmlNodeH child;

	TRACE_ENTER;

	debug("handle %d structure \"%s\" ptr = %p", DATA_COUNT(data),
	      data->elementInfo->name ? data->elementInfo->name : "NULL",
	      data->elementBuf);
	if (data->mode != XML_SMODE_SERIALIZE &&
	    data->mode != XML_SMODE_DESERIALIZE &&
	    data->mode != XML_SMODE_FREE_MEM) {
		retVal = WS_ERR_INVALID_PARAMETER;
		debug("Incorrect data->mode = %d", data->mode);
		goto DONE;
	}
	al = get_struct_align();
	pad = (size_t) ((PTRTOINT) DATA_BUF(data) % al);
	if (pad) {
		pad = al - pad;
	}
	retVal = pad + XML_IS_HEAD(savedElement) ?
	    DATA_SIZE(data) : DATA_ALL_SIZE(data);
	if ((char *) DATA_BUF(data) + retVal > data->stopper) {
		error("size of %d structures \"%s\" exceeds stopper (%p > %p)",
		      DATA_COUNT(data), DATA_ELNAME(data) ? DATA_ELNAME(data) : "NULL",
		      (char *) DATA_BUF(data) + retVal, data->stopper);
		retVal = WS_ERR_INVALID_PARAMETER;
		goto DONE;
	}
	if (DATA_MUST_BE_SKIPPED(data)) {
		debug(" %d elements %s skipped", DATA_COUNT(data),
		      DATA_ELNAME(data) ? DATA_ELNAME(data) : "NULL");
		DATA_BUF(data) = DATA_BUF(data) + retVal;
		goto DONE;
	}
	DATA_BUF(data) = DATA_BUF(data) + pad;
	debug("adjusted ptr= %p", data->elementBuf);

	while (elements[elementCount].proc != NULL) {
		elementCount++;
	}

	count = DATA_COUNT(data);
	struct_size = DATA_SIZE(data);
	if (XML_IS_HEAD(savedElement)) {
		count = data->index + 1;
	} else {
		data->index = 0;
	}

	for (; data->index < count; data->index++) {
		int tmp;
		child = NULL;
		savedLocalIndex = data->index;
		savedLocalElementBuf = DATA_BUF(data);
		data->stopper = savedLocalElementBuf + DATA_SIZE(data);
		debug("%s[%d] = %p", DATA_ELNAME(data) ? DATA_ELNAME(data) : "NULL",
		      data->index,
		      DATA_BUF(data));
		if (data->mode == XML_SMODE_SERIALIZE) {
			child = xml_serializer_add_child(data, NULL);
			data->xmlNode = child;
			if (data->xmlNode == NULL) {
				error("cant add child");
				retVal = WS_ERR_INSUFFICIENT_RESOURCES;
				goto DONE;
			}
		} else if (data->mode == XML_SMODE_DESERIALIZE) {
			child = xml_serializer_get_child(data);
			if (child == NULL) {
				error
				    ("not enough (%d < %d) instances of element %s",
				     data->index, DATA_COUNT(data),
				     DATA_ELNAME(data) ? DATA_ELNAME(data) : "NULL");
				retVal = WS_ERR_XML_PARSING;
				goto DONE;
			}
			data->xmlNode = child;
		}

		debug("before for loop. Struct %s = %p",
		      savedElement->name ? savedElement->name : "NULL", DATA_BUF(data));

		for (i = 0; retVal > 0 && i < elementCount; i++) {
			data->elementInfo = &elements[i];
			debug
			    ("handle %d elements %s of struct %s. dstPtr = %p",
			     DATA_COUNT(data), DATA_ELNAME(data) ? DATA_ELNAME(data) : "NULL",
			     savedElement->name ? savedElement->name : "NULL", DATA_BUF(data));
			if (XML_IS_SKIP(data->elementInfo)) {
				data->mode = XML_SMODE_SKIP;
			}

			tmp = data->elementInfo->proc(data);
			debug("retval: %d", tmp);

			if (tmp < 0) {
				error("handling of element \"%s\" failed = %d",
				      DATA_ELNAME(data) ? DATA_ELNAME(data) : "NULL", tmp);
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
initialize_xml_serialization_data(XmlSerializationData * data,
				  WsSerializerContextH serctx,
				  XmlSerializerInfo * elementInfo,
				  XML_TYPE_PTR dataBuf,
				  int mode,
				  XML_NODE_ATTR * attrs,
				  WsXmlNodeH xmlNode)
{
	debug("Initialize XML Serialization...");
	TRACE_ENTER;
	memset(data, 0, sizeof(XmlSerializationData));
	data->serctx = serctx;
	data->elementInfo = elementInfo;
	data->elementBuf = dataBuf;
	data->mode = mode;
	data->attrs = attrs;
	data->xmlNode = xmlNode;

	debug("Finished initializing XML Serialization...");
	TRACE_EXIT;
	return;
}



int ws_serialize(WsSerializerContextH serctx,
		    WsXmlNodeH xmlNode,
		    XML_TYPE_PTR dataPtr,
		    XmlSerializerInfo * info,
		    const char *name,
		    const char *ns, XML_NODE_ATTR * attrs, int output)
{
	int retVal = WS_ERR_INSUFFICIENT_RESOURCES;
	XmlSerializerInfo myinfo;
	XmlSerializationData data;

	TRACE_ENTER;
	if (info->proc == NULL) {
		error("info->proc == NULL");
		goto DONE;
	}
	memcpy(&myinfo, info, sizeof(XmlSerializerInfo));
	if (name == NULL) {
		error("name == NULL");
		goto DONE;
	}
	myinfo.name = name;
	myinfo.ns = ns;
	myinfo.flags |= SER_HEAD;
	initialize_xml_serialization_data(&data,
					  serctx,
					  &myinfo,
					  dataPtr,
					  XML_SMODE_SERIALIZE,
					  attrs, xmlNode);

	data.stopper = (char *) dataPtr + myinfo.size;
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


int ws_serializer_free_mem(WsSerializerContextH serctx, XML_TYPE_PTR buf,
			      XmlSerializerInfo * info)
{
	int retVal;
	XmlSerializationData data;
	XmlSerializerInfo myinfo;

	TRACE_ENTER;
	memcpy(&myinfo, info, sizeof(XmlSerializerInfo));
	myinfo.flags |= SER_HEAD;
	initialize_xml_serialization_data(&data,
					  serctx,
					  &myinfo,
					  buf,
					  XML_SMODE_FREE_MEM, NULL, NULL);

	data.stopper = (char *) buf + myinfo.size;
	if ((retVal = info->proc(&data)) >= 0) {
		xml_serializer_free(&data, buf);
	}
	TRACE_EXIT;
	return retVal;
}


void *ws_deserialize(WsSerializerContextH serctx,
		     WsXmlNodeH xmlParent,
		     XmlSerializerInfo * info,
		     const char *name,
		     const char *ns,
		     XML_NODE_ATTR ** attrs, int index, int output)
{
	int size;
	void *retPtr = NULL;
	XmlSerializationData data;
	XmlSerializerInfo myinfo;

	TRACE_ENTER;
	memcpy(&myinfo, info, sizeof(XmlSerializerInfo));
	if (name == NULL) {
		error("name == NULL");
	}
	myinfo.name = name;
	myinfo.ns = ns;
	myinfo.flags |= SER_HEAD;
	initialize_xml_serialization_data(&data, serctx, &myinfo, NULL,
					  XML_SMODE_DESERIALIZE,
					  NULL, xmlParent);

	data.index = index;

	if (output) {
		data.skipFlag = SER_IN;
	} else {
		data.skipFlag = SER_OUT;
	}

	size = myinfo.size;
	if ((data.elementBuf = xml_serializer_alloc(&data, size, 1)) != NULL) {
		retPtr = data.elementBuf;
		data.stopper = (char *) retPtr + size;
		if (myinfo.proc && myinfo.proc(&data) <= 0) {
			data.elementBuf = retPtr;
			retPtr = NULL;
			ws_serializer_free_mem(serctx, data.elementBuf,
					       &myinfo);
			error("Error during serialization");
		}
	}
	TRACE_EXIT;
	return retPtr;
}

static void enforce_mustunderstand(WsXmlNodeH node)
{
	WsXmlDocH doc = ws_xml_get_node_doc(node);
	char *ns = ws_xml_get_node_name_ns(ws_xml_get_doc_root(doc));
	ws_xml_add_node_attr(node, ns, SOAP_MUST_UNDERSTAND, "true");
}

int
ws_serialize_str(WsSerializerContextH serctx, WsXmlNodeH parent, const char *str,
		 const char *nameNs, const char *name, int mustunderstand)
{
	WsXmlNodeH node;
	TRACE_ENTER;
	//printf("mustunderstand flag for %s: %d\n", name, mustunderstand );
	node = ws_xml_add_child(parent, nameNs, name, str);
	if (node && mustunderstand) {
		enforce_mustunderstand(node);
	}
	TRACE_EXIT;
	return (node == NULL);
}


int ws_serialize_uint32(WsSerializerContextH serctx, WsXmlNodeH parent,
			unsigned long val, const char *nameNs,
			const char *name, int mustunderstand)
{
	WsXmlNodeH node = ws_xml_add_child(parent, nameNs, name, NULL);
	TRACE_ENTER;
	if (node) {
		ws_xml_set_node_ulong(node, val);
		if (mustunderstand) {
			enforce_mustunderstand(node);
		}
	}
	TRACE_EXIT;
	return (node == NULL);
}


char *ws_deserialize_str(WsSerializerContextH serctx, WsXmlNodeH parent, int index,
			 const char *nameNs, const char *name)
{
	char *str = NULL;
	WsXmlNodeH node = ws_xml_get_child(parent, index, nameNs, name);
	TRACE_ENTER;
	if (node) {
		str = ws_xml_get_node_text(node);
		if (serctx && str) {
			int len = (int )strlen(str) + 1;
			char *tmp = str;
			if ((str = ws_serializer_alloc(serctx,
						 len * sizeof(char))))
				strcpy(str, tmp);
		}
	}
	TRACE_EXIT;
	return str;
}

unsigned long ws_deserialize_uint32(WsSerializerContextH serctx,
				    WsXmlNodeH parent, int index,
				    const char *nameNs, const char *name)
{
	unsigned long val = 0;
	WsXmlNodeH node = ws_xml_get_child(parent, index, nameNs, name);
	TRACE_ENTER;
	if (node) {
		val = ws_xml_get_node_ulong(node);
	}
	TRACE_EXIT;
	return val;
}

/*
 * Returns duration in seconds in <value> argument
 *
 *  the format of OperationTimeout is defined by the
 *  XML Schema Datatypes Section 3.2.6.1 http://www.w3.org/TR/xmlschema-2/
 * 
 * Sample: P0Y0M0DT0H0M60.00S
 * 
 * (must start with 'P', 'T' separated date from time, seconds may have fraction)
 * 
 * FIXME: handle fractions of seconds (don't return a time_t but a struct timeval)
 * 
 */
int ws_deserialize_duration(const char *t, time_t * value)
{
	long years = 0;
	long months = 0;
	long days = 0;
	long hours = 0;
	long mins = 0;
	long secs = 0;
	time_t vs;
	int v;
	double f;               /* float for fractional second */
	char *e;                /* end pointer for strtol */
        int got = 64;           /* bitmask of parsing position, 64==year */
	int res = 1;            /* final result, 1 == error */
	int time_handeled = 0;  /* seen 'T' */
	int negative = 0;

	TRACE_ENTER;
	if (t == NULL) {
		debug("node text == NULL");
		goto DONE;
	}

	if (*t == '-') {
		negative = 1;
		t++;
	}
	if (*t != 'P') {
		debug("Wrong begining of duration");
		goto DONE;
	}
	while (*++t) {
		if (*t == 'T') {
			time_handeled = 1;
			continue;
		}
		v = strtol(t, &e, 10);
		if (t == e) {
			debug("wrong format, missing numeric value");
			goto DONE;
		}
		if (time_handeled && (*e == '.')) {
			f = strtod(t, &e);
			if (*e != 'S') {
				debug("float but not secs");
				goto DONE;
			}
			if (((int)floor(f+0.5)) > v) {
				v++;  /* round value up */
			}
		}
		switch (*e) {
		case 'Y':
			if (got <= 32 || time_handeled) {
				debug("wrong order Y");
				goto DONE;
			}
			years = v;
			got = 32;
			break;
		case 'M':
			if (!time_handeled) {	// months
				if (got <= 16) {
					debug("wrong order M");
					goto DONE;
				}
				months = v;
				got = 16;
				break;
			}
			// minutes
			if (got <= 2 || !time_handeled) {
				debug("wrong order M");
				goto DONE;
			}
			mins = v;
			got = 2;
			break;
		case 'D':
			if (got <= 8 || time_handeled) {
				debug("wrong order D");
				goto DONE;
			}
			days = v;
			got = 8;
			break;
		case 'H':
			if (got <= 4 || !time_handeled) {
				debug("wrong order H");
				goto DONE;
			}
			hours = v;
			got = 4;
			break;
		case 'S':
			if (got <= 1 || !time_handeled) {
				debug("wrong order S");
				goto DONE;
			}
			secs = v;
			got = 1;
			break;
		default:
			debug("wrong format %c", *e);
			goto DONE;
		}
		t = e;
	}

	// invalid if T found and no HMS, or no time value detected
	if ((time_handeled && (got > 4)) || (64 == got)){
		debug("wrong format: floating T or no time value detected");
		goto DONE;
	}

	// We don't know exact date and time of the sender.
	// For simplicity comsider 1 month = 30days;

	vs = secs + 60 * mins + 60 * 60 * hours + 60 * 60 * 24 * days +
	    60 * 60 * 24 * 30 * months + 60 * 60 * 24 * 30 * 12 * years;
	if (negative) {
		vs = 0 - vs;
	}
	*value = vs;
	res = 0; // good at this point

      DONE:
	TRACE_EXIT;
	return res;
}


int ws_deserialize_datetime(const char *text, XML_DATETIME * tmx)
{
	int res = 0;
	int r;
	int hours;
	int mins;
	struct tm tm;
	time_t t;

	TRACE_ENTER;
	if (text == NULL) {
		debug("node text == NULL");
		res = 1;
		goto DONE;
	}
	bzero(tmx, sizeof(XML_DATETIME));

	r = sscanf(text, "%u-%u-%uT%u:%u:%u%d:%u", &tmx->tm.tm_year,
		   &tmx->tm.tm_mon, &tmx->tm.tm_mday,
		   &tmx->tm.tm_hour, &tmx->tm.tm_min, &tmx->tm.tm_sec,
		   &hours, &mins);
	if (r != 8) {
		debug("wrong body of datetime(%d): %s", r, text);
		res = 1;
		goto DONE;
	}
	tmx->tm.tm_year -= 1900;
	tmx->tm.tm_mon -= 1;

	t = time(NULL);
#ifdef _WIN32
	localtime_s(&tm, &t);
#else
	localtime_r(&t, &tm);
#endif
	tmx->tm.tm_isdst = tm.tm_isdst;

	if (hours < 0) {
		tmx->tz_min = 60 * hours - mins;
	} else {
		tmx->tz_min = 60 * hours + mins;
	}
      DONE:
	return res;
}



void *ws_serializer_alloc(WsSerializerContextH serctx, int size)
{
	WsSerializerMemEntry *ptr = NULL;
	TRACE_ENTER;
	if ((ptr = (WsSerializerMemEntry *) u_malloc(sizeof(WsSerializerMemEntry) + size)) != NULL) {
		lnode_t *node;
		u_lock(serctx);
		if ((node = lnode_create(ptr)) == NULL) {
			u_free(ptr);
			ptr = NULL;
		} else {
			list_append(serctx->WsSerializerAllocList, node);
		}
		u_unlock(serctx);
	}
	TRACE_EXIT;
	return ptr ? ptr->buf : NULL;
}


static int do_serializer_free(WsSerializerContextH serctx, void *ptr)
{
	lnode_t *node = NULL;
	lnode_t *node2 = NULL;
	TRACE_ENTER;
	if (serctx) {
		u_lock(serctx);
		node = list_first(serctx->WsSerializerAllocList);
		while (node != NULL) {
			WsSerializerMemEntry *entry =
			    (WsSerializerMemEntry *) node->list_data;

			if (entry && (!ptr || ptr == entry->buf)) {
				u_free(entry);
				node2 = node;
				node = list_delete2(serctx->WsSerializerAllocList, node);
				lnode_destroy (node2);
				if (ptr != NULL) {
					break;
				}
			}
			else
				node = list_next(serctx->WsSerializerAllocList, node);
		}
		u_unlock(serctx);
	}
	TRACE_EXIT;
	return (node != NULL);
}

int ws_serializer_free(WsSerializerContextH serctx, void *ptr)
{
	int retVal = 0;
	TRACE_ENTER;
	if (ptr != NULL)
		retVal = do_serializer_free(serctx, ptr);
	TRACE_EXIT;
	return retVal;
}

void ws_serializer_free_all(WsSerializerContextH serctx)
{
	TRACE_ENTER;
	do_serializer_free(serctx, NULL);
	TRACE_EXIT;
}

int ws_havenilvalue(XML_NODE_ATTR *attrs) 
{
	while(attrs) { 
		if(attrs->ns && attrs->name && attrs->value && 
			strcmp(attrs->ns, XML_NS_SCHEMA_INSTANCE) == 0 && 
			strcmp(attrs->name, XML_SCHEMA_NIL) == 0 && 
			strcasecmp(attrs->value, "true") == 0) 
			return 1; 
		attrs = attrs->next; 
	} 
	return 0; 
} 

