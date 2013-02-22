/*
 * Document-module: Openwsman
 * = About openwsman
 * Openwsman (http://www.openwsman.org) is a project intended to provide an open-source
 * implementation of the Web Services Management specification
 * (WS-Management) and to expose system management information on the
 * Linux operating system using the WS-Management protocol. WS-Management
 * is based on a suite of web services specifications and usage
 * requirements that exposes a set of operations focused on and covers
 * all system management aspects. 
 *
 * = Using the bindings
 * The bindings provide access to the client-side API of openwsman.
 * You start by creating a Client instance and set up ClientOptions
 * to control the communication.
 *
 * The Client instance now provides the WS-Management operations, like
 * enumerate, get, invoke, etc.
 *
 * All client operations return a XmlDoc representing the SOAP response
 * from the system.
 *
 * You can then use XmlDoc methods to extract SOAP elements from the
 * response and dig down through its XmlNode and XmlAttr objects.
 *
 */
 

#if defined(SWIGRUBY)
%module Openwsman

%{
#include <ruby.h>
#if HAVE_RUBY_IO_H
#include <ruby/io.h> /* Ruby 1.9 style */
#else
#include <rubyio.h>
#endif
%}

%typemap(in) FILE* {
#if RUBY_VERSION > 18
  struct rb_io_t *fptr;
#else
  struct OpenFile *fptr;
#endif
  Check_Type($input, T_FILE);
  GetOpenFile($input, fptr);
  /*rb_io_check_writable(fptr);*/
#if RUBY_VERSION > 18
  $1 = rb_io_stdio_file(fptr);
#else
  $1 = GetReadFile(fptr);
#endif
}

#endif

#if defined(SWIGJAVA)
%module OpenWSMan

/* evaluate constants */
%javaconst(1);

/* extend JNI class to automatically load shared library from jar file */
%pragma(java) jniclassimports=%{
	 import java.io.File;
	 import java.io.InputStream;
	 import java.io.FileOutputStream;
%}
%pragma(java) jniclasscode=%{
	public final static String libraryFileName = "libjwsman.so";

	static {
		InputStream inputStream = OpenWSManJNI.class.getClassLoader()
				.getResourceAsStream(libraryFileName);

		try {
			File libraryFile = File.createTempFile(libraryFileName, null);
			libraryFile.deleteOnExit();

			FileOutputStream fileOutputStream = new FileOutputStream(libraryFile);
			byte[] buffer = new byte[8192];
			int bytesRead;
			while ((bytesRead = inputStream.read(buffer)) > 0) {
				fileOutputStream.write(buffer, 0, bytesRead);
			}
			fileOutputStream.close();
			inputStream.close();

			try {
				System.load(libraryFile.getPath());
			} catch (UnsatisfiedLinkError e) {
				System.err.println("Arch: " + System.getProperty("os.arch"));
				System.err.println("Native code library failed to load. See the chapter on Dynamic Linking Problems in the SWIG Java documentation for help.\n"	+ e);
			}
		} catch (Exception e) {
			e.printStackTrace();
		}
	}
%}

/* get the java environment so we can throw exceptions */
%{
    static JNIEnv *jenv;
    jint JNI_OnLoad(JavaVM *vm, void *reserved) {
      (*vm)->AttachCurrentThread(vm, (void **)&jenv, NULL);
      return JNI_VERSION_1_2;
    }
%}
#endif

#if defined(SWIGPYTHON)
%module pywsman
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
#if defined(SWIGPERL)
/* filter_t is defined in Perls CORE/perl.h */
#define filter_t filter_type

/* undef Perl's die and warn macros as libu is going to reuse this names */
#undef die
#undef warn
#endif

#include <u/libu.h>
#include <wsman-types.h>
#include <wsman-client.h>
#include <wsman-client-transport.h>
#include <wsman-api.h>
#include <wsman-xml-binding.h>
#include <wsman-xml.h>
#include <wsman-epr.h>
#include <wsman-filter.h>
#include <wsman-soap.h>
#include <wsman-soap-envelope.h>
#include <openwsman.h>
#if defined(SWIGRUBY)
#include <ruby/helpers.h>

/* Init_ for the %module, defined by Swig */
SWIGEXPORT void Init_Openwsman(void);

/* Init_ for the .so lib, called by Ruby */
SWIGEXPORT void Init__openwsman(void) {
  Init_Openwsman();
}
#endif
#if defined(SWIGPYTHON)
#include <python/helpers.h>
#endif
#if defined(SWIGJAVA)
#include <java/helpers.h>
#endif
#if defined(SWIGPERL)
#include <perl/helpers.h>
#endif

/* Provide WsManTransport definition so it can be used as
 * dedicated datatype in bindings.
 * Internally, its aliased to WsManClient
 */
struct _WsManTransport { };
typedef struct _WsManTransport WsManTransport;

static void set_debug(int dbg);

static void set_debug(int dbg) {
  static int init = 0;

  if (!init && dbg != 0) {
    init = 1;
    debug_add_handler( debug_message_handler, DEBUG_LEVEL_ALWAYS, NULL );
  }
  wsman_debug_set_level( dbg );
	
}

/* module-level methods */

static int get_debug(void);

static int get_debug(void) {
  return (int)wsman_debug_get_level();
}

static WsXmlDocH create_soap_envelope(void);
static WsXmlDocH create_soap_envelope() {
  return ws_xml_create_soap_envelope();
}

static WsXmlDocH create_doc_from_file(const char *filename, const char *encoding);

static WsXmlDocH create_doc_from_file(const char *filename, const char *encoding) {
  return xml_parser_file_to_doc( filename, encoding, 0);                 
}

static WsXmlDocH create_doc_from_string(const char *buf, const char *encoding);

static WsXmlDocH create_doc_from_string(const char *buf, const char *encoding) {
  return xml_parser_memory_to_doc( buf, strlen(buf), encoding, 0);
}

static char *uri_classname(const char *uri);
/* get classname from resource URI */
static char *uri_classname(const char *uri) {
  const char *lastslash = strrchr(uri,'/');
  if (lastslash) {
    return strdup(lastslash+1);
  }
  return NULL;
}


static const char *uri_prefix(const char *classname);
/* get resource URI prefix for a specific classname (resp class schema) */
static const char *uri_prefix(const char *classname) {
  static struct map {
    int len;
    const char *schema;
    const char *prefix;
  } mapping[] = {
    /* dmtf CIM */
    { 3, "CIM", "http://schemas.dmtf.org/wbem/wscim/1/cim-schema/2" },
    /* dmtf reserved */
    { 3, "PRS", "http://schemas.dmtf.org/wbem/wscim/1/cim-schema/2" },
    /* Microsoft WMI */
    { 5, "Win32", "http://schemas.microsoft.com/wbem/wsman/1/wmi" },
    /* openwbem.org */
    { 8, "OpenWBEM", "http://schema.openwbem.org/wbem/wscim/1/cim-schema/2" },
    /* sblim */
    { 5, "Linux", "http://sblim.sf.net/wbem/wscim/1/cim-schema/2" },
    /* omc-project */
    { 3, "OMC", "http://schema.omc-project.org/wbem/wscim/1/cim-schema/2" },
    /* pegasus.org */
    { 2, "PG", "http://schema.openpegasus.org/wbem/wscim/1/cim-schema/2" },
    /* Intel AMT */
    { 3, "AMT", "http://intel.com/wbem/wscim/1/amt-schema/1" },
    /* Intel */
    { 3, "IPS", "http://intel.com/wbem/wscim/1/ips-schema/1" },
    /* Sun */
    { 3, "Sun","http://schemas.sun.com/wbem/wscim/1/cim-schema/2" },
    { 0, NULL, NULL }
  };
  const char *schema_end;
  struct map *map;
  int len;
  if (classname == NULL)
    return NULL;
  if (strcmp(classname, "*") == 0) {
    return "http://schemas.dmtf.org/wbem/wscim/1";
  }
  if ((strcmp(classname, "meta_class") == 0)
      ||(strncmp(classname, "__", 2) == 0)) {
    return "http://schemas.microsoft.com/wbem/wsman/1/wmi";
  }
  schema_end = strchr(classname, '_');
  if (schema_end == NULL)
    return NULL; /* Bad class name */
  len = schema_end - classname;
  map = mapping;
  while (map->len > 0) {
    if ((len == map->len)
        && (strncasecmp(classname, map->schema, map->len) == 0)) {
      return map->prefix;
    }
    ++map;
  }
  return NULL;
}

#if defined(SWIGRUBY)
static epr_t *my_epr_deserialize(WsXmlNodeH node);
static epr_t *my_epr_deserialize(WsXmlNodeH node) {
  if (strcmp(WSA_EPR, ws_xml_get_node_local_name(node)) == 0) {
    /* Use node as-is if its already a WSA_EPR */
    return epr_deserialize(node, NULL, NULL, 1);
  }
  /* else search the WSA_EPR node */
  return epr_deserialize(node, XML_NS_ADDRESSING, WSA_EPR, 1);
}
#endif

static char *epr_prefix(const char *uri);
/* Get prefix from a EPR uri */
static char *epr_prefix(const char *uri) {
  char *classname = uri_classname(uri);
  const char *prefix = uri_prefix(classname);
  if (prefix) {
    const char *lastslash;
    if (strncmp(uri, prefix, strlen(prefix)) == 0) {
      return strdup(prefix);
    }
    lastslash = strrchr(uri, '/');
    if (lastslash) {
      return strndup(uri, lastslash-uri);
    }
  }
  return strdup(uri);
}

%}

#if RUBY_VERSION > 18
%{
  typedef struct {
    WsManClient *client;
    client_opt_t *options;
    filter_t *filter;
    const char *resource_uri;
    epr_t *epr;
    const char *context;
    const char *identifier;
    const char *data;
    size_t size;
    const char *encoding;
    const char *method;
    WsXmlDocH method_args;
  } wsmc_action_args_t;

  WsXmlDocH
  ruby_enumerate_thread(wsmc_action_args_t *args) {
    return wsmc_action_enumerate(args->client, args->resource_uri, args->options, args->filter);
  }
  WsXmlDocH
  ruby_identify_thread(wsmc_action_args_t *args) {
    return wsmc_action_identify(args->client, args->options);
  }
  WsXmlDocH
  ruby_get_from_epr_thread(wsmc_action_args_t *args) {
    return wsmc_action_get_from_epr(args->client, args->epr, args->options);
  }
  WsXmlDocH
  ruby_delete_from_epr_thread(wsmc_action_args_t *args) {
    return wsmc_action_delete_from_epr(args->client, args->epr, args->options);
  }
  WsXmlDocH
  ruby_pull_thread(wsmc_action_args_t *args) {
    return wsmc_action_pull(args->client, args->resource_uri, args->options, args->filter, args->context);
  }
  WsXmlDocH
  ruby_create_fromtext_thread(wsmc_action_args_t *args) {
    return wsmc_action_create_fromtext(args->client, args->resource_uri, args->options, args->data, args->size, args->encoding);
  }
  WsXmlDocH
  ruby_put_fromtext_thread(wsmc_action_args_t *args) {
    return wsmc_action_put_fromtext(args->client, args->resource_uri, args->options, args->data, args->size, args->encoding);
  }
  WsXmlDocH
  ruby_release_thread(wsmc_action_args_t *args) {
    return wsmc_action_release(args->client, args->resource_uri, args->options, args->context);
  }
  WsXmlDocH
  ruby_get_thread(wsmc_action_args_t *args) {
    return wsmc_action_get(args->client, args->resource_uri, args->options);
  }
  WsXmlDocH
  ruby_delete_thread(wsmc_action_args_t *args) {
    return wsmc_action_delete(args->client, args->resource_uri, args->options);
  }
  WsXmlDocH
  ruby_invoke_thread(wsmc_action_args_t *args) { 
    return wsmc_action_invoke(args->client, args->resource_uri, args->options, args->method, args->method_args);
  }
  WsXmlDocH
  ruby_subscribe_thread(wsmc_action_args_t *args) {
    return wsmc_action_subscribe(args->client, args->resource_uri, args->options, args->filter);
  }
  WsXmlDocH
  ruby_unsubscribe_thread(wsmc_action_args_t *args) {
    return wsmc_action_unsubscribe(args->client, args->resource_uri, args->options, args->identifier);
  }
  WsXmlDocH
  ruby_renew_thread(wsmc_action_args_t *args) {
    return wsmc_action_renew(args->client, args->resource_uri, args->options, args->identifier);
  }
%}
#endif


/*
 * hash_t typemaps
 */
#if defined(SWIGJAVA)
%typemap(jni) hash_t * "jobject"
%typemap(jtype) hash_t * "java.util.Map"
%typemap(jstype) hash_t * "java.util.Map"
%typemap(javaout) hash_t * {
	return $jnicall;
 }
#endif
%typemap(out) hash_t * {
#if defined(SWIGJAVA)
 $result = hash2value(jenv, $1);
#else
 $result = hash2value($1);
#endif
}

%typemap(in) hash_t * {
 $input = value2hash(NULL, $1, 0);
}

%ignore __undefined;

%include exception.i

#if defined(SWIGJAVA)

/* if java, pass the new exception macro to C not just SWIG */
#undef SWIG_exception
#define SWIG_exception(code, msg) {SWIG_JavaException(jenv, code, msg); goto fail;}
%inline %{
#undef SWIG_exception
#define SWIG_exception(code, msg) {SWIG_JavaException(jenv, code, msg); goto fail;}
%}

#endif

%include "version.i"

/* start with wsman-xml to get the __WsXmlFoo -> XmlFoo renames right */
%include "wsman-xml.i"

%include "wsman-names.i"

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
#endif

/*
 * Return openwsman debug level.
 */
static void set_debug(int dbg);

#if defined(SWIGRUBY)
  %rename("debug") get_debug();
#endif

/*
 * Set openwsman debug level.
 */
static int get_debug();

/*
 * Create empty SOAP envelope (XmlDoc)
 * 
 */
static WsXmlDocH create_soap_envelope();

/*
 * Read XmlDoc from file
 */
static WsXmlDocH create_doc_from_file(const char *filename, const char *encoding = "UTF-8");

/*
 * Read XmlDoc from string
 */
static WsXmlDocH create_doc_from_string(const char *buf, const char *encoding = "UTF-8");

/*
 * Map classname (class schema) to resource uri prefix
 */
static const char *uri_prefix(const char* classname);
