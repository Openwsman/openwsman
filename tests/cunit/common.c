


#include "wsman_config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>

#include "u/libu.h"
#include "wsman-xml-api.h"
#include "wsman-errors.h"
#include "wsman-soap.h"
#include "wsman-xml.h"
#include "wsman-xml-serializer.h"

#include "wsman-client.h"
#include "wsman-client-transport.h"
#include <CUnit/Basic.h> 

#include "common.h"


WsManClient *cl;
    
actionOptions options;




int init_test(void)
{
  wsman_client_transport_init(NULL);
 
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

  initialize_action_options(&options);
  return 0;
}


int clean_test(void)
{			
  destroy_action_options(&options);		
  wsman_release_client(cl);
  return 0;
}
