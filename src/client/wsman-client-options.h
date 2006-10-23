
#ifndef WSMANCLIENTOPTIONS_H_
#define WSMANCLIENTOPTIONS_H_

struct _WsActions
{
    char* action;
    int   value;
};
typedef struct _WsActions WsActions;


extern int wsman_read_client_config (dictionary *ini);

extern char wsman_parse_options(int argc, char **argv);
extern void wsman_setup_transport_and_library_options(void);

extern int wsman_options_get_debug_level(void);

extern char *wsman_options_get_server(void);
extern int wsman_options_get_server_port(void);
extern char * wsman_options_get_path (void);
extern char *wsman_options_get_username(void);
extern char *wsman_options_get_password(void);
extern char *wsman_options_get_cafile(void);

extern int wsman_options_get_max_elements(void);
extern char * wsman_options_get_test_file(void);
extern int wsman_options_get_action(void);
extern char *wsman_options_get_resource_uri(void);
extern hash_t *wsman_options_get_properties (void);
extern char *wsman_options_get_invoke_method (void);
extern char *wsman_options_get_enum_mode (void);
extern char *wsman_options_get_binding_enum_mode (void);
extern char *wsman_options_get_cim_namespace (void);
extern char wsman_options_get_optimize_enum (void);
extern char wsman_options_get_dump_request (void);
extern char wsman_options_get_estimate_enum (void);
extern char * wsman_options_get_fragment (void);
extern char * wsman_options_get_filter (void);
extern char * wsman_options_get_dialect (void);
extern const char * wsman_options_get_config_file (void);
extern unsigned long wsman_options_get_max_envelope_size (void);
extern unsigned long wsman_options_get_operation_timeout (void);

extern const char ** wsman_options_get_argv (void);
extern const char * wsman_options_get_output_file (void);

#endif /*WSMANCLIENTOPTIONS_H_*/
