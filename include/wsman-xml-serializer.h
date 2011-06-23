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
#include "u/libu.h"

// Errors
#define WS_ERR_INSUFFICIENT_RESOURCES	(-1)
#define WS_ERR_INVALID_PARAMETER	(-2)
#define WS_ERR_XML_PARSING	   	 (-3)
#define WS_ERR_XML_NODE_NOT_FOUND	(-4)
#define WS_ERR_NOT_IMPLEMENTED		(-5)


typedef void* XML_TYPE_PTR;

typedef struct __XmlSerialiseDynamicSizeData
{
    unsigned int count;
    XML_TYPE_PTR data;
}  XmlSerialiseDynamicSizeData;

#ifdef WIN32
typedef char XML_TYPE_INT8;
typedef unsigned char XML_TYPE_UINT8;
typedef short XML_TYPE_INT16;
typedef unsigned short XML_TYPE_UINT16;
typedef long XML_TYPE_INT32;
typedef unsigned long XML_TYPE_UINT32;
typedef long long XML_TYPE_INT64;
typedef unsigned long long XML_TYPE_UINT64;
typedef int XML_TYPE_BOOL;
typedef char XML_TYPE_CHAR;
typedef float XML_TYPE_REAL32;
typedef double XML_TYPE_REAL64;
#define PTRTOINT unsigned long long
#else
#include <sys/types.h>
#if defined (__SVR4) && defined (__sun)
typedef uchar_t XML_TYPE_UINT8;
typedef ushort_t XML_TYPE_UINT16;
typedef uint_t XML_TYPE_UINT32;
typedef u_longlong_t  XML_TYPE_UINT64;
typedef char XML_TYPE_INT8;
typedef short XML_TYPE_INT16;
typedef int XML_TYPE_INT32;
typedef long long XML_TYPE_INT64;
typedef float XML_TYPE_REAL32;
typedef double XML_TYPE_REAL64;

typedef int XML_TYPE_BOOL;
typedef char XML_TYPE_CHAR;
#define PTRTOINT int

#else
typedef u_int8_t XML_TYPE_UINT8;
typedef u_int16_t XML_TYPE_UINT16;
typedef u_int32_t XML_TYPE_UINT32;
typedef u_int64_t XML_TYPE_UINT64;
typedef int8_t XML_TYPE_INT8;
typedef int16_t XML_TYPE_INT16;
typedef int32_t XML_TYPE_INT32;
typedef int64_t XML_TYPE_INT64;
typedef float XML_TYPE_REAL32;
typedef double XML_TYPE_REAL64;

typedef int XML_TYPE_BOOL;
typedef char XML_TYPE_CHAR;
#define PTRTOINT intptr_t
#endif
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
typedef int (*XmlSerializationProc)(struct __XmlSerializationData* data);

struct __XmlSerializerInfo
{
    const char *ns;
    const char* name;
    unsigned int mincount;    /**< Minimal Count */
    unsigned int count;       /**< Maximal Count */
    unsigned int size;        /**< size of serialized/deserialized element */
    unsigned int flags; /**< flags */
    XmlSerializationProc proc; /**< Serialization Processor */
    XML_TYPE_PTR extData;      /**< External Data */
};
typedef struct __XmlSerializerInfo XmlSerializerInfo;

struct __WsSerializerContext
{
	pthread_mutex_t lock;
	list_t *WsSerializerAllocList;
};

typedef struct __WsSerializerContext *WsSerializerContextH;

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

/* Serializer Info base defines with min/max */

#define SER_NS_CNT_UINT8_FLAGS(ns, n, min, max, flags) \
    {(ns), (n), (min), (max),  sizeof (XML_TYPE_UINT8), \
        flags, do_serialize_uint8, NULL \
    }
#define SER_NS_CNT_UINT16_FLAGS(ns, n, min, max, flags) \
    {(ns), (n), (min), (max), sizeof (XML_TYPE_UINT16), \
        flags, do_serialize_uint16, NULL \
    }
#define SER_NS_CNT_UINT32_FLAGS(ns, n, min, max, flags) \
    {(ns), (n), (min), (max), sizeof (XML_TYPE_UINT32), \
        flags, do_serialize_uint32, NULL \
    }
#define SER_NS_CNT_UINT64_FLAGS(ns, n, min, max, flags) \
    {(ns), (n), (min), (max), sizeof (XML_TYPE_UINT64), \
        flags, do_serialize_uint64, NULL \
    }

#define SER_NS_CNT_BOOL_FLAGS(ns, n, min, max, flags) \
    {(ns), (n), (min), (max), sizeof (XML_TYPE_BOOL), \
        flags, do_serialize_bool, NULL \
    }
#define SER_NS_CNT_STR_FLAGS(ns, n, min, max, flags) \
    {(ns), (n), (min), (max), sizeof (XML_TYPE_STR), \
        flags, do_serialize_string, NULL \
    }
#define SER_NS_CNT_STRUCT_FLAGS(ns, n, min, max, flags, t) \
    {(ns), (n), (min), (max), sizeof (t), flags, \
        do_serialize_struct, t##_TypeItems \
    }

// Serializer Info base defines

#define SER_NS_INT8_FLAGS(ns, n, x, flags) \
    {(ns), (n), (x), (x),  sizeof (XML_TYPE_INT8), \
        flags, do_serialize_int8, NULL \
    }
#define SER_NS_UINT8_FLAGS(ns, n, x, flags) \
    {(ns), (n), (x), (x),  sizeof (XML_TYPE_UINT8), \
        flags, do_serialize_uint8, NULL \
    }
#define SER_NS_INT16_FLAGS(ns, n, x, flags) \
    {(ns), (n), (x), (x), sizeof (XML_TYPE_INT16), \
        flags, do_serialize_int16, NULL \
    }
#define SER_NS_UINT16_FLAGS(ns, n, x, flags) \
    {(ns), (n), (x), (x), sizeof (XML_TYPE_UINT16), \
        flags, do_serialize_uint16, NULL \
    }
#define SER_NS_INT32_FLAGS(ns, n, x, flags) \
    {(ns), (n), (x), (x), sizeof (XML_TYPE_INT32), \
        flags, do_serialize_int32, NULL \
    }
#define SER_NS_UINT32_FLAGS(ns, n, x, flags) \
    {(ns), (n), (x), (x), sizeof (XML_TYPE_UINT32), \
        flags, do_serialize_uint32, NULL \
    }
#define SER_NS_INT64_FLAGS(ns, n, x, flags) \
    {(ns), (n), (x), (x), sizeof (XML_TYPE_INT64), \
        flags, do_serialize_int64, NULL \
    }
#define SER_NS_UINT64_FLAGS(ns, n, x, flags) \
    {(ns), (n), (x), (x), sizeof (XML_TYPE_UINT64), \
        flags, do_serialize_uint64, NULL \
    }
#define SER_NS_REAL32_FLAGS(ns, n, x, flags)\
     {(ns), (n), (x), (x), sizeof (XML_TYPE_REAL32), \
        flags, do_serialize_real, NULL \
     }
#define SER_NS_REAL64_FLAGS(ns, n, x, flags)\
     {(ns), (n), (x), (x), sizeof (XML_TYPE_REAL64), \
        flags, do_serialize_real, NULL \
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
#define SER_ATTR_NS_INT8_FLAGS(ns, n, x, flags) \
    {(ns), (n), (x), (x), sizeof (struct {XML_TYPE_INT8 s; XML_NODE_ATTR *a;}), \
        flags | SER_ATTRS, do_serialize_int8, NULL \
    }
#define SER_ATTR_NS_UINT8_FLAGS(ns, n, x, flags) \
    {(ns), (n), (x), (x), sizeof (struct {XML_TYPE_UINT8 s; XML_NODE_ATTR *a;}), \
        flags | SER_ATTRS, do_serialize_uint8, NULL \
    }
#define SER_ATTR_NS_INT16_FLAGS(ns, n, x, flags) \
    {(ns), (n), (x), (x), sizeof (struct {XML_TYPE_INT16 s; XML_NODE_ATTR *a;}), \
        flags | SER_ATTRS, do_serialize_int16, NULL \
    }
#define SER_ATTR_NS_UINT16_FLAGS(ns, n, x, flags) \
    {(ns), (n), (x), (x), sizeof (struct {XML_TYPE_UINT16 s; XML_NODE_ATTR *a;}), \
        flags | SER_ATTRS, do_serialize_uint16, NULL \
    }
#define SER_ATTR_NS_INT32_FLAGS(ns, n, x, flags) \
    {(ns), (n), (x), (x), sizeof (struct {XML_TYPE_INT32 s; XML_NODE_ATTR *a;}), \
        flags | SER_ATTRS, do_serialize_int32, NULL \
    }
#define SER_ATTR_NS_UINT32_FLAGS(ns, n, x, flags) \
    {(ns), (n), (x), (x), sizeof (struct {XML_TYPE_UINT32 s; XML_NODE_ATTR *a;}), \
        flags | SER_ATTRS, do_serialize_uint32, NULL \
    }
#define SER_ATTR_NS_INT64_FLAGS(ns, n, x, flags) \
    {(ns), (n), (x), (x), sizeof (struct {XML_TYPE_INT64 s; XML_NODE_ATTR *a;}), \
        flags | SER_ATTRS, do_serialize_int64, NULL \
    }
#define SER_ATTR_NS_UINT64_FLAGS(ns, n, x, flags) \
    {(ns), (n), (x), (x), sizeof (struct {XML_TYPE_UINT64 s; XML_NODE_ATTR *a;}), \
        flags | SER_ATTRS, do_serialize_uint64, NULL \
    }
#define SER_ATTR_NS_REAL32_FLAGS(ns, n, x, flags) \
    {(ns), (n), (x), (x), sizeof (struct {XML_TYPE_UINT32 s; XML_NODE_ATTR *a;}), \
        flags | SER_ATTRS, do_serialize_real32, NULL \
    }
#define SER_ATTR_NS_REAL64_FLAGS(ns, n, x, flags) \
    {(ns), (n), (x), (x), sizeof (struct {XML_TYPE_REAL64 s; XML_NODE_ATTR *a;}), \
        flags | SER_ATTRS, do_serialize_real64, NULL \
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
#define SER_INT8_FLAGS(n, x, flags)  SER_NS_INT8_FLAGS(NULL, n, x, flags)
#define SER_UINT8_FLAGS(n, x, flags)  SER_NS_UINT8_FLAGS(NULL, n, x, flags)
#define SER_INT16_FLAGS(n, x, flags) SER_NS_INT16_FLAGS(NULL, n, x, flags)
#define SER_UINT16_FLAGS(n, x, flags) SER_NS_UINT16_FLAGS(NULL, n, x, flags)
#define SER_INT32_FLAGS(n, x, flags) SER_NS_INT32_FLAGS(NULL, n, x, flags)
#define SER_UINT32_FLAGS(n, x, flags) SER_NS_UINT32_FLAGS(NULL, n, x, flags)
#define SER_INT64_FLAGS(n, x, flags) SER_NS_INT64_FLAGS(NULL, n, x, flags)
#define SER_UINT64_FLAGS(n, x, flags) SER_NS_UINT64_FLAGS(NULL, n, x, flags)
#define SER_REAL32_FLAGS(n, x, flags) SER_NS_REAL32_FLAGS(NULL, n, x, flags)
#define SER_REAL64_FLAGS(n, x, flags) SER_NS_REAL64_FLAGS(NULL, n, x, flags)
#define SER_BOOL_FLAGS(n, x, flags)   SER_NS_BOOL_FLAGS(NULL, n, x, flags)
#define SER_STR_FLAGS(n, x, flags)    SER_NS_STR_FLAGS(NULL, n, x, flags)
#define SER_STRUCT_FLAGS(n, x, flags, t) \
                        SER_NS_STRUCT_FLAGS(NULL, n, x, flags, t)
#define SER_DYN_ARRAY_FLAGS(n, min, max, flags, t) \
                        SER_NS_DYN_ARRAY_FLAGS(NULL, n, min, max, flags, t)


// Serialization Info for different types
#define SER_INT8(n, x)               SER_INT8_FLAGS(n, x, 0)
#define SER_UINT8(n, x)               SER_UINT8_FLAGS(n, x, 0)
#define SER_INT16(n, x)              SER_INT16_FLAGS(n, x, 0)
#define SER_UINT16(n, x)              SER_UINT16_FLAGS(n, x, 0)
#define SER_INT32(n, x)              SER_INT32_FLAGS(n, x, 0)
#define SER_UINT32(n, x)              SER_UINT32_FLAGS(n, x, 0)
#define SER_INT64(n, x)              SER_INT64_FLAGS(n, x, 0)
#define SER_UINT64(n, x)              SER_UINT64_FLAGS(n, x, 0)
#define SER_REAL32(n, x)              SER_REAL32_FLAGS(n, x, 0)
#define SER_REAL64(n, x)              SER_REAL64_FLAGS(n, x, 0)
#define SER_BOOL(n, x)                SER_BOOL_FLAGS(n, x, 0)
#define SER_STR(n, x)                 SER_STR_FLAGS(n, x, 0)
#define SER_STRUCT(n, x, t)           SER_STRUCT_FLAGS(n, x, 0, t)
#define SER_DYN_ARRAY(n, mn, mx, t)   SER_DYN_ARRAY_FLAGS(n, mn, mx, 0, t)
#define SER_NS_INT8(ns, n, x)         SER_NS_INT8_FLAGS(ns, n, x, 0)
#define SER_NS_UINT8(ns, n, x)        SER_NS_UINT8_FLAGS(ns, n, x, 0)
#define SER_NS_INT16(ns, n, x)        SER_NS_INT16_FLAGS(ns, n, x, 0)
#define SER_NS_UINT16(ns, n, x)       SER_NS_UINT16_FLAGS(ns, n, x, 0)
#define SER_NS_INT32(ns, n, x)        SER_NS_INT32_FLAGS(ns, n, x, 0)
#define SER_NS_UINT32(ns, n, x)       SER_NS_UINT32_FLAGS(ns, n, x, 0)
#define SER_NS_INT64(ns, n, x)        SER_NS_INT64_FLAGS(ns, n, x, 0)
#define SER_NS_UINT64(ns, n, x)       SER_NS_UINT64_FLAGS(ns, n, x, 0)
#define SER_NS_REAL32(ns, n, x)       SER_NS_REAL32_FLAGS(ns, n, x, 0)
#define SER_NS_REAL64(ns, n, x)       SER_NS_REAL64_FLAGS(ns, n, x, 0)
#define SER_NS_BOOL(ns, n, x)         SER_NS_BOOL_FLAGS(ns, n, x, 0)
#define SER_NS_STR(ns, n, x)          SER_NS_STR_FLAGS(ns, n, x, 0)
#define SER_NS_STRUCT(ns, n, x, t)    SER_NS_STRUCT_FLAGS(ns, n, x, 0, t)
#define SER_NS_DYN_ARRAY(ns, n, mn, mx, t)   \
                                 SER_NS_DYN_ARRAY_FLAGS(ns, n, mn, mx, 0, t)


        // Serialization Info to skip in
#define SER_IN_INT8(n, x)            SER_INT8_FLAGS(n, x, SER_IN)
#define SER_IN_UINT8(n, x)            SER_UINT8_FLAGS(n, x, SER_IN)
#define SER_IN_INT16(n, x)           SER_INT16_FLAGS(n, x, SER_IN)
#define SER_IN_UINT16(n, x)           SER_UINT16_FLAGS(n, x, SER_IN)
#define SER_IN_UINT32(n, x)           SER_UINT32_FLAGS(n, x, SER_IN)
#define SER_IN_UINT32(n, x)           SER_UINT32_FLAGS(n, x, SER_IN)
#define SER_IN_INT64(n, x)           SER_INT64_FLAGS(n, x, SER_IN)
#define SER_IN_UINT64(n, x)           SER_UINT64_FLAGS(n, x, SER_IN)
#define SER_IN_REAL32(n, x)           SER_REAL32_FLAGS(n, x, SER_IN)
#define SER_IN_REAL64(n, x)           SER_REAL64_FLAGS(n, x, SER_IN)
#define SER_IN_BOOL(n, x)             SER_BOOL_FLAGS(n, x, SER_IN)
#define SER_IN_STR(n, x)              SER_STR_FLAGS(n, x, SER_IN)
#define SER_IN_STRUCT(n, x, t)        SER_STRUCT_FLAGS(n, x, SER_IN, t)
#define SER_IN_DYN_ARRAY(n, mn, mx, t)  \
                               SER_DYN_ARRAY_FLAGS(n, mn, mx, SER_IN, t)
#define SER_NS_IN_INT8(ns, n, x)     SER_NS_INT8_FLAGS(ns, n, x, SER_IN)
#define SER_NS_IN_UINT8(ns, n, x)     SER_NS_UINT8_FLAGS(ns, n, x, SER_IN)
#define SER_NS_IN_INT16(ns, n, x)    SER_NS_INT16_FLAGS(ns, n, x, SER_IN)
#define SER_NS_IN_UINT16(ns, n, x)    SER_NS_UINT16_FLAGS(ns, n, x, SER_IN)
#define SER_NS_IN_INT32(ns, n, x)    SER_NS_INT32_FLAGS(ns, n, x, SER_IN)
#define SER_NS_IN_UINT32(ns, n, x)    SER_NS_UINT32_FLAGS(ns, n, x, SER_IN)
#define SER_NS_IN_INT64(ns, n, x)    SER_NS_INT64_FLAGS(ns, n, x, SER_IN)
#define SER_NS_IN_UINT64(ns, n, x)    SER_NS_UINT64_FLAGS(ns, n, x, SER_IN)
#define SER_NS_IN_REAL32(ns, n, x)    SER_NS_REAL32_FLAGS(ns, n, x, SER_IN)
#define SER_NS_IN_REAL64(ns, n, x)    SER_NS_REAL64_FLAGS(ns, n, x, SER_IN)
#define SER_NS_IN_BOOL(ns, n, x)      SER_NS_BOOL_FLAGS(ns, n, x, SER_IN)
#define SER_NS_IN_STR(ns, n, x)       SER_NS_STR_FLAGS(ns, n, x, SER_IN)
#define SER_NS_IN_STRUCT(ns, n, x, t) SER_NS_STRUCT_FLAGS(ns, n, x, SER_IN, t)
#define SER_NS_IN_DYN_ARRAY(ns, n, mn, mx, t)  \
                               SER_NS_DYN_ARRAY_FLAGS(ns, n, mn, mx, SER_IN, t)

        // Serialization Info to skip out
#define SER_OUT_INT8(n, x)           SER_INT8_FLAGS(n, x, SER_OUT)
#define SER_OUT_UINT8(n, x)           SER_UINT8_FLAGS(n, x, SER_OUT)
#define SER_OUT_INT16(n, x)          SER_INT16_FLAGS(n, x, SER_OUT)
#define SER_OUT_UINT16(n, x)          SER_UINT16_FLAGS(n, x, SER_OUT)
#define SER_OUT_INT32(n, x)          SER_INT32_FLAGS(n, x, SER_OUT)
#define SER_OUT_UINT32(n, x)          SER_UINT32_FLAGS(n, x, SER_OUT)
#define SER_OUT_INT64(n, x)          SER_INT64_FLAGS(n, x, SER_OUT)
#define SER_OUT_UINT64(n, x)          SER_UINT64_FLAGS(n, x, SER_OUT)
#define SER_OUT_REAL32(n, x)          SER_REAL32_FLAGS(n, x, SER_OUT)
#define SER_OUT_REAL64(n, x)          SER_REAL64_FLAGS(n, x, SER_OUT)
#define SER_OUT_BOOL(n, x)            SER_BOOL_FLAGS(n, x, SER_OUT)
#define SER_OUT_STR(n, x)             SER_STR_FLAGS(n, x, SER_OUT)
#define SER_OUT_STRUCT(n, x, t)       SER_STRUCT_FLAGS(n, x, SER_OUT, t)
#define SER_OUT_DYN_ARRAY(n, mn, mx, t)   \
                                      SER_DYN_ARRAY_FLAGS(n, mn, mx, SER_OUT, t)
#define SER_NS_OUT_INT8(ns, n, x)     SER_NS_INT8_FLAGS(ns, n, x, SER_OUT)
#define SER_NS_OUT_UINT8(ns, n, x)     SER_NS_UINT8_FLAGS(ns, n, x, SER_OUT)
#define SER_NS_OUT_INT16(ns, n, x)    SER_NS_INT16_FLAGS(ns, n, x, SER_OUT)
#define SER_NS_OUT_UINT16(ns, n, x)    SER_NS_UINT16_FLAGS(ns, n, x, SER_OUT)
#define SER_NS_OUT_INT32(ns, n, x)    SER_NS_INT32_FLAGS(ns, n, x, SER_OUT)
#define SER_NS_OUT_UINT32(ns, n, x)    SER_NS_UINT32_FLAGS(ns, n, x, SER_OUT)
#define SER_NS_OUT_INT64(ns, n, x)    SER_NS_INT64_FLAGS(ns, n, x, SER_OUT)
#define SER_NS_OUT_UINT64(ns, n, x)    SER_NS_UINT64_FLAGS(ns, n, x, SER_OUT)
#define SER_NS_OUT_REAL32(ns, n, x)    SER_NS_REAL32_FLAGS(ns, n, x, SER_OUT)
#define SER_NS_OUT_REAL64(ns, n, x)    SER_NS_REAL64_FLAGS(ns, n, x, SER_OUT)
#define SER_NS_OUT_BOOL(ns, n, x)      SER_NS_BOOL_FLAGS(ns, n, x, SER_OUT)
#define SER_NS_OUT_STR(ns, n, x)       SER_NS_STR_FLAGS(ns, n, x, SER_OUT)
#define SER_NS_OUT_STRUCT(ns, n, x, t) SER_NS_STRUCT_FLAGS(ns, n, x, SER_OUT, t)
#define SER_NS_OUT_DYN_ARRAY(ns, n, mn, mx, t)   \
                              SER_NS_DYN_ARRAY_FLAGS(ns, n, mn, mx, SER_OUT, t)


        // Serialization Info to skip inout
#define SER_INOUT_INT8(n, x)      SER_INT8_FLAGS(n, x, SER_IN | SER_OUT)
#define SER_INOUT_UINT8(n, x)      SER_UINT8_FLAGS(n, x, SER_IN | SER_OUT)
#define SER_INOUT_INT16(n, x)     SER_INT16_FLAGS(n, x, SER_IN | SER_OUT)
#define SER_INOUT_UINT16(n, x)     SER_UINT16_FLAGS(n, x, SER_IN | SER_OUT)
#define SER_INOUT_INT32(n, x)     SER_INT32_FLAGS(n, x, SER_IN | SER_OUT)
#define SER_INOUT_UINT32(n, x)     SER_UINT32_FLAGS(n, x, SER_IN | SER_OUT)
#define SER_INOUT_INT64(n, x)     SER_INT64_FLAGS(n, x, SER_IN | SER_OUT)
#define SER_INOUT_UINT64(n, x)     SER_UINT64_FLAGS(n, x, SER_IN | SER_OUT)
#define SER_INOUT_REAL32(n, x)     SER_REAL32_FLAGS(n, x, SER_IN | SER_OUT)
#define SER_INOUT_REAL64(n, x)     SER_REAL64_FLAGS(n, x, SER_IN | SER_OUT)
#define SER_INOUT_BOOL(n, x)       SER_BOOL_FLAGS(n, x, SER_IN | SER_OUT)
#define SER_INOUT_STR(n, x)        SER_STR_FLAGS(n, x, SER_IN | SER_OUT)
#define SER_INOUT_STRUCT(n, x, t)          \
                            SER_STRUCT_FLAGS(n, x, SER_IN | SER_OUT, t)
#define SER_INOUT_DYN_ARRAY(n, mn, mx, t)  \
                             SER_DYN_ARRAY_FLAGS(n, mn, mx, SER_IN | SER_OUT, t)
#define SER_NS_INOUT_INT8(ns, n, x)            \
                        SER_NS_INT8_FLAGS(ns, n, x, SER_IN | SER_OUT)
#define SER_NS_INOUT_UINT8(ns, n, x)            \
                        SER_NS_UINT8_FLAGS(ns, n, x, SER_IN | SER_OUT)
#define SER_NS_INOUT_INT16(ns, n, x)           \
                        SER_NS_INT16_FLAGS(ns, n, x, SER_IN | SER_OUT)
#define SER_NS_INOUT_UINT16(ns, n, x)           \
                        SER_NS_UINT16_FLAGS(ns, n, x, SER_IN | SER_OUT)
#define SER_NS_INOUT_INT32(ns, n, x)           \
                        SER_NS_INT32_FLAGS(ns, n, x, SER_IN | SER_OUT)
#define SER_NS_INOUT_UINT32(ns, n, x)           \
                        SER_NS_UINT32_FLAGS(ns, n, x, SER_IN | SER_OUT)
#define SER_NS_INOUT_INT64(ns, n, x)           \
                        SER_NS_INT64_FLAGS(ns, n, x, SER_IN | SER_OUT)
#define SER_NS_INOUT_UINT64(ns, n, x)           \
                        SER_NS_UINT64_FLAGS(ns, n, x, SER_IN | SER_OUT)
#define SER_NS_INOUT_REAL32(ns, n, x)           \
                        SER_NS_REAL32_FLAGS(ns, n, x, SER_IN | SER_OUT)
#define SER_NS_INOUT_REAL64(ns, n, x)           \
                        SER_NS_REAL64_FLAGS(ns, n, x, SER_IN | SER_OUT)
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
#define SER_TYPEINFO_INT8 \
XmlSerializerInfo int8_TypeInfo[] = {\
    SER_INT8("int8", 1), \
    SER_NULL \
}
#define SER_TYPEINFO_UINT8_ATTR \
XmlSerializerInfo uint8_TypeInfo[] = {\
    SER_ATTR_NS_UINT8_FLAGS(NULL, "uint8", 1, SER_ATTRS), \
    SER_NULL \
}
#define SER_TYPEINFO_INT8_ATTR \
XmlSerializerInfo int8_TypeInfo[] = {\
    SER_ATTR_NS_INT8_FLAGS(NULL, "int8", 1, SER_ATTRS), \
    SER_NULL \
}
#define SER_TYPEINFO_UINT16 \
XmlSerializerInfo uint16_TypeInfo[] = { \
    SER_UINT16("uint16", 1), \
    SER_NULL \
}
#define SER_TYPEINFO_INT16 \
XmlSerializerInfo int16_TypeInfo[] = { \
    SER_INT16("int16", 1), \
    SER_NULL \
}
#define SER_TYPEINFO_UINT16_ATTR \
XmlSerializerInfo uint16_TypeInfo[] = { \
    SER_ATTR_NS_UINT16_FLAGS(NULL, "uint16", 1, SER_ATTRS), \
    SER_NULL \
}
#define SER_TYPEINFO_INT16_ATTR \
XmlSerializerInfo int16_TypeInfo[] = { \
    SER_ATTR_NS_INT16_FLAGS(NULL, "int16", 1, SER_ATTRS), \
    SER_NULL \
}
#define SER_TYPEINFO_UINT32 \
XmlSerializerInfo uint32_TypeInfo[] = { \
    SER_UINT32("uint32", 1), \
    SER_NULL \
}
#define SER_TYPEINFO_INT32 \
XmlSerializerInfo int32_TypeInfo[] = { \
    SER_INT32("int32", 1), \
    SER_NULL \
}
#define SER_TYPEINFO_UINT32_ATTR \
XmlSerializerInfo uint32_TypeInfo[] = { \
    SER_ATTR_NS_UINT32_FLAGS(NULL, "uint32", 1, SER_ATTRS), \
    SER_NULL \
}
#define SER_TYPEINFO_INT32_ATTR \
XmlSerializerInfo int32_TypeInfo[] = { \
    SER_ATTR_NS_INT32_FLAGS(NULL, "int32", 1, SER_ATTRS), \
    SER_NULL \
}
#define SER_TYPEINFO_UINT64 \
XmlSerializerInfo uint64_TypeInfo[] = { \
    SER_UINT64("uint64", 1), \
    SER_NULL \
}
#define SER_TYPEINFO_INT64 \
XmlSerializerInfo int64_TypeInfo[] = { \
    SER_INT64("int64", 1), \
    SER_NULL \
}
#define SER_TYPEINFO_UINT64_ATTR \
XmlSerializerInfo uint64_TypeInfo[] = { \
    SER_ATTR_NS_UINT64_FLAGS(NULL, "uint64", 1, SER_ATTRS), \
    SER_NULL \
}
#define SER_TYPEINFO_INT64_ATTR \
XmlSerializerInfo int64_TypeInfo[] = { \
    SER_ATTR_NS_INT64_FLAGS(NULL, "int64", 1, SER_ATTRS), \
    SER_NULL \
}
#define SER_TYPEINFO_REAL32 \
XmlSerializerInfo real32_TypeInfo[] = { \
    SER_REAL32("real32", 1), \
    SER_NULL \
}
#define SER_TYPEINFO_REAL32_ATTR \
XmlSerializerInfo real32_TypeInfo[] = { \
    SER_ATTR_NS_REAL32_FLAGS(NULL, "real32", 1, SER_ATTRS), \
    SER_NULL \
}
#define SER_TYPEINFO_REAL64 \
XmlSerializerInfo real64_TypeInfo[] = { \
    SER_REAL32("real64", 1), \
    SER_NULL \
}
#define SER_TYPEINFO_REAL64_ATTR \
XmlSerializerInfo real64_TypeInfo[] = { \
    SER_ATTR_NS_REAL64_FLAGS(NULL, "real64", 1, SER_ATTRS), \
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

int do_serialize_uint8(struct __XmlSerializationData* data);
int do_serialize_uint16(struct __XmlSerializationData* data);
int do_serialize_uint32(struct __XmlSerializationData* data);
int do_serialize_uint64(struct __XmlSerializationData* data);
int do_serialize_int8(struct __XmlSerializationData* data);
int do_serialize_int16(struct __XmlSerializationData* data);
int do_serialize_int32(struct __XmlSerializationData* data);
int do_serialize_int64(struct __XmlSerializationData* data);
int do_serialize_real32(struct __XmlSerializationData* data);
int do_serialize_real64(struct __XmlSerializationData* data);
int do_serialize_string(struct __XmlSerializationData* data);
int do_serialize_bool(struct __XmlSerializationData* data);
int do_serialize_dyn_size_array(struct __XmlSerializationData* data);
int do_serialize_struct(struct __XmlSerializationData* data);
int do_serialize_attrs(struct __XmlSerializationData* data);

int ws_havenilvalue(XML_NODE_ATTR *attrs);


// Serializer user interface

int ws_serialize(WsSerializerContextH serctx,
                WsXmlNodeH xmlNode,
                XML_TYPE_PTR dataPtr,
                XmlSerializerInfo *info,
                const char *name,
                const char *ns,
                XML_NODE_ATTR *attrs,
                int output);

void *ws_deserialize(WsSerializerContextH serctx,
                WsXmlNodeH xmlParent,
                XmlSerializerInfo *info,
                const char *name,
                const char *ns,
                XML_NODE_ATTR **attrs,
                int index,
                int output);



int ws_serialize_str(WsSerializerContextH serctx,
                WsXmlNodeH parent,
                const char *str,
                const char *nameNs,
                const char *name,
                int mustunderstand);

int ws_serialize_uint32(
                WsSerializerContextH serctx,
                WsXmlNodeH parent,
                unsigned long val,
                const char *nameNs,
                const char *name,
                int mustunderstand);

char *ws_deserialize_str(WsSerializerContextH serctx,
                WsXmlNodeH parent,
                int index,
                const char *nameNs,
                const char *name);

unsigned long ws_deserialize_uint32(
                WsSerializerContextH serctx,
                WsXmlNodeH parent,
                int index,
                const char *nameNs,
                const char *name);

int ws_deserialize_duration(
                const char* text,
                time_t *value);

int ws_deserialize_datetime(
                const char *text,
                XML_DATETIME *tmx);


int ws_serializer_free_mem(WsSerializerContextH serctx,
                XML_TYPE_PTR buf,
                XmlSerializerInfo *info);







/** @} */

#endif //WS_XML_SERIALIZER_H
