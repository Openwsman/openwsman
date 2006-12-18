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

typedef unsigned char XML_TYPE_UINT8;
typedef unsigned short XML_TYPE_UINT16;
typedef unsigned long XML_TYPE_UINT32;
typedef int XML_TYPE_BOOL;
typedef char XML_TYPE_CHAR;
typedef void* XML_TYPE_PTR;
typedef char* XML_TYPE_STR;

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
    unsigned short flagsMinCount; /**< Minimal Count */
    unsigned short funcMaxCount; /**< Maximal Count */
    XmlSerializationProc proc; /**< Serialization Processor */
    XML_TYPE_PTR extData; /**< External Data */
};
typedef struct __XmlSerializerInfo XmlSerializerInfo;



typedef struct __XmlSerialiseDynamicSizeData
{
    int count;
    XML_TYPE_PTR data;
}  XmlSerialiseDynamicSizeData;

//#define SER_ALIGNMENT_MASK	0x000F

        // flags in funcMaxCount
#define SER_IN				((unsigned short)0x8000)
#define SER_OUT				((unsigned short)0x4000)

#define SER_IN_OUT			(SER_OUT | SER_IN)
#define SER_MAXCOUNT_FLAGS_MASK   (SER_OUT | SER_IN)


        // flags in flagsMinCount
#define SER_PTR				0x8000
#define SER_SKIP			0x4000

#define SER_MINCOUNT_FLAGS_MASK   (SER_PTR | SER_SKIP)




#define XML_MAX_OCCURS(x) \
      ((int)((unsigned short)((x)->funcMaxCount & ~SER_MAXCOUNT_FLAGS_MASK)))
#define XML_MIN_OCCURS(x) \
      ((int)((unsigned short)((x)->flagsMinCount & ~SER_MINCOUNT_FLAGS_MASK)))

#define XML_IS_PTR(x) ((x)->flagsMinCount & SER_PTR)
#define XML_SET_PTR(x) ((x)->flagsMinCount = (x)->flagsMinCount | SER_PTR)
#define XML_UNSET_PTR(x) ((x)->flagsMinCount = (x)->flagsMinCount & ~SER_PTR)
#define XML_IS_SKIP(x) ((x)->flagsMinCount & SER_SKIP)

#define XML_IS_IN(x) ((x)->funcMaxCount & SER_IN)
#define XML_IS_OUT(x) ((x)->funcMaxCount & SER_OUT)
//#define XML_IS_RETURN(x) ((x)->funcMaxCount & SER_RET_FLAG)



        // Serializer Info for different types

#define SER_UINT8(n, x, y)\
	{(n), (x), (y), do_serialize_uint8, NULL}
#define SER_UINT16(n, x, y)\
	{(n), (x), (y), do_serialize_uint16, NULL}
#define SER_UINT32(n, x, y)\
	{(n), (x), (y), do_serialize_uint32, NULL}
#define SER_BOOL(n, x, y)\
	{(n), (x), (y), do_serialize_bool, NULL}
#define SER_STR(n, x, y)\
    {(n), (x), (y), do_serialize_string, NULL}
#define SER_STRUCT(n, x, y, t)\
    {(n), (x), (y), do_serialize_struct, t##_TypeItems}
#define SER_DYN_ARRAY(n, t)\
    {(n), 1, 1, do_serialize_dyn_size_array, t##_TypeInfo}


#define SER_UINT8_PTR (n, x, y)       SER_UINT8(n, SER_PTR | (x), y)
#define SER_UINT16_PTR (n, x, y)      SER_UINT16(n, SER_PTR | (x), y)
#define SER_UINT32_PTR(n, x, y)       SER_UINT32(n, SER_PTR | (x), y)
#define SER_BOOL_PTR(n, x, y)         SER_BOOL(n, SER_PTR | (x), y)
#define SER_STR_PTR(n, x, y)          SER_STR(n, SER_PTR | (x), y)
#define SER_DYN_ARRAY_PTR(n, t)  \
    {(n), SER_PTR | 1, 1, do_serialize_dyn_size_array, t##_TypeInfo}



        // Serialization Info to skip in 

#define SER_IN_UINT8(n)               SER_UINT8(n, 1, 1 | SER_IN)
#define SER_IN_UINT16(n)              SER_UINT16(n, 1, 1 | SER_IN)
#define SER_IN_UINT32(n)              SER_UINT32(n, 1, 1 | SER_IN)
#define SER_IN_BOOL(n)                SER_BOOL(n, 1, 1 | SER_IN)
#define SER_IN_STR(n)                 SER_STR(n, 1, 1 | SER_IN)
#define SER_IN_STRUCT(n, t)           SER_STRUCT(n, 1, 1 | SER_IN, t)
#define SER_IN_DYN_ARRAY(n, t)        SER_DYN_ARRAY(n, 1, 1 | SER_IN, t)

#define SER_IN_UINT8_PTR(n)           SER_UINT8_PTR(n, 1, 1 | SER_IN)
#define SER_IN_UINT16_PTR(n)          SER_UINT16_PTR(n, 1, 1 | SER_IN)
#define SER_IN_UINT32_PTR(n)          SER_UINT32_PTR(n, 1, 1 | SER_IN)
#define SER_IN_BOOL_PTR(n)            SER_BOOL_PTR(n, 1, 1 | SER_IN)
#define SER_IN_STR_PTR(n)             SER_STR_PTR(n, 1, 1 | SER_IN)
#define SER_IN_DYN_ARRAY_PTR(n, t)    SER_DYN_ARRAY_PTR(n, 1, 1 | SER_IN)

        // Serialization Info to skip in 

#define SER_OUT_UINT8(n)              SER_UINT8(n, 1, 1 | SER_OUT)
#define SER_OUT_UINT16(n)             SER_UINT16(n, 1, 1 | SER_OUT)
#define SER_OUT_UINT32(n)             SER_UINT32(n, 1, 1 | SER_OUT)
#define SER_OUT_BOOL(n)               SER_BOOL(n, 1, 1 | SER_OUT)
#define SER_OUT_STR(n)                SER_STR(n, 1, 1 | SER_OUT)
#define SER_OUT_STRUCT(n, t)          SER_STRUCT(n, 1, 1 | SER_OUT, t)
#define SER_OUT_DYN_ARRAY(n, t)       SER_DYN_ARRAY(n, 1, 1 | SER_OUT, t)

#define SER_OUT_UINT8_PTR(n)          SER_UINT8_PTR(n, 1, 1 | SER_OUT)
#define SER_OUT_UINT16_PTR(n)         SER_UINT16_PTR(n, 1, 1 | SER_OUT)
#define SER_OUT_UINT32_PTR(n)         SER_UINT32_PTR(n, 1, 1 | SER_OUT)
#define SER_OUT_BOOL_PTR(n)           SER_BOOL_PTR(n, 1, 1 | SER_OUT)
#define SER_OUT_STR_PTR(n)            SER_STR_PTR(n, 1, 1 | SER_OUT)
#define SER_OUT_DYN_ARRAY_PTR(n, t)   SER_DYN_ARRAY_PTR(n, 1, 1 | SER_OUT)



        // Serialization Info to skip in

#define SER_INOUT_UINT8(n)            SER_UINT8(n, 1, 1 | SER_IN_OUT)
#define SER_INOUT_UINT16(n)           SER_UINT16(n, 1, 1 | SER_IN_OUT)
#define SER_INOUT_UINT32(n)           SER_UINT32(n, 1, 1 | SER_IN_OUT)
#define SER_INOUT_BOOL(n)             SER_BOOL(n, 1, 1 | SER_IN_OUT)
#define SER_INOUT_STR(n)              SER_STR(n, 1, 1 | SER_IN_OUT)
#define SER_INOUT_STRUCT(n, t)        SER_STRUCT(n, 1, 1 | SER_IN_OUT, t)
#define SER_INOUT_DYN_ARRAY(n, t)     SER_DYN_ARRAY(n, 1, 1 | SER_IN_OUT, t)

#define SER_INOUT_UINT8_PTR(n)        SER_UINT8_PTR(n, 1, 1 | SER_IN_OUT)
#define SER_INOUT_UINT16_PTR(n)       SER_UINT16_PTR(n, 1, 1 | SER_IN_OUT)
#define SER_INOUT_UINT32_PTR(n)       SER_UINT32_PTR(n, 1, 1 | SER_IN_OUT)
#define SER_INOUT_BOOL_PTR(n)         SER_BOOL_PTR(n, 1, 1 | SER_IN_OUT)
#define SER_INOUT_STR_PTR(n)          SER_STR_PTR(n, 1, 1 | SER_IN_OUT)
#define SER_INOUT_DYN_ARRAY_PTR(n, t) SER_DYN_ARRAY_PTR(n, 1, 1 | SER_IN_OUT)


/*
#define SER_RET_VAL_UINT8 (n)\
	{(n), 1, 1 | SER_RET_VAL, do_serialize_uint8, NULL}
#define SER_RET_VAL_UINT16 (n)\
	{(n), 1, 1 | SER_RET_VAL, do_serialize_uint16, NULL}
#define SER_RET_VAL_UINT32(n)\
	{(n), 1, 1 | SER_RET_VAL, do_serialize_uint32, NULL}
#define SER_RET_VAL_BOOL(n)\
	{(n), 1, 1 | SER_RET_VAL, do_serialize_bool, NULL}

#define SER_RET_VAL_UINT8_PTR (n)\
	{(n), SER_RET_VAL_PTR | 1, 1 | SER_RET_VAL, do_serialize_uint8, NULL}
#define SER_RET_VAL_UINT16_PTR (n)\
	{(n), SER_RET_VAL_PTR | 1, 1 | SER_RET_VAL, do_serialize_uint16, NULL}
#define SER_RET_VAL_UINT32_PTR(n)\
	{(n), SER_RET_VAL_PTR | 1, 1 | SER_RET_VAL, do_serialize_uint32, NULL}
#define SER_RET_VAL_BOOL_PTR(n)\
	{(n), SER_RET_VAL_PTR | 1, 1 | SER_RET_VAL, do_serialize_bool, NULL}

#define SER_RET_VAL_STR(n)\
	{(n), SER_RET_VAL_PTR | 1, 1 | SER_RET_VAL, do_serialize_string, NULL}

#define SER_RET_VAL_STRUCT_PTR(n, t)\
	{(n), SER_PTR | 1, 1 | SER_RET_VAL, do_serialize_struct, t##_TypeItems}


*/

#define SER_TYPEINFO_UINT8 \
XmlSerializerInfo uint8_TypeInfo[] = {\
    SER_UINT8("uint8", 1, 1), \
    {NULL, 0, 0, NULL, NULL} \
}
#define SER_TYPEINFO_UINT16 \
XmlSerializerInfo uint16_TypeInfo[] = {\
    SER_UINT16("uint16", 1, 1), \
    {NULL, 0, 0, NULL, NULL} \
}
#define SER_TYPEINFO_UINT32 \
XmlSerializerInfo uint32_TypeInfo[] = {\
    SER_UINT32("uint32", 1, 1), \
    {NULL, 0, 0, NULL, NULL} \
}
#define SER_TYPEINFO_BOOL \
XmlSerializerInfo bool_TypeInfo[] = {\
    SER_BOOL("bool", 1, 1), \
    {NULL, 0, 0, NULL, NULL} \
}
#define SER_TYPEINFO_STRING \
XmlSerializerInfo string_TypeInfo[] = { \
    SER_STR("string", 1, 1), \
    {NULL, 0, 0, NULL, NULL} \
}



#define SER_TYPE_STRUCT(n, t) \
XmlSerializerInfo t##_TypeInfo[] = {\
    {(n), 1, 1, do_serialize_struct, t##_TypeItems}, \
    {NULL, 0, 0, NULL, NULL}\
}


#define SER_START_ITEMS(n, t) \
XmlSerializerInfo t##_TypeItems[] = {

#define SER_END_ITEMS(n, t)\
	{NULL, 0, 0, NULL, NULL} };\
	SER_TYPE_STRUCT(n, t)

#define SER_DECLARE_TYPE(t)\
extern XmlSerializerInfo t##_TypeInfo[];\
extern XmlSerializerInfo t##_TypeItems[]






        // XmlSerializationProc functions for different types

int do_serialize_uint8(struct __XmlSerializationData* data);
int do_serialize_uint16(struct __XmlSerializationData* data);
int do_serialize_uint32(struct __XmlSerializationData* data);
int do_serialize_string(struct __XmlSerializationData* data);
int do_serialize_char_array(struct __XmlSerializationData* data);
int do_serialize_bool(struct __XmlSerializationData* data);
int do_serialize_fixed_size_array(struct __XmlSerializationData* data);
int do_serialize_dyn_size_array(struct __XmlSerializationData* data);
int do_serialize_struct(struct __XmlSerializationData* data);


        // Serializer user interface


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

int ws_serialize(WsContextH cntx, 
                WsXmlNodeH xmlNode, 
                XML_TYPE_PTR dataPtr, 
                XmlSerializerInfo *info, 
                char *name, 
                char *nameNs, 
                char *elementNs, 
                int output);

int ws_serializer_free_mem(WsContextH cntx, 
                XML_TYPE_PTR buf, 
                XmlSerializerInfo *info);

void *ws_deserialize(WsContextH cntx, 
                WsXmlNodeH xmlParent, 
                XmlSerializerInfo *info, 
                char *name, 
                char *nameNs, 
                char *elementNs, 
                int index, 
                int output);





/** @} */

#endif //WS_XML_SERIALIZER_H
