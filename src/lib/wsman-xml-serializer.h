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

/**
 * @defgroup XMLSerializer XML Serializer
 * @brief Serializer and Deserializer Functions and Macros
 * 
 * @{
 */
 
#ifndef WS_XML_SERIALIZER_H
#define WS_XML_SERIALIZER_H

#include "wsman-types.h"
#include "wsman-names.h"

typedef void* XML_TYPE_PTR;

typedef struct __XmlSerialiseDynamicSizeData
{
    unsigned int count;
    XML_TYPE_PTR data;
}  XmlSerialiseDynamicSizeData;

#ifdef WIN32
typedef unsigned char XML_TYPE_UINT8;
typedef unsigned short XML_TYPE_UINT16;
typedef unsigned long XML_TYPE_UINT32;
typedef int XML_TYPE_BOOL;
typedef char XML_TYPE_CHAR;
#define PTRTOINT unsigned long long
#else
#include <sys/types.h>
typedef u_int8_t XML_TYPE_UINT8;
typedef u_int16_t XML_TYPE_UINT16;
typedef u_int32_t XML_TYPE_UINT32;
typedef int XML_TYPE_BOOL;
typedef char XML_TYPE_CHAR;
#define PTRTOINT unsigned long
#endif
typedef struct {
    struct tm tm;
    int    tz_min; // timezone GMT shift in minutes 
} XML_DATETIME;
typedef char* XML_TYPE_STR;
typedef XmlSerialiseDynamicSizeData XML_TYPE_DYN_ARRAY;
struct _XML_NODE_ATTR;
struct _XML_NODE_ATTR {
    struct _XML_NODE_ATTR *next;
    char *ns;
    char *name;
    char *value;
};
typedef struct _XML_NODE_ATTR XML_NODE_ATTR;

#define SER_ENUMERATOR_PREFIX	"_XmlSerializeEnum"




/**
 * Count can be 1 for single elementd; > 1 for fixed size array; 
 * 0 for dynamic array. 
 */



struct __XmlSerializationData;
typedef size_t (*XmlSerializationProc)(struct __XmlSerializationData* data);

struct __XmlSerializerInfo
{
    char *ns;
    char* name;
    size_t mincount;    /**< Minimal Count */
    size_t count;       /**< Maximal Count */
    size_t size;        /**< size of serialized/deserialized element */
    unsigned int flags; /**< flags */
    XmlSerializationProc proc; /**< Serialization Processor */
    XML_TYPE_PTR extData;      /**< External Data */
};
typedef struct __XmlSerializerInfo XmlSerializerInfo;



#define SER_IN            0x8000
#define SER_OUT           0x4000
#define SER_SKIP          0x2000
#define SER_HEAD          0x1000
#define SER_ATTRS         0x0800

#define XML_NUM_OCCURS(x) (x)->count
#define XML_MAX_OCCURS(x) (x)->count
#define XML_MIN_OCCURS(x) (x)->mincount
#define XML_IS_SKIP(x) ((x)->flags & SER_SKIP)
#define XML_IS_IN(x)   ((x)->flags & SER_IN)
#define XML_IS_OUT(x)  ((x)->flags & SER_OUT)
#define XML_IS_HEAD(x)  ((x)->flags & SER_HEAD)
#define XML_IS_ATTRS(x)  ((x)->flags & SER_ATTRS)


#define SER_NULL {NULL, NULL, 0, 0, 0, 0, NULL, NULL}

        // Serializer Info base defines

#define SER_NS_UINT8_FLAGS(ns, n, x, flags) \
    {(ns), (n), (x), (x),  sizeof (XML_TYPE_UINT8), \
        flags, do_serialize_uint8, NULL \
    }
#define SER_NS_UINT16_FLAGS(ns, n, x, flags) \
    {(ns), (n), (x), (x), sizeof (XML_TYPE_UINT16), \
        flags, do_serialize_uint16, NULL \
    }
#define SER_NS_UINT32_FLAGS(ns, n, x, flags) \
    {(ns), (n), (x), (x), sizeof (XML_TYPE_UINT32), \
        flags, do_serialize_uint32, NULL \
    }
#define SER_NS_BOOL_FLAGS(ns, n, x, flags) \
    {(ns), (n), (x), (x), sizeof (XML_TYPE_BOOL), \
        flags, do_serialize_bool, NULL \
    }
#define SER_NS_STR_FLAGS(ns, n, x, flags) \
    {(ns), (n), (x), (x), sizeof (XML_TYPE_STR), \
        flags, do_serialize_string, NULL \
    }
#define SER_NS_STRUCT_FLAGS(ns, n, x, flags, t) \
    {(ns), (n), (x), (x), sizeof (t), flags, \
        do_serialize_struct, t##_TypeItems \
    }

#define SER_NS_DYN_ARRAY_FLAGS(ns, n, min, max, flags, t)\
    {(ns), (n), (min), (max), sizeof (XmlSerialiseDynamicSizeData), flags, \
         do_serialize_dyn_size_array, t##_TypeInfo \
    }


            // nodes with attributes
#define SER_ATTR_NS_UINT8_FLAGS(ns, n, x, flags) \
    {(ns), (n), (x), (x), sizeof (struct {XML_TYPE_UINT8 s; XML_NODE_ATTR *a;}), \
        flags | SER_ATTRS, do_serialize_uint8, NULL \
    }
#define SER_ATTR_NS_UINT16_FLAGS(ns, n, x, flags) \
    {(ns), (n), (x), (x), sizeof (struct {XML_TYPE_UINT16 s; XML_NODE_ATTR *a;}), \
        flags | SER_ATTRS, do_serialize_uint16, NULL \
    }
#define SER_ATTR_NS_UINT32_FLAGS(ns, n, x, flags) \
    {(ns), (n), (x), (x), sizeof (struct {XML_TYPE_UINT32 s; XML_NODE_ATTR *a;}), \
        flags | SER_ATTRS, do_serialize_uint32, NULL \
    }
#define SER_ATTR_NS_BOOL_FLAGS(ns, n, x, flags) \
    {(ns), (n), (x), (x), sizeof (struct {XML_TYPE_BOOL s; XML_NODE_ATTR *a;}), \
        flags | SER_ATTRS, do_serialize_bool, NULL \
    }
#define SER_ATTR_NS_STR_FLAGS(ns, n, x, flags) \
    {(ns), (n), (x), (x), sizeof (struct {XML_TYPE_STR s; XML_NODE_ATTR *a;}), \
        flags | SER_ATTRS, do_serialize_string, NULL \
    }
#define SER_ATTR_NS_STRUCT_FLAGS(ns, n, x, flags, t) \
    {(ns), (n), (x), (x), sizeof (struct {t s; XML_NODE_ATTR *a;}), \
        flags | SER_ATTRS, do_serialize_struct, t##_TypeItems \
    }


        // No namespace defines
#define SER_UINT8_FLAGS(n, x, flags)  SER_NS_UINT8_FLAGS(NULL, n, x, flags)
#define SER_UINT16_FLAGS(n, x, flags) SER_NS_UINT16_FLAGS(NULL, n, x, flags)
#define SER_UINT32_FLAGS(n, x, flags) SER_NS_UINT32_FLAGS(NULL, n, x, flags)
#define SER_BOOL_FLAGS(n, x, flags)   SER_NS_BOOL_FLAGS(NULL, n, x, flags)
#define SER_STR_FLAGS(n, x, flags)    SER_NS_STR_FLAGS(NULL, n, x, flags)
#define SER_STRUCT_FLAGS(n, x, flags, t) \
                        SER_NS_STRUCT_FLAGS(NULL, n, x, flags, t)
#define SER_DYN_ARRAY_FLAGS(n, min, max, flags, t) \
                        SER_NS_DYN_ARRAY_FLAGS(NULL, n, min, max, flags, t)


        // Serialization Info for different types
#define SER_UINT8(n, x)               SER_UINT8_FLAGS(n, x, 0)
#define SER_UINT16(n, x)              SER_UINT16_FLAGS(n, x, 0)
#define SER_UINT32(n, x)              SER_UINT32_FLAGS(n, x, 0)
#define SER_BOOL(n, x)                SER_BOOL_FLAGS(n, x, 0)
#define SER_STR(n, x)                 SER_STR_FLAGS(n, x, 0)
#define SER_STRUCT(n, x, t)           SER_STRUCT_FLAGS(n, x, 0, t)
#define SER_DYN_ARRAY(n, mn, mx, t)   SER_DYN_ARRAY_FLAGS(n, mn, mx, 0, t)
#define SER_NS_UINT8(ns, n, x)        SER_NS_UINT8_FLAGS(ns, n, x, 0)
#define SER_NS_UINT16(ns, n, x)       SER_NS_UINT16_FLAGS(ns, n, x, 0)
#define SER_NS_UINT32(ns, n, x)       SER_NS_UINT32_FLAGS(ns, n, x, 0)
#define SER_NS_BOOL(ns, n, x)         SER_NS_BOOL_FLAGS(ns, n, x, 0)
#define SER_NS_STR(ns, n, x)          SER_NS_STR_FLAGS(ns, n, x, 0)
#define SER_NS_STRUCT(ns, n, x, t)    SER_NS_STRUCT_FLAGS(ns, n, x, 0, t)
#define SER_NS_DYN_ARRAY(ns, n, mn, mx, t)   \
                                 SER_NS_DYN_ARRAY_FLAGS(ns, n, mn, mx, 0, t)


        // Serialization Info to skip in 

#define SER_IN_UINT8(n, x)            SER_UINT8_FLAGS(n, x, SER_IN)
#define SER_IN_UINT16(n, x)           SER_UINT16_FLAGS(n, x, SER_IN)
#define SER_IN_UINT32(n, x)           SER_UINT32_FLAGS(n, x, SER_IN)
#define SER_IN_BOOL(n, x)             SER_BOOL_FLAGS(n, x, SER_IN)
#define SER_IN_STR(n, x)              SER_STR_FLAGS(n, x, SER_IN)
#define SER_IN_STRUCT(n, x, t)        SER_STRUCT_FLAGS(n, x, SER_IN, t)
#define SER_IN_DYN_ARRAY(n, mn, mx, t)  \
                               SER_DYN_ARRAY_FLAGS(n, mn, mx, SER_IN, t)
#define SER_NS_IN_UINT8(ns, n, x)     SER_NS_UINT8_FLAGS(ns, n, x, SER_IN)
#define SER_NS_IN_UINT16(ns, n, x)    SER_NS_UINT16_FLAGS(ns, n, x, SER_IN)
#define SER_NS_IN_UINT32(ns, n, x)    SER_NS_UINT32_FLAGS(ns, n, x, SER_IN)
#define SER_NS_IN_BOOL(ns, n, x)      SER_NS_BOOL_FLAGS(ns, n, x, SER_IN)
#define SER_NS_IN_STR(ns, n, x)       SER_NS_STR_FLAGS(ns, n, x, SER_IN)
#define SER_NS_IN_STRUCT(ns, n, x, t) SER_NS_STRUCT_FLAGS(ns, n, x, SER_IN, t)
#define SER_NS_IN_DYN_ARRAY(ns, n, mn, mx, t)  \
                               SER_NS_DYN_ARRAY_FLAGS(ns, n, mn, mx, SER_IN, t)

        // Serialization Info to skip out
#define SER_OUT_UINT8(n, x)           SER_UINT8_FLAGS(n, x, SER_OUT)
#define SER_OUT_UINT16(n, x)          SER_UINT16_FLAGS(n, x, SER_OUT)
#define SER_OUT_UINT32(n, x)          SER_UINT32_FLAGS(n, x, SER_OUT)
#define SER_OUT_BOOL(n, x)            SER_BOOL_FLAGS(n, x, SER_OUT)
#define SER_OUT_STR(n, x)             SER_STR_FLAGS(n, x, SER_OUT)
#define SER_OUT_STRUCT(n, x, t)       SER_STRUCT_FLAGS(n, x, SER_OUT, t)
#define SER_OUT_DYN_ARRAY(n, mn, mx, t)   \
                                      SER_DYN_ARRAY_FLAGS(n, mn, mx, SER_OUT, t)
#define SER_NS_OUT_UINT8(ns, n, x)     SER_NS_UINT8_FLAGS(ns, n, x, SER_OUT)
#define SER_NS_OUT_UINT16(ns, n, x)    SER_NS_UINT16_FLAGS(ns, n, x, SER_OUT)
#define SER_NS_OUT_UINT32(ns, n, x)    SER_NS_UINT32_FLAGS(ns, n, x, SER_OUT)
#define SER_NS_OUT_BOOL(ns, n, x)      SER_NS_BOOL_FLAGS(ns, n, x, SER_OUT)
#define SER_NS_OUT_STR(ns, n, x)       SER_NS_STR_FLAGS(ns, n, x, SER_OUT)
#define SER_NS_OUT_STRUCT(ns, n, x, t) SER_NS_STRUCT_FLAGS(ns, n, x, SER_OUT, t)
#define SER_NS_OUT_DYN_ARRAY(ns, n, mn, mx, t)   \
                              SER_NS_DYN_ARRAY_FLAGS(ns, n, mn, mx, SER_OUT, t)


        // Serialization Info to skip inout
#define SER_INOUT_UINT8(n, x)      SER_UINT8_FLAGS(n, x, SER_IN | SER_OUT)
#define SER_INOUT_UINT16(n, x)     SER_UINT16_FLAGS(n, x, SER_IN | SER_OUT)
#define SER_INOUT_UINT32(n, x)     SER_UINT32_FLAGS(n, x, SER_IN | SER_OUT)
#define SER_INOUT_BOOL(n, x)       SER_BOOL_FLAGS(n, x, SER_IN | SER_OUT)
#define SER_INOUT_STR(n, x)        SER_STR_FLAGS(n, x, SER_IN | SER_OUT)
#define SER_INOUT_STRUCT(n, x, t)          \
                            SER_STRUCT_FLAGS(n, x, SER_IN | SER_OUT, t)
#define SER_INOUT_DYN_ARRAY(n, mn, mx, t)  \
                             SER_DYN_ARRAY_FLAGS(n, mn, mx, SER_IN | SER_OUT, t)
#define SER_NS_INOUT_UINT8(ns, n, x)            \
                        SER_NS_UINT8_FLAGS(ns, n, x, SER_IN | SER_OUT)
#define SER_NS_INOUT_UINT16(ns, n, x)           \
                        SER_NS_UINT16_FLAGS(ns, n, x, SER_IN | SER_OUT)
#define SER_NS_INOUT_UINT32(ns, n, x)           \
                        SER_NS_UINT32_FLAGS(ns, n, x, SER_IN | SER_OUT)
#define SER_NS_INOUT_BOOL(ns, n, x)             \
                        SER_NS_BOOL_FLAGS(ns, n, x, SER_IN | SER_OUT)
#define SER_NS_INOUT_STR(ns, n, x)              \
                        SER_NS_STR_FLAGS(ns, n, x, SER_IN | SER_OUT)
#define SER_NS_INOUT_STRUCT(ns, n, x, t)         \
                        SER_NS_STRUCT_FLAGS(ns, n, x, SER_IN | SER_OUT, t)
#define SER_NS_INOUT_DYN_ARRAY(ns, n, mn, mx, t)  \
                        SER_NS_DYN_ARRAY_FLAGS(ns, n, mn, mx, SER_IN | SER_OUT, t)


        // TypeInfo structures for base types
        // to use in dynamic arrays

#define SER_TYPEINFO_UINT8 \
XmlSerializerInfo uint8_TypeInfo[] = {\
    SER_UINT8("uint8", 1), \
    SER_NULL \
}
#define SER_TYPEINFO_UINT8_ATTR \
XmlSerializerInfo uint8_TypeInfo[] = {\
    SER_ATTR_NS_UINT8_FLAGS(NULL, "uint8", 1, SER_ATTRS), \
    SER_NULL \
}

#define SER_TYPEINFO_UINT16 \
XmlSerializerInfo uint16_TypeInfo[] = { \
    SER_UINT16("uint16", 1), \
    SER_NULL \
}
#define SER_TYPEINFO_UINT16_ATTR \
XmlSerializerInfo uint16_TypeInfo[] = { \
    SER_ATTR_NS_UINT16_FLAGS(NULL, "uint16", 1, SER_ATTRS), \
    SER_NULL \
}

#define SER_TYPEINFO_UINT32 \
XmlSerializerInfo uint32_TypeInfo[] = { \
    SER_UINT32("uint32", 1), \
    SER_NULL \
}
#define SER_TYPEINFO_UINT32_ATTR \
XmlSerializerInfo uint32_TypeInfo[] = { \
    SER_ATTR_NS_UINT32_FLAGS(NULL, "uint32", 1, SER_ATTRS), \
    SER_NULL \
}

#define SER_TYPEINFO_BOOL \
XmlSerializerInfo bool_TypeInfo[] = {\
    SER_BOOL("bool", 1), \
    SER_NULL \
}
#define SER_TYPEINFO_BOOL_ATTR \
XmlSerializerInfo bool_TypeInfo[] = { \
    SER_ATTR_NS_BOOL_FLAGS(NULL, "bool", 1, SER_ATTRS), \
    SER_NULL \
}

#define SER_TYPEINFO_STRING \
XmlSerializerInfo string_TypeInfo[] = { \
    SER_STR("string", 1), \
    SER_NULL \
}
#define SER_TYPEINFO_STRING_ATTR \
XmlSerializerInfo string_attr_TypeInfo[] = { \
    SER_ATTR_NS_STR_FLAGS(NULL, "string", 1, SER_ATTRS), \
    SER_NULL \
}

        // Define to declare TypeInfo for structures

#define SER_DECLARE_TYPE(t)\
extern XmlSerializerInfo t##_TypeInfo[];\
extern XmlSerializerInfo t##_TypeItems[]


        // Defines to define TypeInfo for structures

#define SER_START_ITEMS(t) \
XmlSerializerInfo t##_TypeItems[] = \
{

#define SER_END_ITEMS(t) \
	SER_NULL \
}; \
 \
XmlSerializerInfo t##_TypeInfo[] = {\
    SER_STRUCT_FLAGS(NULL, 1, 0, t), \
    SER_NULL \
}


        // TypeInfo structures for well known types







        // XmlSerializationProc functions for different types

size_t do_serialize_uint8(struct __XmlSerializationData* data);
size_t do_serialize_uint16(struct __XmlSerializationData* data);
size_t do_serialize_uint32(struct __XmlSerializationData* data);
size_t do_serialize_string(struct __XmlSerializationData* data);
size_t do_serialize_bool(struct __XmlSerializationData* data);
size_t do_serialize_dyn_size_array(struct __XmlSerializationData* data);
size_t do_serialize_struct(struct __XmlSerializationData* data);
size_t do_serialize_attrs(struct __XmlSerializationData* data);




        // Serializer user interface

size_t ws_serialize(WsContextH cntx,
                WsXmlNodeH xmlNode,
                XML_TYPE_PTR dataPtr,
                XmlSerializerInfo *info,
                char *name,
                char *ns,
                XML_NODE_ATTR *attrs,
                int output);

void *ws_deserialize(WsContextH cntx,
                WsXmlNodeH xmlParent,
                XmlSerializerInfo *info,
                char *name,
                char *ns,
                XML_NODE_ATTR **attrs,
                int index,
                int output);



int ws_serialize_str(WsContextH cntx, 
                WsXmlNodeH parent, 
                char *str, 
                char *nameNs, 
                char *name,
                int mustunderstand);

int ws_serialize_uint32(
                WsContextH cntx, 
                WsXmlNodeH parent, 
                unsigned long val, 
                char *nameNs, 
                char *name,
                int mustunderstand);

char *ws_deserialize_str(WsContextH cntx, 
                WsXmlNodeH parent, 
                int index, 
                char *nameNs, 
                char *name);

unsigned long ws_deserialize_uint32(
                WsContextH cntx, 
                WsXmlNodeH parent, 
                int index, 
                char *nameNs, 
                char *name);

int ws_deserialize_duration(
                char* text,
                time_t *value);

int ws_deserialize_datetime(
                char *text,
                XML_DATETIME *tmx);


size_t ws_serializer_free_mem(WsContextH cntx, 
                XML_TYPE_PTR buf, 
                XmlSerializerInfo *info);







/** @} */

#endif //WS_XML_SERIALIZER_H
