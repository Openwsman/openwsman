
#ifdef HAVE_CONFIG_H
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

#ifdef SWIGRUBY
#define PLUGINSCRIPT "rbwsmanplugin"
#define PLUGINKLASS "WsmanPlugin"
static VALUE klass = 0;
#endif

struct __Swig
{
        char* xml;
};
typedef struct __Swig Swig;


// Service endpoint declaration
static int
Swig_Identify_EP( WsContextH cntx )
{
  return FIX2INT( rb_funcall( klass, rb_intern( "identify" ), 0 ) );
}

static int
Swig_Enumerate_EP( WsContextH cntx, WsEnumerateInfo* enumInfo,
		WsmanStatus *status,
		void *opaqueData )
{
  return FIX2INT( rb_funcall( klass, rb_intern( "enumerate" ), 0 ) );
}


static int
Swig_Release_EP( WsContextH cntx, WsEnumerateInfo* enumInfo,
		WsmanStatus *status,
		void *opaqueData )
{
  return FIX2INT( rb_funcall( klass, rb_intern( "release" ), 0 ) );
}


static int
Swig_Pull_EP( WsContextH cntx, WsEnumerateInfo* enumInfo,
		WsmanStatus *status,
		void *opaqueData )
{
  return FIX2INT( rb_funcall( klass, rb_intern( "pull" ), 0 ) );
}


static int
Swig_Get_EP( SoapOpH op, void* appData, void *opaqueData )
{
  return FIX2INT( rb_funcall( klass, rb_intern( "get" ), 0 ) );
}


static int
Swig_Custom_EP( SoapOpH op, void* appData, void *opaqueData )
{
  return FIX2INT( rb_funcall( klass, rb_intern( "custom" ), 0 ) );
}


static int
Swig_Put_EP( SoapOpH op, void* appData, void *opaqueData )
{
  return FIX2INT( rb_funcall( klass, rb_intern( "put" ), 0 ) );
}


static int
Swig_Create_EP( SoapOpH op, void* appData, void *opaqueData ){
  return FIX2INT( rb_funcall( klass, rb_intern( "create" ), 0 ) );
}



static int
Swig_Delete_EP( SoapOpH op, void* appData, void *opaqueData )
{
  return FIX2INT( rb_funcall( klass, rb_intern( "delete" ), 0 ) );
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
FINISH_END_POINTS(Swig);

/* register end points with server
 *
 */

void
get_endpoints(void *self, void **data)
{
    char *namespace = NULL;

    fprintf(stderr, "swig-plugin.c: get_endpoints (%p, %p)\n", self, data);
  
    if (!klass)
      init( self, data );
    if (!klass)
      return;
#ifdef SWIGRUBY
    /*
     * Get namespaces
     */
  
    list_t *namespaces = list_create(LISTCOUNT_T_MAX);
    VALUE rbnamespaces = rb_funcall( klass, rb_intern( "namespaces" ), 0 );
    VALUE ary = rb_check_array_type( rbnamespaces );
    if (NIL_P(ary)) {
      rb_raise( rb_eArgError, "%s.namespaces is not array", PLUGINKLASS);
    }
    int len = RARRAY(ary)->len;
    if (len <= 0) {
      rb_raise( rb_eArgError, "%s.namespaces returned array with %d elements", PLUGINKLASS, len);
    }
    int i;
    for (i = 0; i < len; ++i) {
      lnode_t *node;
      VALUE elem = RARRAY(ary)->ptr[i];
      VALUE pair = rb_check_array_type( elem );
      if (NIL_P(pair)) {
	rb_raise( rb_eArgError, "%s.namespaces must return array of arrays", PLUGINKLASS);
      }
      if (RARRAY(pair)->len != 2) {
	rb_raise( rb_eArgError, "%s.namespaces must return array of ['<namespace>','<class_prefix>']", PLUGINKLASS);
      }
      WsSupportedNamespaces *ns = (WsSupportedNamespaces *)u_malloc(sizeof(WsSupportedNamespaces));
      ns->ns = StringValuePtr( RARRAY(pair)->ptr[0] );
      if (namespace == 0) namespace = ns->ns;
      ns->class_prefix = StringValuePtr( RARRAY(pair)->ptr[1] );
      node = lnode_create(ns);
      list_append(namespaces, node);
    }
#endif
    WsDispatchEndPointInfo *epi = Swig_EndPoints;
    while (epi->serviceEndPoint) {
      epi->data = namespace;
      ++epi;
    }
    WsDispatchInterfaceInfo *ifc = (WsDispatchInterfaceInfo *)data;	
    ifc->flags = 0;
    ifc->actionUriBase = NULL;
    ifc->version = PACKAGE_VERSION;
    ifc->config_id = "swig";
    ifc->vendor = "Novell, Inc.";
    ifc->displayName = PLUGINSCRIPT;
    ifc->notes = "Swig based Ruby plugin";
    ifc->compliance = XML_NS_WS_MAN;
    ifc->wsmanResourceUri = NULL;
    ifc->namespaces = namespaces;
    ifc->extraData = NULL;
    ifc->endPoints = Swig_EndPoints;
}

/*===============================================================*/
#ifdef SWIGRUBY

/* protected code (might raise)
 *  to load external Ruby script
 */
static VALUE
load_code()
{    
    fprintf(stderr, "swig-plugin.c: load_code ()\n");
    rb_require(PLUGINSCRIPT);
}

static VALUE
create_plugin()
{
    fprintf(stderr, "swig-plugin.c: create_plugin ()\n");
    klass = rb_class_new_instance(0, NULL, rb_const_get(rb_cObject, rb_intern(PLUGINKLASS)));
    fprintf(stderr, "swig-plugin.c: create_plugin => %p\n", klass);
    return klass;
}
#endif


/*----------------------------------------------------------------
 * ??
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
#ifdef SWIGRUBY
    ruby_finalize();
#endif
#ifdef SWIGPYTHON
    Py_Finalize();
#endif
    return;
}


/*----------------------------------------------------------------
 * initialize plugin
 */

int
init( void *self, void **data )
{
    fprintf(stderr, "swig-plugin.c: init (%p, %p)\n", self, data);
    int rc = 1;
#ifdef SWIGRUBY
    int error = 0;
    ruby_init();
    ruby_init_loadpath();
    /* name the script */
    ruby_script(PLUGINSCRIPT);
    /* load the script */
    rb_protect(load_code, Qnil, &error);
    if (error) {
	debug("Ruby: FAILED loading %s.rb", PLUGINSCRIPT);
        rc = 0;
    }
    else {
        VALUE wsmanplugin;
        debug("Ruby: loaded %s.rb", PLUGINSCRIPT);
        wsmanplugin = rb_protect(create_plugin, Qnil, &error);
        if (error) {
            debug("Ruby: FAILED creating %s", PLUGINKLASS);
	    rc = 0;
	}
	else {
	    debug("Ruby: WsmanPlugin at %p", wsmanplugin);
	}
    }
#endif
    return rc;
}
