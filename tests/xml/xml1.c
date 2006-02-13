

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <glib.h>

#include "ws_utilities.h"


#include "ws_xml_api.h"
#include "ws_errors.h"
#include "soap_api.h"
#include "xml_api_generic.h"
#include "xml_serializer.h"


int main(void)
{
    
    WsContextH cntx = ws_create_runtime(NULL);

    ws_xml_parser_destroy((SoapH)cntx);
    soap_free(cntx);
    return 0;
}
