#if defined(SWIGJAVA)
%module jwsman
#endif

#if defined(SWIGPYTHON)
%module pywsman
#endif

#if defined(SWIGCSHARP)
%module cswsman
#endif

#if defined(SWIGRUBY)

%module rbwsman

%{
#include <rubyio.h>
#include <ruby.h>
%}

%typemap(in) FILE* {
  struct OpenFile *fptr;

  Check_Type($input, T_FILE);
  GetOpenFile($input, fptr);
  /*rb_io_check_writable(fptr);*/
  $1 = GetReadFile(fptr);
}

/*SWIG 1.3.33: %feature("autodoc","1")*/
#endif

#if defined(SWIGPERL)
%module openwsman

//==================================
// Typemap: Allow FILE* as PerlIO
//----------------------------------
%typemap(in) FILE* {
    $1 = PerlIO_findFILE(IoIFP(sv_2io($input)));
}
#endif

%{
#include <wsman-types.h>
#include <wsman-client.h>
#include <wsman-api.h>
#include <wsman-xml-api.h>
#include <wsman-epr.h>
#include <wsman-filter.h>
/*#include "openwsman.h"*/
%}

%include "wsman-types.i"

%include "wsman-names.i"

%include "wsman-xml.i"

%include "wsman-epr.i"

%include "wsman-filter.i"

%include "client_opt.i"

#if defined(SWIGRUBY)
/*
 * call-seq:
 *   WsMan::debug
 *
 * Return openwsman debug level.
 */

static VALUE
rwsman_debug_get( VALUE module )
{
    debug_level_e d = wsman_debug_get_level();
    return INT2FIX( d );
}


/*
 * call-seq:
 *   WsMan::debug = 1
 *   WsMan::debug = 0
 *
 * Set openwsman debug level.
 */

static VALUE
rwsman_debug_set( VALUE module, VALUE dbg )
{
    static int init = 0;
    int d = FIX2INT( dbg );

    if (!init && d != 0) {
	init = 1;
	debug_add_handler( debug_message_handler, DEBUG_LEVEL_ALWAYS, NULL );
    }
    wsman_debug_set_level( d );

    return Qnil;
}
#endif

/*-----------------------------------------------------------------*/
/* client */

%extend WsManClient {
  /* constructor */
  WsManClient( const char *uri ) {
    return wsmc_create_from_uri( uri );
  }
  WsManClient(const char *hostname,
              const int port, const char *path,
              const char *scheme,
              const char *username,
              const char *password) {
    return wsmc_create( hostname, port, path, scheme, username, password );
  }
  /* destructor */
  ~WsManClient() {
    wsmc_release( $self );
  }
  /* set dumpfile */
#if defined(SWIGRUBY)
  %rename( "dumpfile=" ) set_dumpfile( FILE *f );
#endif
  void set_dumpfile( FILE *f ) {
    wsmc_set_dumpfile( $self, f );
  }
  long response_code() {
    return wsmc_get_response_code( $self );
  }
  char *scheme() {
    return wsmc_get_scheme( $self );
  }
  char *host() {
    return wsmc_get_hostname( $self );
  }
  int port() {
    return wsmc_get_port( $self );
  }
  char *path() {
    return wsmc_get_path( $self );
  }
  char *user() {
    return wsmc_get_user( $self );
  }
  char *password() {
    return wsmc_get_password( $self );
  }
}


/*-----------------------------------------------------------------*/
/* actions */

%extend WsManClient {
  WsXmlDocH identify( client_opt_t *options ) {
    return wsmc_action_identify( $self, options );
  }
  
  WsXmlDocH get_from_epr( client_opt_t *options , epr_t *epr) {
    return wsmc_action_get_from_epr( $self, epr, options);
  }
  WsXmlDocH delete_from_epr( client_opt_t *options , epr_t *epr) {
    return wsmc_action_delete_from_epr( $self, epr, options);
  }

  WsXmlDocH enumerate( client_opt_t *options , filter_t *filter, char *resource_uri) {
    return wsmc_action_enumerate( $self, resource_uri, options, filter);
  }
  WsXmlDocH pull( client_opt_t *options , filter_t *filter, char *resource_uri, char *enum_ctx) {
    return wsmc_action_pull( $self, resource_uri, options, filter, enum_ctx);
  }
  WsXmlDocH create( client_opt_t *options, char *resource_uri, char *data, size_t size, char *encoding) {
    return wsmc_action_create_fromtext( $self, resource_uri, options, data, size, encoding);
  }
  WsXmlDocH put( client_opt_t *options , char *resource_uri,  char *data, size_t size, char *encoding) {
    return wsmc_action_put_fromtext( $self, resource_uri, options, data, size, encoding);
  }
  WsXmlDocH release( client_opt_t *options , char *resource_uri, char *enum_ctx) {
    return wsmc_action_release( $self, resource_uri, options, enum_ctx);
  }
  WsXmlDocH get( client_opt_t *options , char *resource_uri) {
    return wsmc_action_get( $self, resource_uri, options);
  }
  WsXmlDocH delete( client_opt_t *options , char *resource_uri) {
    return wsmc_action_delete( $self, resource_uri, options);
  }
  WsXmlDocH invoke( client_opt_t *options , char *resource_uri, char *method, char *data, size_t size, char *encoding) {
    return wsmc_action_invoke_fromtext( $self, resource_uri, options, method, data, size, encoding);
  }

  WsXmlDocH subscribe(client_opt_t *options , filter_t *filter, char *resource_uri) {
    return wsmc_action_subscribe($self,  resource_uri, options, filter);
  }

  WsXmlDocH unsubscribe(client_opt_t *options , filter_t *filter, char *resource_uri, char *identifier) {
    return wsmc_action_unsubscribe($self, resource_uri, options, identifier);
  }

  WsXmlDocH renew(client_opt_t *options , char *resource_uri, char *identifier) {
    return wsmc_action_renew($self, resource_uri, options, identifier);
  }

}



