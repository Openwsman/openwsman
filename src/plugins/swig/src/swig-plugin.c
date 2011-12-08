/*
 * swig-plugin.c
 * 
 * 'C' interface for swig-based openwsman server plugins
 * 
 * This file implements the plugin API as needed by openwsman
 * 
 */

/*****************************************************************************
* Copyright (C) 2008 Novell Inc. All rights reserved.
* Copyright (C) 2008 SUSE Linux Products GmbH. All rights reserved.
* 
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
* 
*   - Redistributions of source code must retain the above copyright notice,
*     this list of conditions and the following disclaimer.
* 
*   - Redistributions in binary form must reproduce the above copyright notice,
*     this list of conditions and the following disclaimer in the documentation
*     and/or other materials provided with the distribution.
* 
*   - Neither the name of Novell Inc. nor of SUSE Linux Products GmbH nor the
*     names of its contributors may be used to endorse or promote products
*     derived from this software without specific prior written permission.
* 
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS ``AS IS''
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
* ARE DISCLAIMED. IN NO EVENT SHALL Novell Inc. OR SUSE Linux Products GmbH OR
* THE CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
* EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, 
* PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; 
* OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, 
* WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR 
* OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF 
* ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*****************************************************************************/

#ifdef HAVE_CONFIG_H
#undef PACKAGE_NAME
#undef PACKAGE_TARNAME
#undef PACKAGE_VERSION
#undef PACKAGE_STRING
#undef PACKAGE_BUGREPORT
#undef SIZEOF_SHORT
#undef SIZEOF_INT
#undef SIZEOF_LONG
#undef SIZEOF_LONG_LONG
#include "wsman_config.h"
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#ifdef HAVE_CTYPE_H
#include <ctype.h>
#endif

#include <wsman-xml-api.h>
#include <wsman-xml-serializer.h>
#include <wsman-declarations.h>

#define OWN 1

int init( void *self, void **data );

/*
 * string2target
 * char* -> Target_Type
 * 
 * *must* be called within TARGET_THREAD_BEGIN_BLOCK/TARGET_THREAD_END_BLOCK
 */

static Target_Type
string2target(const char *s)
{
    Target_Type obj;

    if (s == NULL)
        return Target_Null;

    obj = Target_String(s);
 
    return obj;
}


/*
 * proplist2target
 * char** -> Target_Type
 * 
 * *must* be called within TARGET_THREAD_BEGIN_BLOCK/TARGET_THREAD_END_BLOCK
 */

static Target_Type
proplist2target(const char** cplist)
{
    Target_Type pl;

    if (cplist == NULL)
    {
        Target_INCREF(Target_Void);
        return Target_Void; 
    }
 
    pl = Target_Array(); 
    for (; (cplist!=NULL && *cplist != NULL); ++cplist)
    {
        Target_Append(pl, Target_String(*cplist)); 
    }
 
    return pl; 
}


static char *
fmtstr(const char* fmt, ...)
{
    va_list ap; 
    int len; 
    char* str;

    va_start(ap, fmt); 
    len = vsnprintf(NULL, 0, fmt, ap); 
    va_end(ap); 
    if (len <= 0)
    {
        return NULL; 
    }
    str = (char*)malloc(len+1); 
    if (str == NULL)
    {
        return NULL; 
    }
    va_start(ap, fmt); 
    vsnprintf(str, len+1, fmt, ap); 
    va_end(ap); 
    return str; 
}


/*
**==============================================================================
**
** Local definitions:
**
**==============================================================================
*/


/*
 * There is one target interpreter, serving multiple plugins
 * The number of plugins using the interpreter is counted in _PLUGIN_COUNT,
 * when the last user goes aways, the target interpreter is unloaded.
 * 
 * _PLUGIN_INIT_MUTEX protects this references counter from concurrent access.
 * 
 */

#if defined(SWIGPERL)
static PerlInterpreter * _TARGET_INIT = 0; /* acts as a boolean - is target initialized? */
#else
static int _TARGET_INIT = 0; /* acts as a boolean - is target initialized? */
#endif
static int _PLUGIN_COUNT = 0;    /* use count, number of plugins */
static pthread_mutex_t _PLUGIN_INIT_MUTEX = PTHREAD_MUTEX_INITIALIZER;  /* mutex around _PLUGIN_COUNT */
static Target_Type _TARGET_MODULE = Target_Null;  /* The target module (aka namespace) */


#if defined(SWIGPYTHON)
#include "target_python.c"
#endif

#if defined(SWIGRUBY)
#include "target_ruby.c"
#endif

#if defined(SWIGPERL)
#include "target_perl.c"
#endif

struct __Swig
{
        char* xml;
};
typedef struct __Swig Swig;


/*
 * Service endpoint declarations
 * 
 */

static int
Swig_Identify_EP( WsContextH cntx )
{
    int rc;
    Target_Type _context;
    TARGET_THREAD_BEGIN_BLOCK; 
    _context = SWIG_NewPointerObj((void*) cntx, SWIGTYPE_p__WS_CONTEXT, OWN);
    rc = TargetCall(cntx->indoc, _TARGET_MODULE, "identify", 1, _context); 
    TARGET_THREAD_END_BLOCK;
    return rc;
}

static int
Swig_Enumerate_EP( WsContextH cntx, WsEnumerateInfo* enumInfo,
		WsmanStatus *status,
		void *opaqueData )
{
    Target_Type _context, _enumInfo;
    Target_Type _status;
    int rc;
    TARGET_THREAD_BEGIN_BLOCK; 
    debug("Swig_Enumerate_EP(cntx %p, enumInfo %p, status %p, opaqueData %p", cntx, enumInfo, status, opaqueData);
    debug("enumInfo.epr_to %s, epr_uri %s", enumInfo->epr_to, enumInfo->epr_uri);
    _context = SWIG_NewPointerObj((void*) cntx, SWIGTYPE_p__WS_CONTEXT, OWN);
    _enumInfo = SWIG_NewPointerObj((void*) enumInfo, SWIGTYPE_p___WsEnumerateInfo, OWN);
    _status = SWIG_NewPointerObj((void*) status, SWIGTYPE_p__WsmanStatus, OWN);
    rc = TargetCall(cntx->indoc, _TARGET_MODULE, "enumerate", 3, _context, _enumInfo, _status );
    TARGET_THREAD_END_BLOCK;
    return rc;
}


static int
Swig_Release_EP( WsContextH cntx, WsEnumerateInfo* enumInfo,
		WsmanStatus *status,
		void *opaqueData )
{
    Target_Type _context;
    Target_Type _enumInfo;
    Target_Type _status;
    int rc;
    TARGET_THREAD_BEGIN_BLOCK; 
    _context = SWIG_NewPointerObj((void*) cntx, SWIGTYPE_p__WS_CONTEXT, OWN);
    _enumInfo = SWIG_NewPointerObj((void*) enumInfo, SWIGTYPE_p___WsEnumerateInfo, OWN);
    _status = SWIG_NewPointerObj((void*) status, SWIGTYPE_p__WsmanStatus, OWN);
    rc = TargetCall(cntx->indoc, _TARGET_MODULE, "release", 3, _context, _enumInfo, _status );
    TARGET_THREAD_END_BLOCK;
    return rc;
}


static int
Swig_Pull_EP( WsContextH cntx, WsEnumerateInfo* enumInfo,
		WsmanStatus *status,
		void *opaqueData )
{
    Target_Type _context;
    Target_Type _enumInfo;
    Target_Type _status;
    int rc;
    TARGET_THREAD_BEGIN_BLOCK; 
    _context = SWIG_NewPointerObj((void*) cntx, SWIGTYPE_p__WS_CONTEXT, OWN);
    _enumInfo = SWIG_NewPointerObj((void*) enumInfo, SWIGTYPE_p___WsEnumerateInfo, OWN);
    _status = SWIG_NewPointerObj((void*) status, SWIGTYPE_p__WsmanStatus, OWN);
    rc = TargetCall(cntx->indoc, _TARGET_MODULE, "pull", 3, _context, _enumInfo, _status );
    TARGET_THREAD_END_BLOCK;
    return rc;
}


static int
Swig_Get_EP( SoapOpH op, void* appData, void *opaqueData )
{
    Target_Type _op;
    int rc;
    WsXmlDocH in_doc = soap_get_op_doc( op, 1 );
    TARGET_THREAD_BEGIN_BLOCK; 
    _op = SWIG_NewPointerObj((void*) op, SWIGTYPE_p___SoapOp, OWN);
  
    rc = TargetCall(in_doc, _TARGET_MODULE, "get", 1, _op );
    TARGET_THREAD_END_BLOCK;
    return rc;
}


static int
Swig_Custom_EP( SoapOpH op, void* appData, void *opaqueData )
{
    Target_Type _op;
    int rc;
    WsXmlDocH in_doc = soap_get_op_doc( op, 1 );
    TARGET_THREAD_BEGIN_BLOCK; 
    _op = SWIG_NewPointerObj((void*) op, SWIGTYPE_p___SoapOp, OWN);
    rc = TargetCall(in_doc, _TARGET_MODULE, "custom", 1, _op );
    TARGET_THREAD_END_BLOCK;
    return rc;
}


static int
Swig_Put_EP( SoapOpH op, void* appData, void *opaqueData )
{
    Target_Type _op;
    int rc;
    WsXmlDocH in_doc = soap_get_op_doc( op, 1 );
    TARGET_THREAD_BEGIN_BLOCK; 
    _op = SWIG_NewPointerObj((void*) op, SWIGTYPE_p___SoapOp, OWN);
    rc = TargetCall(in_doc, _TARGET_MODULE, "put", 1, _op );
    TARGET_THREAD_END_BLOCK;
    return rc;
}


static int
Swig_Create_EP( SoapOpH op, void* appData, void *opaqueData )
{
    Target_Type _op;
    int rc;
    WsXmlDocH in_doc = soap_get_op_doc( op, 1 );
    TARGET_THREAD_BEGIN_BLOCK; 
    _op = SWIG_NewPointerObj((void*) op, SWIGTYPE_p___SoapOp, OWN);
    rc = TargetCall(in_doc, _TARGET_MODULE, "create", 1, _op );
    TARGET_THREAD_END_BLOCK;
    return rc;
}



static int
Swig_Delete_EP( SoapOpH op, void* appData, void *opaqueData )
{
    Target_Type _op;
    int rc;
    WsXmlDocH in_doc = soap_get_op_doc( op, 1 );
    TARGET_THREAD_BEGIN_BLOCK; 
    _op = SWIG_NewPointerObj((void*) op, SWIGTYPE_p___SoapOp, OWN);
    rc = TargetCall(in_doc, _TARGET_MODULE, "delete", 1, _op );
    TARGET_THREAD_END_BLOCK;
    return rc;
}


static int
Swig_UnSubscribe_EP(WsContextH cntx,
		WsSubscribeInfo* subsInfo,
		WsmanStatus *status,
		void *opaqueData)
{
	return -1;
}


static int
Swig_SubscriptionCancel_EP(WsEventThreadContextH cntx)
{
    WsmanStatus status;
    return Swig_UnSubscribe_EP(cntx->soap->cntx, cntx->subsInfo, &status, NULL);
}


static int
Swig_Subscribe_EP(WsContextH cntx,
		WsSubscribeInfo* subsInfo,
		WsmanStatus *status,
		void *opaqueData)
{
    return -1;
}


static int
Swig_Renew_EP(WsContextH cntx,
		WsSubscribeInfo* subsInfo,
		WsmanStatus *status,
		void *opaqueData)
{
    return -1;
}


/* ************** Array of end points for resource ****************
 *
 * Must follow general convention xxx_EndPoints
 */

SER_START_ITEMS(Swig)
SER_END_ITEMS(Swig);
 
START_END_POINTS(Swig)
   END_POINT_TRANSFER_DIRECT_GET(Swig, NULL),
   END_POINT_TRANSFER_DIRECT_PUT(Swig, NULL),
   END_POINT_TRANSFER_DIRECT_CREATE(Swig, NULL),
   END_POINT_TRANSFER_DIRECT_DELETE(Swig, NULL),
   END_POINT_ENUMERATE(Swig, NULL),
   END_POINT_DIRECT_PULL(Swig, NULL),
   END_POINT_RELEASE(Swig, NULL),
#ifdef ENABLE_EVENTING_SUPPORT
   END_POINT_SUBSCRIBE(Swig,NULL),
   END_POINT_UNSUBSCRIBE(Swig,NULL),
   END_POINT_RENEW(Swig,NULL),
#endif
   END_POINT_PULL(Swig,NULL),
   END_POINT_CUSTOM_METHOD(Swig, NULL),
   END_POINT_IDENTIFY(Swig, NULL),
FINISH_END_POINTS(Swig);

/*----------------------------------------------------------------
 * register end points with server
 * called from wsman_init_plugins()
 * 
 * self: p_handle
 * data: ifc (WsDispatchInterfaceInfo *)
 */

void
get_endpoints(void *self, void *data_ifc)
{
    char *namespace = NULL;
    WsDispatchInterfaceInfo *ifc = (WsDispatchInterfaceInfo *)data_ifc;	

    debug("get_endpoints (%p, %p)", self, ifc);
  
    list_t *namespaces = TargetEndpoints( self, ifc->extraData );

    WsDispatchEndPointInfo *epi = Swig_EndPoints;
    while (epi->serviceEndPoint) {
      epi->data = namespace;
      ++epi;
    }
    ifc->flags = 0;
    ifc->actionUriBase = NULL;
    ifc->version = OPENWSMAN_PLUGIN_API_VERSION;
    ifc->config_id = "swig";
    ifc->vendor = "SUSE Linux Products GmbH";
    ifc->displayName = PLUGIN_FILE;
#if defined(SWIGPYTHON)
    ifc->notes = "Python plugin";
#endif
#if defined(SWIGRUBY)
    ifc->notes = "Ruby plugin";
#endif
    ifc->compliance = XML_NS_WS_MAN;
    ifc->wsmanResourceUri = NULL;
    ifc->namespaces = namespaces;
    ifc->endPoints = Swig_EndPoints;
}

/*----------------------------------------------------------------
 * set configuration
 * called from wsman_init_plugins()
 * 
 * self: p_handle
 * config: listener->config
 */

void
set_config( void *self, dictionary *config )
{
    return;
}

/*----------------------------------------------------------------
 * cleanup plugin
 */

void
cleanup( void  *self, void *data )
{
    TargetCleanup( self, data );
    return;
}


/*----------------------------------------------------------------
 * initialize plugin
 * return zero on error
 * 
 * self: p_handle
 * data: pointer to 'void *data'
 *   init can fill this data which is passed to every other call
 */

int
init( void *self, void **data )
{
    return (TargetInitialize( self, data ) == 0);
}
