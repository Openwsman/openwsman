#include <stdio.h>
#include <string.h>
#include "CUnit/Basic.h"
#include "WsmanEPR.h"
#include "u/libu.h"
#include "wsman-epr.h"
#include "wsman-xml-api.h"
#include "wsman-xml.h"
#include "wsman-names.h"

using namespace std;
using namespace WsmanClientNamespace;


/* The suite initialization function.
 * Opens the temporary file used by the tests.
 * Returns zero on success, non-zero otherwise.
 */
int init_WsmanEPR(void)
{
      return 0;
}

/* The suite cleanup function.
 * Closes the temporary file used by the tests.
 * Returns zero on success, non-zero otherwise.
 */
int clean_WsmanEPR(void)
{
      return 0;
}


void test_addTextSelector(void)
{
   WsmanEPR ws_epr1("http://schema.omc-project.org/wbem/wscim/1/cim-schema/2/CIM_IndicationFilter?Name=OperatingSystemFilter0&CreationClassName=CIM_IndicationFilter&SystemName=localhost.localdomain&SystemCreationClassName=CIM_ComputerSystem");
   WsmanEPR ws_epr2("http://schema.omc-project.org/wbem/wscim/1/cim-schema/2/CIM_IndicationFilter", "");
   WsmanEPR ws_epr3(ws_epr1);

   CU_ASSERT(0 == ws_epr1.addTextSelector(CIM_NAMESPACE_SELECTOR, "root/interop"));
   CU_ASSERT(0 == ws_epr2.addTextSelector(CIM_NAMESPACE_SELECTOR, "root/interop"));
   CU_ASSERT(0 == ws_epr3.addTextSelector(CIM_NAMESPACE_SELECTOR, "root/interop"));
}


void test_addEprSelector(void)
{
   WsmanEPR ws_epr1("http://schema.omc-project.org/wbem/wscim/1/cim-schema/2/CIM_IndicationFilter?Name=OperatingSystemFilter0&CreationClassName=CIM_IndicationFilter&SystemName=localhost.localdomain&SystemCreationClassName=CIM_ComputerSystem");
   WsmanEPR ws_epr2("http://schema.omc-project.org/wbem/wscim/1/cim-schema/2/CIM_IndicationFilter", "");
   WsmanEPR ws_epr3(ws_epr1);

   CU_ASSERT(0 == ws_epr1.addEprSelector(CIM_NAMESPACE_SELECTOR, ws_epr3));
   CU_ASSERT(0 == ws_epr2.addEprSelector(CIM_NAMESPACE_SELECTOR, ws_epr3));
   CU_ASSERT(0 == ws_epr3.addEprSelector(CIM_NAMESPACE_SELECTOR, ws_epr1));


}

void test_deleteSelector(void)
{

   WsmanEPR ws_epr1("http://schema.omc-project.org/wbem/wscim/1/cim-schema/2/CIM_IndicationFilter?Name=OperatingSystemFilter0&CreationClassName=CIM_IndicationFilter&SystemName=localhost.localdomain&SystemCreationClassName=CIM_ComputerSystem");
   WsmanEPR ws_epr2("http://schema.omc-project.org/wbem/wscim/1/cim-schema/2/CIM_IndicationFilter", "");
   WsmanEPR ws_epr3(ws_epr1);

   CU_ASSERT(0 == ws_epr1.addEprSelector(CIM_NAMESPACE_SELECTOR, ws_epr3));
   CU_ASSERT(0 == ws_epr2.addEprSelector(CIM_NAMESPACE_SELECTOR, ws_epr3));
   CU_ASSERT(0 == ws_epr3.addEprSelector(CIM_NAMESPACE_SELECTOR, ws_epr1));



   CU_ASSERT(0 == ws_epr1.deleteSelector(CIM_NAMESPACE_SELECTOR));
   CU_ASSERT(0 == ws_epr2.deleteSelector(CIM_NAMESPACE_SELECTOR));
   CU_ASSERT(0 == ws_epr3.deleteSelector(CIM_NAMESPACE_SELECTOR));


}

void test_compare(void)
{
   WsmanEPR ws_epr1("http://schema.omc-project.org/wbem/wscim/1/cim-schema/2/CIM_IndicationFilter?Name=OperatingSystemFilter0&CreationClassName=CIM_IndicationFilter&SystemName=localhost.localdomain&SystemCreationClassName=CIM_ComputerSystem");
   WsmanEPR ws_epr2("http://schema.omc-project.org/wbem/wscim/1/cim-schema/2/CIM_IndicationFilter", "");
   WsmanEPR ws_epr3(ws_epr1);

   CU_ASSERT(0 == ws_epr1.compare(&ws_epr1, &ws_epr3));
   CU_ASSERT(0 != ws_epr1.compare(&ws_epr1, &ws_epr2));


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
   pSuite = CU_add_suite("WsmanEPR", init_WsmanEPR, clean_WsmanEPR);
   if (NULL == pSuite) {
      CU_cleanup_registry();
      return CU_get_error();
   }

   /* add the tests to the suite */
   if ((NULL == CU_add_test(pSuite, "test of addTextSelector()", test_addTextSelector)) ||
       (NULL == CU_add_test(pSuite, "test of addEprSelector()", test_addEprSelector))||
       (NULL == CU_add_test(pSuite, "test of deleteSelector()", test_deleteSelector))||
       (NULL == CU_add_test(pSuite, "test of compare()", test_compare)))
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
