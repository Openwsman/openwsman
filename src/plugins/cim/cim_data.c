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
 * @author Sumeet Kukreja, Dell Inc.
 */

#include "wsman_config.h"

#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "ctype.h"

#include "u/libu.h"

#include "wsman-xml-api.h"
#include "wsman-dispatcher.h"
#include "cim-interface.h"

#include "cim_data.h"

static char *cim_namespace = NULL;
hash_t *vendor_namespaces = NULL;
char *cim_host = "localhost";
char *cim_port = DEFAULT_HTTP_CIMOM_PORT;
char *server_port = "8889";
char *cim_client_frontend = "XML";
int omit_schema_optional = 0;
char *cim_indication_SourceNamespace = NULL;

SER_START_ITEMS(CimResource)
SER_END_ITEMS(CimResource);

START_END_POINTS(CimResource)
  END_POINT_TRANSFER_DIRECT_GET(CimResource, XML_NS_CIM_CLASS),
  END_POINT_TRANSFER_DIRECT_PUT(CimResource, XML_NS_CIM_CLASS),
  END_POINT_TRANSFER_DIRECT_CREATE(CimResource, XML_NS_CIM_CLASS),
  END_POINT_TRANSFER_DIRECT_DELETE(CimResource, XML_NS_CIM_CLASS),
  END_POINT_ENUMERATE(CimResource, XML_NS_CIM_CLASS),
  END_POINT_DIRECT_PULL(CimResource, XML_NS_CIM_CLASS),
  END_POINT_RELEASE(CimResource, XML_NS_CIM_CLASS),
#ifdef ENABLE_EVENTING_SUPPORT
  END_POINT_SUBSCRIBE(CimResource,XML_NS_CIM_CLASS),
  END_POINT_UNSUBSCRIBE(CimResource,XML_NS_CIM_CLASS),
  END_POINT_RENEW(CimResource,XML_NS_CIM_CLASS),
#endif
  END_POINT_PULL(CimResource,XML_NS_CIM_CLASS),
  END_POINT_CUSTOM_METHOD(CimResource, XML_NS_CIM_CLASS),
FINISH_END_POINTS(CimResource);


START_NAMESPACES(CimResource)
  ADD_NAMESPACE( XML_NS_CIM_CLASS, "CIM"),
  ADD_NAMESPACE( XML_NS_CIM_ALL_CLASS, "CIMALL"),
FINISH_NAMESPACES(CimResource);



static list_t *
set_vendor_namespaces(void) 
{
  hscan_t hs;
  hnode_t *hn;
  int i;

  list_t *l = list_create(LISTCOUNT_T_MAX);
  for (i = 0; CimResource_Namespaces[i].ns != NULL; i++) {
    WsSupportedNamespaces *ns =
          (WsSupportedNamespaces *)u_malloc(sizeof(WsSupportedNamespaces));
    ns->class_prefix = CimResource_Namespaces[i].class_prefix;
    ns->ns = (char*) CimResource_Namespaces[i].ns;
    lnode_t *node = lnode_create(ns);
    list_append(l, node);
  }

  if (vendor_namespaces && hash_count(vendor_namespaces) > 0 ) {
    hash_scan_begin(&hs, vendor_namespaces);
    while ((hn = hash_scan_next(&hs))) {
      WsSupportedNamespaces *ns =
           (WsSupportedNamespaces *)u_malloc(sizeof(WsSupportedNamespaces));
      ns->class_prefix = (char*)hnode_getkey(hn);
      ns->ns = (char*) hnode_get(hn);
      lnode_t *node = lnode_create(ns);
      list_append(l, node);
    }
  }
  return l;
}

void
get_endpoints( void *self, 
               void **data)
{
  debug("Registering interface");
  WsDispatchInterfaceInfo *ifc = (WsDispatchInterfaceInfo *)data;
  ifc->flags = 0;
  ifc->actionUriBase = NULL;
  ifc->version = PACKAGE_VERSION;
  ifc->config_id = "cim";
  ifc->vendor = "Openwsman Project";
  ifc->displayName = "CIM Resource";
  ifc->notes = "CIM Resource";
  ifc->compliance = XML_NS_WS_MAN;
  ifc->wsmanResourceUri = NULL;
  //ifc->namespaces = CimResource_Namespaces;
  ifc->namespaces = set_vendor_namespaces();
  ifc->extraData = NULL;
  ifc->endPoints = CimResource_EndPoints;
  return;
}

int init( void *self, void **data )
{
  return 1;
}

void cleanup( void *self, void *data )
{
  return;
}

void set_config( void *self, dictionary *config )
{
  debug("reading configuration file options");
  if (config) {
    cim_namespace = iniparser_getstr (config, "cim:default_cim_namespace");
    char *namespaces = iniparser_getstr (config, "cim:vendor_namespaces");
    cim_host = iniparser_getstring(config, "cim:host", "localhost");
    cim_client_frontend = iniparser_getstring(config, "cim:cim_client_frontend", "XML");
    cim_port = iniparser_getstring(config, "cim:port", DEFAULT_HTTP_CIMOM_PORT);
    server_port = iniparser_getstring(config, "server:port", "8889");
    omit_schema_optional = iniparser_getboolean(config, "cim:omit_schema_optional", 0);
    cim_indication_SourceNamespace = iniparser_getstr(config, "cim:indication_source_namespace");
    debug("vendor namespaces: %s", namespaces);
    if (namespaces) {
      hash_t * t = u_parse_query(namespaces);
      if (t) {
        vendor_namespaces = t;
      }
      else
        vendor_namespaces = NULL;
    }
    debug("cim namespace: %s", cim_namespace);
  }
  return;
}



hash_t*
get_vendor_namespaces()
{
  return vendor_namespaces;
}

char *
get_cim_host()
{
    return cim_host;
}

char *
get_cim_client_frontend()
{
    return cim_client_frontend;
}

int get_omit_schema_optional()
{
	return omit_schema_optional;
}

char *
get_cim_port()
{
    return cim_port;
}

char *get_server_port(void)
{
   return server_port;
}

char*
get_cim_namespace()
{
  if (cim_namespace)
    return cim_namespace;
  else
    return CIM_NAMESPACE;
}

char *
get_cim_indication_SourceNamespace()
{
	return cim_indication_SourceNamespace;
}

