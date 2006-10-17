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
 * @author Anas Nashif
 */


#include <wsman_config.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <errno.h>
#include <assert.h>

#include "u/libu.h"
#include "wsmand-daemon.h"


#ifdef LIBSOUP_LISTENER
#include <glib.h>
#endif

#define DEFAULT_BASIC_AUTH  "libwsman_file_auth.so"

int facility = LOG_DAEMON;

static const char **wsmand_argv = NULL;


static int server_port =  -1;
static int server_ssl_port = -1;
static int use_digest = 0;
static char *ssl_key_file = NULL;
static char *service_path = DEFAULT_SERVICE_PATH;
static char *ssl_cert_file = NULL;
static char *pid_file = DEFAULT_PID_PATH;
static int daemon_flag = 0;
static int no_plugin_flag = 0;
static int debug_level = -1;
static int foreground_debug = 0;
static int syslog_level = -1;
static char *log_location = NULL;
static char *digest_password_file = NULL;
static char *basic_password_file = NULL;
static char *basic_authenticator_arg = NULL;
static char *basic_authenticator = DEFAULT_BASIC_AUTH;
static int max_threads = 1;
static int min_threads = 4;


static char *config_file = NULL;



int wsmand_parse_options(int argc, char **argv) 
{
    u_option_context_t *opt_ctx;
    u_error_t *error = NULL;

    u_option_entry_t options[] = {
        { "no-plugins", 	'n', U_OPTION_ARG_NONE, 	&no_plugin_flag,"Do not load any plugins", NULL }, 
        { "debug", 		'd', U_OPTION_ARG_NONE, 	&foreground_debug, 	"Start daemon in foreground and turn on debugging", NULL },
        { "syslog", 		's', U_OPTION_ARG_INT, 	&syslog_level,  "Set the verbosity of syslog output.", "0-6" },
        { "config-file",	'c', U_OPTION_ARG_STRING, 	&config_file,  	"Alternate configuration file", "<file>" },
        { "pid-file",	        'p', U_OPTION_ARG_STRING, 	&pid_file,  	"PID file", "<file>" },

        { NULL }
    };	

    wsmand_argv = (const char **)argv;
    opt_ctx = u_option_context_new("WS-Management Server");
    u_option_context_set_ignore_unknown_options(opt_ctx, FALSE);
    u_option_context_add_main_entries(opt_ctx, options, "wsman");  	
    char retval = u_option_context_parse(opt_ctx, &argc, &argv, &error);
    if (error) {
        if (error->message)
            printf ("%s\n", error->message);
        retval = 0;
    }

    u_error_free(error);
    u_option_context_free(opt_ctx);
    return retval;
}

const char ** wsmand_options_get_argv (void)
{
    return wsmand_argv;
}

int wsmand_read_config (dictionary *ini)
{
    if (!iniparser_find_entry(ini, "server")) {
        return 0;
    }

    server_port = iniparser_getint (ini, "server:port", -1);
    server_ssl_port =  iniparser_getint(ini, "server:ssl_port",-1);
    debug_level = iniparser_getint (ini, "server:debug_level", 0);
    service_path = iniparser_getstring (ini, "server:service_path", "/wsman");
    ssl_key_file = iniparser_getstr (ini, "server:ssl_key_file");
    ssl_cert_file = iniparser_getstr (ini, "server:ssl_cert_file");
    use_digest = iniparser_getboolean (ini, "server:use_digest", 0);
    digest_password_file = iniparser_getstr (ini,
                            "server:digest_password_file");
    basic_password_file = iniparser_getstr (ini, "server:basic_password_file");
    basic_authenticator = iniparser_getstr (ini, "server:basic_authenticator");
    basic_authenticator_arg = iniparser_getstr (ini,
                            "server:basic_authenticator_arg");
    log_location = iniparser_getstr (ini, "server:log_location");
    min_threads  = iniparser_getint(ini, "server:min_threads", 1);
    max_threads  = iniparser_getint(ini, "server:max_threads", 4);

    return 1;
}

const char *
wsmand_options_get_config_file (void)
{
    if (config_file == NULL) {
         config_file = DEFAULT_CONFIG_FILE;
    }
    if (config_file != NULL && !u_path_is_absolute (config_file)) {
        char cwd[PATH_MAX];
        getcwd (cwd, PATH_MAX);
          
        config_file = u_strdup_printf ("%s/%s", cwd, config_file);
    }
    return config_file;
}

char *wsmand_options_get_digest_password_file (void)
{
    return digest_password_file;
}

char *wsmand_options_get_basic_password_file (void)
{
    return basic_password_file;
}


char *wsmand_options_get_service_path (void)
{
    return service_path;
}

int wsmand_options_get_daemon_flag (void)
{
    return daemon_flag;
}

int wsmand_options_get_no_plugins_flag (void)
{
    return no_plugin_flag;
}

int wsmand_options_get_foreground_debug (void)
{
    if (foreground_debug)
        return 6;
    else
        return -1;
}



int wsmand_options_get_debug_level (void)
{
    return debug_level;
}


int wsmand_options_get_syslog_level (void)
{
    return syslog_level;
}


int wsmand_options_get_server_port (void)
{
    return server_port;
}


int
wsmand_options_get_server_ssl_port (void)
{
    return server_ssl_port;
}



char*
wsmand_options_get_ssl_key_file (void)
{
    return ssl_key_file;
}

char*
wsmand_options_get_ssl_cert_file (void)
{
    return ssl_cert_file;
}

int wsmand_options_get_digest (void)
{
    return use_digest;
}

int wsmand_options_get_min_threads (void)
{
    return min_threads;
}


char*
wsmand_options_get_pid_file (void)
{
    return pid_file;
}


int wsmand_options_get_max_threads (void)
{
    return max_threads;
}

int wsmand_is_default_basic_authenticator()
{
    return !strcmp(basic_authenticator, DEFAULT_BASIC_AUTH);
}
char *wsmand_default_basic_authenticator()
{
    return DEFAULT_BASIC_AUTH;
}

char *wsmand_option_get_basic_authenticator() {
    return basic_authenticator;
}

char *wsmand_option_get_basic_authenticator_arg()
{
    return basic_authenticator_arg;
}

typedef struct _ShutdownHandler ShutdownHandler;
struct _ShutdownHandler {
    WsmandShutdownFn fn;
    void* user_data;
};

static list_t *shutdown_handlers = NULL;
static int shutdown_counter = 0;
static int shutdown_pending = 0;
static int shutting_down = 0;

void
wsmand_shutdown_add_handler(WsmandShutdownFn fn,
                            void*     user_data)
{
    ShutdownHandler *handler;

    if (fn == NULL) return;

    handler = u_zalloc (sizeof(ShutdownHandler));
    handler->fn = fn;
    handler->user_data = user_data;

    lnode_t *n = lnode_create(handler);

    if (!shutdown_handlers)
        shutdown_handlers = list_create(LISTCOUNT_T_MAX);

    list_prepend(shutdown_handlers, n );
}

void
wsmand_shutdown_block (void)
{
    if (shutdown_counter < 0)
        return;

    if (shutting_down)
    {
        debug( "Attempting to block shut-down while shut-down is already in progress!");
    }
    ++shutdown_counter;
}

void
wsmand_shutdown_allow (void)
{
    if (shutdown_counter <= 0) return;
    --shutdown_counter;

    if (shutdown_counter == 0 && shutdown_pending) {
        wsmand_shutdown ();
    }
}

static int
shutdown_idle_cb (void* user_data)
{
    int restart = (int )user_data;

    lnode_t *n = list_first(shutdown_handlers);

    debug ("shutdown_idle_cb started");
                
    while (n) {
        ShutdownHandler *handler = n->list_data;
        
        if (handler && handler->fn) 
            handler->fn (handler->user_data);

        u_free (handler);
        n = list_next(shutdown_handlers, n);
    }

    //list_destroy_nodes (shutdown_handlers);
    // list_destroy (shutdown_handlers);

    if (!restart) {
        /* We should be quitting the main loop (which will cause us to
           exit) in a handler.  If not, we'll throw in an exit just to be
           sure. */
        exit (0);
    }
    else 
    {
        const char **argv = wsmand_options_get_argv ();

        errno = 0;
        if ((execv (argv[0], (char **) argv)) < 0) {
            debug( "Can not restart wsmand: %s",
                      strerror (errno));
            exit (EXIT_FAILURE);
        }
    }

    /* We should never reach here... */
    assert (1 == 1);
    return 0;
}

static void
do_shutdown (int restart)
{
    if (shutdown_counter > 0) {
        debug ("Shutting down pended");
        shutdown_pending = TRUE;
        return;
    }
    debug( "Shutting down daemon...");

    if (shutting_down) {
        debug("Shut-down request received while shut-down is already in progress!");
        return;
    }

    shutting_down = TRUE;

#ifdef LIBSOUP_LISTENER
    g_idle_add (shutdown_idle_cb, GINT_TO_POINTER (restart));
#else
    shutdown_idle_cb((void *) (restart));
#endif
}

void
wsmand_shutdown (void)
{
    do_shutdown (FALSE);
}

void
wsmand_restart (void)
{
    do_shutdown (TRUE);
}


