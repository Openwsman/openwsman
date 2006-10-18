
#ifndef WSMANCLIENTOPTIONS_H_
#define WSMANCLIENTOPTIONS_H_

struct _WsActions
{
    char* action;
    int   value;
};
typedef struct _WsActions WsActions;


// Possible authentication methods
#define AUTH_BASIC      0
#define AUTH_DIGEST     1
#define AUTH_NTLM       2
#define AUTH_MAX        3



char wsman_parse_options(int argc, char **argv);

int wsman_options_get_debug_level(void);
int wsman_options_get_server_port(void);
int wsman_options_get_max_elements(void);

char *wsman_options_get_cafile(void);
char * wsman_options_get_test_file(void);
char *wsman_options_get_username(void);
char *wsman_options_get_password(void);
char *wsman_options_get_server(void);
char *wsman_options_get_proxy(void);
char *wsman_options_get_proxyauth(void);
int wsman_options_get_action(void);
char *wsman_options_get_resource_uri(void);
hash_t *wsman_options_get_properties (void);
char *wsman_options_get_invoke_method (void);
char *wsman_options_get_enum_mode (void);
char *wsman_options_get_binding_enum_mode (void);
char *wsman_options_get_cim_namespace (void);
char wsman_options_get_optimize_enum (void);
char wsman_options_get_dump_request (void);
char wsman_options_get_estimate_enum (void);
char * wsman_options_get_fragment (void);
char * wsman_options_get_filter (void);
char * wsman_options_get_dialect (void);
char * wsman_options_get_agent (void);
const char * wsman_options_get_config_file (void);
int wsman_read_client_config (dictionary *ini);
char * wsman_options_get_path (void);
char * wsman_options_get_auth_method (void);
int wsman_is_auth_method(int method);
int wsman_options_get_no_verify_peer (void);



unsigned long wsman_options_get_max_envelope_size (void);
unsigned long wsman_options_get_operation_timeout (void);

const char ** wsman_options_get_argv (void);
const char * wsman_options_get_output_file (void);


#endif /*WSMANCLIENTOPTIONS_H_*/
