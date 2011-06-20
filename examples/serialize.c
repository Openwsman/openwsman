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
 * @author Vadim Revyakin
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>

#include "u/libu.h"
#include "wsman-soap.h"
#include "wsman-xml.h"
#include "wsman-xml-serializer.h"
#include "wsman-xml-serialize.h"
#include "wsman-epr.h"
#include "wsman-debug.h"

#define CLASSNAME "Sample"



static void example1(void)
{

#define EX1_NS "http://schemas.dmtf.org/wbem/wscim/1/cim-schema/2/CIM_ComputerSystem"

	XML_TYPE_UINT16 myshorts[] = { 5, 11, 14, 19, 27, 36 };
	SER_TYPEINFO_UINT16;
	struct __Sample_Servie {
		XML_TYPE_BOOL AcceptPause;
		XML_TYPE_BOOL AcceptStop;
		XML_TYPE_STR Caption;
		XML_TYPE_UINT32 CheckPoint;
		XML_TYPE_STR CreationClassName;
		XML_TYPE_STR Description;
		XML_TYPE_BOOL DesktopInteract;
		XML_TYPE_STR DisplayName;
		XML_TYPE_STR ErrorControl;
		XML_TYPE_UINT32 ExitCode;
		XML_TYPE_STR InstallDate;
		XML_TYPE_STR Name;
		XML_TYPE_STR PathName;
		XML_TYPE_UINT32 ProcessId;
		XML_TYPE_UINT32 ServiceSpecificExitCode;
		XML_TYPE_STR ServiceType;
		XML_TYPE_BOOL Started;
		XML_TYPE_STR StartMode;
		XML_TYPE_STR StartName;
		XML_TYPE_STR State;
		XML_TYPE_STR Status;
		XML_TYPE_STR SystemCreationClassName;
		XML_TYPE_STR SystemName;
		XML_TYPE_UINT32 TagId;
		XML_TYPE_UINT32 WaitHint;
		XML_TYPE_UINT64 Uint64;
		XML_TYPE_DYN_ARRAY shorts;
	};
	typedef struct __Sample_Servie Sample_Servie;

	Sample_Servie servie = {
		0,
		1,
		"Caption",
		30,
		"CreationClassName",
		"Description",
		1,
		"DisplayName",
		"ErrorControl",
		50,
		"InstallDate",
		"Name",
		"PathName",
		60,
		70,
		"ServiceType",
		0,
		"StartMode",
		"StartName",
		"State",
		"Status",
		"SystemCreationClassName",
		"SystemName",
		90,
		100,
		1000000,
		{6, myshorts }
	};

	SER_START_ITEMS(Sample_Servie)
	    SER_BOOL("AcceptPause", 1),
	    SER_BOOL("AcceptStop", 1),
	    SER_STR("Caption", 1),
	    SER_UINT32("CheckPoint", 1),
	    SER_STR("CreationClassName", 1),
	    SER_STR("Description", 1),
	    SER_BOOL("DesktopInteract", 1),
	    SER_NS_STR(EX1_NS, "DisplayName", 1),
	    SER_STR("ErrorControl", 1),
	    SER_UINT32("ExitCode", 1),
	    SER_STR("InstallDate", 1),
	    SER_STR("Name", 1),
	    SER_STR("PathName", 1),
	    SER_UINT32("ProcessId", 1),
	    SER_UINT32("ServiceSpecificExitCode", 1),
	    SER_STR("ServiceType", 1),
	    SER_BOOL("Started", 1),
	    SER_STR("StartMode", 1),
	    SER_STR("StartName", 1),
	    SER_STR("State", 1),
	    SER_STR("Status", 1),
	    SER_STR("SystemCreationClassName", 1),
	    SER_STR("SystemName", 1),
	    SER_UINT32("TagId", 1),
	    SER_UINT32("WaitHint", 1), 
	    SER_UINT64("Uint64", 1), 
	    SER_DYN_ARRAY("shorts", 0, 1000, uint16),
	    SER_END_ITEMS(Sample_Servie);

	WsSerializerContextH cntx;
	WsXmlDocH doc;
	WsXmlNodeH node;
	int retval;

	printf("\n\n   ********   example1. Basic types  ********\n");

	cntx = ws_serializer_init();
	if (cntx == NULL) {
		printf("Error ws_create_runtime\n");
		return;
	}
	doc = ws_xml_create_doc( NULL, "example");
	node = ws_xml_get_doc_root(doc);

	retval = ws_serialize(cntx, node, &servie, Sample_Servie_TypeInfo,
			      CLASSNAME, NULL, NULL, 0);
	printf("ws_serialize: %d\n", retval);
	ws_xml_dump_node_tree(stdout, node);
	node = ws_xml_get_doc_root(doc);

	//WsXmlDocH xx = ws_xml_read_file( ws_context_get_runtime(cntx), "./test.xml", "UTF-8", 0 );
	//node = ws_xml_get_doc_root(xx);
	Sample_Servie *cs = (Sample_Servie *) ws_deserialize(cntx,
							     node,
							     Sample_Servie_TypeInfo,
							     CLASSNAME,
							     NULL, NULL,
							     0, 0);
	if (cs == NULL) {
		printf("Error in ws_serialize\n");
		return;
	}
	

	retval = memcmp(cs, &servie, sizeof(&servie));
	if (retval) {
		printf("Not compared (%d)   -    FAILED\n", retval);
		printf("%d   :  %d\n", servie.AcceptPause, cs->AcceptPause);
		printf("%d   :  %d\n", servie.AcceptStop, cs->AcceptStop);
		printf("%s   :  %s\n", servie.Caption, cs->Caption);
	}
}




static void example2(void)
{

	typedef struct {
		XML_TYPE_UINT8 byte1;
		XML_TYPE_UINT32 int1;
		XML_TYPE_UINT8 byte2;
	} Foo;

	typedef struct {
		XML_TYPE_UINT8 byte1;
		XML_TYPE_UINT16 short1;
		XML_TYPE_UINT32 int1;
		char *string1;
		Foo foo;
	} Sample;


	Sample sample = { 1, 2, 4, "string", {5, 196, 8} };

	SER_START_ITEMS(Foo)
	    SER_UINT8("FOOBYTE1", 1),
	    SER_UINT32("FOOINT32", 1),
	    SER_UINT8("FOOBYTE2", 1), 
	SER_END_ITEMS(Foo);


	SER_START_ITEMS(Sample)
	    SER_UINT8("BYTE", 1),
	    SER_UINT16("SHORT", 1),
	    SER_UINT32("INT32", 1),
	    SER_STR("STRING", 1),
	    SER_STRUCT("FOO", 1, Foo), 
	SER_END_ITEMS(Sample);

	WsSerializerContextH cntx;
	WsXmlDocH doc;
	WsXmlNodeH node;
	int retval;

	printf
	    ("\n\n   ********   example2. Structure with pads.  ********\n");

	cntx = ws_serializer_init();
	if (cntx == NULL) {
		printf("Error ws_create_runtime\n");
		return;
	}
	doc = ws_xml_create_doc( NULL, "example");
	node = ws_xml_get_doc_root(doc);

	retval = ws_serialize(cntx, node, &sample, Sample_TypeInfo,
			      CLASSNAME, NULL, NULL, 0);
	printf("ws_serialize: %d\n", retval);
	ws_xml_dump_node_tree(stdout, node);

	node = ws_xml_get_doc_root(doc);
	Sample *cs = (Sample *) ws_deserialize(cntx,
					       node,
					       Sample_TypeInfo,
					       CLASSNAME, NULL, NULL,
					       0, 0);
	if (cs == NULL) {
		printf("Errror ws_deserialize\n");
		return;
	}

	printf("\n      initial and deserialized structures\n");
	printf("     byte1    =    %d : %d\n", sample.byte1, cs->byte1);
	printf("     short1   =    %d : %d\n", sample.short1, cs->short1);
	printf("     int1     =    %d : %d\n", sample.int1, cs->int1);
	printf("     string1  =    <%s> : <%s>\n", sample.string1,
	       cs->string1);
	printf("     foo :\n");
	printf("        byte1    =    %d : %d\n", sample.foo.byte1,
	       cs->foo.byte1);
	printf("        int1     =    %d : %d\n", sample.foo.int1,
	       cs->foo.int1);
	printf("        byte2    =    %d : %d\n", sample.foo.byte2,
	       cs->foo.byte2);
}



static void example3(void)
{
	typedef struct {
		XML_TYPE_UINT8 a;
		XML_TYPE_UINT8 b;
		XML_TYPE_UINT8 c;
		XML_TYPE_UINT8 pad;
		XML_TYPE_STR string;
	} Sample;



	SER_START_ITEMS(Sample)
	    SER_UINT8("a", 1),
	    SER_UINT8("b", 1),
	    SER_UINT8("c", 1),
	    SER_IN_UINT8("pad", 1),
	    SER_STR("string", 1), SER_END_ITEMS(Sample);

	Sample sample = { 'a', 'b', 'c', 'x', "simple string" };
	// Sample *p = NULL;


	WsSerializerContextH cntx;
	WsXmlDocH doc;
	WsXmlNodeH node;
	int retval;

	printf("\n\n   ********   example3. Skip elements.  ********\n");

	cntx = ws_serializer_init();
	if (cntx == NULL) {
		printf("Error ws_create_runtime\n");
		return;
	}
	doc = ws_xml_create_doc( NULL, "example");
	node = ws_xml_get_doc_root(doc);

	retval = ws_serialize(cntx, node, &sample, Sample_TypeInfo,
			      CLASSNAME, NULL, NULL, 0);
	printf("ws_serialize: %d\n", retval);
	ws_xml_dump_node_tree(stdout, node);

	printf("\n\nws_deserialize (prints original : result):\n");

	node = ws_xml_get_doc_root(doc);
	Sample *cs = (Sample *) ws_deserialize(cntx,
					       node,
					       Sample_TypeInfo,
					       CLASSNAME, NULL, NULL,
					       0, 0);
	if (cs == NULL) {
		printf("Errror ws_serialize\n");
		return;
	}

	printf("a   = %c   :  %c\n", sample.a, cs->a);
	printf("b   = %c   :  %c\n", sample.b, cs->b);
	printf("c   = %c   :  %c\n", sample.c, cs->c);
	printf("pad = %c(%d)   :  %c(%d)\n", sample.pad, sample.pad,
	       cs->pad, cs->pad);
	printf("string = <%s>   :  <%s>\n", sample.string, cs->string);
}


static void example4(void)
{

	typedef struct {
		XML_TYPE_BOOL a;
		XML_TYPE_STR string;
		XML_TYPE_BOOL b;
	} Embed;

	typedef struct {
		XML_TYPE_UINT32 A;
		Embed EMBED[2];
		XML_TYPE_STR STRING;
	} Sample;


	Sample sample = {
		10,
		{{1, "string 1", 0}, {0, "string 2", 1},},
		"STRING",
	};

	SER_START_ITEMS(Embed)
	    SER_BOOL("a", 1),
	    SER_STR("string", 1), SER_BOOL("b", 1), SER_END_ITEMS(Embed);

	SER_START_ITEMS(Sample)
	    SER_UINT32("A", 1),
	    SER_STRUCT("EMBED", 2, Embed),
	    SER_STR("STRING", 1), SER_END_ITEMS(Sample);

	WsSerializerContextH cntx;
	WsXmlDocH doc;
	WsXmlNodeH node;
	int retval;

	printf
	    ("\n\n   ********   example4. Static structure array  ********\n");
	cntx = ws_serializer_init();
	if (cntx == NULL) {
		printf("Error ws_create_runtime\n");
		return;
	}
	doc = ws_xml_create_doc(NULL, "example");
	node = ws_xml_get_doc_root(doc);

	retval = ws_serialize(cntx, node, &sample, Sample_TypeInfo,
			      CLASSNAME, NULL, NULL, 0);
	printf("ws_serialize: %d\n", retval);
	ws_xml_dump_node_tree(stdout, node);
}





static void example5(void)
{


	typedef struct {
		XML_TYPE_BOOL AcceptPause;
		XML_TYPE_STR Caption;
	} Foo;

	Foo foos[] = {
		{1, "Caption 1"},
		{0, "Caption 2"},
		{1, "Caption 1",},
		{0, "Caption 2",},
	};


	SER_START_ITEMS(Foo)
	    SER_BOOL("AcceptPause", 1),
	    SER_STR("Caption", 1), SER_END_ITEMS(Foo);

	XML_TYPE_UINT16 myshorts[] = { 5, 11, 14, 19, 27, 36 };
	SER_TYPEINFO_UINT16;

	typedef struct {
		XML_TYPE_STR city;
		XML_TYPE_DYN_ARRAY shorts;
		XML_TYPE_DYN_ARRAY foos;
		XML_TYPE_UINT16 tag;
	} Sample;

	Sample sample = { "Moscow", {6, myshorts}, {2, foos}, 99 };

	SER_START_ITEMS(Sample)
	    SER_STR("city", 1),
	    SER_DYN_ARRAY("shorts", 0, 1000, uint16),
	    SER_DYN_ARRAY("foos", 0, 1000, Foo),
	    SER_UINT16("tag", 1), SER_END_ITEMS(Sample);

	WsSerializerContextH cntx;
	WsXmlDocH doc;
	WsXmlNodeH node;
	int retval;

	printf("\n\n   ********   example 5: Dynamic arrays  ********\n");

	cntx = ws_serializer_init();
	if (cntx == NULL) {
		printf("Error ws_create_runtime\n");
		return;
	}
	doc = ws_xml_create_doc( NULL, "example");
	node = ws_xml_get_doc_root(doc);

	retval = ws_serialize(cntx, node, &sample, Sample_TypeInfo,
			      CLASSNAME, NULL, NULL, 0);
	printf("\n\nws_serialize: %d\n", retval);
	ws_xml_dump_node_tree(stdout, node);

	node = ws_xml_get_doc_root(doc);

	printf("\n\nws_deserialize:\n");
	Sample *cs = (Sample *) ws_deserialize(cntx,
					       node,
					       Sample_TypeInfo,
					       CLASSNAME, NULL, NULL,
					       0, 0);
	if (cs == NULL) {
		printf("Error ws_deserialize\n");
		return;
	}
	int i;
	printf("shorts count = %d\n", cs->shorts.count);
	printf("foos count   = %d\n", cs->foos.count);
	printf("\n");
	printf("    city = <%s>\n", cs->city);
	if (cs->shorts.data == NULL) {
		printf("No uint16 objects\n");
		goto AFTER_SHORTS;
	}
	unsigned short *newuints = (unsigned short *) cs->shorts.data;
	printf("    shorts = {");
	for (i = 0; i < cs->shorts.count; i++) {
		printf("%u, ", *newuints);
		newuints++;
	}
	printf("}\n");
      AFTER_SHORTS:
	if (cs->foos.data == NULL) {
		printf("No foo objects\n");
		goto AFTER_FOOS;
	}
	Foo *newfoos = cs->foos.data;
	for (i = 0; i < cs->foos.count; i++) {
		printf("   ====   Foo %d =====\n", i);
		printf("    AcceptPause  =   %d\n", newfoos->AcceptPause);
		printf("    Caption       =   <%s>\n", newfoos->Caption);
		printf("   ====   End of Foo %d =====\n", i);
		newfoos++;
	}
      AFTER_FOOS:
	printf("    tag = %d\n", cs->tag);
}

static void example6(void)
{
	typedef struct {
		struct {
			XML_TYPE_UINT8 body;
			XML_NODE_ATTR *attrs;
		} uint8_with_attrs;
		struct {
			XML_TYPE_UINT16 body;
			XML_NODE_ATTR *attrs;
		} uint16_with_attrs;
		struct {
			XML_TYPE_UINT32 body;
			XML_NODE_ATTR *attrs;
		} uint32_with_attrs;
		struct {
			XML_TYPE_BOOL body;
			XML_NODE_ATTR *attrs;
		} bool_with_attrs;
		struct {
			XML_TYPE_STR body;
			XML_NODE_ATTR *attrs;
		} str_with_attrs;
	} Dummy;

	typedef struct {
		struct {
			Dummy body;
			XML_NODE_ATTR *attrs;
		} struct_with_attrs;
	} Sample;

	XML_NODE_ATTR attrs[] = {
		{NULL, NULL, "Uint8AttrName1", "Uint8AttrValue1"},
		{NULL, NULL, "Uint16AttrName1", "Uint16AttrValue1"},
		{NULL, NULL, "Uint32AttrName1", "Uint32AttrValue1"},
		{NULL, NULL, "BoolAttrName1", "BoolAttrValue1"},
		{NULL, NULL, "StringAttrName1", "StringAttrValue1"},
	};
	XML_NODE_ATTR str_attrs[3] = {
		{NULL, NULL, "AttrName1", "AttrValue1"},
		{NULL, NULL, "AttrName2", "AttrValue2"},
		{NULL, XML_NS_ADDRESSING, "AttrQName", "AttrValue3"},
	};
	Sample sample = {
		{
		 {
		  {8, &attrs[0]},
		  {16, &attrs[1]},
		  {32, &attrs[2]},
		  {0, &attrs[3]},
		  {"string", &attrs[4]},
		  },
		 &str_attrs[0]
		 }
	};
	str_attrs[0].next = &str_attrs[1];
	str_attrs[1].next = &str_attrs[2];

	SER_START_ITEMS(Dummy)
	    SER_ATTR_NS_UINT8_FLAGS(NULL, "UINT8", 1, 0),
	    SER_ATTR_NS_UINT16_FLAGS(NULL, "UINT16", 1, 0),
	    SER_ATTR_NS_UINT32_FLAGS(NULL, "UINT32", 1, 0),
	    SER_ATTR_NS_BOOL_FLAGS(NULL, "BOOL", 1, 0),
	    SER_ATTR_NS_STR_FLAGS(NULL, "STRING", 1, 0),
	    SER_END_ITEMS(Dummy);

	SER_START_ITEMS(Sample)
	    SER_ATTR_NS_STRUCT_FLAGS(XML_NS_WS_MAN, "STRUCT", 1, 0, Dummy),
	    SER_END_ITEMS(Sample);

	WsSerializerContextH cntx;
	WsXmlDocH doc;
	WsXmlNodeH node;
	int retval;

	printf
	    ("\n\n   ********   example 6. Nodes with attributes  ********\n");

	cntx = ws_serializer_init();
	if (cntx == NULL) {
		printf("Error ws_create_runtime\n");
		return;
	}
	doc = ws_xml_create_doc( NULL, "example");
	node = ws_xml_get_doc_root(doc);

	retval = ws_serialize(cntx, node, &sample, Sample_TypeInfo,
			      CLASSNAME, XML_NS_WS_MAN, NULL, 0);
	printf("\n\nws_serialize: %d\n", retval);
	ws_xml_dump_node_tree(stdout, node);

	Sample *news;
	printf("\n\nws_deserialize:\n");
	news = (Sample *) ws_deserialize(cntx,
					 node,
					 Sample_TypeInfo,
					 CLASSNAME,
					 XML_NS_ADDRESSING, NULL, 0, 0);
	if (news == NULL) {
		printf("Errror ws_deserialize\n");
		return;
	}

	XML_NODE_ATTR *nattrs;
	Dummy *dm = &(news->struct_with_attrs.body);
	printf("****     Deserialized document  %p *****\n", news);
	printf("struct_with_attrs.body (");
	nattrs = news->struct_with_attrs.attrs;
	while (nattrs) {
		printf("%s:%s=\"%s\" ", nattrs->ns, nattrs->name,
		       nattrs->value);
		nattrs = nattrs->next;
	}
	printf(")\n");

	printf("    uint8_with_attrs = %d (", dm->uint8_with_attrs.body);
	nattrs = dm->uint8_with_attrs.attrs;
	while (nattrs) {
		printf("%s:%s=\"%s\" ", nattrs->ns, nattrs->name,
		       nattrs->value);
		nattrs = nattrs->next;
	}
	printf(")\n");

	printf("    uint16_with_attrs = %d (", dm->uint16_with_attrs.body);
	nattrs = dm->uint16_with_attrs.attrs;
	while (nattrs) {
		printf("%s:%s=\"%s\" ", nattrs->ns, nattrs->name,
		       nattrs->value);
		nattrs = nattrs->next;
	}
	printf(")\n");

	printf("    uint32_with_attrs = %d (", dm->uint32_with_attrs.body);
	nattrs = dm->uint32_with_attrs.attrs;
	while (nattrs) {
		printf("%s:%s=\"%s\" ", nattrs->ns, nattrs->name,
		       nattrs->value);
		nattrs = nattrs->next;
	}
	printf(")\n");

	printf("    bool_with_attrs = %d (", dm->bool_with_attrs.body);
	nattrs = dm->bool_with_attrs.attrs;
	while (nattrs) {
		printf("%s:%s=\"%s\" ", nattrs->ns, nattrs->name,
		       nattrs->value);
		nattrs = nattrs->next;
	}
	printf(")\n");

	printf("    str_with_attrs = %s (", dm->str_with_attrs.body);
	nattrs = dm->str_with_attrs.attrs;
	while (nattrs) {
		printf("%s:%s=\"%s\" ", nattrs->ns, nattrs->name,
		       nattrs->value);
		nattrs = nattrs->next;
	}
	printf(")\n");
}




static void example7(void)
{
	typedef struct {
		XML_TYPE_STR value;
		XML_NODE_ATTR *attrs;
	} Selector;

	SER_TYPEINFO_STRING_ATTR;


	typedef struct {
		XML_TYPE_DYN_ARRAY selectors;
	} SelectorSet;

	SER_START_ITEMS(SelectorSet)
	    SER_NS_DYN_ARRAY(XML_NS_WS_MAN, WSM_SELECTOR, 0, 1000,
			     string_attr), SER_END_ITEMS(SelectorSet);


	typedef struct {
		XML_TYPE_STR uri;
		SelectorSet selectorset;
	} ReferenceParameters;

	SER_START_ITEMS(ReferenceParameters)
	    SER_NS_STR(XML_NS_WS_MAN, WSM_RESOURCE_URI, 1),
	    SER_NS_STRUCT(XML_NS_WS_MAN, WSM_SELECTOR_SET, 1, SelectorSet),
	    SER_END_ITEMS(ReferenceParameters);

	typedef struct {
		XML_TYPE_STR address;
		ReferenceParameters refparams;
	} EPR;

	SER_START_ITEMS(EPR)
	    SER_NS_STR(XML_NS_ADDRESSING, WSA_ADDRESS, 1),
	    SER_NS_STRUCT(XML_NS_ADDRESSING, WSA_REFERENCE_PARAMETERS, 1,
			  ReferenceParameters), SER_END_ITEMS(EPR);

	XML_NODE_ATTR attrs[3] = {
		{NULL, NULL, "Name", "SelName1"},
		{NULL, NULL, "Name", "SelName2"},
		{NULL, NULL, "Name", "SelName3"},
	};
	Selector selectors[] = {
		{"selector1", &attrs[0]},
		{"selector2", &attrs[1]},
		{"selector3", &attrs[2]}
	};
	EPR Epr = {
		"http://localhost:8889/wsman",
		{"http://acme.org/hardware/2005/02/storage/physDisk",
		 {{3, selectors}}
		 }
	};

	WsSerializerContextH cntx;
	WsXmlDocH doc;
	WsXmlNodeH node;
	int retval;

	printf
	    ("\n\n   ********   example7. Endpoint Reference  ********\n");

	cntx = ws_serializer_init();
	if (cntx == NULL) {
		printf("Error ws_create_runtime\n");
		return;
	}
	doc = ws_xml_create_doc( NULL, "example");
	node = ws_xml_get_doc_root(doc);

	retval = ws_serialize(cntx, node, &Epr, EPR_TypeInfo,
			      "EndpointReference", XML_NS_ADDRESSING, NULL,
			      0);
	printf("\n\nws_serialize: %d\n", retval);
	ws_xml_dump_node_tree(stdout, node);

	EPR *newEPR;
	node = ws_xml_get_doc_root(doc);

	printf("\n\nws_deserialize:\n");
	newEPR = (EPR *) ws_deserialize(cntx,
					node,
					EPR_TypeInfo,
					"EndpointReference",
					XML_NS_ADDRESSING, NULL, 0, 0);
	if (newEPR == NULL) {
		printf("Errror ws_deserialize\n");
		return;
	}

	printf("****     Deserialized document   *****\n");
	printf("address             = %s\n", newEPR->address);
	printf("refparams.uri       = %s\n", newEPR->refparams.uri);
	int i;
	Selector *ss =
	    (Selector *) newEPR->refparams.selectorset.selectors.data;
	if (ss == NULL) {
		printf
		    ("    !!!!  newEPR->refparams.selectors.data == NULL\n");
		return;
	}
	for (i = 0; i < newEPR->refparams.selectorset.selectors.count; i++) {
		Selector *s;
		s = ss + i;
		printf("    Selector(");
		XML_NODE_ATTR *a = s->attrs;
		while (a) {
			printf("%s:%s=%s",
			       a->ns ? a->ns : "", a->name, a->value);
			a = a->next;
		}
		printf(")  =  %s\n", s->value);
	}
}



static void example8(void)
{

	printf
	    ("\n\n   ********   example8. XML datetime deserialization  ********\n");

	char xml_dttm[] = "2007-02-13T12:39:14-03:30";
	XML_DATETIME dttm;

	if (ws_deserialize_datetime(xml_dttm, &dttm)) {
		printf("deserialization failed\n");
		return;
	}
	printf("XML datetime =  %s\n", xml_dttm);
	printf("deserialed = %u-%u-%uT%u:%u:%u  %i\n", dttm.tm.tm_year,
	       dttm.tm.tm_mon, dttm.tm.tm_mday, dttm.tm.tm_hour,
	       dttm.tm.tm_min, dttm.tm.tm_sec, dttm.tz_min);
}





// No serialization examples(shttpd_0)

#if 0
static void example106()
{
	char *data =
	    "<dummy><qq>This is qq body</qq><pp>This is pp</pp></dummy>";
	//      WsXmlDocH       response;
	WsManClient *cl = wsman_create_client("mstevbakrov.ims.intel.com",
					      8889,
					      "/wsman",
					      "http",
					      "wsman",
					      "secret");

	client_opt_t *options = wsmc_options_init();

	WsXmlDocH request = wsmc_create_request(cl,
							"http://schemas.dmtf.org/wbem/wscim/1/cim-schema/2/CIM_ComputerSystem",
							options,
							WSMAN_ACTION_TRANSFER_CREATE,
							NULL, NULL);
	WsXmlDocH d =
	    wsmc_read_memory(cl, data, strlen(data), NULL, 0);
	ws_xml_dump_node_tree(stdout, ws_xml_get_doc_root(d));
	//      WsXmlNodeH n = ws_xml_get_doc_root(d);

	ws_xml_duplicate_tree(ws_xml_get_soap_body(request),
			      ws_xml_get_doc_root(d));
	ws_xml_destroy_doc(d);
	//      ws_xml_copy_node(ws_xml_get_doc_root(d), ws_xml_get_soap_body(request));
	ws_xml_dump_node_tree(stdout, ws_xml_get_doc_root(request));
	wsman_release_client(cl);
	ws_xml_destroy_doc(request);
}

#endif


static void example9(void)
{
	typedef struct {
	}empty;
	SER_START_ITEMS(empty)
	SER_END_ITEMS(empty);
	char xmlstring[] = "<example></example>";
	WsXmlDocH doc = ws_xml_read_memory(xmlstring, strlen(xmlstring), "UTF-8", 0);
	WsXmlNodeH node = ws_xml_get_doc_root(doc);
	WsSerializerContextH cntx = ws_serializer_init();
	if (cntx == NULL) {
                printf("Error ws_create_runtime\n");
                return;
        }
	void *ptr = NULL;
	ptr = ws_deserialize(cntx,
                                        node,
                                        empty_TypeItems,
                                        NULL,
                                        NULL, NULL, 0, 0);
	printf("ptr = %p\n", ptr);
	ws_serializer_cleanup(cntx); 

}

static void example10()
{
	char xmlstring[] = "<example><PowerState xsi:nil=\"true\" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\"/></example>";
//	char xmlstring[] = "<example><PowerState state = \"critical\">5</PowerState></example>";
	typedef struct {
                XML_TYPE_STR value;
                XML_NODE_ATTR *attrs;
        } item;
        SER_TYPEINFO_STRING_ATTR;
	WsXmlDocH doc = ws_xml_read_memory(xmlstring, strlen(xmlstring), "UTF-8", 0);
	WsXmlNodeH node = ws_xml_get_doc_root(doc);
	 printf
            ("\n\n   ********   example10. xsi:nil attibute test  ********\n");
	WsSerializerContextH cntx = ws_serializer_init();
	if (cntx == NULL) {
                printf("Error ws_create_runtime\n");
                return;
        }
	item *value = (item *)ws_deserialize(cntx, node, string_attr_TypeInfo, "PowerState", NULL, NULL, 0, 0);
	if(value->attrs)
		printf("*********attrs->ns = %s attrs->name = %s attrs->value = %s*********\n", value->attrs->ns, value->attrs->name, value->attrs->value);
	if(ws_havenilvalue(value->attrs))
		printf("******** the attribute has xsi:nil ********\n");
	ws_xml_destroy_doc(doc);
	ws_serializer_cleanup(cntx);
}

static void example11()
{
	typedef struct {
		XML_TYPE_INT8 byte1;
		XML_TYPE_INT32 int1;
		XML_TYPE_INT8 byte2;
	} Foo;

	typedef struct {
		XML_TYPE_INT8 byte1;
		XML_TYPE_INT16 short1;
		XML_TYPE_INT32 int1;
		char *string1;
		Foo foo;
	} Sample;


	Sample sample = { -1, -127, 4, "string", {-12, 196, 8} };

	SER_START_ITEMS(Foo)
	    SER_INT8("FOOBYTE1", 1),
	    SER_INT32("FOOINT32", 1),
	    SER_INT8("FOOBYTE2", 1), 
	SER_END_ITEMS(Foo);


	SER_START_ITEMS(Sample)
	    SER_INT8("BYTE", 1),
	    SER_INT16("SHORT", 1),
	    SER_INT32("INT32", 1),
	    SER_STR("STRING", 1),
	    SER_STRUCT("FOO", 1, Foo), 
	SER_END_ITEMS(Sample);

	WsSerializerContextH cntx;
	WsXmlDocH doc;
	WsXmlNodeH node;
	int retval;

	printf
	    ("\n\n   ********   example11. Structure with pads (signed int test).  ********\n");

	cntx = ws_serializer_init();
	if (cntx == NULL) {
		printf("Error ws_create_runtime\n");
		return;
	}
	doc = ws_xml_create_doc( NULL, "example");
	node = ws_xml_get_doc_root(doc);

	retval = ws_serialize(cntx, node, &sample, Sample_TypeInfo,
			      CLASSNAME, NULL, NULL, 0);
	printf("ws_serialize: %d\n", retval);
	ws_xml_dump_node_tree(stdout, node);

	node = ws_xml_get_doc_root(doc);
	Sample *cs = (Sample *) ws_deserialize(cntx,
					       node,
					       Sample_TypeInfo,
					       CLASSNAME, NULL, NULL,
					       0, 0);
	if (cs == NULL) {
		printf("Errror ws_deserialize\n");
		return;
	}

	printf("\n      initial and deserialized structures\n");
	printf("     byte1    =    %d : %d\n", sample.byte1, cs->byte1);
	printf("     short1   =    %d : %d\n", sample.short1, cs->short1);
	printf("     int1     =    %d : %d\n", sample.int1, cs->int1);
	printf("     string1  =    <%s> : <%s>\n", sample.string1,
	       cs->string1);
	printf("     foo :\n");
	printf("        byte1    =    %d : %d\n", sample.foo.byte1,
	       cs->foo.byte1);
	printf("        int1     =    %d : %d\n", sample.foo.int1,
	       cs->foo.int1);
	printf("        byte2    =    %d : %d\n", sample.foo.byte2,
	       cs->foo.byte2);
}

static void example12()
{
	typedef struct {
                XML_TYPE_REAL32 value;
                XML_NODE_ATTR *attrs;
        } item;
	item org = {1.257,NULL};
	SER_TYPEINFO_REAL32_ATTR;
	WsXmlDocH doc;
	WsXmlNodeH node = ws_xml_get_doc_root(doc);
	 printf
            ("\n\n   ********   example12. real32/64 deserialize/serialize  ********\n");
	WsSerializerContextH cntx = ws_serializer_init();
	if (cntx == NULL) {
                printf("Error ws_create_runtime\n");
                return;
        }
	doc = ws_xml_create_doc( NULL, "example");
        node = ws_xml_get_doc_root(doc);

        int retval = ws_serialize(cntx, node, &org, real32_TypeInfo,
                              "PowerState", NULL, NULL, 0);
        printf("ws_serialize: %d\n", retval);
        ws_xml_dump_node_tree(stdout, node);

	item *con = (item *)ws_deserialize(cntx, node, real32_TypeInfo, "PowerState", NULL, NULL, 0, 0);
	if(con->value != org.value) {
		printf("Mismatched! org.value = %f con->value = %f\n", org.value, con->value);
	}
	ws_xml_destroy_doc(doc);
	ws_serializer_cleanup(cntx);
}

static void example13()
{
	XML_TYPE_UINT8 myshorts[] = { 5, 11, 14};
	typedef struct {
		XML_TYPE_UINT16 id;
		XML_TYPE_DYN_ARRAY shorts;
	}item;
	XML_TYPE_DYN_ARRAY array_int8;
	SER_TYPEINFO_UINT8;
	SER_TYPEINFO_UINT16;
	SER_START_ITEMS(item)
	SER_INT16("id", 1),
	SER_DYN_ARRAY("temp", 0, 100, uint8),
	SER_END_ITEMS(item);
	char xmlstring[] = "<example><Sample><id>3</id></Sample></example>";
	printf
	    ("\n\n ****** example 13. de-serialize to int8 array (empty array de-serialization test)  ******\n");
	WsXmlDocH doc = ws_xml_read_memory(xmlstring, strlen(xmlstring), "UTF-8", 0);
	WsXmlNodeH node = ws_xml_get_doc_root(doc);

	WsSerializerContextH cntx = ws_serializer_init();
	if (cntx == NULL) {
                printf("Error ws_create_runtime\n");
                return;
        }
	item *desvalue = (item *)ws_deserialize(cntx, node, item_TypeInfo, CLASSNAME, NULL, NULL, 0, 0);
	printf("desvlaue = %p\n", desvalue);
	array_int8 = desvalue->shorts;
	int i = 0;
	printf("id = %d, array_int8->count = %d\n",desvalue->id,  array_int8.count);
	while(i < array_int8.count) {
		printf("%d ", ((char *)array_int8.data)[i]);
		i++;
	}
	printf("\n");
	ws_serializer_cleanup(cntx);
}

static void example14()
{
	typedef struct{
	}empty;
	SER_START_ITEMS(empty)
	SER_END_ITEMS(empty);
	typedef struct {
		XML_TYPE_INT16 num;
		empty empty1;
	}A;
	SER_TYPEINFO_INT16;
	SER_START_ITEMS(A)
	SER_INT16("id",1),
	SER_NS_STRUCT(NULL, "empty", 1, empty),
	SER_END_ITEMS(A);
	A a1 = {-12, {}};
	printf
	     ("\n\n ******* example 14. serialize/de-serialze with empty t_TypeItems *******\n");
	WsSerializerContextH cntx = ws_serializer_init();
        if (cntx == NULL) {
                printf("Error ws_create_runtime\n");
                return;
        }
	WsXmlDocH doc = ws_xml_create_doc(NULL, "example");
	WsXmlNodeH node = ws_xml_get_doc_root(doc);
	empty values = {};
	int ret = ws_serialize(cntx, node, &values, A_TypeInfo, CLASSNAME, NULL, NULL, 0);
	printf("serialize  %i bytes\n", ret);
	ws_xml_dump_node_tree(stdout, node);
	
	void *ptr = ws_deserialize(cntx, node, A_TypeInfo, CLASSNAME, NULL, NULL, 0, 0);
	printf("ptr = %p\n", ptr);
	ws_serializer_cleanup(cntx);
}

static void initialize_logging(void)
{
	debug_add_handler(wsman_debug_message_handler, DEBUG_LEVEL_ALWAYS,
			  NULL);
}

int debug_level = 0;
int main(int argc, char **argv)
{
	int num;
	int i;
	char retval = 0;
	u_error_t *error = NULL;
	u_option_entry_t opt[] = {
		{"debug", 'd', U_OPTION_ARG_INT, &debug_level,
		 "Set the verbosity of debugging output.", "1-6"}
	};
	u_option_context_t *opt_ctx;
	opt_ctx = u_option_context_new("");
	u_option_context_set_ignore_unknown_options(opt_ctx, FALSE);
	u_option_context_add_main_entries(opt_ctx, opt, "wsmid_identify");
	retval = u_option_context_parse(opt_ctx, &argc, &argv, &error);

	u_option_context_free(opt_ctx);

	if (error) {
		if (error->message)
			printf("%s\n", error->message);
		u_error_free(error);
		return 1;
	}
	u_error_free(error);

	if (debug_level) {
		initialize_logging();
		wsman_debug_set_level(debug_level);
	}

	if (argc == 1) {
		// execute all
		example1();
		example2();
		example3();
		example4();
		example5();
		example6();
		example7();
		example8();
		example9();
		example10();
		example11();
		example12();
		example13();
		example14();
		return 0;
	}

	for (i = 1; i < argc; i++) {
		num = atoi(argv[i]);
		switch (num) {
		case 1:
			example1();
			break;
		case 2:
			example2();
			break;
		case 3:
			example3();
			break;
		case 4:
			example4();
			break;
	        case 5: 
			example5(); 
			break;
		case 6:
			example6();
			break;
		case 7:
			example7();
			break;
		case 8:
			example8();
			break;
		case 9:
			example9();
			break;
		case 10:
			example10();
			break;
		case 11:
			example11();
			break;
		case 12:
			example12();
			break;
		case 13:
			example13();
			break;
		case 14:
			example14();
			break;
		default:
			printf("\n    No example%d()\n", num);
			break;
		}
	}
	return 0;
}
