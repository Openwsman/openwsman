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
    int count;
    XML_TYPE_PTR data;
}  XmlSerialiseDynamicSizeData;

typedef unsigned char XML_TYPE_UINT8;
typedef unsigned short XML_TYPE_UINT16;
typedef unsigned long XML_TYPE_UINT32;
typedef int XML_TYPE_BOOL;
typedef char XML_TYPE_CHAR;
typedef char* XML_TYPE_STR;
typedef XmlSerialiseDynamicSizeData XML_TYPE_DYN_ARRAY;

#define SER_ENUMERATOR_PREFIX	"_XmlSerializeEnum"




/**
 * Count can be 1 for single elementd; > 1 for fixed size array; 
 * 0 for dynamic array. 
 */



struct __XmlSerializationData;
typedef int (*XmlSerializationProc)(struct __XmlSerializationData* data);

struct __XmlSerializerInfo
{
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

#define XML_NUM_OCCURS(x) (x)->count
#define XML_MAX_OCCURS(x) (x)->count
#define XML_MIN_OCCURS(x) (x)->mincount
#define XML_IS_SKIP(x) ((x)->flags & SER_SKIP)
#define XML_IS_IN(x)   ((x)->flags & SER_IN)
#define XML_IS_OUT(x)  ((x)->flags & SER_OUT)
#define XML_IS_HEAD(x)  ((x)->flags & SER_HEAD)


#define SER_NULL {NULL, 0, 0, 0, 0, NULL, NULL}
        // Serializer Info base defines

#define SER_UINT8_FLAGS(n, x, flags) \
	{(n), (x), (x),  sizeof (XML_TYPE_UINT8), flags, do_serialize_uint8, NULL}
#define SER_UINT16_FLAGS(n, x, flags) \
	{(n), (x), (x), sizeof (XML_TYPE_UINT16), flags, do_serialize_uint16, NULL}
#define SER_UINT32_FLAGS(n, x, flags) \
	{(n), (x), (x), sizeof (XML_TYPE_UINT32), flags, do_serialize_uint32, NULL}
#define SER_BOOL_FLAGS(n, x, flags) \
	{(n), (x), (x), sizeof (XML_TYPE_BOOL), flags, do_serialize_bool, NULL}
#define SER_STR_FLAGS(n, x, flags)\
    {(n), (x), (x), sizeof (XML_TYPE_STR), flags, do_serialize_string, NULL}
#define SER_STRUCT_FLAGS(n, x, flags, t)\
    {(n), (x), (x), sizeof (t), flags, do_serialize_struct, t##_TypeItems}
#define SER_DYN_ARRAY_FLAGS(n, min, max, flags, t)\
    {(n), (min), (max), sizeof (XmlSerialiseDynamicSizeData), flags, \
         do_serialize_dyn_size_array, t##_TypeInfo \
    }


        // Serialization Info for different types
#define SER_UINT8(n, x)               SER_UINT8_FLAGS(n, x, 0)
#define SER_UINT16(n, x)              SER_UINT16_FLAGS(n, x, 0)
#define SER_UINT32(n, x)              SER_UINT32_FLAGS(n, x, 0)
#define SER_BOOL(n, x)                SER_BOOL_FLAGS(n, x, 0)
#define SER_STR(n, x)                 SER_STR_FLAGS(n, x, 0)
#define SER_STRUCT(n, x, t)           SER_STRUCT_FLAGS(n, x, 0, t)
#define SER_DYN_ARRAY(n, mn, mx, t)   SER_DYN_ARRAY_FLAGS(n, mn, mx, 0, t)


        // Serialization Info to skip in 

#define SER_IN_UINT8(n, x)            SER_UINT8_FLAGS(n, x, SER_IN)
#define SER_IN_UINT16(n, x)           SER_UINT16_FLAGS(n, x, SER_IN)
#define SER_IN_UINT32(n, x)           SER_UINT32_FLAGS(n, x, SER_IN)
#define SER_IN_BOOL(n, x)             SER_BOOL_FLAGS(n, x, SER_IN)
#define SER_IN_STR(n, x)              SER_STR_FLAGS(n, x, SER_IN)
#define SER_IN_STRUCT(n, x, t)        SER_STRUCT_FLAGS(n, x, SER_IN, t)
#define SER_IN_DYN_ARRAY(n, mn, mx, t)  \
                                      SER_DYN_ARRAY_FLAGS(n, mn, mx, SER_IN, t)


        // Serialization Info to skip in 
#define SER_OUT_UINT8(n, x)           SER_UINT8_FLAGS(n, x, SER_OUT)
#define SER_OUT_UINT16(n, x)          SER_UINT16_FLAGS(n, x, SER_OUT)
#define SER_OUT_UINT32(n, x)          SER_UINT32_FLAGS(n, x, SER_OUT)
#define SER_OUT_BOOL(n, x)            SER_BOOL_FLAGS(n, x, SER_OUT)
#define SER_OUT_STR(n, x)             SER_STR_FLAGS(n, x, SER_OUT)
#define SER_OUT_STRUCT(n, x, t)       SER_STRUCT_FLAGS(n, x, SER_OUT, t)
#define SER_OUT_DYN_ARRAY(n, mn, mx, t)   \
                                      SER_DYN_ARRAY_FLAGS(n, mn, mx, SER_OUT, t)

        // Serialization Info to skip in

#define SER_INOUT_UINT8(n, x)           SER_UINT8_FLAGS(n, x, SER_IN | SER_OUT)
#define SER_INOUT_UINT16(n, x)          SER_UINT16_FLAGS(n, x, SER_IN | SER_OUT)
#define SER_INOUT_UINT32(n, x)          SER_UINT32_FLAGS(n, x, SER_IN | SER_OUT)
#define SER_INOUT_BOOL(n, x)            SER_BOOL_FLAGS(n, x, SER_IN | SER_OUT)
#define SER_INOUT_STR(n, x)             SER_STR_FLAGS(n, x, SER_IN | SER_OUT)
#define SER_INOUT_STRUCT(n, x, t)       SER_STRUCT_FLAGS(n, x, SER_IN | SER_OUT, t)
#define SER_INOUT_DYN_ARRAY(n, mn, mx, t)  \
                             SER_DYN_ARRAY_FLAGS(n, mn, mx, SER_IN | SER_OUT, t)



        // TypeInfo structures for base types

#define SER_TYPEINFO_UINT8 \
XmlSerializerInfo uint8_TypeInfo[] = {\
    SER_UINT8("uint8", 1), \
    SER_NULL \
}
#define SER_TYPEINFO_UINT16 \
XmlSerializerInfo uint16_TypeInfo[] = {\
    SER_UINT16("uint16", 1), \
    SER_NULL \
}
#define SER_TYPEINFO_UINT32 \
XmlSerializerInfo uint32_TypeInfo[] = {\
    SER_UINT32("uint32", 1), \
    SER_NULL \
}
#define SER_TYPEINFO_BOOL \
XmlSerializerInfo bool_TypeInfo[] = {\
    SER_BOOL("bool", 1), \
    SER_NULL \
}
#define SER_TYPEINFO_STRING \
XmlSerializerInfo string_TypeInfo[] = { \
    SER_STR("string", 1), \
    SER_NULL \
}




        // Define to declare TypeInfo for structures

#define SER_DECLARE_TYPE(t)\
extern XmlSerializerInfo t##_TypeInfo[];\
extern XmlSerializerInfo t##_TypeItems[]


        // Defines to define TypeInfo for structures

#define SER_START_ITEMS(n, t) \
XmlSerializerInfo t##_TypeItems[] = \
{

#define SER_END_ITEMS(n, t)\
	SER_NULL \
};\
XmlSerializerInfo t##_TypeInfo[] = {\
    SER_STRUCT_FLAGS(n, 1, 0, t), \
    SER_NULL \
}








        // XmlSerializationProc functions for different types

int do_serialize_uint8(struct __XmlSerializationData* data);
int do_serialize_uint16(struct __XmlSerializationData* data);
int do_serialize_uint32(struct __XmlSerializationData* data);
int do_serialize_string(struct __XmlSerializationData* data);
//int do_serialize_char_array(struct __XmlSerializationData* data);
int do_serialize_bool(struct __XmlSerializationData* data);
//int do_serialize_fixed_size_array(struct __XmlSerializationData* data);
int do_serialize_dyn_size_array(struct __XmlSerializationData* data);
int do_serialize_struct(struct __XmlSerializationData* data);




        // Serializer user interface

int ws_serialize(WsContextH cntx, 
                WsXmlNodeH xmlNode, 
                XML_TYPE_PTR dataPtr, 
                XmlSerializerInfo *info, 
                char *name, 
                char *nameNs, 
                char *elementNs, 
                int output);

void *ws_deserialize(WsContextH cntx, 
                WsXmlNodeH xmlParent, 
                XmlSerializerInfo *info, 
                char *name, 
                char *nameNs, 
                char *elementNs, 
                int index, 
                int output);



int ws_serialize_str(WsContextH cntx, 
                WsXmlNodeH parent, 
                char *str, 
                char *nameNs, 
                char *name);

int ws_serialize_uint32(
                WsContextH cntx, 
                WsXmlNodeH parent, 
                unsigned long val, 
                char *nameNs, 
                char *name);

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

int ws_serializer_free_mem(WsContextH cntx, 
                XML_TYPE_PTR buf, 
                XmlSerializerInfo *info);







/** @} */

#endif //WS_XML_SERIALIZER_H
