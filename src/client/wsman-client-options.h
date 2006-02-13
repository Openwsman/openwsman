
#ifndef WSMANCLIENTOPTIONS_H_
#define WSMANCLIENTOPTIONS_H_



gboolean wsman_parse_options(int argc, char **argv);

int wsman_options_get_debug_level(void);
int wsman_options_get_syslog_level(void);
int wsman_options_get_server_port(void);

char *wsman_options_get_cafile(void);
char *wsman_options_get_username(void);
char *wsman_options_get_password(void);
char *wsman_options_get_server(void);
int wsman_options_get_action(void);
char *wsman_options_get_resource_uri(void);


const char **
wsman_options_get_argv (void);


#endif /*WSMANCLIENTOPTIONS_H_*/
