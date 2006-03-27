#include <pthread.h>
#include <time.h>

#include <glib.h>
#include "libsoup/soup.h"
#include "libsoup/soup-session.h"

#include "ws-eventing.h"
#include "wsman-client.h"
#include "wsman-debug.h"

static char       g_event_source_url[] = "http://www.otc_event.com/event_source";
static EventingH  g_event_handler;

static void *publish_events(void *soap_handler);

static void authenticate (SoupSession *session, 
        SoupMessage *msg,
        const char *auth_type, 
        const char *auth_realm,
        char **username, 
        char **password, 
        gpointer data)
{
    printf("authenticating...\n");
    WsManClient *cl = data;
    WsManClientEnc *wsc =(WsManClientEnc*)cl;
    *username = g_strdup (wsc->data.user);
    *password = g_strdup (wsc->data.pwd);
}


static void reauthenticate (SoupSession *session, SoupMessage *msg,
        const char *auth_type, const char *auth_realm,
        char **username, char **password, gpointer data)
{
    char *pw;
    char user[21];

    fprintf(stderr,"Authentication failed, please retry\n");
    printf("User name: ");
    fflush(stdout); 
    fgets(user, 20, stdin);

    if (strchr(user, '\n'))
        (*(strchr(user, '\n'))) = '\0';
    *username = g_strdup_printf ("%s", user);

    pw = getpass("Password: ");
    *password = g_strdup_printf ("%s", pw);

}

/*
   This routine is adapted from wsman_client_handler().
   Try to return connection info through user_data.
*/
static void wse_client_handler( 
        WsManClient *cl, 
        WsXmlDocH rqstDoc,
        gpointer user_data) 
{

    SoupSession *session = NULL;
    SoupMessage *msg= NULL;
    int         *ret_ptr = (int *)user_data;
    char *buf = NULL;
    int len;

    if(ret_ptr)
       *ret_ptr = 0;
    
    WsManClientEnc *wsc =(WsManClientEnc*)cl;
    WsManConnection *con = wsc->connection;

    session = soup_session_sync_new ();    
    g_signal_connect (session, "authenticate",
            G_CALLBACK (authenticate), cl);
    g_signal_connect (session, "reauthenticate",
            G_CALLBACK (reauthenticate), cl);

    ;                                   
    msg = soup_message_new_from_uri (SOUP_METHOD_POST, soup_uri_new(wsc->data.endpoint));
    if (!msg) 
    {
        fprintf (stderr, "Could not parse URI\n");
        return;
    }


    // FIXME: define in header file and use real version
    soup_message_add_header(msg->request_headers, "User-Agent", "OpenWSMAN/0.01");
    // ws_xml_dump_node_tree(stdout, ws_xml_get_doc_root(rqstDoc), 1);

    ws_xml_dump_memory_enc(rqstDoc, &buf, &len, "UTF-8");

    soup_message_set_request(msg, 
            SOAP1_2_CONTENT_TYPE,
            SOUP_BUFFER_SYSTEM_OWNED,
            buf,
            len);

    // Send the message...        
    soup_session_send_message (session, msg);

    if (msg->status_code != SOUP_STATUS_UNAUTHORIZED &&
            msg->status_code != SOUP_STATUS_OK) 
    {
        printf ("Connection to server failed: %s (%d)\n", 
                msg->reason_phrase, 
                msg->status_code);        
        if(ret_ptr)
           *ret_ptr = -1;
    }

    if (msg->response.body) 
    {    
        con->response = g_malloc0 (SOUP_MESSAGE (msg)->response.length + 1);
        strncpy (con->response, SOUP_MESSAGE (msg)->response.body, SOUP_MESSAGE (msg)->response.length);		
    } 

    g_object_unref (session);
    g_object_unref (msg);

    return;
}

static WsManClient *connection_initialize(void *p_data)
{
   WsContextH cntx = ws_create_runtime(NULL);

   WsManClient *cl = 
        wsman_connect(
                cntx,
                "localhost",
                666,
                NULL,
                "wsman",
                "secret",
                NULL);

   if (cl == NULL)
   {
      fprintf(stderr, "%s %d: Null Client\n", __FILE__, __LINE__);
   } 

   wsman_client_add_handler(wse_client_handler, p_data);

   return cl;
}

void start_event_source(SoapH soap)
{
   pthread_t   publiser_thread;
   WsManClient *client;

   client = connection_initialize(&g_notify_connection_status);
   g_event_handler = wse_initialize_server(soap, client, g_event_source_url);
   pthread_create(&publiser_thread, NULL, publish_events, soap);
   
   printf("Eventing source started...\n");
}

void *publish_events(void *soap_handler)
{
   char            *action_list[] = {"http://mynamespace.org/random_event"};
   WsePublisherH   publisher_handler;
   WsXmlDocH       event_doc;
   struct timespec time_val = {1, 0};
   
   publisher_handler = wse_publisher_initialize((EventingH)g_event_handler, 1, action_list, NULL, NULL);
   while(1)
   {
      nanosleep(&time_val, NULL);
      event_doc = ws_xml_create_doc((SoapH)soap_handler, "http://mynamespace.org", "random_event");  //TBD
      /* Note: use NULL for no ack mode, g_event_source_url for ack mode */
      wse_send_notification(publisher_handler, event_doc, "http://mynamespace.org",  NULL /* g_event_source_url */);      
   }
}
