


#include "wsman_config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>

#include "u/libu.h"
#include "wsman-client-api.h"
#include "wsman-xml-serializer.h"
#include "wsman-client-transport.h"

#include "common.h"


WsManClient *cl;
char *host = "langley.home.planux.com";




int init_test(void) {
  wsman_client_transport_init(NULL);
  if (getenv("OPENWSMAN_TEST_HOST")) {
    host = getenv("OPENWSMAN_TEST_HOST");
  }
  ServerData sd[] = {
    {"localhost", 8889, "/wsman", "http", "wsman", "secret"}
  };

  cl = wsman_create_client( 
		      sd[0].server,
		      sd[0].port,
		      sd[0].path,
		      sd[0].scheme,
		      sd[0].username,
		      sd[0].password);
  return 0;
}


int clean_test(void) {
  wsman_release_client(cl);
  wsman_client_transport_fini();
  return 0;
}
