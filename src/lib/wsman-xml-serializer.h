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

typedef unsigned char XML_TYPE_UINT8;
typedef unsigned short XML_TYPE_UINT16;
typedef unsigned long XML_TYPE_UINT32;
typedef int XML_TYPE_BOOL;
typedef char XML_TYPE_CHAR;
typedef void* XML_TYPE_PTR;
typedef char* XML_TYPE_STR;

#define SER_ENUMERATOR_PREFIX	"_XmlSerializeEnum"


//#define SER_ALIGNMENT_MASK	0x000F

// following four for RPC modeling 
#define SER_INOUT_MASK		((unsigned short)0xE000)
#define SER_FLAGS_MASK		((unsigned short)0xE000)

#define SER_IN				((unsigned short)0x8000)
#define SER_OUT				((unsigned short)0x4000)
#define SER_IN_OUT			(SER_OUT | SER_IN)
#define SER_RET_FLAG		((unsigned short)0x2000)
#define SER_RET_VAL			(SER_RET_FLAG | SER_OUT)

#define SER_PTR				0x8000
#define SER_SKIP			0x4000

#define XML_MAX_OCCURS(x) ((int)((unsigned short)((x)->funcMaxCount & (~SER_INOUT_MASK))))
#define XML_MIN_OCCURS(x) ((int)((unsigned short)((x)->flagsMinCount & (~SER_FLAGS_MASK))))

#define XML_IS_PTR(x) ((x)->flagsMinCount & SER_PTR)
#define XML_IS_SKIP(x) ((x)->flagsMinCount & SER_SKIP)

#define XML_IS_IN(x) ((x)->funcMaxCount & SER_IN)
#define XML_IS_OUT(x) ((x)->funcMaxCount & SER_OUT)
#define XML_IS_RETURN(x) ((x)->funcMaxCount & SER_RET_FLAG)


//#define SER_DYNAMIC_SIZE		0x0100
//#define SER_DYNAMIC_ENUM		0x0200
//#define SER_DYNAMIC_ARRAY	(SER_DYNAMIC_ENUM | SER_DYNAMIC_SIZE)

//#define SER_READ_ONLY		0x2000
//#define SER_OPTIONAL			0x4000 // reserved for options like verbose/full

struct __XmlSerializerInfo;
struct __XmlSerializationData;

#define XML_SMAX_INDENT_COUNT	16

struct __XmlSerialiseDynamicSizeData
{
	int count;
	XML_TYPE_PTR data;
};
typedef struct __XmlSerialiseDynamicSizeData XmlSerialiseDynamicSizeData;
	
	
struct __XmlSerialiseDynamicEnumData
{
	XML_TYPE_PTR (*initialize)(struct __XmlSerializationData* serializerData);
	XML_TYPE_PTR (*get)(XML_TYPE_PTR context, int* dataSize);
	int (*put)(XML_TYPE_PTR context, XML_TYPE_PTR data, int dataSize);
	void (*cleanUp)(XML_TYPE_PTR context);
	XML_TYPE_PTR data;
};
typedef struct __XmlSerialiseDynamicEnumData XmlSerialiseDynamicEnumData;

union __XmlSerialiseDynamicArrayData
{
	struct __XmlSerialiseDynamicSizeData dynSize;
	struct __XmlSerialiseDynamicEnumData dynEnum;
};
typedef union __XmlSerialiseDynamicArrayData XmlSerialiseDynamicArrayData;

struct __XmlSerializationData;
// returns binary size of the data element on success, < 0 on failure
typedef int (*XmlSerializationProc)(struct __XmlSerializationData* data);


/**
 * Count can be 1 for single elementd; > 1 for fixed size array; 
 * 0 for dynamic array. 
 */
struct __XmlSerializerInfo
{
	char* name;
	unsigned short flagsMinCount; /**< Minimal Count */
	unsigned short funcMaxCount; /**< Maximal Count */
	//unsigned long ptrAdjustment; // in case of ptr this 2, 4, or 8
	//unsigned long elenemtSize;
	XmlSerializationProc proc; /**< Serialization Processor */
	XML_TYPE_PTR extData; /**< External Data */
	//XmlSerializationEnumProc enumProc;

	// validation info
	// unsigned short minCount;
	// unsigned short maxCount;
	// unsigned short minLength;
	// unsigned short maxLength;
};
typedef struct __XmlSerializerInfo XmlSerializerInfo;

struct __WsSerializerMemEntry
{
    WsContextH cntx;
    char buf[1];
};
typedef struct __WsSerializerMemEntry WsSerializerMemEntry;


#define ELEMENT_COUNT_MASK	0xffff

struct __dummy
{
	char __x;
};

#define XML_SADJUSTMENT		(sizeof(struct __dummy))


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

/**
 * @def SER_UINT8
 * Serialize Unsigned Integer 8Bit
 * @param n Variable name
 * @param x Minimum count, 1 for single elementd; > 1 for fixed size array;  0 for dynamic array. 
 * @param y Maximum count, 1 for single elementd; > 1 for fixed size array;  0 for dynamic array. 
 */
#define SER_UINT8(n, x, y)\
	{(n), (x), (y), do_serialize_uint8, NULL}
#define SER_UINT16(n, x, y)\
	{(n), (x), (y), do_serialize_uint16, NULL}
#define SER_UINT32(n, x, y)\
	{(n), (x), (y), do_serialize_uint32, NULL}
#define SER_BOOL(n, x, y)\
	{(n), (x), (y), do_serialize_bool, NULL}

#define SER_UINT8_PTR (n, x, y)\
	{(n), SER_PTR | (x), (y), do_serialize_uint8, NULL}
#define SER_UINT16_PTR (n, x, y)\
	{(n), SER_PTR | (x), (y), do_serialize_uint16, NULL}
#define SER_UINT32_PTR(n, x, y)\
	{(n), SER_PTR | (x), (y), do_serialize_uint32, NULL}
#define SER_BOOL_PTR(n, x, y)\
	{(n), SER_PTR | (x), (y), do_serialize_bool, NULL}

#define SER_STR(n, x, y)\
	{(n), SER_PTR | (x), (y), do_serialize_string, NULL}

#define SER_STRUCT(n, x, y, t)\
	{(n), (x), (y), do_serialize_struct, t##_TypeItems}
	
#define SER_DYN_ARRAY_PTR(t)\
	{NULL, SER_PTR, 1, do_serialize_dyn_size_array, t##_TypeInfo}

#define SER_DYN_ARRAY(t)\
	{NULL, 1, 1, do_serialize_dyn_size_array, t##_TypeInfo}


#define SER_IN_UINT8 (n)\
	{(n), 1, 1 | SER_IN, do_serialize_uint8, NULL}
#define SER_IN_UINT16(n)\
	{(n), 1, 1 | SER_IN, do_serialize_uint16, NULL}
#define SER_IN_UINT32(n)\
	{(n), 1, 1 | SER_IN, do_serialize_uint32, NULL}
#define SER_IN_BOOL(n)\
	{(n), 1, 1 | SER_IN, do_serialize_bool, NULL}

#define SER_IN_UINT8_PTR (n)\
	{(n), SER_PTR | 1, 1 | SER_IN, do_serialize_uint8, NULL}
#define SER_IN_UINT16_PTR(n)\
	{(n), SER_PTR | 1, 1 | SER_IN, do_serialize_uint16, NULL}
#define SER_IN_UINT32_PTR(n)\
	{(n), SER_PTR | 1, 1 | SER_IN, do_serialize_uint32, NULL}
#define SER_IN_BOOL_PTR(n)\
	{(n), SER_PTR | 1, 1 | SER_IN, do_serialize_bool, NULL}

#define SER_IN_STR(n)\
	{(n), SER_PTR | 1, 1 | SER_IN, do_serialize_string, NULL}

#define SER_IN_STRUCT_PTR(n, t)\
	{(n), SER_PTR | 1, 1 | SER_IN, do_serialize_struct, t##_TypeItems}


#define SER_OUT_UINT8 (n)\
	{(n), 1, 1 | SER_OUT, do_serialize_uint8, NULL}
#define SER_OUT_UINT16 (n)\
	{(n), 1, 1 | SER_OUT, do_serialize_uint16, NULL}
#define SER_OUT_UINT32(n)\
	{(n), 1, 1 | SER_OUT, do_serialize_uint32, NULL}
#define SER_OUT_BOOL(n)\
	{(n), 1, 1 | SER_OUT, do_serialize_bool, NULL}

#define SER_OUT_UINT8_PTR(n)\
	{(n), SER_PTR | 1, 1 | SER_OUT, do_serialize_uint8, NULL}
#define SER_OUT_UINT16_PTR(n)\
	{(n), SER_PTR | 1, 1 | SER_OUT, do_serialize_uint16, NULL}
#define SER_OUT_UINT32_PTR(n)\
	{(n), SER_PTR | 1, 1 | SER_OUT, do_serialize_uint32, NULL}
#define SER_OUT_BOOL_PTR(n)\
	{(n), SER_PTR | 1, 1 | SER_OUT, do_serialize_bool, NULL}

#define SER_OUT_STR(n)\
	{(n), SER_PTR | 1, 1 | SER_OUT, do_serialize_string, NULL}

#define SER_OUT_STRUCT_PTR(n, t)\
	{(n), SER_PTR | 1, 1 | SER_OUT, do_serialize_struct, t##_TypeItems}


#define SER_INOUT_UINT8 (n)\
	{(n), 1, 1 | SER_INOUT, do_serialize_uint8, NULL}
#define SER_INOUT_UINT16 (n)\
	{(n), 1, 1 | SER_INOUT, do_serialize_uint16, NULL}
#define SER_INOUT_UINT32(n)\
	{(n), 1, 1 | SER_INOUT, do_serialize_uint32, NULL}
#define SER_INOUT_BOOL(n)\
	{(n), 1, 1 | SER_INOUT, do_serialize_bool, NULL}

#define SER_INOUT_UINT8_PTR (n)\
	{(n), SER_INOUT_PTR | 1, 1 | SER_INOUT, do_serialize_uint8, NULL}
#define SER_INOUT_UINT16_PTR (n)\
	{(n), SER_INOUT_PTR | 1, 1 | SER_INOUT, do_serialize_uint16, NULL}
#define SER_INOUT_UINT32_PTR(n)\
	{(n), SER_INOUT_PTR | 1, 1 | SER_INOUT, do_serialize_uint32, NULL}
#define SER_INOUT_BOOL_PTR(n)\
	{(n), SER_INOUT_PTR | 1, 1 | SER_INOUT, do_serialize_bool, NULL}

#define SER_INOUT_STR(n)\
	{(n), SER_INOUT_PTR | 1, 1 | SER_INOUT, do_serialize_string, NULL}

#define SER_INOUT_STRUCT_PTR(n, t)\
	{(n), SER_PTR | 1, 1 | SER_INOUT, do_serialize_struct, t##_TypeItems}


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

#define SER_TYPE_STRUCT(n, t)\
struct __XmlSerializerInfo t##_TypeInfo[] = {\
{(n), 1, 1, do_serialize_struct, t##_TypeItems} }

#define SER_LAST {NULL, 0, 0, NULL, NULL}


#define SER_START_ITEMS(n, t)\
struct __XmlSerializerInfo t##_TypeItems[] = {

#define SER_END_ITEMS(n, t)\
	{NULL, 0, 0, NULL, NULL} };\
	SER_TYPE_STRUCT(n, t)

#define SER_START_END_POINTS(t) WsDispatchEndPointInfo t##_EndPoints[] = {

#define SER_FINISH_END_POINTS(t) END_POINT_LAST }


#define SER_START_NAMESPACES(t) WsSupportedNamespaces t##_Namespaces[] = {

#define SER_FINISH_NAMESPACES(t) NAMESPACE_LAST }

#define SER_DECLARE_EP_ARRAY(t)\
extern WsDispatchEndPointInfo t##_EndPoints[]

#define SER_DECLARE_TYPE(t)\
extern struct __XmlSerializerInfo t##_TypeInfo[];\
extern struct __XmlSerializerInfo t##_TypeItems[]




//#define SER_END_ITEMS		{NULL, 0, 0, NULL, NULL} }

struct __XmlSerializationData;
int do_serialize_uint8(struct __XmlSerializationData* data);
int do_serialize_uint16(struct __XmlSerializationData* data);
int do_serialize_uint32(struct __XmlSerializationData* data);
int do_serialize_string(struct __XmlSerializationData* data);
int do_serialize_char_array(struct __XmlSerializationData* data);
int do_serialize_bool(struct __XmlSerializationData* data);
int do_serialize_fixed_size_array(struct __XmlSerializationData* data);
int do_serialize_dyn_size_array(struct __XmlSerializationData* data);
int do_serialize_struct(struct __XmlSerializationData* data);


/** @} */

#endif //WS_XML_SERIALIZER_H
