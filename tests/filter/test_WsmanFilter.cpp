#include <stdio.h>
#include <string.h>
#include "CUnit/Basic.h"
#include "WsmanFilter.h"
#include "WsmanEPR.h"
#include "u/libu.h"
#include "wsman-filter.h"
#include "wsman-xml-api.h"
#include "wsman-xml.h"
#include "wsman-names.h"

using namespace std;
using namespace WsmanClientNamespace;


/* The suite initialization function.
 * Opens the temporary file used by the tests.
 * Returns zero on success, non-zero otherwise.
 */
int init_WsmanFilter(void)
{
      return 0;
}

/* The suite cleanup function.
 * Closes the temporary file used by the tests.
 * Returns zero on success, non-zero otherwise.
 */
int clean_WsmanFilter(void)
{
      return 0;
}


void test_addSelector(void)
{
   WsmanEPR ws_epr1("http://schema.omc-project.org/wbem/wscim/1/cim-schema/2/CIM_IndicationFilter?Name=OperatingSystemFilter0&CreationClassName=CIM_IndicationFilter&SystemName=localhost.localdomain&SystemCreationClassName=CIM_ComputerSystem");
   
   WsmanFilter ws_filter1(WSM_WQL_FILTER_DIALECT, "select * from CIM_ComputerSystem");
   WsmanFilter ws_filter2(&ws_epr1, WSMAN_ASSOCIATED);
   WsmanFilter ws_filter3(ws_filter1);

   CU_ASSERT(0 == ws_filter1.addSelector("__cimnamespace", "root/interop"));
   CU_ASSERT(0 == ws_filter2.addSelector("__cimnamespace", "root/interop"));
   CU_ASSERT(0 == ws_filter3.addSelector("__cimnamespace", "root/interop"));
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
   pSuite = CU_add_suite("WsmanFilter", init_WsmanFilter, clean_WsmanFilter);
   if (NULL == pSuite) {
      CU_cleanup_registry();
      return CU_get_error();
   }

   /* add the tests to the suite */
   if ((NULL == CU_add_test(pSuite, "test of addSelector()", test_addSelector)))
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
