#include <pthread.h>
#include <time.h>

#include "ws-eventing.h"
#include "wsman-debug.h"

static char g_event_source_url[] = "http://www.otc_event.com/event_source";
static EventingH  g_event_handler;

static void *publish_events(void *soap_handler);

void start_event_source(SoapH soap)
{
   pthread_t  publiser_thread;

   g_event_handler = wse_initialize_server(soap, g_event_source_url);
   pthread_create(&publiser_thread, NULL, publish_events, soap);
   
   printf("Eventing source started...\n");
}

void *publish_events(void *soap_handler)
{
   char            *action_list[] = {"random_event"};
   WsePublisherH   publisher_handler;
   WsXmlDocH       event_doc;
   struct timespec time_val = {10, 0};
   
   publisher_handler = wse_publisher_initialize((EventingH)g_event_handler, 1, action_list, NULL, NULL);

   while(1)
   {
      nanosleep(&time_val, NULL);
      event_doc = ws_xml_create_doc((SoapH)soap_handler, "rootNsUri", "rootName");  //TBD
      wse_send_notification(publisher_handler, event_doc, "http://otc.eventing.org/");      
   }
}
