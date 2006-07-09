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


#include <config.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <errno.h>

#include "glib.h"
#include "wsman-debug.h"
#include "wsmand-daemon.h"

#define DEFAULT_CONFIG_FILE SYSCONFDIR "/openwsman.conf"

static const char **wsmand_argv = NULL;

static gint server_port =  -1;
static gint server_ssl_port = -1;
static gboolean use_digest = FALSE;
static gchar *ssl_key_file = NULL;
static gchar *ssl_cert_file = NULL;
static gboolean daemon_flag = FALSE;
static gboolean no_plugin_flag = FALSE;
static gint debug_level = -1;
static gboolean foreground_debug = FALSE;
static gint syslog_level = -1;
static gchar *log_location = NULL;
static gchar *digest_password_file = NULL;

static char *config_file = NULL;



gboolean wsmand_parse_options(int argc, char **argv) 
{
    GOptionContext *opt_ctx;
    gboolean retval = FALSE;
    GError *error = NULL;

    GOptionEntry options[] = {
        { "no-plugins", 	'n', 0 ,G_OPTION_ARG_NONE, 	&no_plugin_flag,"Do not load any plugins", NULL }, 
        { "debug", 		'd', 0 ,G_OPTION_ARG_NONE, 	&foreground_debug, 	"Start daemon in foreground and turn on debugging", NULL },
        { "syslog", 		's', 0, G_OPTION_ARG_INT, 	&syslog_level,  "Set the verbosity of syslog output.", "0-6" },
        { "config-file",	'c', 0, G_OPTION_ARG_FILENAME, 	
            &config_file,  	"Alternate configuration file", "<file>" },

        { NULL }
    };	

    wsmand_argv = (const char **)argv;
    opt_ctx = g_option_context_new("WS-Management Server");
    g_option_context_set_ignore_unknown_options(opt_ctx, FALSE);
    g_option_context_add_main_entries(opt_ctx, options, "wsman");  	
    retval = g_option_context_parse(opt_ctx, &argc, &argv, &error);
    if (error) {
        if (error->message)
            printf ("%s\n", error->message);
        return FALSE;
    }
    if (!wsmand_read_config()) {
        fprintf(stderr, "Configuration file not found\n");
        return FALSE;
    }

    g_free(error);
    g_option_context_free(opt_ctx);
    return retval;    
}

const char ** wsmand_options_get_argv (void)
{
    return wsmand_argv;
}

int wsmand_read_config (void)
{
    GKeyFile *cf;
    char *filename;
    filename = (char *)wsmand_options_get_config_file();
    if (!filename) 
        filename = DEFAULT_CONFIG_FILE;
    cf = g_key_file_new ();
    if (g_key_file_load_from_file (cf, filename, G_KEY_FILE_NONE, NULL))
    {
        if (g_key_file_has_group (cf, "server"))
        {
            if (g_key_file_has_key (cf, "server", "port", NULL))
                server_port = g_key_file_get_integer (cf, "server", "port", NULL);
            
            if (g_key_file_has_key (cf, "server", "ssl_port", NULL))
                server_ssl_port = g_key_file_get_integer (cf, "server", "ssl_port", NULL);
            
            if (g_key_file_has_key (cf, "server", "debug_level", NULL))
                debug_level = g_key_file_get_integer (cf, "server", "debug_level", NULL);
            
            if (g_key_file_has_key (cf, "server", "ssl_key_file", NULL))
                ssl_key_file = g_key_file_get_string (cf, "server", "ssl_key_file", NULL);
            
            if (g_key_file_has_key (cf, "server", "ssl_cert_file", NULL))
                ssl_cert_file = g_key_file_get_string (cf, "server", "ssl_cert_file", NULL);
            
            if (g_key_file_has_key (cf, "server", "use_digest", NULL))
                use_digest = g_key_file_get_boolean (cf, "server", "use_digest", NULL);
            
            if (g_key_file_has_key (cf, "server", "digest_password_file", NULL))
                digest_password_file = g_key_file_get_string (cf, "server", "digest_password_file", NULL);
            
            if (g_key_file_has_key (cf, "server", "log_location", NULL))
                log_location = g_key_file_get_string (cf, "server", "log_location", NULL);
        }
    } else {
        return 0;
    }
    g_key_file_free (cf);
    return 1;
}

const char *
wsmand_options_get_config_file (void) {
    if (config_file != NULL && !g_path_is_absolute (config_file)) {
        char cwd[PATH_MAX];
        char *new_config_file;

        getcwd (cwd, PATH_MAX);
          
        new_config_file = g_strconcat (cwd, "/", config_file, NULL);

        g_free (config_file);
        config_file = new_config_file;
    }
    return config_file;
}


gboolean wsmand_options_get_daemon_flag (void)
{
    return daemon_flag;
}

gboolean wsmand_options_get_no_plugins_flag (void)
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

gboolean
wsmand_options_get_digest (void)
{
    return use_digest;
}


typedef struct _ShutdownHandler ShutdownHandler;
struct _ShutdownHandler {
    WsmandShutdownFn fn;
    gpointer user_data;
};

static GSList *shutdown_handlers = NULL;
static int shutdown_counter = 0;
static gboolean shutdown_pending = FALSE;
static gboolean shutting_down = FALSE;

void
wsmand_shutdown_add_handler (WsmandShutdownFn fn,
                          gpointer      user_data)
{
    ShutdownHandler *handler;

    g_return_if_fail (fn != NULL);

    handler = g_new0 (ShutdownHandler, 1);
    handler->fn = fn;
    handler->user_data = user_data;

    shutdown_handlers = g_slist_prepend (shutdown_handlers,
                                         handler);
}

void
wsmand_shutdown_block (void)
{
    g_return_if_fail (shutdown_counter >= 0);

    if (shutting_down)
    {
        wsman_debug (WSMAN_DEBUG_LEVEL_WARNING,
                  "Attempting to block shut-down while shut-down is already in progress!");
    }
    ++shutdown_counter;
}

void
wsmand_shutdown_allow (void)
{
    g_return_if_fail (shutdown_counter > 0);
    --shutdown_counter;

    if (shutdown_counter == 0 && shutdown_pending) {
        wsmand_shutdown ();
    }
}

static gboolean
shutdown_idle_cb (gpointer user_data)
{
    gboolean restart = GPOINTER_TO_INT (user_data);
    GSList *iter;    

    for (iter = shutdown_handlers; iter != NULL; iter = iter->next) {
        ShutdownHandler *handler = iter->data;
        
        if (handler && handler->fn) 
            handler->fn (handler->user_data);

        g_free (handler);
    }

    g_slist_free (shutdown_handlers);

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
            wsman_debug (WSMAN_DEBUG_LEVEL_ERROR, "Can not restart wsmand: %s",
                      strerror (errno));
            exit (EXIT_FAILURE);
        }
    }

    /* We should never reach here... */
    g_assert_not_reached ();
    return FALSE;
}

static void
do_shutdown (gboolean restart)
{
    if (shutdown_counter > 0) {
        shutdown_pending = TRUE;
        return;
    }

    wsman_debug (WSMAN_DEBUG_LEVEL_MESSAGE, "Shutting down daemon...");

    if (shutting_down) {
        wsman_debug (WSMAN_DEBUG_LEVEL_WARNING,
                  "Shut-down request received while shut-down is already in progress!");
        return;
    }

    shutting_down = TRUE;

    g_idle_add (shutdown_idle_cb, GINT_TO_POINTER (restart));
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


