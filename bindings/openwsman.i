%{
/*
 * type definitions to keep the C code generic
 */
 
#if defined(SWIGPYTHON)
#define Target_Null_p(x) (x == Py_None)
#define Target_INCREF(x) Py_INCREF(x)
#define Target_DECREF(x) Py_DECREF(x)
#define Target_True Py_True
#define Target_False Py_False
#define Target_Null Py_None
#define Target_Type PyObject*
#define Target_Bool(x) PyBool_FromLong(x)
#define Target_Char16(x) PyInt_FromLong(x)
#define Target_Int(x) PyInt_FromLong(x)
#define Target_String(x) PyString_FromString(x)
#define Target_Real(x) Py_None
#define Target_Array() PyList_New(0)
#define Target_SizedArray(len) PyList_New(len)
#define Target_ListSet(x,n,y) PyList_SetItem(x,n,y)
#define Target_Append(x,y) PyList_Append(x,y)
#include <Python.h>
#define TARGET_THREAD_BEGIN_BLOCK SWIG_PYTHON_THREAD_BEGIN_BLOCK
#define TARGET_THREAD_END_BLOCK SWIG_PYTHON_THREAD_END_BLOCK
#define TARGET_THREAD_BEGIN_ALLOW SWIG_PYTHON_THREAD_BEGIN_ALLOW
#define TARGET_THREAD_END_ALLOW SWIG_PYTHON_THREAD_END_ALLOW
#endif

#if defined(SWIGRUBY)
#define Target_Null_p(x) NIL_P(x)
#define Target_INCREF(x) 
#define Target_DECREF(x) 
#define Target_True Qtrue
#define Target_False Qfalse
#define Target_Null Qnil
#define Target_Type VALUE
#define Target_Bool(x) ((x)?Qtrue:Qfalse)
#define Target_Char16(x) INT2FIX(x)
#define Target_Int(x) INT2FIX(x)
#define Target_String(x) rb_str_new2(x)
#define Target_Real(x) rb_float_new(x)
#define Target_Array() rb_ary_new()
#define Target_SizedArray(len) rb_ary_new2(len)
#define Target_ListSet(x,n,y) rb_ary_store(x,n,y)
#define Target_Append(x,y) rb_ary_push(x,y)
#define TARGET_THREAD_BEGIN_BLOCK do {} while(0)
#define TARGET_THREAD_END_BLOCK do {} while(0)
#define TARGET_THREAD_BEGIN_ALLOW do {} while(0)
#define TARGET_THREAD_END_ALLOW do {} while(0)
#include <ruby.h>
#if HAVE_RUBY_IO_H
#include <ruby/io.h> /* Ruby 1.9 style */
#else
#include <rubyio.h>
#endif
#if HAVE_RUBY_VERSION_H
#include <ruby/version.h>
#endif
#endif

#if defined(SWIGPERL)
#define TARGET_THREAD_BEGIN_BLOCK do {} while(0)
#define TARGET_THREAD_END_BLOCK do {} while(0)
#define TARGET_THREAD_BEGIN_ALLOW do {} while(0)
#define TARGET_THREAD_END_ALLOW do {} while(0)

SWIGINTERNINLINE SV *SWIG_From_long  SWIG_PERL_DECL_ARGS_1(long value);
SWIGINTERNINLINE SV *SWIG_FromCharPtr(const char *cptr);
SWIGINTERNINLINE SV *SWIG_From_double  SWIG_PERL_DECL_ARGS_1(double value);

#define Target_Null_p(x) (x == NULL)
#define Target_INCREF(x) 
#define Target_DECREF(x) 
#define Target_True (&PL_sv_yes)
#define Target_False (&PL_sv_no)
#define Target_Null NULL
#define Target_Type SV *
#define Target_Bool(x) (x)?Target_True:Target_False
#define Target_Char16(x) SWIG_From_long(x)
#define Target_Int(x) SWIG_From_long(x)
#define Target_String(x) SWIG_FromCharPtr(x)
#define Target_Real(x) SWIG_From_double(x)
#define Target_Array() (SV *)newAV()
#define Target_SizedArray(len) (SV *)newAV()
#define Target_ListSet(x,n,y) av_store((AV *)(x),n,y)
#define Target_Append(x,y) av_push(((AV *)(x)), y)
#include <perl.h>
#include <EXTERN.h>
#endif

%}

#if defined(SWIGRUBY)
%module Openwsman

%{
#include <ruby.h>
#if HAVE_RUBY_IO_H
#include <ruby/io.h> /* Ruby 1.9 style */
#else
#include <rubyio.h>
#endif

#if HAVE_RUBY_THREAD_H /* New threading model */
#include <ruby/thread.h>
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

#endif /* SWIGRUBY */

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
    /*:nodoc:*/
    jint JNI_OnLoad(JavaVM *vm, void *reserved) {
      (*vm)->AttachCurrentThread(vm, (void **)&jenv, NULL);
      return JNI_VERSION_1_2;
    }
%}
#endif /* SWIGJAVA */

#if defined(SWIGPYTHON)
%module pywsman
/* Wrap file operations, as Python's native ones are incompatible */
FILE *fopen(char *, char *);
void fclose(FILE *f);
#endif

#if defined(SWIGPERL)
%module openwsman

//==================================
// Typemap: Allow FILE* as PerlIO
//----------------------------------
%typemap(in) FILE* {
    $1 = PerlIO_findFILE(IoIFP(sv_2io($input)));
}
#endif /* SWIGPERL */

%{
#if defined(SWIGPERL)
/* filter_t is defined in Perls CORE/perl.h */
#define filter_t filter_type

/* undef Perl's die and warn macros as libu is going to reuse this names */
#undef die
#undef warn
#endif /* SWIGPERL */

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

SWIGEXPORT
/* Init_ for the %module, defined by Swig
 * :nodoc:
 */
void Init_Openwsman(void);

SWIGEXPORT
/* Init_ for the .so lib, called by Ruby
 * :nodoc:
 */
void Init__openwsman(void) {
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
 * :nodoc:ly, its aliased to WsManClient
 *
 */
struct _WsManTransport { };
typedef struct _WsManTransport WsManTransport;

static void set_debug(int dbg);

/*
 * Set openwsman debug level.
 * call-seq:
 *   Openwsman::debug = -1 # full debug
 *
 */
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

/*
 * Return openwsman debug level.
 * call-seq:
 *   Openwsman::debug -> Integer
 *
 */
static int get_debug(void) {
  return (int)wsman_debug_get_level();
}

static WsXmlDocH create_soap_envelope(void);

/*
 * Create empty SOAP envelope
 * call-seq:
 *   Openwsman::create_soap_envelope -> XmlDoc
 *
 */
static WsXmlDocH create_soap_envelope() {
  return ws_xml_create_soap_envelope();
}

static WsXmlDocH create_doc_from_file(const char *filename, const char *encoding);

/*
 * Read XmlDoc from file
 * call-seq:
 *   Openwsman::create_doc_from_file("/path/to/file", "utf-8") -> XmlDoc
 *
 */
static WsXmlDocH create_doc_from_file(const char *filename, const char *encoding) {
  return xml_parser_file_to_doc( filename, encoding, 0);                 
}

static WsXmlDocH create_doc_from_string(const char *buf, const char *encoding);

/*
 * Read XmlDoc from string
 * call-seq:
 *   Openwsman::create_doc_from_string("<xml ...>", "utf-8") -> XmlDoc
 *
 */
static WsXmlDocH create_doc_from_string(const char *buf, const char *encoding) {
  return xml_parser_memory_to_doc( buf, strlen(buf), encoding, 0);
}

static char *uri_classname(const char *uri);
/*
 * get classname from resource URI
 * call-seq:
 *   Openwsman::uri_classname("http://sblim.sf.net/wbem/wscim/1/cim-schema/2/Linux_OperatingSystem") -> "Linux_OperatingSystem"
 *
 */
static char *uri_classname(const char *uri) {
  const char *lastslash = strrchr(uri,'/');
  if (lastslash) {
    return strdup(lastslash+1);
  }
  return NULL;
}

static const char *uri_prefix(const char *classname);
/*
 * Map classname (class schema) to resource uri prefix
 * call-seq:
 *   Openwsman::uri_prefix("Linux") -> "http://sblim.sf.net/wbem/wscim/1/cim-schema/2"
 *   Openwsman::uri_prefix("Win32") -> "http://schemas.microsoft.com/wbem/wsman/1/wmi"
 *
 */
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
    /* Microsoft HyperV */
    { 4, "Msvm", "http://schemas.microsoft.com/wbem/wsman/1/wmi" },
    /* Dell DRAC */
    { 4, "DCIM", "http://schemas.dell.com/wbem/wscim/1/cim-schema/2" },
    /* Unisys */
    { 4, "SPAR", "http://schema.unisys.com/wbem/wscim/1/cim-schema/2" },
    /* Fujitsu */
    { 3, "SVS", "http://schemas.ts.fujitsu.com/wbem/wscim/1/cim-schema/2" },
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
/*:nodoc:*/
static epr_t *my_epr_deserialize(WsXmlNodeH node) {
  if (strcmp(WSA_EPR, ws_xml_get_node_local_name(node)) == 0) {
    /* Use node as-is if its already a WSA_EPR */
    return epr_deserialize(node, NULL, NULL, 1);
  }
  /* else search the WSA_EPR node */
  return epr_deserialize(node, XML_NS_ADDRESSING, WSA_EPR, 1);
}

static VALUE kv_list_to_hash(list_t *list)
{
  VALUE v = Qnil;
  if (!list_isempty(list)) {
  v = rb_hash_new();
  lnode_t *node = list_first(list);
  while (node) {
    key_value_t *kv = (key_value_t *)node->list_data;
    if (kv->type == 0) {
      rb_hash_aset( v, makestring(kv->key), makestring(kv->v.text));
    }
    else {
      VALUE epr = SWIG_NewPointerObj((void*)kv->v.epr, SWIGTYPE_p_epr_t, 0);
      rb_hash_aset( v, makestring(kv->key), epr);
    }
    node = list_next(list, node);
    }
  }
  return v;
}
#endif

static char *epr_prefix(const char *uri);
/* Get prefix from a EPR uri
 * :nodoc:
 */
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

%include "wsman-client_opt.i"

%include "wsman-client.i"

/*-----------------------------------------------------------------*/
/* debug */

#if defined(SWIGRUBY)
  %rename("debug=") set_debug(int debug);
#endif
static void set_debug(int dbg);
#if defined(SWIGRUBY)
  %rename("debug") get_debug();
#endif
static int get_debug();
static WsXmlDocH create_soap_envelope();
static WsXmlDocH create_doc_from_file(const char *filename, const char *encoding = "UTF-8");
static WsXmlDocH create_doc_from_string(const char *buf, const char *encoding = "UTF-8");
static const char *uri_prefix(const char* classname);
