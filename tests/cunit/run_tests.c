/*******************************************************************************
 * Copyright (C) 2004-2006 Dell Inc. All rights reserved.
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
 *  - Neither the name of Dell Inc. nor the names of its
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
 * @author Nathan Rakoff
 */
#include <stdio.h>
#include <CUnit/Basic.h> 
#include "common.h"

#define TRUE    1
#define FALSE	0


static CU_pSuite
setup_client_suite(void) {
   int num_tests = 0;
   /* add a suite to the registry */
   CU_pSuite ps = CU_add_suite("OpenWSman Client Library Tests",
                                        init_test, clean_test);
   if (NULL == ps) {
      return NULL;
   }

   /* add the tests to the suite */
   num_tests += add_enumeration_tests(ps);
   num_tests += add_identify_tests(ps);
   num_tests += add_transfer_get_tests(ps);
   if (num_tests == 0) {
        printf("No tests to run\n");
        // nothing to do 
     return NULL;
   }
   return ps;
}



int
main(int argc, char** argv) {
   CU_pSuite pSuite = NULL;

   /* initialize the CUnit test registry */
   if (CUE_SUCCESS != CU_initialize_registry())
      return CU_get_error();

   /* add client suite to the registry */
   pSuite = (CU_pSuite)setup_client_suite();
   if (NULL == pSuite) {
      CU_cleanup_registry();
      return CU_get_error();
   }

   /* Run all tests using the basic interface */
   CU_basic_set_mode(CU_BRM_VERBOSE);
   CU_basic_run_tests();
   printf("\n");
   CU_basic_show_failures(CU_get_failure_list());
   printf("\n\n");

   /* Clean up registry and return */
   CU_cleanup_registry();
   return CU_get_error();
}
