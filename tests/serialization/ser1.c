
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>

#include "CUnit/Basic.h"
#include "u/libu.h"
#include "wsman-soap.h"
#include "wsman-xml.h"
#include "wsman-soap.h"
#include "wsman-xml-serializer.h"
#include "wsman-xml-serialize.h"
#include "wsman-epr.h"
#include "wsman-debug.h"

#define CLASSNAME "Sample"


/* The suite initialization function.
 * Opens the temporary file used by the tests.
 * Returns zero on success, non-zero otherwise.
 */
static int init_ser1(void)
{
	return 0;
}

/* The suite cleanup function.
 * Closes the temporary file used by the tests.
 * Returns zero on success, non-zero otherwise.
 */
static int clean_ser1(void)
{
	return 0;
}


static void test_basic_types(void)
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

	WsContextH cntx;
	WsXmlDocH doc;
	WsXmlNodeH node;
	int retval;

	cntx = ws_create_runtime(NULL);
	CU_ASSERT_PTR_NOT_NULL(cntx);
	doc = ws_xml_create_doc(cntx->soap, NULL, "example");
	node = ws_xml_get_doc_root(doc);

	retval = ws_serialize(cntx, node, &servie, Sample_Servie_TypeInfo,
			      CLASSNAME, NULL, NULL, 0);
	//ws_xml_dump_node_tree(stdout, node);
	node = ws_xml_get_doc_root(doc);

	Sample_Servie *cs = (Sample_Servie *) ws_deserialize(cntx,
							     node,
							     Sample_Servie_TypeInfo,
							     CLASSNAME,
							     NULL, NULL,
							     0, 0);
	CU_ASSERT_PTR_NOT_NULL(cs);
	retval = memcmp(cs, &servie, sizeof(&servie));
	if (!retval) {
		CU_ASSERT_EQUAL(  servie.AcceptPause, cs->AcceptPause );
		CU_ASSERT_EQUAL(  servie.AcceptStop, cs->AcceptStop );
		CU_TEST( !strcmp(servie.Caption, cs->Caption) );
	} else {
		CU_FAIL("memcpy failed");
	}
}

static void test_static_struct(void)
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
	    SER_STR("string", 1),
	    SER_BOOL("b", 1), 
	SER_END_ITEMS(Embed);

	SER_START_ITEMS(Sample)
	    SER_UINT32("A", 1),
	    SER_STRUCT("EMBED", 2, Embed),
	    SER_STR("STRING", 1),
	SER_END_ITEMS(Sample);

	WsContextH cntx;
	WsXmlDocH doc;
	WsXmlNodeH node;
	int retval;

	cntx = ws_create_runtime(NULL);
	CU_ASSERT_PTR_NOT_NULL(cntx);
	doc = ws_xml_create_doc(cntx->soap, NULL, "example");
	CU_ASSERT_PTR_NOT_NULL(doc);
	node = ws_xml_get_doc_root(doc);
	CU_ASSERT_PTR_NOT_NULL(node);
	retval = ws_serialize(cntx, node, &sample, Sample_TypeInfo,
			      CLASSNAME, NULL, NULL, 0);
	//ws_xml_dump_node_tree(stdout, node);
	node = ws_xml_get_doc_root(doc);
	Sample *sample_out = (Sample *) ws_deserialize(cntx,
							     node,
							     Sample_TypeInfo,
							     CLASSNAME,
							     NULL, NULL,
							     0, 0);
	CU_ASSERT_PTR_NOT_NULL(sample_out);
	CU_ASSERT_TRUE(sample_out->A == 10 );
	CU_ASSERT_TRUE(sample_out->EMBED[0].a);
	CU_ASSERT_FALSE(sample_out->EMBED[1].a);

}


/* The main() function for setting up and running the tests.
 * Returns a CUE_SUCCESS on successful running, another
 * CUnit error code on failure.
 */
int main()
{
   CU_pSuite pSuite = NULL;

   /* initialize the CUnit test registry */
   if (CUE_SUCCESS != CU_initialize_registry())
      return CU_get_error();

   /* add a suite to the registry */
   pSuite = CU_add_suite("Suite_1", init_ser1, clean_ser1);
   if (NULL == pSuite) {
      CU_cleanup_registry();
      return CU_get_error();
   }

   /* add the tests to the suite */
   /* NOTE - ORDER IS IMPORTANT - MUST TEST fread() AFTER fprintf() */
   if ( (NULL == CU_add_test(pSuite, "test of static struct array", test_static_struct)) ||
    	(NULL == CU_add_test(pSuite, "test of basic_types", test_basic_types)) )
   {
      CU_cleanup_registry();
      return CU_get_error();
   }

   /* Run all tests using the CUnit Basic interface */
   CU_basic_set_mode(CU_BRM_VERBOSE);
   CU_basic_run_tests();
   CU_cleanup_registry();
   return CU_get_error();
}
