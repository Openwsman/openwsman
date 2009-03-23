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
#include <wsman-client-transport.h>
#include <wsman-api.h>
#include <wsman-xml.h>
#include <wsman-epr.h>
#include <wsman-filter.h>
#include <wsman-soap.h>
#include <wsman-soap-envelope.h>
#include <openwsman.h>
#if defined(SWIGRUBY)
#include <ruby/helpers.c>
#endif
#if defined(SWIGPYTHON)
#include <python/helpers.c>
#endif
#if defined(SWIGJAVA)
#include <java/helpers.c>
#endif

/* fool swig into aliasing WsManClient and WsManTransport */
struct _WsManTransport { };
typedef struct _WsManTransport WsManTransport;

static void set_debug(int dbg) {
  static int init = 0;

  if (!init && dbg != 0) {
    init = 1;
    debug_add_handler( debug_message_handler, DEBUG_LEVEL_ALWAYS, NULL );
  }
  wsman_debug_set_level( dbg );
	
}

static int get_debug() {
  return (int)wsman_debug_get_level();
}

static WsXmlDocH create_soap_envelope() {
  return ws_xml_create_soap_envelope();
}

%}

%include "wsman-types.i"

%include "wsman-names.i"

%include "wsman-xml.i"

%include "wsman-epr.i"

%include "wsman-filter.i"

%include "wsman-soap.i"

%include "wsman-transport.i"

%include "client_opt.i"

%include "wsman-client.i"

/*-----------------------------------------------------------------*/
/* debug */

#if defined(SWIGRUBY)
  %rename("debug=") set_debug(int debug);
/*
 * call-seq:
 *   Rbwsman::debug
 *
 * Return openwsman debug level.
 */
#endif

static void set_debug(int dbg);

#if defined(SWIGRUBY)
  %rename("debug") get_debug();
/*
 * call-seq:
 *   Rbwsman::debug = 1
 *   Rbwsman::debug = 0
 *
 * Set openwsman debug level.
 */
#endif

static int get_debug();

static WsXmlDocH create_soap_envelope();

