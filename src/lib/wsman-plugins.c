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

#ifndef WSMAN_PLUGINS_H_
#define WSMAN_PLUGINS_H_

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "stdlib.h"
#include "stdio.h"
#include <string.h>
#include <dirent.h>
#include <glib.h>


#include "wsman-util.h"
#include "wsman-debug.h"

#include "wsman-plugins.h"



static void
free_string_list ( GList * plugin_list )
{
    int i ;

    if( plugin_list )
    {
        for (i = 0; i < g_list_length (plugin_list); i++)
            g_free (g_list_nth (plugin_list, i)->data);
        g_list_free (plugin_list);
    }
}

static 
GList* scan_files_in_dir (const char *dir, int (*select)(const struct dirent *))
{
    struct dirent **namelist;
    GList *files = NULL;
    int n;

    g_return_val_if_fail (dir != NULL, NULL);
    if (0 > (n = scandir (dir, &namelist, select, alphasort)))
    {
        return NULL;
    }
    else
    {
        while (n--)
        {
            files = g_list_append (files, g_strdup (namelist[n]->d_name));
            free(namelist[n]);
        }
        free(namelist);
    }
    return files;
}


static WsManPlugin *plugin_new(void)
{
    WsManPlugin *self = g_new0(WsManPlugin, 1);
    return self ;
}

static void plugin_free(WsManPlugin *self)
{
    wsman_debug (WSMAN_DEBUG_LEVEL_MESSAGE, "Un-loading plugins: %s", self->p_name ); 
    g_return_if_fail(self);
    if( self->p_handle && self->cleanup )
        (*self->cleanup)( self->p_handle, self->data );
    if(self->p_name)
        g_free(self->p_name);
    if( self->p_handle )
        g_module_close( self->p_handle );
    g_free( self );
}

static WsManPluginError plugin_init(WsManPlugin *self, const gchar *p_name)
{
    WsManPluginError PluginError = PLUGIN_ERROR_OK ;
    g_return_val_if_fail(self, PLUGIN_ERROR_BADPARMS );
    g_return_val_if_fail(p_name && strlen(p_name), PLUGIN_ERROR_BADPARMS);
    self->p_name = g_strdup(p_name) ;
    if (NULL != (self->p_handle = g_module_open(p_name, 0)))
    {
        if (                
                g_module_symbol(self->p_handle, "get_endpoints", (void *)&self->get_endpoints)
                &&  g_module_symbol(self->p_handle, "init", (void *)&self->init)
           )
        {
            self->started = (*self->init)(self->p_handle, &self->data);
            if( !self->started )
            {
                PluginError = PLUGIN_ERROR_INITFAILED;
            }
        } 
        else
        {
            self->init			        = 0 ;
            self->started	                = 0 ;
            PluginError = PLUGIN_ERROR_SYMBOLSNOTFOUND ;
        }
    } 
    else 
    {
        PluginError = PLUGIN_ERROR_NOTLOADED ;
    }
    return PluginError ;
}

static gboolean
load_plugin(WsManPlugin *self, const gchar *p_name)
{
    wsman_debug (WSMAN_DEBUG_LEVEL_MESSAGE,  "Loading plugin: %s", p_name );
    gboolean ok = FALSE;
    WsManPluginError err = plugin_init(self, p_name);
    const gchar	*plugin_err = g_module_error();
    if( NULL == plugin_err )
        plugin_err = "";
    switch( err )
    {
    default:
        wsman_debug (WSMAN_DEBUG_LEVEL_ERROR,  "Unable to load plugin %s. Error: %s", p_name, plugin_err );
        break;
    case PLUGIN_ERROR_NOTLOADED:
        wsman_debug (WSMAN_DEBUG_LEVEL_ERROR, "Unable to load plugin %s. Error: %s", p_name, plugin_err );
        break;
    case PLUGIN_ERROR_SYMBOLSNOTFOUND:
        wsman_debug (WSMAN_DEBUG_LEVEL_ERROR, "Plugin protocol %s unknown Error:%s", p_name, plugin_err );
        break;
    case PLUGIN_ERROR_INITFAILED:
        wsman_debug (WSMAN_DEBUG_LEVEL_ERROR,"Unable to start plugin %s", p_name );
        break;
    case PLUGIN_ERROR_BADPARMS:
        wsman_debug (WSMAN_DEBUG_LEVEL_ERROR, "Bad parameters to plugin %s. Error: %s", p_name, plugin_err );
        break;
    case PLUGIN_ERROR_OK:
        ok = TRUE ;
        break;
    }
    if( !ok  )
        wsman_debug (WSMAN_DEBUG_LEVEL_ERROR,"Unable to load plugin %s.Error: %s", p_name, plugin_err);
    return ok ;
}

static void
plugins_foreach_delete( gpointer data, gpointer user_data )
{
    plugin_free((WsManPlugin *)data);
}

static void
free_plugins(GList * plugin_list)
{
    if( plugin_list )
    {
        g_list_foreach( plugin_list, plugins_foreach_delete, NULL );
        g_list_free (plugin_list);
    }
}

static int
select_all_files (const struct dirent *e)
{
    return TRUE; 
}

static GList *
scan_plugins_in_directory (WsManListenerH *listener, 
        const gchar *dir_name, 
        GList *plugin_list)
{
    GList *files;
    GList *node;

    if (!g_module_supported ())
        return NULL ;

    g_return_val_if_fail (listener != NULL, plugin_list);
    g_return_val_if_fail (((NULL != dir_name) && strlen(dir_name)), plugin_list);

    files = NULL;

    files = scan_files_in_dir (dir_name, select_all_files);

    node = files;
    while (node)
    {
        const char* entry_name;
        gboolean ok = FALSE;

        entry_name = (const char*) node->data;
        node = g_list_next (node);

        if ((NULL != entry_name) && strlen (entry_name) > 3
                && (0 == strcmp (&entry_name[strlen(entry_name)-3], ".so")))
        {
            gchar *plugin_path = g_strdup_printf ("%s/%s", dir_name, entry_name);
            WsManPlugin *plugin = plugin_new();

            if ((NULL != plugin) && (NULL != plugin_path))
            {
                if (load_plugin(plugin, plugin_path))
                {
                    plugin_list = g_list_prepend (plugin_list, plugin);
                    ok = TRUE ;
                }
            }
            else
            {
                wsman_debug (WSMAN_DEBUG_LEVEL_ERROR,"Out of memory scanning for plugins.");
            }

            g_free (plugin_path);

            if (!ok && (NULL != plugin))
                plugin_free(plugin);
        }
    }

    free_string_list (files);

    return plugin_list;
}

gboolean wsman_plugins_load(WsManListenerH *listener)
{   

    listener->plugins = scan_plugins_in_directory(listener, PACKAGE_PLUGIN_DIR, NULL);
    return TRUE;
}

gboolean wsman_plugins_unload(WsManListenerH *listener)
{

    free_plugins(listener->plugins);
    listener->plugins  = NULL;
    return TRUE;
}



#endif
