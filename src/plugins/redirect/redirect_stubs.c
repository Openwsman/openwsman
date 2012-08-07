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
 * @author Praveen K Paladugu
 */

#include "wsman_config.h"

#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "ctype.h"


#include "u/libu.h"

#include "wsman-xml-api.h"
#include "wsman-soap.h"
#include "wsman-xml.h"
#include "wsman-client-api.h"
#include "wsman-xml-serializer.h"

#include <wsman-client-transport.h>
#include <wsman-debug.h>

#include "wsman-soap-envelope.h"
#include "wsman-soap-message.h"

#include "redirect.h"


void setup_client (WsContextH cntx,  WsEnumerateInfo* enumInfo);
int wsman_get_transport_timeout(WsContextH cntx, WsXmlDocH doc);
int check_response_code(WsManClient *cl, WsXmlDocH doc);

//DEBUG
static void xml_print( WsXmlDocH doc);

WsManClient *cl=NULL; 
client_opt_t *cl_options=NULL;


Redirect  Redirect_Get_EP(WsContextH cntx)
{
    debug ("Test Get Endpoint Called"); 
	 // Return a some value/item here.
    return; 
}

int Redirect_Put_EP(WsContextH cntx)
{
    debug( "Put Endpoint Called"); 
}


int Redirect_Enumerate_EP(WsContextH cntx, 
			WsEnumerateInfo* enumInfo,
			WsmanStatus *status, void *opaqueData)
{


  filter_t *filter = NULL;

  WsXmlDocH doc=NULL, header=NULL, node=NULL, body=NULL;
  char *resource_uri, *remote_enumContext;
  int op;



 if (cl == NULL)
 {
	setup_client(cntx,  enumInfo);
 }


//debug("The OptionSet Passed=%s",wsman_get_option_set(cntx, NULL,WSMB_SHOW_EXTENSION));

//DEBUG
//wsmc_set_action_option(cl_options, FLAG_DUMP_REQUEST);

//TODO: Handle the Filters/Selector properly
//

// Set the op to enumerate.WSMAN_ACTION_ENUMERATION, 
    op = WSMAN_ACTION_ENUMERATION;
 	

   filter = filter_create_simple(NULL, NULL);
 

   resource_uri=wsman_get_resource_uri(cntx,doc);

   enumInfo->pullResultPtr = wsmc_action_enumerate(cl, resource_uri, cl_options, filter);
  

 if (check_response_code(cl, enumInfo->pullResultPtr)) 
	return 0; //Not sending a response code as the pullResultPtr will caputre the Fault info.
//TODO: When a Fault is observed in the respose, return an error code. Update the wsenum_***_stub functions to forward the fault doc to the final client. That is handle the case when the pullResultPtr is set and the return value if non-zero for endPoint function. 

remote_enumContext = wsmc_get_enum_context(enumInfo->pullResultPtr);
//Get the total number of items
     header=ws_xml_get_soap_header(enumInfo->pullResultPtr);
     node=ws_xml_get_child(header,0,XML_NS_WS_MAN, WSM_TOTAL_ESTIMATE );
     int TotalItems=(!node) ? 0: atoi(ws_xml_get_node_text(node));
     enumInfo->totalItems=TotalItems;


strcpy (enumInfo->enumId,remote_enumContext);

//ws_xml_destroy_doc(enum_response);

debug ("The context on the remote host=%s", remote_enumContext);

return 0;

}

int Redirect_Release_EP(WsContextH cntx, WsEnumerateInfo* enumInfo)
{
    debug( "Release Endpoint Called");    
    return 0;
}

int Redirect_Pull_EP(WsContextH cntx, WsEnumerateInfo* enumInfo)
{
	debug( "Pull Endpoint Called"); 
  WsXmlDocH doc=NULL;
  filter_t *filter = NULL;
  filter = filter_create_simple(NULL, NULL);


  char *resource_uri, *remote_enumContext;  


if (cl == NULL)
{
	setup_client( cntx, enumInfo);
}

   cl_options->max_elements = wsman_get_max_elements(cntx,NULL);

  resource_uri=wsman_get_resource_uri(cntx,doc); 
     enumInfo->pullResultPtr = wsmc_action_pull(cl, resource_uri, cl_options, filter,enumInfo->enumId);

//DEBUG
xml_print(enumInfo->pullResultPtr);

	if(check_response_code(cl, enumInfo->pullResultPtr))
		return 1;//Not sending a response code as the pullResultPtr will caputre the Fault info.


    return 0;
}


////OUTPUT functoin

static void wsman_output(WsManClient * cl, WsXmlDocH doc)
{
        FILE *f = stdout;
        const char *filename = "my_out";
        WS_LASTERR_Code err;

        err = wsmc_get_last_error(cl);
        if (err != WS_LASTERR_OK) {
                return;
        }
        if (!doc) {
                error("doc with NULL content");
                return;
        }
        if (filename) {
                f = fopen(filename, "w");
                if (f == NULL) {
                        error("Could not open file for writing");
                        return;
                }
        }
        ws_xml_dump_node_tree(f, ws_xml_get_doc_root(doc));
        if (f != stdout) {
                fclose(f);
        }
        return;
}



/*
 *  * output pull results to separate files (appending "-<index>" to the name)
 *   * 
 *    */
static void wsman_output_pull(WsManClient * cl, WsXmlDocH doc, int index)
{
   char *strbuf, *output_file="my_out", *origfile = "my_out";
   int count = strlen("my_out") + 16;

   strbuf = (char*)calloc(count, 1);
   snprintf(strbuf, count, "%s-%u.xml", output_file, index);
   output_file = strbuf;
   wsman_output(cl, doc);
   free(strbuf);
   
}



void setup_client(WsContextH cntx, WsEnumerateInfo* enumInfo)
{
	


//	debug ("Rediret: for client, %s, %d, %s, %s, %s, %s", get_remote_server(), get_remote_server_port(), get_remote_url_path(), get_remote_cainfo(), get_remote_username(), get_remote_password()); 

//TODO: Use the incoming requests username/password when not provided in the conf file.

	cl = wsmc_create(get_remote_server() ,
                           get_remote_server_port() ,
                           get_remote_url_path(),
                           get_remote_cainfo() ? "https" : "http",
                           get_remote_username() ? get_remote_username() : strdup(enumInfo->auth_data.username),
                           get_remote_password() ? get_remote_password() : strdup(enumInfo->auth_data.password)
                            );


 
  
//TODO: use the same authentication method as the original WSMAN request, instead of falling back to "basic"  
	wsman_transport_set_auth_method(cl, get_remote_authentication_method()  );

//TODO: may be use the cainfo for the original WSMAN request?
        if ( get_remote_cainfo() ) {
                wsman_transport_set_cainfo(cl, get_remote_cainfo() );
        }

	if (get_remote_cl_cert()){
		wsman_transport_set_cert(cl, get_remote_cert());
		if (!get_remote_cainfo())
                        debug("Warning: cainfo not set to enable SSL operation in Redirect Plugin\n");

	}

        if ( get_remote_sslkey())
        {
		wsman_transport_set_cert(cl, get_remote_sslkey());
		if (!get_remote_cainfo())
		{
			debug("Warning: cainfo not set to enable SSL operation in Redirect Plugin\n");
		}
	}


        wsman_transport_set_verify_peer(cl, get_remote_cainfo()? !get_remote_noverifypeer() : 0);
        wsman_transport_set_verify_host(cl, get_remote_cainfo() ? !get_remote_noverifyhost(): 0 );
        

	cl_options = wsmc_options_init();
        cl_options->properties = hash_create(HASHCOUNT_T_MAX, 0, 0) ; //properties are only valid for put, get & create.. so skippping
        cl_options->cim_ns = get_remote_cim_namespace() ;

//Pass this option to request the Estimated Total num of Items in the response.
	cl_options->flags |= FLAG_ENUMERATION_COUNT_ESTIMATION; 
	cl_options->max_elements = wsman_get_max_elements(cntx,NULL);
	
        cl_options->max_envelope_size = wsman_get_max_envelope_size(cntx,NULL);
	cl_options->timeout= wsman_get_operation_timeout(cntx, NULL) * 1000;
	
	if (strcmp( wsman_get_option_set(cntx, NULL,WSMB_SHOW_EXTENSION) , "true" == 0 ) )
			cl_options->flags |= FLAG_CIM_EXTENSIONS ;

}



//TODO: Move this function to the right library
int wsman_get_operation_timeout(WsContextH cntx, WsXmlDocH doc)
{
	int my_timeout=0;
	if (doc == NULL)
	{
		doc=cntx->indoc;
	}	
	
       if (doc) {
                WsXmlNodeH header = ws_xml_get_soap_header(doc);
		WsXmlNodeH timeout_child = ws_xml_get_child(header, 0, XML_NS_WS_MAN, WSM_OPERATION_TIMEOUT);
		if (timeout_child){
			char *text = ws_xml_get_node_text(timeout_child);
			if (text){
				char* text_pt=calloc(strlen(text),1);  	
					strncpy(text_pt,text+2,strlen(text)-2); 
					
					strcpy(text,text_pt);
					
				 	strncpy(text_pt,text,strlen(text)-1);
				my_timeout=atoi(text_pt);
			}
		}	
	
	}

	return my_timeout;
}

static void xml_print( WsXmlDocH doc)
{
        char *filename="/tmp/tmp";
        FILE *f;


        if (filename) {
                f = fopen(filename, "w");
                if (f == NULL) {
                        error("Could not open file for writing");
                        return;
                }
        }
        ws_xml_dump_node_tree(f, ws_xml_get_doc_root(doc));
                fclose(f);
        return;

}

int check_response_code(WsManClient *cl, WsXmlDocH doc)
{

	//Check the http response codes and also for Wsman Faults.     
                   if (wsmc_get_response_code(cl) != 200
                                        && wsmc_get_response_code(cl) != 400
                                        && wsmc_get_response_code(cl) != 500) {
                                return 1;
                        }

		if (doc != NULL)
		{
			WsXmlNodeH body = ws_xml_get_soap_body(doc);
			if (body && ws_xml_get_child(body, 0, XML_NS_SOAP_1_2, SOAP_FAULT))
				return 1;
		}


	return 0;
}	
