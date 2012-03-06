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

%}

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
 $input = value2hash(NULL, $1);
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
