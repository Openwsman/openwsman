
#ifndef _COMMON_H
#define _COMMON_H


typedef struct {
  const char *server;
  int port;
  const char *path;
  const char *scheme;
  const char *username;
  const char *password;
} ServerData;

typedef struct {						
  /* Explanation of what you should see */
  const char *explanation;

  /* Resource UR to test against */
  const char *resource_uri;

  /* Selectors in the form of a URI query   key=value&key2=value2 */
  const char *selectors;

  const char* xpath_expression;
  const char* expected_value;

  /* What the final status code should be. */
  unsigned int final_status;		

  unsigned char       flags;

  unsigned int		max_elements;

} TestData;


int init_test(void);
int clean_test(void);
int add_enumeration_tests(CU_pSuite ps);
int add_identify_tests(CU_pSuite ps);
int add_transfer_get_tests(CU_pSuite ps);

#endif
