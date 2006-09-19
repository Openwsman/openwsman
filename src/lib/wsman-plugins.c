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
#include <wsman_config.h>
#endif

#include "stdlib.h"
#include "stdio.h"
#include <string.h>
#include <dirent.h>

#include <dlfcn.h>
#include "u/libu.h"
#include "wsman-plugins.h"



static void
free_string_list ( list_t * plugin_list )
{
    list_destroy_nodes(plugin_list);
}

static 
list_t *scan_files_in_dir ( const char *dir, int (*select)(const struct dirent *))
{
    struct dirent **namelist;
    int n;
    list_t *files = list_create(LISTCOUNT_T_MAX);

    if (0 > (n = scandir (dir, &namelist, select, alphasort)))
    {
        return files;
    } else {
        while (n--)
        {
            lnode_t *node = lnode_create(u_strdup(namelist[n]->d_name));
            list_append(files, node );
            debug("plugin file found: %s", namelist[n]->d_name );
            u_free(namelist[n]);
        }
        u_free(namelist);
    }
    return files;
}


static WsManPlugin *plugin_new(void)
{
    WsManPlugin *self = u_malloc(sizeof(WsManPlugin));
    return self ;
}

static void plugin_free(WsManPlugin *self)
{
    debug( "Un-loading plugins: %s", self->p_name ); 

    //g_return_if_fail(self);
    //
    if( self->p_handle && self->cleanup )
        (*self->cleanup)( self->p_handle, self->data );
    if(self->p_name)
        u_free(self->p_name);
    if( self->p_handle )
        dlclose( self->p_handle );
    u_free( self );
}

static WsManPluginError plugin_init(WsManPlugin *self, const char *p_name)
{
    WsManPluginError PluginError = PLUGIN_ERROR_OK ;
    /*
    g_return_val_if_fail(self, PLUGIN_ERROR_BADPARMS );
    g_return_val_if_fail(p_name && strlen(p_name), PLUGIN_ERROR_BADPARMS);
    */
    self->p_name = u_strdup(p_name) ;
    if (NULL != (self->p_handle = dlopen(p_name, RTLD_LAZY)))
    {
        
        if (  dlsym(self->p_handle, "get_endpoints")
                &&  dlsym(self->p_handle, "init"))
        {
            /*
            self->started = (*self->init)(self->p_handle, &self->data);
            if( !self->started ) {
                PluginError = PLUGIN_ERROR_INITFAILED;
            }
            */
        } else {
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

static int
load_plugin(WsManPlugin *self, const char *p_name)
{
    debug("Loading plugin: %s", p_name );

    int retv = -1;
    WsManPluginError err = plugin_init(self, p_name);
    const char	*plugin_err = dlerror();
    if( NULL == plugin_err )
        plugin_err = "";
    switch( err )
    {
    default:
        debug(  "Unable to load plugin %s. Error: %s", p_name, plugin_err );
        break;
    case PLUGIN_ERROR_NOTLOADED:
        debug( "Unable to load plugin %s. Error: %s", p_name, plugin_err );
        break;
    case PLUGIN_ERROR_SYMBOLSNOTFOUND:
        debug( "Plugin protocol %s unknown Error:%s", p_name, plugin_err );
        break;
    case PLUGIN_ERROR_INITFAILED:
        debug("Unable to start plugin %s", p_name );
        break;
    case PLUGIN_ERROR_BADPARMS:
        debug( "Bad parameters to plugin %s. Error: %s", p_name, plugin_err );
        break;
    case PLUGIN_ERROR_OK:
        retv = 0 ;
        break;
    }
    if( retv < 0  )
        debug("Unable to load plugin %s.Error: %s", p_name, plugin_err);
    return retv ;
}

static void
plugins_foreach_delete( void  *data, void *user_data )
{
    plugin_free((WsManPlugin *)data);
}

static void
free_plugins(list_t * plugin_list)
{
    if( plugin_list )
    {
        // FIXME: g_list_foreach( plugin_list, plugins_foreach_delete, NULL );
        // FIXME: g_list_free (plugin_list);
    }
}

static int
select_all_files (const struct dirent *e)
{
    return 1; 
}

static void
scan_plugins_in_directory (
        WsManListenerH *listener, 
        const char *dir_name, 
        list_t *plugin_list)
{


    /*
    g_return_val_if_fail (listener != NULL, plugin_list);
    g_return_val_if_fail (((NULL != dir_name) && strlen(dir_name)), plugin_list);
    */

    list_t *files = scan_files_in_dir ( dir_name, select_all_files);
    debug("found %lu files", list_count(files));

    lnode_t *node = list_first(files);
    while (node != NULL)
    {
        const char* entry_name;
        int retv = -1;
        entry_name = (const char*) node->list_data;
        node = list_next(files, node);

        if ((NULL != entry_name) && strlen (entry_name) > 3
                && (0 == strcmp (&entry_name[strlen(entry_name)-3], ".so")))
        {
            char *plugin_path = u_strdup_printf ("%s/%s", dir_name, entry_name);
            WsManPlugin *plugin = plugin_new();

            if ((NULL != plugin) && (NULL != plugin_path))
            {
                if (load_plugin(plugin, plugin_path) == 0 )
                {
                    lnode_t *plg = lnode_create (plugin);
                    list_append (plugin_list, plg);
                    retv = 0 ;
                }
            } else {
                debug("Out of memory scanning for plugins.");
            }
            u_free (plugin_path);
            if (retv != 0  && (NULL != plugin))
                plugin_free(plugin);
        }
    }
    //free_string_list (files);
    return;
}

int wsman_plugins_load(WsManListenerH *listener)
{   
    listener->plugins = list_create(LISTCOUNT_T_MAX);
    scan_plugins_in_directory(listener, PACKAGE_PLUGIN_DIR, listener->plugins);
    return 0;
}

int  wsman_plugins_unload(WsManListenerH *listener)
{

    free_plugins(listener->plugins);
    listener->plugins  = NULL;
    return 0;
}



#endif
